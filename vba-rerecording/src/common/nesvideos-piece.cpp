#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

/* Note: This module assumes everyone uses RGB15 as display depth */

static std::string VIDEO_CMD =
    "mencoder - -o test0.avi"
    " -noskip -mc 0"
    " -ovc lavc"
    " -oac mp3lame"
    " -lameopts preset=256:aq=2:mode=3"
    " -lavcopts vcodec=ffv1:context=0:format=BGR32:coder=0:vstrict=-1"
    " >& mencoder.log";

static void FlushWrite(FILE* fp, const unsigned char*buf, unsigned length);

#define BGR24 (0x42475218)  // BGR24 fourcc
#define BGR16 (0x42475210)  // BGR16 fourcc
#define BGR15 (0x4247520F)  // BGR15 fourcc

static FILE* (*openFunc)  (const char*, const char*) = NULL;
static int (*closeFunc) (FILE*) = NULL;

#if (defined(WIN32) || defined(win32)) // capital is standard, but check for either
 #include <cstdlib>
 #define popen _popen;
 #define pclose _pclose;
#endif

#define u32(n) (n)&255,((n)>>8)&255,((n)>>16)&255,((n)>>24)&255
#define u16(n) (n)&255,((n)>>8)&255
#define s4(s) s[0],s[1],s[2],s[3]

static const unsigned FPS_SCALE = (0x1000000);

// general-purpose A/V sync debugging, ignored unless explicitly enabled with NESVideoEnableDebugging
static void (*debugVideoMessageFunc)(const char *msg) = NULL;
static void (*debugAudioMessageFunc)(const char *msg) = NULL;
// logo adds 1 "frame" to audio, so offset that (A/V frames shouldn't necessarily match up depending on the rates, but should at least make them start out matching in case they do)
static unsigned audioFramesWritten=0, videoFramesWritten=1;
static double audioSecondsWritten=0, videoSecondsWritten=0;


static class AVI
{
    FILE* avifp;
    
    bool KnowVideo;
    unsigned width;
    unsigned height;
    unsigned fps_scaled;
    std::vector<unsigned char> VideoBuffer;
    
    bool KnowAudio;
    unsigned rate;
    unsigned chans;
    unsigned bits;
    std::vector<unsigned char> AudioBuffer;
    
public:
    AVI() :
        avifp(NULL),
        KnowVideo(false),
        KnowAudio(false)
    {
    }
    ~AVI()
    {
        if(avifp) closeFunc(avifp);
    }
    
