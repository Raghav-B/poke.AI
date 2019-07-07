#ifndef VBA_WIN32_SOUND_H
#define VBA_WIN32_SOUND_H

class ISound
{
public:
	virtual ~ISound() {};

	virtual bool init()   = 0;
	virtual void pause()  = 0;
	virtual void reset()  = 0;
	virtual void resume() = 0;
	virtual void write()  = 0;
	virtual void setSpeed(float rate) = 0;
	virtual bool isPlaying() = 0;
	virtual void clearAudioBuffer() {}
};

#endif
