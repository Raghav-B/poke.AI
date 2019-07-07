// System.cpp : Defines the system behaviors for the emulator.
//
#include "stdafx.h"
#include "Sound.h"
#include "Input.h"
#include "IUpdate.h"
#include "WinMiscUtil.h"
#include "WinResUtil.h"
#include "resource.h"
#include "VBA.h"
#include "../gba/GBA.h"
#include "../gba/GBAGlobals.h"
#include "../gba/GBAinline.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
//#include "../common/System.h"
#include "../common/SystemGlobals.h"
#include "../common/Text.h"
#include "../common/Util.h"
#include "../common/movie.h"
#include "../common/nesvideos-piece.h"
#include "../common/vbalua.h"
#include "../version.h"
#include "Dialogs/ram_search.h"
#include <cassert>

u32	 RGB_LOW_BITS_MASK		 = 0;
int	 systemCartridgeType	 = IMAGE_GBA;
int	 systemSpeed			 = 0;
bool systemSoundOn			 = false;
u32	 systemColorMap32[0x10000];
u16	 systemColorMap16[0x10000];
u16	 systemGbPalette[24];
int	 systemRedShift			 = 0;
int	 systemBlueShift		 = 0;
int	 systemGreenShift		 = 0;
int	 systemColorDepth		 = 16;
int	 systemDebug			 = 0;
int	 systemVerbose			 = 0;
int	 systemFrameSkip		 = 0;
int	 systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

// static_assertion that BUTTON_REGULAR_RECORDING_MASK should be an u16 constant
namespace { const void * const s_STATIC_ASSERTION_(static_cast<void *>(BUTTON_REGULAR_RECORDING_MASK & 0xFFFF0000)); }

#define BMP_BUFFER_MAX_WIDTH (256)
#define BMP_BUFFER_MAX_HEIGHT (224)
#define BMP_BUFFER_MAX_DEPTH (4)
static u8 bmpBuffer[BMP_BUFFER_MAX_WIDTH * BMP_BUFFER_MAX_HEIGHT * BMP_BUFFER_MAX_DEPTH];

static int s_stockThrottleValues[] = {
	6, 15, 25, 25, 37, 50, 75, 87, 100, 112, 125, 150, 200, 300, 400, 600, 800, 1000
};

extern bool vbaShuttingDown;

void winSignal(int, int);
void winOutput(const char *, u32);

void (*dbgSignal)(int, int)	= &winSignal;
void (*dbgOutput)(const char *, u32) = &winOutput;

// Win32 stuff

//////////////////////////////////////////////
// ultility

void winSignal(int, int)
{}

void winOutput(const char *s, u32 addr)
{
	if (s)
	{
		log(s);
	}
	else
	{
		CString str;
		while (char c = CPUReadByteQuick(addr++))
		{
			str += c;
		}
		log(str);
	}
}

#ifdef SDL
void log(const char *defaultMsg, ...)
{
	char	buffer[2048];
	va_list valist;

	va_start(valist, defaultMsg);
	vsprintf(buffer, defaultMsg, valist);

	if (out == NULL)
	{
		out = fopen("trace.log", "w");
	}

	fputs(buffer, out);

	va_end(valist);
}
#else
void log(const char *msg, ...)
{
	va_list valist;
	va_start(valist, msg);

#if 1
	CString buffer;
	buffer.FormatV(msg, valist);
	extern void toolsLog(const char *);
	toolsLog(buffer);
#else
	extern void winlog(const char *msg, ...);
	winlog(msg, valist);
#endif

	va_end(valist);
}
#endif

////////////////////////////////////

// input
int systemGetDefaultJoypad()
{
	return theApp.joypadDefault;
}

void systemSetDefaultJoypad(int which)
{
	theApp.joypadDefault = which;
}

bool systemReadJoypads()
{
	// this function is called at every frame, even if vba is fast-forwarded.
	// so we try to limit the input frequency here just in case.
	static u32 lastTime = systemGetClock();
	if ((u32)(systemGetClock() - lastTime) < 10)
		return false;
	lastTime = systemGetClock();

	if (theApp.input)
		return theApp.input->readDevices();
	return false;
}