    void Audio(unsigned r,unsigned b,unsigned c,
               const unsigned char*d, unsigned nsamples)
    {
        if(!KnowAudio)
        {
            rate = r;
            chans = c;
            bits = b;
            KnowAudio = true;
            CheckFlushing();
        }
        unsigned bytes = nsamples*chans*(bits/8);

		if(debugAudioMessageFunc)
		{
			audioFramesWritten++;
			audioSecondsWritten += (double)nsamples / (double)rate; // += bytes times seconds per byte
			char temp [64];
			sprintf(temp, "A: %.2lf s, %u f", audioSecondsWritten, audioFramesWritten);
			debugAudioMessageFunc(temp);
		}

        if(KnowVideo)
            SendAudioFrame(d, bytes);
        else
        {
            AudioBuffer.insert(AudioBuffer.end(), d, d+bytes);
            fprintf(stderr, "Buffering %u bytes of audio\n", bytes);
        }
    }
    void Video(unsigned w,unsigned h,unsigned f, const unsigned char*d)
    {
        if(!KnowVideo)
        {
            width=w;
            height=h;
            fps_scaled=f;
            KnowVideo = true;
            CheckFlushing();
        }
        
        unsigned bytes = width*height*2;
        
        //std::vector<unsigned char> tmp(bytes, 'k');
        //d = &tmp[0];

		if(debugVideoMessageFunc)
		{
			videoFramesWritten++;
			videoSecondsWritten += (double)FPS_SCALE / (double)fps_scaled; // += seconds per frame
			char temp [64];
			sprintf(temp, "V: %.2lf s, %u f", videoSecondsWritten, videoFramesWritten);
			debugVideoMessageFunc(temp);
		}

        if(KnowAudio)
            SendVideoFrame(d, bytes);
        else
        {
            VideoBuffer.insert(VideoBuffer.end(), d, d+bytes);
            fprintf(stderr, "Buffering %u bytes of video\n", bytes);
        }
    }

private:
    void CheckFlushing()
    {
        //AudioBuffer.clear();
        //VideoBuffer.clear();
        
        if(KnowAudio && KnowVideo)
        {
            unsigned last_offs;
            
            // Flush Audio
            
            last_offs = 0;
            while(last_offs < AudioBuffer.size())
            {
                unsigned bytes = rate / (fps_scaled / FPS_SCALE);
                bytes *= chans*(bits/8);
                
                unsigned remain = AudioBuffer.size() - last_offs;
                if(bytes > remain) bytes = remain;
                if(!bytes) break;
                
                unsigned begin = last_offs;
                last_offs += bytes;
                SendAudioFrame(&AudioBuffer[begin], bytes);
            }
            AudioBuffer.erase(AudioBuffer.begin(), AudioBuffer.begin()+last_offs);
            
            // Flush Video
            
            last_offs = 0;
            while(last_offs < VideoBuffer.size())
            {
                unsigned bytes  = width*height*2;
                unsigned remain = VideoBuffer.size() - last_offs;
                if(bytes > remain) bytes = remain;
                if(!bytes)break;
                
                unsigned begin = last_offs;
                last_offs += bytes;
                SendVideoFrame(&VideoBuffer[begin], bytes);
            }
            VideoBuffer.erase(VideoBuffer.begin(), VideoBuffer.begin()+last_offs);
        }
    }
    
    void SendVideoFrame(const unsigned char* vidbuf, unsigned framesize)
    {
        CheckBegin();
        
        //fprintf(stderr, "Writing 00dc of %u bytes\n", framesize);
        
        const unsigned char header[] = { s4("00dc"), u32(framesize) };
        FlushWrite(avifp, header, sizeof(header));
        FlushWrite(avifp, vidbuf, framesize);
    }

    void SendAudioFrame(const unsigned char* audbuf, unsigned framesize)
    {
        CheckBegin();
        
        //fprintf(stderr, "Writing 01wb of %u bytes\n", framesize);
        
        const unsigned char header[] = { s4("01wb"), u32(framesize) };
        FlushWrite(avifp, header, sizeof(header));
        FlushWrite(avifp, audbuf, framesize);
    }

