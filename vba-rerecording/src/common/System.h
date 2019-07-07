#ifndef VBA_SYSTEM_H
#define VBA_SYSTEM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zlib.h"
#include "../Port.h"

extern void log(const char *, ...);

extern void systemGbPrint(u8 *, int, int, int, int);
extern int32 systemGetLCDSizeType();
extern void systemGetLCDResolution(int32 &width, int32 &height);
extern void systemGetLCDBaseSize(int32 &width, int32 &height);
extern void systemGetLCDBaseOffset(int32 &xofs, int32 &yofs);
extern void systemClonePixBuffer(u8 *dst);
extern int  systemScreenCapture(int captureNumber);
extern void systemRenderLua(u8 *data, int pitch);
extern void systemRefreshScreen();
extern void systemRenderFrame();
extern void systemRedrawScreen();
extern void systemUpdateListeners();
// updates the motion sensor
extern void systemSetSensorX(int32);
extern void systemSetSensorY(int32);
extern void systemResetSensor();
extern int32 systemGetSensorX();
extern int32 systemGetSensorY();
extern void systemUpdateMotionSensor(int);
// updates the joystick data
extern int  systemGetDefaultJoypad();
extern void systemSetDefaultJoypad(int);
extern bool systemReadJoypads();
// return information about the given joystick, -1 for default joystick...
// the bool is for if motion sensor should be handled too
extern u32  systemGetOriginalJoypad(int, bool);
extern u32  systemGetJoypad(int, bool);
extern void systemSetJoypad(int, u32);
extern void systemClearJoypads();
extern void systemMessage(int, const char *, ...);
extern void systemScreenMessage(const char *msg, int slot = 0, int duration = 3000, const char *colorList = NULL);
// sound
extern bool systemSoundInit();
extern void systemSoundShutdown();
extern void systemSoundPause();
extern void systemSoundResume();
extern bool systemSoundIsPaused();
extern void systemSoundResetBuffer();
extern void systemSoundWriteToBuffer();
extern void systemSoundClearBuffer();
extern bool systemSoundCleanInit();
extern void systemSoundEnableChannels(int);
extern void systemSoundDisableChannels(int);
extern int systemSoundGetEnabledChannels();
extern void systemSoundSetQuality(int quality);
extern bool systemSoundAppliesDSP();
extern void systemSoundMixReset();
extern void systemSoundMixSilence();
extern void systemSoundMix(int resL, int resR);
extern void systemSoundNext();
// speed-related stuff
extern u32 systemGetFrameRateDividend();
extern u32 systemGetFrameRateDivisor();
extern double systemGetFrameRate();
extern u32  systemGetClock();
extern void systemSetTitle(const char *);
extern void systemShowSpeed(int);
extern void systemIncreaseThrottle();
extern void systemDecreaseThrottle();
extern void systemSetThrottle(int);
extern int  systemGetThrottle();
extern void systemFrame();
extern int  systemFramesToSkip();
extern void systemCleanUp();
extern void systemReset();
extern bool systemFrameDrawingRequired();
extern void systemFrameBoundaryWork();
extern bool systemIsEmulating();
extern void systemGbBorderOn();
extern bool systemIsRunningGBA();
extern bool systemIsSpedUp();
extern bool systemIsPaused();
extern void systemSetPause(bool pause);
extern bool systemPausesNextFrame();
extern bool systemLoadBIOS(const char *biosFileName, bool useBiosFile);

extern int	systemCartridgeType;
extern int  systemSpeed;
extern bool systemSoundOn;
extern u16  systemColorMap16[0x10000];
extern u32  systemColorMap32[0x10000];
extern u16  systemGbPalette[24];
extern int  systemRedShift;
extern int  systemGreenShift;
extern int  systemBlueShift;
extern int  systemColorDepth;
extern int  systemDebug;
extern int  systemVerbose;
extern int  systemFrameSkip;
extern int  systemSaveUpdateCounter;

// constances
#define SYSTEM_SAVE_UPDATED 30
#define SYSTEM_SAVE_NOT_UPDATED 0
#define SYSTEM_SENSOR_INIT_VALUE 2047

enum NativeDisplayResolutions
{
	UNKNOWN_NDR = -1,
	GBA_NDR = 0,
	GB_NDR = 1,
	SGB_NDR = 2
};

// FIXME: Not better way?
#define PIX_MALLOC(n) \
	(void *)((char *)malloc((n) + 4) + 4)
#define PIX_CALLOC(n) \
	(void *)((char *)calloc(1, (n) + 4) + 4)
#define PIX_FREE(p) \
	free((char *)(p) - 4)

// FIXME: Should fix RAM Search and RAM Watch's OoB intead
#define RAM_MALLOC(n) \
	malloc((n) + 4)
#define RAM_CALLOC(n) \
	calloc(1, (n) + 4)
// FIXME: Just to be consistent
#define RAM_FREE(p) \
	free(p)

#endif // VBA_SYSTEM_H