u32 systemGetOriginalJoypad(int i, bool sensor)
{
	if (i < 0 || i > 3)
		i = 0;

	u32 res = 0;
	if (theApp.input)
		res = theApp.input->readDevice(i, sensor);

	// +auto input, XOR
	// maybe these should be moved into DirectInput.cpp
	if (theApp.autoFire || theApp.autoFire2)
	{
		res ^= (theApp.autoFireToggle ? theApp.autoFire : theApp.autoFire2);
		if (!theApp.autofireAccountForLag || !systemCounters.laggedLast)
		{
			theApp.autoFireToggle = !theApp.autoFireToggle;
		}
	}
	if (theApp.autoHold)
	{
		res ^= theApp.autoHold;
	}

	// filter buttons
	// maybe better elsewhere?
	if (!theApp.allowLeftRight)
	{
		// disallow L+R or U+D to being pressed at the same time
		if ((res & (BUTTON_MASK_RIGHT | BUTTON_MASK_LEFT)) == (BUTTON_MASK_RIGHT | BUTTON_MASK_LEFT))
			res &= ~BUTTON_MASK_RIGHT;  // leave only LEFT on
		if ((res & (BUTTON_MASK_DOWN | BUTTON_MASK_UP)) == (BUTTON_MASK_DOWN | BUTTON_MASK_UP))
			res &= ~BUTTON_MASK_DOWN;  // leave only UP on
	}

	if (!sensor)
	{
		if (res & BUTTON_MOTION_MASK)
			res &= ~BUTTON_MOTION_MASK;
	}

	if (systemCartridgeType != IMAGE_GBA && !gbSgbMode) // regular GB has no L/R buttons
	{
		if (res & (BUTTON_GBA_ONLY))
			res &= ~BUTTON_GBA_ONLY;
	}

	return res;
}

// screen

void systemRenderLua(u8 *data, int pitch)
{
	int copyW, copyH;
	int screenW, screenH;
	int copyOffsetX, copyOffsetY;
	systemGetLCDResolution(copyW, copyH);
	systemGetLCDBaseSize(screenW, screenH);
	systemGetLCDBaseOffset(copyOffsetX, copyOffsetY);

	//int displayW = right - left;
	//int displayH = bottom - top;
	//screenW = screenW * displayW / copyW;
	//screenH = screenH * displayH / copyH;
	//copyOffsetX = left + copyOffsetX * displayW / copyW;
	//copyOffsetY = top  + copyOffsetY * displayH / copyH;

	//int pitch = (((ppl * systemColorDepth + 7)>>3)+3)&~3;
	//int pitch = ppl * (systemColorDepth / 8) + (systemColorDepth == 24 ? 0 : 4);

	VBALuaGui(&data[(copyOffsetY + 1) * pitch + copyOffsetX * (systemColorDepth / 8)], pitch, screenW, screenH);
	VBALuaClearGui();
}

// delayed repaint
void systemRefreshScreen()
{
	if (theApp.m_pMainWnd)
	{
		theApp.m_pMainWnd->PostMessage(WM_PAINT, NULL, NULL);
	}
}

static void systemRecordAviFrame(int width, int height)
{
	extern u32 linearSoundFrameCount;
	extern u32 linearFrameCount;

	++linearFrameCount;
	if (!theApp.sound)
	{
		u32 wrap = systemGetFrameRateDividend();
		if (linearFrameCount > wrap)
			linearFrameCount -= wrap;
		linearSoundFrameCount = linearFrameCount;
	}

	// record avi
	bool firstFrameLogged = false;
	--linearFrameCount;
	do
	{
		++linearFrameCount;

		if (theApp.aviRecording && (!theApp.altAviRecordMethod || !firstFrameLogged))
		{
			// usually aviRecorder is created when vba starts avi recording, though
			if (theApp.aviRecorder == NULL)
			{
				theApp.aviRecorder = new AVIWrite();

				BITMAPINFOHEADER bi;
				memset(&bi, 0, sizeof(bi));
				bi.biSize	   = 0x28;
				bi.biPlanes	   = 1;
				bi.biBitCount  = 24;
				bi.biWidth	   = width;
				bi.biHeight	   = height;
				bi.biSizeImage = 3 * width * height;
				theApp.aviRecorder->SetVideoFormat(&bi);
				if (!theApp.aviRecorder->Open(theApp.aviRecordName))
				{
					delete theApp.aviRecorder;
					theApp.aviRecorder	= NULL;
					theApp.aviRecording = false;
				}
			}

			if (theApp.aviRecorder != NULL && !theApp.aviRecorder->IsPaused())
			{
				assert(
					width <= BMP_BUFFER_MAX_WIDTH && height <= BMP_BUFFER_MAX_HEIGHT
					&& systemColorDepth <= BMP_BUFFER_MAX_DEPTH * 8);
				utilWriteBMP(bmpBuffer, width, height, systemColorDepth, pix);
				theApp.aviRecorder->AddFrame(bmpBuffer);
			}
		}

		if (theApp.nvVideoLog)
		{
			// convert from whatever bit depth to 16-bit, while stripping away extra pixels
			assert(width <= BMP_BUFFER_MAX_WIDTH && height <= BMP_BUFFER_MAX_HEIGHT && 16 <= BMP_BUFFER_MAX_DEPTH * 8);
			utilWriteBMP(bmpBuffer, width, -height, 16, pix);
			NESVideoLoggingVideo((u8 *)bmpBuffer, width, height, 0x1000000 * 60);
		}

		firstFrameLogged = true;
	}
	while (linearFrameCount < linearSoundFrameCount); // compensate for frames lost due to frame skip being nonzero, etc.
}