    void CheckBegin()
    {
        if(avifp) return;
        
		if(!openFunc) openFunc = popen; // default
		if(!closeFunc) closeFunc = pclose; // default

        avifp = openFunc(VIDEO_CMD.c_str(), "wb");
        if(!avifp) return;

        const unsigned fourcc = BGR16;
        const unsigned framesize = width*height*2;
        
        const unsigned aud_rate  = rate;
        const unsigned aud_chans = chans;
        const unsigned aud_bits  = bits;

        const unsigned nframes    = 0; //unknown
        const unsigned scale      = FPS_SCALE;
        const unsigned scaled_fps = fps_scaled;
        
        const unsigned SIZE_strh_vids = 4 + 4*2 + 2*2 + 8*4 + 2*4;
        const unsigned SIZE_strf_vids = 4*3 + 2*2 + 4*6;
        const unsigned SIZE_strl_vids = 4+ 4+(4+SIZE_strh_vids) + 4+(4+SIZE_strf_vids);

        const unsigned SIZE_strh_auds = 4 + 4*3 + 2*2 + 4*8 + 2*4;
        const unsigned SIZE_strf_auds = 2*2 + 4*2 + 2*3;
        const unsigned SIZE_strl_auds = 4+ 4+(4+SIZE_strh_auds) + 4+(4+SIZE_strf_auds);
        
        const unsigned SIZE_avih = 4*12;
        const unsigned SIZE_hdrl = 4+4+ (4+SIZE_avih) + 4 + (4+SIZE_strl_vids) + 4 + (4+SIZE_strl_auds);
        const unsigned SIZE_movi = 4 + nframes*(4+4+framesize);
        const unsigned SIZE_avi = 4+4+ (4+SIZE_hdrl) + 4 + (4+SIZE_movi);
        
        const unsigned char AVIheader[] =
        {
            s4("RIFF"),
            u32(SIZE_avi),
            s4("AVI "),   
            
            // HEADER

            s4("LIST"),   
            u32(SIZE_hdrl),
             s4("hdrl"),   
             
             s4("avih"),
             u32(SIZE_avih),
              u32(0),
              u32(0),
              u32(0),
              u32(0),
              u32(nframes),
              u32(0),
              u32(2), // two streams
              u32(0),
              u32(0),
              u32(0),
              u32(0),
              u32(0),
             
             // VIDEO HEADER
             
             s4("LIST"),
             u32(SIZE_strl_vids),
              s4("strl"),   
              
               s4("strh"),
               u32(SIZE_strh_vids),
                s4("vids"),
                u32(0),
                u32(0),
                u16(0),
                u16(0),
                u32(0),
                u32(scale),
                u32(scaled_fps),
                u32(0),
                u32(0),
                u32(0),
                u32(0),
                u32(0),
                u16(0),
                u16(0),
                u16(0),
                u16(0),
               
               s4("strf"),
               u32(SIZE_strf_vids),
                u32(0),
                u32(width),
                u32(height),
                u16(0),
                u16(0),
                u32(fourcc),
                u32(0),
                u32(0),
                u32(0),
                u32(0),
                u32(0),
             
             // AUDIO HEADER
             
             s4("LIST"),
             u32(SIZE_strl_auds),
              s4("strl"),   
              
               s4("strh"),
               u32(SIZE_strh_auds),
                s4("auds"),
                u32(0), //fourcc
                u32(0), //handler
                u32(0), //flags
                u16(0), //prio
                u16(0), //lang
                u32(0), //init frames
                u32(1), //scale
                u32(aud_rate),
                u32(0), //start
                u32(0), //rate*length
                u32(1048576), //suggested bufsize
                u32(0), //quality
                u32(aud_chans * (aud_bits / 8)), //sample size
                u16(0), //frame size
                u16(0),
                u16(0),
                u16(0),
               
               s4("strf"),
               u32(SIZE_strf_auds),
                u16(1), // pcm format
                u16(aud_chans),
                u32(aud_rate),
                u32(aud_rate * aud_chans * (aud_bits/8)), // samples per second
                u16(aud_chans * (aud_bits/8)), //block align
                u16(aud_bits), //bits
                u16(0), //cbSize

            // MOVIE

            s4("LIST"),
            u32(SIZE_movi),
             s4("movi")
        };
          
        FlushWrite(avifp, AVIheader, sizeof(AVIheader));
    }
} AVI;

