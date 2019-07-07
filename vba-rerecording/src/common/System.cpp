#include "System.h"
#include "SystemGlobals.h"
#include "inputGlobal.h"
#include "../gb/gbGlobals.h"
#include "../gba/GBAGlobals.h"
#include "../gba/GBA.h"
#include "../common/movie.h"
#include "../common/vbalua.h"

// systemABC stuff are core-related

// evil macros
#ifndef countof
#define countof(a)  (sizeof(a) / sizeof(a[0]))
#endif

// evil static variables
static u32	 lastFrameTime	= 0;
static int32 frameSkipCount	= 0;
static int32 frameCount		= 0;

static s16	 soundFilter[4000];
static s16	 soundRight[5]  = { 0, 0, 0, 0, 0 };
static s16	 soundLeft[5]   = { 0, 0, 0, 0, 0 };
static int32 soundEchoIndex = 0;

// motion sensor
void systemSetSensorX(int32 x)
{
	sensorX = x;
}

void systemSetSensorY(int32 y)
{
	sensorY = y;
}

void systemResetSensor()
{
	sensorX = sensorY = SYSTEM_SENSOR_INIT_VALUE;
}

int32 systemGetSensorX()
{
	return sensorX;
}

int32 systemGetSensorY()
{
	return sensorY;
}

void systemUpdateMotionSensor(int i)
{
	if (i < 0 || i > 3)
		i = 0;

	if (currentButtons[i] & BUTTON_MASK_LEFT_MOTION)
	{
		sensorX += 3;
		if (sensorX > 2197)
			sensorX = 2197;
		if (sensorX < 2047)
			sensorX = 2057;
	}
	else if (currentButtons[i] & BUTTON_MASK_RIGHT_MOTION)
	{
		sensorX -= 3;
		if (sensorX < 1897)
			sensorX = 1897;
		if (sensorX > 2047)
			sensorX = 2037;
	}
	else if (sensorX > 2047)
	{
		sensorX -= 2;
		if (sensorX < 2047)
			sensorX = 2047;
	}
	else
	{
		sensorX += 2;
		if (sensorX > 2047)
			sensorX = 2047;
	}

	if (currentButtons[i] & BUTTON_MASK_UP_MOTION)
	{
		sensorY += 3;
		if (sensorY > 2197)
			sensorY = 2197;
		if (sensorY < 2047)
			sensorY = 2057;
	}
	else if (currentButtons[i] & BUTTON_MASK_DOWN_MOTION)
	{
		sensorY -= 3;
		if (sensorY < 1897)
			sensorY = 1897;
		if (sensorY > 2047)
			sensorY = 2037;
	}
	else if (sensorY > 2047)
	{
		sensorY -= 2;
		if (sensorY < 2047)
			sensorY = 2047;
	}
	else
	{
		sensorY += 2;
		if (sensorY > 2047)
			sensorY = 2047;
	}
}

// joypads
u32 systemGetJoypad(int i, bool sensor)
{
	if (i < 0 || i > 3)
		i = 0;

	// input priority: original = raw+auto < Lua < movie, correct this if wrong

	// get original+auto input
	u32 res = systemGetOriginalJoypad(i, sensor);

	// Lua input, shouldn't have any side effect within it
	if (VBALuaUsingJoypad(i))
		res = VBALuaReadJoypad(i);

	// therefore, lua is currently allowed to modify the extbuttons...
	u32 extButtons = res & BUTTON_NONRECORDINGONLY_MASK;

	nextButtons[i] = VBAMovieGetNextInputOf(i);

	// movie input has the highest priority
	if (VBAMovieIsPlaying())
	{
		currentButtons[i] = res & BUTTON_REGULAR_RECORDING_MASK;
		// VBAMovieRead() overwrites currentButtons[i]
		VBAMovieRead(i, sensor);
	}
	else
	{
		currentButtons[i] = res & BUTTON_REGULAR_RECORDING_MASK;
		if (VBAMovieIsRecording())
		{
			// the "current buttons" input buffer will be read by the movie routine
			VBAMovieWrite(i, sensor);
		}
	}

	return currentButtons[i] | extButtons;
}

void systemSetJoypad(int which, u32 buttons)
{
	if (which < 0 || which > 3)
		which = 0;

	currentButtons[which] = buttons;
	//lastButtons[which] = 0;
}

void systemClearJoypads()
{
	for (int i = 0; i < 4; ++i)
	{
		currentButtons[i] = 0;
		lastButtons[i] = 0;
	}
}

// emulation
u32 systemGetFrameRateDividend()
{
	extern const u32 frameRateDividend;
#if 0
	extern const u32 gbFrameRateDividend;
	if (systemCartridgeType == IMAGE_GBA)
		return frameRateDividend;
	else
		return gbFrameRateDividend;
#endif
	return frameRateDividend;
}