void systemRenderFrame()
{
	++theApp.renderedFrames;

	// "in-game" text rendering
	if (textMethod == 0) // transparent text can only be painted once, so timed messages will not be updated
	{
		int pitch = theApp.filterWidth * (systemColorDepth / 8) + (systemColorDepth == 24 ? 0 : 4);
		int copyW, copyH;
		systemGetLCDResolution(copyW, copyH);
		systemRenderLua(&pix[pitch], pitch);
		DrawTextMessages(&pix[pitch], pitch, 0, copyH);
	}

	VBAUpdateFrameCountDisplay();
	VBAUpdateButtonPressDisplay();

	// avi dump
	int copyX, copyY;
	systemGetLCDResolution(copyX, copyY);
	systemRecordAviFrame(copyX, copyY);

	// interframe blending
	if (theApp.ifbFunction)
	{
		if (systemColorDepth == 16)
			theApp.ifbFunction(pix + theApp.filterWidth * 2 + 4, theApp.filterWidth * 2 + 4,
			                   theApp.filterWidth, theApp.filterHeight);
		else
			theApp.ifbFunction(pix + theApp.filterWidth * 4 + 4, theApp.filterWidth * 4 + 4,
			                   theApp.filterWidth, theApp.filterHeight);
	}

	systemRedrawScreen();

	Signal_RAM_Search_New_Frame(); // updates RAM search regions
}

void systemRedrawScreen()
{
	if (vbaShuttingDown)
		return;

	if (theApp.display)
		theApp.display->render();

	systemUpdateListeners();
}

void systemUpdateListeners()
{
	if (vbaShuttingDown)
		return;

	Update_RAM_Search(); // updates RAM search and RAM watch

	// update viewers etc.
	if (theApp.updateCount)
	{
		POSITION pos = theApp.updateList.GetHeadPosition();
		while (pos)
		{
			IUpdateListener *up = theApp.updateList.GetNext(pos);
			if (up)
				up->update();
		}
	}
}

int systemScreenCapture(int captureNumber)
{
	return winScreenCapture(captureNumber);
}

void systemMessage(int number, const char *defaultMsg, ...)
{
	CString buffer;
	va_list valist;
	CString msg = defaultMsg;
	if (number)
		msg = winResLoadString(number);

	va_start(valist, defaultMsg);
	buffer.FormatV(msg, valist);

	theApp.winCheckFullscreen();
	systemSoundClearBuffer();
	AfxGetApp()->m_pMainWnd->MessageBox(buffer, winResLoadString(IDS_ERROR), MB_OK | MB_ICONERROR);

	va_end(valist);
}

void systemScreenMessage(const char *msg, int slot, int duration, const char *colorList)
{
	if (slot < 0 || slot > SCREEN_MESSAGE_SLOTS)
		return;

	theApp.screenMessage[slot] = true;
	theApp.screenMessageTime[slot]		  = GetTickCount();
	theApp.screenMessageDuration[slot]	  = duration;
	theApp.screenMessageBuffer[slot]	  = msg;
	theApp.screenMessageColorBuffer[slot] = colorList ? colorList : "";

	if (theApp.screenMessageBuffer[slot].GetLength() > 40)
		theApp.screenMessageBuffer[slot] = theApp.screenMessageBuffer[slot].Left(40);

	// update the display when a main slot message appears while the game is paused
	if (slot == 0 && (theApp.paused || (theApp.frameSearching)))
		systemRefreshScreen();
}