extern "C"
{
    int LoggingEnabled = 0; /* 0=no, 1=yes, 2=recording! */

    const char* NESVideoGetVideoCmd()
    {
        return VIDEO_CMD.c_str();
    }
    void NESVideoSetVideoCmd(const char *cmd)
    {
        VIDEO_CMD = cmd;
    }
	void NESVideoEnableDebugging( void videoMessageFunc(const char *msg), void audioMessageFunc(const char *msg) )
	{
		debugVideoMessageFunc = videoMessageFunc;
		debugAudioMessageFunc = audioMessageFunc;
	}
	void NESVideoSetFileFuncs( FILE* open(const char *,const char *), int close(FILE*) )
	{
		openFunc = open;
		closeFunc = close;
	}

    void NESVideoLoggingVideo
        (const void*data, unsigned width,unsigned height,
         unsigned fps_scaled
        )
    {
        if(LoggingEnabled < 2) return;
        
        unsigned LogoFrames = fps_scaled >> 24;

        static bool First = true;
        if(First)
        {
            First=false;
            /* Bisqwit's logo addition routine. */
            /* If you don't have his files, this function does nothing
             * and it does not matter at all.
             */
            
            const char *background =
                width==320 ? "logo320_240"
              : width==160 ? "logo160_144"
              : width==240 ? "logo240_160"
              : height>224 ? "logo256_240"
              :              "logo256_224";
            
            /* Note: This should be 1 second long. */
            for(unsigned frame = 0; frame < LogoFrames; ++frame)
            {
                char Buf[4096];
                sprintf(Buf, "/shares/home/bisqwit/povray/nesvlogo/%s_f%u.tga",
                    background, frame);
                
                FILE*fp = fopen(Buf, "rb");
                if(!fp) // write blackness when missing frames to keep the intro 1 second long:
				{
			        unsigned bytes = width*height*2;
					unsigned char* buf = (unsigned char*)malloc(bytes);
					if(buf)
					{
						memset(buf,0,bytes);
						AVI.Video(width,height,fps_scaled, buf);
						if(debugVideoMessageFunc) videoFramesWritten--;
						free(buf);
					}
				}
				else // write 1 frame of the logo:
				{
					int idlen = fgetc(fp);
					/* Silently ignore all other header data.
					 * These files are assumed to be uncompressed BGR24 tga files with Y swapped.
					 * Even their geometry is assumed to match perfectly.
					 */
					fseek(fp, 1+1+2+2+1+ /*org*/2+2+ /*geo*/2+2+ 1+1+idlen, SEEK_CUR);

					bool yflip=true;
					std::vector<unsigned char> data(width*height*3);
					for(unsigned y=height; y-->0; )
						fread(&data[y*width*3], 1, width*3, fp);
					fclose(fp);
	                
					std::vector<unsigned short> result(width*height);
					for(unsigned pos=0, max=result.size(); pos<max; ++pos)
					{
						unsigned usepos = pos;
						if(yflip)
						{
							unsigned y = pos/width;
							usepos = (usepos%width) + (height-y-1)*width;
						}
	                    
						unsigned B = data[usepos*3+0];
						unsigned G = data[usepos*3+1];
						unsigned R = data[usepos*3+2];
						result[pos] = ((B*31/255)<<0)
									| ((G*63/255)<<5)
									| ((R*31/255)<<11);
					}
					AVI.Video(width,height,fps_scaled, (const unsigned char*)&result[0]);
					if(debugVideoMessageFunc) videoFramesWritten--;
				}
            }
        }
        AVI.Video(width,height,fps_scaled,  (const unsigned char*) data);
    }

    void NESVideoLoggingAudio
        (const void*data,
         unsigned rate, unsigned bits, unsigned chans,
         unsigned nsamples)
    {
        if(LoggingEnabled < 2) return;

        static bool First = true;
        if(First)
        {
            First=false;
            
			const unsigned n = rate; // assumes 1 second of logo to write silence for
            if(n > 0)
			{
				unsigned bytes = n*chans*(bits/8);
				unsigned char* buf = (unsigned char*)malloc(bytes);
				if(buf)
				{
					memset(buf,0,bytes);
					AVI.Audio(rate,bits,chans, buf, n);
					free(buf);
				}
			}
        }
        
        AVI.Audio(rate,bits,chans, (const unsigned char*) data, nsamples);
    }
} /* extern "C" */



static void FlushWrite(FILE* fp, const unsigned char*buf, unsigned length)
{
///	unsigned failures = 0;
///	const static int FAILURE_THRESH = 8092; // don't want to loop infinitely if we keep failing to make progress - actually maybe you would want this, so the checking is disabled
    while(length > 0 /*&& failures < FAILURE_THRESH*/)
    {
        unsigned written = fwrite(buf, 1, length, fp);
///		if(written == 0)
///			failures++;
///		else
///		{
			length -= written;
			buf += written;
///			failures = 0;
///		}
    }
///	if(failures >= FAILURE_THRESH)
///	{
///		fprintf(stderr, "FlushWrite() failed to write %d bytes %d times - giving up.", length, failures);
///		LoggingEnabled = 0;
///	}
}

// for the UB tech
#undef BGR24
#undef BGR16
#undef BGR15

#undef u32
#undef u16
#undef s4
