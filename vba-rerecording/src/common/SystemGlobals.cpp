#include "SystemGlobals.h"

// FIXME: it must be admitted that the naming schema is a whole mess
EmulatedSystem theEmulator;

EmulatedSystemCounters systemCounters =
{
	// frameCount
	0,
	// lagCount
	0,
	// extraCount
	0,
	// lagged
	true,
	// laggedLast
	true,
};

int emulating = 0;

u8 *bios = NULL;

struct Pix
{
	u8 *data;
	int pitch;
	int bpp;
	int width;
	int height;
};

u8 *pix	 = NULL;

u16	  currentButtons[4] = { 0, 0, 0, 0 };
u16	  lastButtons[4] = { 0, 0, 0, 0 };
u16	  nextButtons[4] = { 0, 0, 0, 0 };

int32 sensorX  = 0;
int32 sensorY  = 0;

bool  newFrame		  = true;
bool8 speedup		  = false;
u32	  extButtons	  = 0;
bool8 capturePrevious = false;
int32 captureNumber	  = 0;

soundtick_t USE_TICKS_AS  = 0;
soundtick_t soundTickStep = soundQuality * USE_TICKS_AS;
soundtick_t soundTicks	  = 0;

u32	  soundIndex		= 0;
int32 soundPaused		= 1;
int32 soundPlay			= 0;
u32	  soundNextPosition = 0;

u8	soundBuffer[6][735];
u32 soundBufferLen		= 1470;
u32 soundBufferTotalLen = 14700;
u32 soundBufferIndex	= 0;

u16	  soundFinalWave[1470];
u16	  soundFrameSound[735 * 30 * 2]; // for avi logging
int32 soundFrameSoundWritten = 0;

bool tempSaveSafe	  = true;
int	 tempSaveID		  = 0;
int	 tempSaveAttempts = 0;

// settings
bool  synchronize = true;
int32 gbFrameSkip = 0;
int32 frameSkip	  = 0;

bool  cpuDisableSfx = false;
int32 layerSettings = 0xff00;

#ifdef USE_GB_CORE_V7
bool gbNullInputHackEnabled		= false;
bool gbNullInputHackTempEnabled = false;
#else
bool gbV20GBFrameTimingHack		= false;
bool gbV20GBFrameTimingHackTemp = false;
#endif

#ifdef USE_GBA_CORE_V7
bool memLagEnabled	   = false;
bool memLagTempEnabled = false;
#endif

bool8 useOldFrameTiming	  = false;
bool8 useBios			  = false;
bool8 skipBios			  = false;
bool8 skipSaveGameBattery = false;
bool8 skipSaveGameCheats  = false;
bool8 cheatsEnabled		  = true;
bool8 mirroringEnable	  = false;

bool8 cpuEnhancedDetection = true;
int32 cpuSaveType		   = 0;

int32 soundVolume	  = 0;
int32 soundQuality	  = 2;
bool8 soundEcho		  = false;
bool8 soundLowPass	  = false;
bool8 soundReverse	  = false;
int32 soundEnableFlag = 0x3ff;
bool8 soundOffFlag	  = false;

// I am just too lazy...
u8 osd[4 * 257 * 226];