void systemShowSpeed(int speed)
{
	systemSpeed = speed;
	theApp.showRenderedFrames = theApp.renderedFrames;
	theApp.renderedFrames	  = 0;
	if (theApp.videoOption <= VIDEO_4X && theApp.showSpeed)
	{
		CString buffer;
		if (theApp.showSpeed == 1)
			buffer.Format(VBA_NAME_AND_VERSION " %3d%%", systemSpeed);
		else
			buffer.Format(VBA_NAME_AND_VERSION " %3d%% (%d fps | %d skipped)",
			              systemSpeed,
			              theApp.showRenderedFrames,
			              systemFrameSkip);

		systemSetTitle(buffer);
	}
}

void systemSetTitle(const char *title)
{
	if (theApp.m_pMainWnd != NULL)
	{
		AfxGetApp()->m_pMainWnd->SetWindowText(title);
	}
}

// timing/speed

u32 systemGetClock()
{
	return timeGetTime();
}

void systemIncreaseThrottle()
{
	int throttle = theApp.throttle;

	if (throttle < 6)
		++throttle;
	else if (throttle < s_stockThrottleValues[_countof(s_stockThrottleValues) - 1])
	{
		int i = 0;
		while (throttle >= s_stockThrottleValues[i])
		{
			++i;
		}
		throttle = s_stockThrottleValues[i];
	}

	systemSetThrottle(throttle);
}

void systemDecreaseThrottle()
{
	int throttle = theApp.throttle;

	if (throttle > 6)
	{
		int i = _countof(s_stockThrottleValues) - 1;
		while (throttle <= s_stockThrottleValues[i])
		{
			--i;
		}
		throttle = s_stockThrottleValues[i];
	}
	else if (throttle > 1)
		--throttle;

	systemSetThrottle(throttle);
}

void systemSetThrottle(int throttle)
{
	theApp.throttle = throttle;
	char str[256];
	sprintf(str, "%d%% throttle speed", theApp.throttle);
	systemScreenMessage(str);
}

int systemGetThrottle()
{
	return theApp.throttle;
}

void systemFrame()
{
	if (theApp.altAviRecordMethod && theApp.aviRecording)
	{
		if (theApp.aviRecorder)
		{
			if (!theApp.aviRecorder->IsSoundAdded())
			{
				WAVEFORMATEX wfx;
				memset(&wfx, 0, sizeof(wfx));
				wfx.wFormatTag		= WAVE_FORMAT_PCM;
				wfx.nChannels		= 2;
				wfx.nSamplesPerSec	= 44100 / soundQuality;
				wfx.wBitsPerSample	= 16;
				wfx.nBlockAlign		= (wfx.wBitsPerSample / 8) * wfx.nChannels;
				wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
				wfx.cbSize = 0;
				theApp.aviRecorder->SetSoundFormat(&wfx);
			}
			theApp.aviRecorder->AddSound((u8 *)soundFrameSound, soundFrameSoundWritten * 2);
		}
	}

	soundFrameSoundWritten = 0;

	// no more stupid updates :)

	extern int quitAfterTime;                   // from VBA.cpp
	void	   VBAMovieStop(bool8 suppress_message); // from ../movie.cpp
	if (quitAfterTime >= 0 && systemCounters.frameCount == quitAfterTime)
	{
		VBAMovieStop(true);
		AfxPostQuitMessage(0);
	}

	// change the sound speed, or set it to normal - must always do this or it won't get reset after a change, but that's OK
	// because it's inexpensive
	if (theApp.sound)
	{
		theApp.sound->setSpeed(
		    speedup || theApp.winPauseNextFrame || !synchronize || theApp.accuratePitchThrottle || theApp.useOldSync
			? 1.0f : (float)theApp.throttle / 100.0f);
	}

	// if a throttle speed is set and we're not allowed to change the sound frequency to achieve it,
	// sleep for a certain amount each time we get here to approximate the necessary slowdown
	if (synchronize && (theApp.accuratePitchThrottle || !theApp.sound || theApp.throttle < 6) /*&& !theApp.winPauseNextFrame*/)
	{
		/// FIXME: this is still a horrible way of achieving a certain frame time
		///        (look at what Snes9x does - it's complicated but much much better)

		static float sleepAmt = 0.0f; // variable to smooth out the sleeping amount so it doesn't oscillate so fast
//		if(!theApp.wasPaused) {
		if (!speedup)
		{
			u32 time = systemGetClock();
			u32 diff = time - theApp.throttleLastTime;
			if (theApp.wasPaused)
				diff = 0;

			int target = (100000 / (systemGetFrameRate() * theApp.throttle));
			int d	   = (target - diff);

			if (d > 1000) // added to avoid 500-day waits for vba to start emulating.
				d = 1000;  // I suspect most users aren't that patient, and would find 1 second to be a more reasonable delay.

			sleepAmt = 0.8f * sleepAmt + 0.2f * (float)d;
			if (d - sleepAmt <= 1.5f && d - sleepAmt >= -1.5f)
				d = (int)(sleepAmt);

			if (d > 0)
			{
				Sleep(d);
			}
		}
		theApp.throttleLastTime = systemGetClock();
		//}
		//else
		//{
		// Sleep(100);
		//}
	}

	if (systemCounters.frameCount % 10 == 0)
	{
		if (theApp.rewindMemory)
		{
			if (++theApp.rewindCounter >= (theApp.rewindTimer))
			{
				theApp.rewindSaveNeeded = true;
				theApp.rewindCounter	= 0;
			}
		}
		if (systemSaveUpdateCounter)
		{
			if (--systemSaveUpdateCounter <= SYSTEM_SAVE_NOT_UPDATED)
			{
				winWriteBatteryFile();
				systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
			}
		}
	}

	theApp.wasPaused = false;
///  theApp.autoFrameSkipLastTime = time;
}