u32 systemGetFrameRateDivisor()
{
	extern const u32 frameRateDivisor;
#if 0
	extern const u32 gbFrameRateDivisor;
	if (systemCartridgeType == IMAGE_GBA)
		return frameRateDivisor;
	else
		return gbFrameRateDivisor;
#else
	return frameRateDivisor;
#endif
}

double systemGetFrameRate()
{
	extern const double frameRate;
#if 0
	extern const double gbFrameRate;
	if (systemCartridgeType == IMAGE_GBA)
		return frameRate;
	else
		return gbFrameRate;
#else
	return frameRate;
#endif
}

void systemCleanUp()
{
	// frame counting
	frameCount	   = 0;
	frameSkipCount = systemFramesToSkip();
	lastFrameTime  = systemGetClock();

	extButtons = 0;
	newFrame   = true;

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	systemResetSensor();

	systemCounters.frameCount = 0;
	systemCounters.extraCount = 0;
	systemCounters.lagCount	  = 0;
	systemCounters.lagged	  = true;
	systemCounters.laggedLast = true;

	systemClearJoypads();
}

void systemReset()
{
	// frame counting
	frameCount	   = 0;
	frameSkipCount = systemFramesToSkip();
	lastFrameTime  = systemGetClock();

	extButtons = 0;
	newFrame   = true;

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	systemResetSensor();

	if (!VBAMovieIsActive())
	{
		systemCounters.frameCount = 0;
		systemCounters.extraCount = 0;
		systemCounters.lagCount	  = 0;
		systemCounters.lagged	  = true;
		systemCounters.laggedLast = true;
	}
}

bool systemFrameDrawingRequired()
{
	return frameSkipCount >= systemFramesToSkip();
}

void systemFrameBoundaryWork()
{
	newFrame = true;

	systemFrame();

	++frameCount;
	u32 currentTime = systemGetClock();
	if (currentTime - lastFrameTime >= 1000)
	{
		systemShowSpeed(int(float(frameCount) * 100000 / (float(currentTime - lastFrameTime) * systemGetFrameRate()) + .5f));
		lastFrameTime = currentTime;
		frameCount = 0;
	}

	++systemCounters.frameCount;
	if (systemCounters.lagged)
	{
		++systemCounters.lagCount;
	}
	systemCounters.laggedLast = systemCounters.lagged;
	systemCounters.lagged	 = true;

	if (systemFrameDrawingRequired())
	{
		systemRenderFrame();
		frameSkipCount = 0;

		bool capturePressed = (extButtons & 2) != 0;
		if (capturePressed && !capturePrevious)
		{
			captureNumber = systemScreenCapture(captureNumber);
		}
		capturePrevious = capturePressed && !systemPausesNextFrame();
	}
	else
	{
		++frameSkipCount;
	}

	CallRegisteredLuaFunctions(LUACALL_AFTEREMULATION);

	if (VBALuaRunning())
	{
		VBALuaFrameBoundary();
	}

	VBAMovieUpdateState();

	if (systemPausesNextFrame())
	{
		systemSetPause(true);
	}
}

// BIOS, only GBA BIOS supported at present
bool systemLoadBIOS(const char *biosFileName, bool useBiosFile)
{
	if (systemCartridgeType != IMAGE_GBA) return false;

	useBios = false;
	if (useBiosFile)
	{
		useBios = utilLoadBIOS(bios, biosFileName, 4);
	}
	if (!useBios)
	{
		CPULoadInternalBios();
	}
	return useBios;
}

// graphics
// overloads #0
int32 systemGetLCDSizeType()
{
	switch (systemCartridgeType)
	{
	case IMAGE_GBA:
		return GBA_NDR;
	case IMAGE_GB:
		return gbBorderOn ? SGB_NDR : GB_NDR;
	default:
		return UNKNOWN_NDR;
	}
}

void systemGetLCDResolution(int32 &width, int32& height)
{
	switch (systemGetLCDSizeType())
	{
	default:
		width = 240;
		height = 160;
		break;
	case GB_NDR:
		width = 160;
		height = 144;
		break;
	case SGB_NDR:
		width = 256;
		height = 224;
		break;
	}
}

void systemGetLCDBaseSize(int32 &width, int32& height)
{
	switch (systemGetLCDSizeType())
	{
	default:
		width = 240;
		height = 160;
		break;
	case GB_NDR:
	case SGB_NDR:
		width = 160;
		height = 144;
		break;
	}
}

void systemGetLCDBaseOffset(int32 &xofs, int32 &yofs)
{
	switch (systemGetLCDSizeType())
	{
	default:
		xofs = 0;
		yofs = 0;
		break;
	case SGB_NDR:
		xofs = 48;
		yofs = 40;
		break;
	}
}

void systemClonePixBuffer(u8 *dst)
{
	memcpy(dst, pix, systemIsRunningGBA() ? pixBufferSize : gbPixBufferSize);
}

// sound