int systemFramesToSkip()
{
	int framesToSkip = systemFrameSkip;

	bool fastForward = speedup;

	fastForward = (fastForward || theApp.frameSearchSkipping);
	int throttle = theApp.throttle;
	if (theApp.frameSearching && throttle < 100)
		throttle = 100;

	if (theApp.aviRecording || theApp.nvVideoLog || systemPausesNextFrame())
	{
		framesToSkip = 0; // render all frames
	}
	else
	{
		if (fastForward)
			framesToSkip = 9;  // try 6 FPS during speedup
		else if (throttle != 100)
			framesToSkip = (framesToSkip * throttle) / 100;
	}

	return framesToSkip;
}

// sound

bool systemSoundInit()
{
	if (theApp.sound)
		delete theApp.sound;

	extern ISound *newDirectSound();
	theApp.sound = newDirectSound();
	return theApp.sound->init();
}

void systemSoundShutdown()
{
	if (theApp.sound)
		delete theApp.sound;
	theApp.sound = NULL;
}

void systemSoundPause()
{
	if (theApp.sound)
		theApp.sound->pause();
	soundPaused = 1;
}

void systemSoundResume()
{
	if (theApp.sound)
		theApp.sound->resume();
	soundPaused = 0;
}

bool systemSoundIsPaused()
{
//	return soundPaused;
	return !(theApp.sound && theApp.sound->isPlaying());
}

void systemSoundClearBuffer()
{
	if (theApp.sound)
		theApp.sound->clearAudioBuffer();
}

void systemSoundResetBuffer()
{
	if (theApp.sound)
		theApp.sound->reset();
}

void systemSoundWriteToBuffer()
{
	if (theApp.sound)
		theApp.sound->write();
}

bool systemSoundAppliesDSP()
{
	return !(theApp.soundRecording || theApp.aviRecording || theApp.nvAudioLog);
}

// emulation

bool systemIsEmulating()
{
	return emulating != 0;
}

bool systemIsGbBorderOn()
{
	return gbBorderOn != 0;
}

void systemGbBorderOn()
{
	if (vbaShuttingDown)
		return;

	if (emulating && systemCartridgeType == 1)
	{
		theApp.updateWindowSize(theApp.videoOption);
	}
}

bool systemIsRunningGBA()
{
	return (systemCartridgeType == IMAGE_GBA);
}

bool systemIsSpedUp()
{
	return theApp.speedupToggle;
}

bool systemIsPaused()
{
	return theApp.paused;
}

void systemSetPause(bool pause)
{
	if (pause)
	{
		capturePrevious	 = false;
		theApp.wasPaused = true;
		theApp.paused	 = true;
		theApp.speedupToggle = false;
		theApp.winPauseNextFrame = false;
		systemSoundPause();
		systemRefreshScreen();;
	}
	else
	{
		theApp.paused = false;
		systemSoundResume();
	}
}

__forceinline bool systemPausesNextFrame()
{
	return theApp.winPauseNextFrame && (!theApp.nextframeAccountForLag || !systemCounters.laggedLast);
}