bool systemSoundCleanInit()
{
	if (systemSoundInit())
	{
		memset(soundBuffer[0], 0, 735);
		memset(soundBuffer[1], 0, 735);
		memset(soundBuffer[2], 0, 735);
		memset(soundBuffer[3], 0, 735);
		memset(soundFinalWave, 0, soundBufferLen);

		soundPaused = 1;
		return true;
	}
	return false;
}

void systemSoundEnableChannels(int channels)
{
	int c = (channels & 0x0f) << 4;
	soundEnableFlag |= ((channels & 0x30f) | c);
}

void systemSoundDisableChannels(int channels)
{
	int c = (channels & 0x0f) << 4;
	soundEnableFlag &= ~((channels & 0x30f) | c);
}

int systemSoundGetEnabledChannels()
{
	return (soundEnableFlag & 0x30f);
}

void systemSoundSetQuality(int quality)
{
	if (soundQuality != quality)
	{
		if (!soundOffFlag)
			systemSoundShutdown();
		soundQuality = quality;
		if (!soundOffFlag)
			systemSoundCleanInit();
	}

	soundNextPosition = 0;
	soundTickStep	  = USE_TICKS_AS * soundQuality;
	soundIndex		  = 0;
	soundBufferIndex  = 0;
}

void systemSoundMixReset()
{
	soundBufferIndex  = 0;
	memset(soundFinalWave, 0, soundBufferLen);
	memset(soundFilter, 0, sizeof(soundFilter));
	soundEchoIndex = 0;
}

void systemSoundMixSilence()
{
	soundFinalWave[soundBufferIndex++] = 0;
	soundFinalWave[soundBufferIndex++] = 0;
	if ((soundFrameSoundWritten + 1) < countof(soundFrameSound))
	{
		soundFrameSound[soundFrameSoundWritten++] = 0;
		soundFrameSound[soundFrameSoundWritten++] = 0;
	}
}

void systemSoundMix(int resL, int resR)
{
	bool usesDSP = systemSoundAppliesDSP();
	if (soundEcho)
	{
		if (soundEchoIndex >= countof(soundFilter))
			soundEchoIndex = 0;

		resL += soundFilter[soundEchoIndex] / 2;
		soundFilter[soundEchoIndex++] = resL;

		resR += soundFilter[soundEchoIndex] / 2;
		soundFilter[soundEchoIndex++] = resR;
	}

	if (soundLowPass)
	{
		soundLeft[4] = soundLeft[3];
		soundLeft[3] = soundLeft[2];
		soundLeft[2] = soundLeft[1];
		soundLeft[1] = soundLeft[0];
		soundLeft[0] = resL;
		resL = (soundLeft[4] + 2 * soundLeft[3] + 8 * soundLeft[2] + 2 * soundLeft[1] + soundLeft[0]) / 14;

		soundRight[4] = soundRight[3];
		soundRight[3] = soundRight[2];
		soundRight[2] = soundRight[1];
		soundRight[1] = soundRight[0];
		soundRight[0] = resR;
		resR = (soundRight[4] + 2 * soundRight[3] + 8 * soundRight[2] + 2 * soundRight[1] +  soundRight[0]) / 14;
	}

	if (usesDSP)
	{
		switch (soundVolume)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			resL *= (soundVolume + 1);
			resR *= (soundVolume + 1);
			break;
		case 4:
			resL >>= 2;
			resR >>= 2;
			break;
		case 5:
			resL >>= 1;
			resR >>= 1;
			break;
		}
	}

	if (resL > 32767)
		resL = 32767;
	else if (resL < -32768)
		resL = -32768;

	if (resR > 32767)
		resR = 32767;
	else if (resR < -32768)
		resR = -32768;

	if (soundReverse && usesDSP)
	{
		soundFinalWave[soundBufferIndex++] = resR;
		soundFinalWave[soundBufferIndex++] = resL;
		if ((soundFrameSoundWritten + 1) < countof(soundFrameSound))
		{
			soundFrameSound[soundFrameSoundWritten++] = resR;
			soundFrameSound[soundFrameSoundWritten++] = resL;
		}
	}
	else
	{
		soundFinalWave[soundBufferIndex++] = resL;
		soundFinalWave[soundBufferIndex++] = resR;
		if (soundFrameSoundWritten + 1 < countof(soundFrameSound))
		{
			soundFrameSound[soundFrameSoundWritten++] = resL;
			soundFrameSound[soundFrameSoundWritten++] = resR;
		}
	}
}

void systemSoundNext()
{
	soundIndex++;

	if (2 * soundBufferIndex >= soundBufferLen)
	{
		if (systemSoundOn)
		{
			if (soundPaused && !systemIsPaused())	// this checking is for the old frame timing
			{
				systemSoundResume();
			}

			systemSoundWriteToBuffer();
		}
		soundIndex		 = 0;
		soundBufferIndex = 0;
	}
}
