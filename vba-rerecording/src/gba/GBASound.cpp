#include <cstring>

#include "../common/System.h" // SDL build needs this
#include "../common/SystemGlobals.h"
#include "../common/Util.h"
#include "GBA.h"
#include "GBAGlobals.h"
#include "GBASound.h"

#define SOUND_MAGIC   0x60000000
#define SOUND_MAGIC_2 0x30000000
#define NOISE_MAGIC (2097152.0 / 44100.0)

extern bool8 stopState;

u8 soundWavePattern[4][32] = {
	{ 0x01, 0x01, 0x01, 0x01,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff },
	{ 0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff },
	{ 0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff },
	{ 0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0x01, 0x01, 0x01, 0x01,
	  0xff, 0xff, 0xff, 0xff,
	  0xff, 0xff, 0xff, 0xff }
};

int32 soundFreqRatio[8] = {
	1048576, // 0
	524288, // 1
	262144, // 2
	174763, // 3
	131072, // 4
	104858, // 5
	87381, // 6
	74898  // 7
};

int32 soundShiftClock[16] = {
	2,   // 0
	4,   // 1
	8,   // 2
	16,  // 3
	32,  // 4
	64,  // 5
	128, // 6
	256, // 7
	512, // 8
	1024, // 9
	2048, // 10
	4096, // 11
	8192, // 12
	16384, // 13
	1,   // 14
	1    // 15
};

int32 soundLevel1 = 0;
int32 soundLevel2 = 0;
int32 soundBalance = 0;
int32 soundMasterOn = 0;
int32 soundDebug = 0;

int32 sound1On = 0;
int32 sound1ATL = 0;
int32 sound1Skip = 0;
int32 sound1Index = 0;
int32 sound1Continue = 0;
int32 sound1EnvelopeVolume	  = 0;
int32 sound1EnvelopeATL		  = 0;
int32 sound1EnvelopeUpDown	  = 0;
int32 sound1EnvelopeATLReload = 0;
int32 sound1SweepATL		  = 0;
int32 sound1SweepATLReload	  = 0;
int32 sound1SweepSteps		  = 0;
int32 sound1SweepUpDown		  = 0;
int32 sound1SweepStep		  = 0;
u8 *  sound1Wave			  = soundWavePattern[2];

int32 sound2On = 0;
int32 sound2ATL = 0;
int32 sound2Skip = 0;
int32 sound2Index = 0;
int32 sound2Continue = 0;
int32 sound2EnvelopeVolume	  = 0;
int32 sound2EnvelopeATL		  = 0;
int32 sound2EnvelopeUpDown	  = 0;
int32 sound2EnvelopeATLReload = 0;
u8 *  sound2Wave			  = soundWavePattern[2];

int32 sound3On = 0;
int32 sound3ATL			= 0;
int32 sound3Skip		= 0;
int32 sound3Index		= 0;
int32 sound3Continue	= 0;
int32 sound3OutputLevel = 0;
int32 sound3Last		= 0;
u8	  sound3WaveRam[0x20];
int32 sound3Bank		 = 0;
int32 sound3DataSize	 = 0;
int32 sound3ForcedOutput = 0;

int32 sound4On = 0;
int32 sound4Clock = 0;
int32 sound4ATL = 0;
int32 sound4Skip = 0;
int32 sound4Index = 0;
int32 sound4ShiftRight		  = 0x7f;
int32 sound4ShiftSkip		  = 0;
int32 sound4ShiftIndex		  = 0;
int32 sound4NSteps			  = 0;
int32 sound4CountDown		  = 0;
int32 sound4Continue		  = 0;
int32 sound4EnvelopeVolume	  = 0;
int32 sound4EnvelopeATL		  = 0;
int32 sound4EnvelopeUpDown	  = 0;
int32 sound4EnvelopeATLReload = 0;

int32 soundDSFifoAIndex		 = 0;
int32 soundDSFifoACount		 = 0;
int32 soundDSFifoAWriteIndex = 0;
bool8 soundDSAEnabled		 = false;
int32 soundDSATimer = 0;
u8	  soundDSFifoA[32];
u8	  soundDSAValue = 0;

int32 soundDSFifoBIndex		 = 0;
int32 soundDSFifoBCount		 = 0;
int32 soundDSFifoBWriteIndex = 0;
bool8 soundDSBEnabled		 = false; // but bool8 is not big enough for int32
int32 soundDSBTimer = 0;
u8	  soundDSFifoB[32];
u8	  soundDSBValue = 0;

int32 soundControl = 0;

// dummy variables
static int32  soundTicks_int32;
static int32  soundTickStep_int32;
static int32  soundDSBValue_int32;
static int32  soundDSBEnabled_int32;

variable_desc soundSaveStruct[] = {
	{ &soundPaused,				sizeof(int32) },
	{ &soundPlay,				sizeof(int32) },
	{ &soundTicks_int32,		sizeof(int32) },
	{ &soundTickStep_int32,		sizeof(int32) },
	{ &soundLevel1,				sizeof(int32) },
	{ &soundLevel2,				sizeof(int32) },
	{ &soundBalance,			sizeof(int32) },
	{ &soundMasterOn,			sizeof(int32) },
	{ &soundIndex,				sizeof(int32) },
	{ &sound1On,				sizeof(int32) },
	{ &sound1ATL,				sizeof(int32) },
	{ &sound1Skip,				sizeof(int32) },
	{ &sound1Index,				sizeof(int32) },
	{ &sound1Continue,			sizeof(int32) },
	{ &sound1EnvelopeVolume,	sizeof(int32) },
	{ &sound1EnvelopeATL,		sizeof(int32) },
	{ &sound1EnvelopeATLReload, sizeof(int32) },
	{ &sound1EnvelopeUpDown,	sizeof(int32) },
	{ &sound1SweepATL,			sizeof(int32) },
	{ &sound1SweepATLReload,	sizeof(int32) },
	{ &sound1SweepSteps,		sizeof(int32) },
	{ &sound1SweepUpDown,		sizeof(int32) },
	{ &sound1SweepStep,			sizeof(int32) },
	{ &sound2On,				sizeof(int32) },
	{ &sound2ATL,				sizeof(int32) },
	{ &sound2Skip,				sizeof(int32) },
	{ &sound2Index,				sizeof(int32) },
	{ &sound2Continue,			sizeof(int32) },
	{ &sound2EnvelopeVolume,	sizeof(int32) },
	{ &sound2EnvelopeATL,		sizeof(int32) },
	{ &sound2EnvelopeATLReload, sizeof(int32) },
	{ &sound2EnvelopeUpDown,	sizeof(int32) },
	{ &sound3On,				sizeof(int32) },
	{ &sound3ATL,				sizeof(int32) },
	{ &sound3Skip,				sizeof(int32) },
	{ &sound3Index,				sizeof(int32) },
	{ &sound3Continue,			sizeof(int32) },
	{ &sound3OutputLevel,		sizeof(int32) },
	{ &sound4On,				sizeof(int32) },
	{ &sound4ATL,				sizeof(int32) },
	{ &sound4Skip,				sizeof(int32) },
	{ &sound4Index,				sizeof(int32) },
	{ &sound4Clock,				sizeof(int32) },
	{ &sound4ShiftRight,		sizeof(int32) },
	{ &sound4ShiftSkip,			sizeof(int32) },
	{ &sound4ShiftIndex,		sizeof(int32) },
	{ &sound4NSteps,			sizeof(int32) },
	{ &sound4CountDown,			sizeof(int32) },
	{ &sound4Continue,			sizeof(int32) },
	{ &sound4EnvelopeVolume,	sizeof(int32) },
	{ &sound4EnvelopeATL,		sizeof(int32) },
	{ &sound4EnvelopeATLReload, sizeof(int32) },
	{ &sound4EnvelopeUpDown,	sizeof(int32) },
	{ &soundEnableFlag,			sizeof(int32) },
	{ &soundControl,			sizeof(int32) },
	{ &soundDSFifoAIndex,		sizeof(int32) },
	{ &soundDSFifoACount,		sizeof(int32) },
	{ &soundDSFifoAWriteIndex,	sizeof(int32) },
	{ &soundDSAEnabled,			sizeof(bool8) },
	{ &soundDSATimer,			sizeof(int32) },
	{ &soundDSFifoA[0],			32			  },
	{ &soundDSAValue,			sizeof(u8)	  },
	{ &soundDSFifoBIndex,		sizeof(int32) },
	{ &soundDSFifoBCount,		sizeof(int32) },
	{ &soundDSFifoBWriteIndex,	sizeof(int32) },
	{ &soundDSBEnabled_int32,	sizeof(int32) }, // now you know why bool8 is false salvation
	{ &soundDSBTimer,			sizeof(int32) },
	{ &soundDSFifoB[0],			32			  },
	{ &soundDSBValue_int32,		sizeof(int32) }, // save as int32 because of a mistake of the past.
	{ &soundBuffer[0][0],		6 * 735		  },
	{ &soundFinalWave[0],		2 * 735		  },
	{ NULL,						0			  }
};

variable_desc soundSaveStructV2[] = {
	{ &sound3WaveRam[0],   0x20			 },
	{ &sound3Bank,		   sizeof(int32) },
	{ &sound3DataSize,	   sizeof(int32) },
	{ &sound3ForcedOutput, sizeof(int32) },
	{ NULL,				   0			 }
};

//variable_desc soundSaveStructV3[] = {
//  { &soundTicks, sizeof(soundtick_t) },
//  { &soundTickStep, sizeof(soundtick_t) },
//  { &USE_TICKS_AS, sizeof(soundtick_t) },
//  { NULL, 0 }
//};

void soundEvent(u32 address, u8 data)
{
	int freq = 0;

	switch (address)
	{
	case NR10:
		data &= 0x7f;
		sound1SweepATL	  = sound1SweepATLReload = 344 * ((data >> 4) & 7);
		sound1SweepSteps  = data & 7;
		sound1SweepUpDown = data & 0x08;
		sound1SweepStep	  = 0;
		ioMem[address]	  = data;
		break;
	case NR11:
		sound1Wave	   = soundWavePattern[data >> 6];
		sound1ATL	   = 172 * (64 - (data & 0x3f));
		ioMem[address] = data;
		break;
	case NR12:
		sound1EnvelopeUpDown	= data & 0x08;
		sound1EnvelopeATLReload = 689 * (data & 7);
		if ((data & 0xF8) == 0)
			sound1EnvelopeVolume = 0;
		ioMem[address] = data;
		break;
	case NR13:
		freq	  = (((int)(ioMem[NR14] & 7)) << 8) | data;
		sound1ATL = 172 * (64 - (ioMem[NR11] & 0x3f));
		freq	  = 2048 - freq;
		if (freq)
		{
			sound1Skip = SOUND_MAGIC / freq;
		}
		else
			sound1Skip = 0;
		ioMem[address] = data;
		break;
	case NR14:
		data &= 0xC7;
		freq = (((int)(data & 7) << 8) | ioMem[NR13]);
		freq = 2048 - freq;
		sound1ATL	   = 172 * (64 - (ioMem[NR11] & 0x3f));
		sound1Continue = data & 0x40;
		if (freq)
		{
			sound1Skip = SOUND_MAGIC / freq;
		}
		else
			sound1Skip = 0;
		if (data & 0x80)
		{
			ioMem[NR52] |= 1;
			sound1EnvelopeVolume = ioMem[NR12] >> 4;
			sound1EnvelopeUpDown = ioMem[NR12] & 0x08;
			sound1ATL = 172 * (64 - (ioMem[NR11] & 0x3f));
			sound1EnvelopeATLReload = sound1EnvelopeATL = 689 * (ioMem[NR12] & 7);
			sound1SweepATL			= sound1SweepATLReload = 344 * ((ioMem[NR10] >> 4) & 7);
			sound1SweepSteps		= ioMem[NR10] & 7;
			sound1SweepUpDown		= ioMem[NR10] & 0x08;
			sound1SweepStep			= 0;

			sound1Index = 0;
			sound1On	= 1;
		}
		ioMem[address] = data;
		break;
	case NR21:
		sound2Wave	   = soundWavePattern[data >> 6];
		sound2ATL	   = 172 * (64 - (data & 0x3f));
		ioMem[address] = data;
		break;
	case NR22:
		sound2EnvelopeUpDown	= data & 0x08;
		sound2EnvelopeATLReload = 689 * (data & 7);
		if ((data & 0xF8) == 0)
			sound2EnvelopeVolume = 0;
		ioMem[address] = data;
		break;
	case NR23:
		freq	  = (((int)(ioMem[NR24] & 7)) << 8) | data;
		sound2ATL = 172 * (64 - (ioMem[NR21] & 0x3f));
		freq	  = 2048 - freq;
		if (freq)
		{
			sound2Skip = SOUND_MAGIC / freq;
		}
		else
			sound2Skip = 0;
		ioMem[address] = data;
		break;
	case NR24:
		data &= 0xC7;
		freq = (((int)(data & 7) << 8) | ioMem[NR23]);
		freq = 2048 - freq;
		sound2ATL	   = 172 * (64 - (ioMem[NR21] & 0x3f));
		sound2Continue = data & 0x40;
		if (freq)
		{
			sound2Skip = SOUND_MAGIC / freq;
		}
		else
			sound2Skip = 0;
		if (data & 0x80)
		{
			ioMem[NR52] |= 2;
			sound2EnvelopeVolume = ioMem[NR22] >> 4;
			sound2EnvelopeUpDown = ioMem[NR22] & 0x08;
			sound2ATL = 172 * (64 - (ioMem[NR21] & 0x3f));
			sound2EnvelopeATLReload = sound2EnvelopeATL = 689 * (ioMem[NR22] & 7);

			sound2Index = 0;
			sound2On	= 1;
		}
		ioMem[address] = data;
		break;
	case NR30:
		data &= 0xe0;
		if (!(data & 0x80))
		{
			ioMem[NR52] &= 0xfb;
			sound3On	 = 0;
		}
		if (((data >> 6) & 1) != sound3Bank)
			memcpy(&ioMem[0x90], &sound3WaveRam[(((data >> 6) & 1) * 0x10) ^ 0x10],
			       0x10);
		sound3Bank	   = (data >> 6) & 1;
		sound3DataSize = (data >> 5) & 1;
		ioMem[address] = data;
		break;
	case NR31:
		sound3ATL	   = 172 * (256 - data);
		ioMem[address] = data;
		break;
	case NR32:
		data &= 0xe0;
		sound3OutputLevel  = (data >> 5) & 3;
		sound3ForcedOutput = (data >> 7) & 1;
		ioMem[address]	   = data;
		break;
	case NR33:
		freq = 2048 - (((int)(ioMem[NR34] & 7) << 8) | data);
		if (freq)
		{
			sound3Skip = SOUND_MAGIC_2 / freq;
		}
		else
			sound3Skip = 0;
		ioMem[address] = data;
		break;
	case NR34:
		data &= 0xc7;
		freq  = 2048 - (((data & 7) << 8) | (int)ioMem[NR33]);
		if (freq)
		{
			sound3Skip = SOUND_MAGIC_2 / freq;
		}
		else
		{
			sound3Skip = 0;
		}
		sound3Continue = data & 0x40;
		if ((data & 0x80) && (ioMem[NR30] & 0x80))
		{
			ioMem[NR52] |= 4;
			sound3ATL	 = 172 * (256 - ioMem[NR31]);
			sound3Index	 = 0;
			sound3On	 = 1;
		}
		ioMem[address] = data;
		break;
	case NR41:
		data &= 0x3f;
		sound4ATL	   = 172 * (64 - (data & 0x3f));
		ioMem[address] = data;
		break;
	case NR42:
		sound4EnvelopeUpDown	= data & 0x08;
		sound4EnvelopeATLReload = 689 * (data & 7);
		if ((data & 0xF8) == 0)
			sound4EnvelopeVolume = 0;
		ioMem[address] = data;
		break;
	case NR43:
		freq		 = soundFreqRatio[data & 7];
		sound4NSteps = data & 0x08;

		sound4Skip = freq * NOISE_MAGIC;

		sound4Clock = data >> 4;

		freq = freq / soundShiftClock[sound4Clock];

		sound4ShiftSkip = freq * NOISE_MAGIC;
		ioMem[address]	= data;
		break;
	case NR44:
		data &= 0xc0;
		sound4Continue = data & 0x40;
		if (data & 0x80)
		{
			ioMem[NR52] |= 8;
			sound4EnvelopeVolume = ioMem[NR42] >> 4;
			sound4EnvelopeUpDown = ioMem[NR42] & 0x08;
			sound4ATL = 172 * (64 - (ioMem[NR41] & 0x3f));
			sound4EnvelopeATLReload = sound4EnvelopeATL = 689 * (ioMem[NR42] & 7);

			sound4On = 1;

			sound4Index		 = 0;
			sound4ShiftIndex = 0;

			freq = soundFreqRatio[ioMem[NR43] & 7];

			sound4Skip = freq * NOISE_MAGIC;

			sound4NSteps = ioMem[NR43] & 0x08;

			freq = freq / soundShiftClock[ioMem[NR43] >> 4];

			sound4ShiftSkip = freq * NOISE_MAGIC;
			if (sound4NSteps)
				sound4ShiftRight = 0x7f;
			else
				sound4ShiftRight = 0x7fff;
		}
		ioMem[address] = data;
		break;
	case NR50:
		data &= 0x77;
		soundLevel1	   = data & 7;
		soundLevel2	   = (data >> 4) & 7;
		ioMem[address] = data;
		break;
	case NR51:
		soundBalance   = (data & soundEnableFlag);
		ioMem[address] = data;
		break;
	case NR52:
		data &= 0x80;
		data |= ioMem[NR52] & 15;
		soundMasterOn = data & 0x80;
		if (!(data & 0x80))
		{
			sound1On = 0;
			sound2On = 0;
			sound3On = 0;
			sound4On = 0;
		}
		ioMem[address] = data;
		break;
	case 0x90:
	case 0x91:
	case 0x92:
	case 0x93:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x97:
	case 0x98:
	case 0x99:
	case 0x9a:
	case 0x9b:
	case 0x9c:
	case 0x9d:
	case 0x9e:
	case 0x9f:
		sound3WaveRam[(sound3Bank * 0x10) ^ 0x10 + (address & 15)] = data;
		break;
	}
}

void soundEvent(u32 address, u16 data)
{
	switch (address)
	{
	case SGCNT0_H:
		data		&= 0xFF0F;
		soundControl = data & 0x770F;
		if (data & 0x0800)
		{
			soundDSFifoAWriteIndex = 0;
			soundDSFifoAIndex	   = 0;
			soundDSFifoACount	   = 0;
			soundDSAValue = 0;
			memset(soundDSFifoA, 0, 32);
		}
		soundDSAEnabled = (data & 0x0300) ? true : false;
		soundDSATimer	= (data & 0x0400) ? 1 : 0;
		if (data & 0x8000)
		{
			soundDSFifoBWriteIndex = 0;
			soundDSFifoBIndex	   = 0;
			soundDSFifoBCount	   = 0;
			soundDSBValue = 0;
			memset(soundDSFifoB, 0, 32);
		}
		soundDSBEnabled = (data & 0x3000) ? true : false;
		soundDSBTimer	= (data & 0x4000) ? 1 : 0;
		*((u16 *)&ioMem[address]) = data;
		break;
	case FIFOA_L:
	case FIFOA_H:
		soundDSFifoA[soundDSFifoAWriteIndex++] = data & 0xFF;
		soundDSFifoA[soundDSFifoAWriteIndex++] = data >> 8;
		soundDSFifoACount		 += 2;
		soundDSFifoAWriteIndex	 &= 31;
		*((u16 *)&ioMem[address]) = data;
		break;
	case FIFOB_L:
	case FIFOB_H:
		soundDSFifoB[soundDSFifoBWriteIndex++] = data & 0xFF;
		soundDSFifoB[soundDSFifoBWriteIndex++] = data >> 8;
		soundDSFifoBCount		 += 2;
		soundDSFifoBWriteIndex	 &= 31;
		*((u16 *)&ioMem[address]) = data;
		break;
	case 0x88:
		data &= 0xC3FF;
		*((u16 *)&ioMem[address]) = data;
		break;
	case 0x90:
	case 0x92:
	case 0x94:
	case 0x96:
	case 0x98:
	case 0x9a:
	case 0x9c:
	case 0x9e:
		*((u16 *)&sound3WaveRam[(sound3Bank * 0x10) ^ 0x10 + (address & 14)]) = data;
		*((u16 *)&ioMem[address]) = data;
		break;
	}
}

void soundChannel1()
{
	int vol = sound1EnvelopeVolume;

	int freq  = 0;
	int value = 0;

	if (sound1On && (sound1ATL || !sound1Continue))
	{
		sound1Index += soundQuality * sound1Skip;
		sound1Index &= 0x1fffffff;

		value = ((s8)sound1Wave[sound1Index >> 24]) * vol;
	}

	soundBuffer[0][soundIndex] = value;

	if (sound1On)
	{
		if (sound1ATL)
		{
			sound1ATL -= soundQuality;

			if (sound1ATL <= 0 && sound1Continue)
			{
				ioMem[NR52] &= 0xfe;
				sound1On	 = 0;
			}
		}

		if (sound1EnvelopeATL)
		{
			sound1EnvelopeATL -= soundQuality;

			if (sound1EnvelopeATL <= 0)
			{
				if (sound1EnvelopeUpDown)
				{
					if (sound1EnvelopeVolume < 15)
						sound1EnvelopeVolume++;
				}
				else
				{
					if (sound1EnvelopeVolume)
						sound1EnvelopeVolume--;
				}

				sound1EnvelopeATL += sound1EnvelopeATLReload;
			}
		}

		if (sound1SweepATL)
		{
			sound1SweepATL -= soundQuality;

			if (sound1SweepATL <= 0)
			{
				freq = (((int)(ioMem[NR14] & 7) << 8) | ioMem[NR13]);

				int updown = 1;

				if (sound1SweepUpDown)
					updown = -1;

				int newfreq = 0;
				if (sound1SweepSteps)
				{
					newfreq = freq + updown * freq / (1 << sound1SweepSteps);
					if (newfreq == freq)
						newfreq = 0;
				}
				else
					newfreq = freq;

				if (newfreq < 0)
				{
					sound1SweepATL += sound1SweepATLReload;
				}
				else if (newfreq > 2047)
				{
					sound1SweepATL = 0;
					sound1On	   = 0;
					ioMem[NR52]	  &= 0xfe;
				}
				else
				{
					sound1SweepATL += sound1SweepATLReload;
					sound1Skip		= SOUND_MAGIC / (2048 - newfreq);

					ioMem[NR13] = newfreq & 0xff;
					ioMem[NR14] = (ioMem[NR14] & 0xf8) | ((newfreq >> 8) & 7);
				}
			}
		}
	}
}

void soundChannel2()
{
	//  int freq = 0;
	int vol = sound2EnvelopeVolume;

	int value = 0;

	if (sound2On && (sound2ATL || !sound2Continue))
	{
		sound2Index += soundQuality * sound2Skip;
		sound2Index &= 0x1fffffff;

		value = ((s8)sound2Wave[sound2Index >> 24]) * vol;
	}

	soundBuffer[1][soundIndex] = value;

	if (sound2On)
	{
		if (sound2ATL)
		{
			sound2ATL -= soundQuality;

			if (sound2ATL <= 0 && sound2Continue)
			{
				ioMem[NR52] &= 0xfd;
				sound2On	 = 0;
			}
		}

		if (sound2EnvelopeATL)
		{
			sound2EnvelopeATL -= soundQuality;

			if (sound2EnvelopeATL <= 0)
			{
				if (sound2EnvelopeUpDown)
				{
					if (sound2EnvelopeVolume < 15)
						sound2EnvelopeVolume++;
				}
				else
				{
					if (sound2EnvelopeVolume)
						sound2EnvelopeVolume--;
				}
				sound2EnvelopeATL += sound2EnvelopeATLReload;
			}
		}
	}
}

void soundChannel3()
{
	int value = sound3Last;

	if (sound3On && (sound3ATL || !sound3Continue))
	{
		sound3Index += soundQuality * sound3Skip;
		if (sound3DataSize)
		{
			sound3Index &= 0x3fffffff;
			value		 = sound3WaveRam[sound3Index >> 25];
		}
		else
		{
			sound3Index &= 0x1fffffff;
			value		 = sound3WaveRam[sound3Bank * 0x10 + (sound3Index >> 25)];
		}

		if ((sound3Index & 0x01000000))
		{
			value &= 0x0f;
		}
		else
		{
			value >>= 4;
		}

		value -= 8;
		value *= 2;

		if (sound3ForcedOutput)
		{
			value = ((value >> 1) + value) >> 1;
		}
		else
		{
			switch (sound3OutputLevel)
			{
			case 0:
				value = 0;
				break;
			case 1:
				break;
			case 2:
				value = (value >> 1);
				break;
			case 3:
				value = (value >> 2);
				break;
			}
		}
		//value += 1;
		sound3Last = value;
	}

	soundBuffer[2][soundIndex] = value;

	if (sound3On)
	{
		if (sound3ATL)
		{
			sound3ATL -= soundQuality;

			if (sound3ATL <= 0 && sound3Continue)
			{
				ioMem[NR52] &= 0xfb;
				sound3On	 = 0;
			}
		}
	}
}

void soundChannel4()
{
	int vol = sound4EnvelopeVolume;

	int value = 0;

	if (sound4Clock <= 0x0c)
	{
		if (sound4On && (sound4ATL || !sound4Continue))
		{
	  #define NOISE_ONE_SAMP_SCALE  0x200000

			sound4Index		 += soundQuality * sound4Skip;
			sound4ShiftIndex += soundQuality * sound4ShiftSkip;

			if (sound4NSteps)
			{
				while (sound4ShiftIndex >= NOISE_ONE_SAMP_SCALE)
				{
					sound4ShiftRight = (((sound4ShiftRight << 6) ^
					                     (sound4ShiftRight << 5)) & 0x40) |
					                   (sound4ShiftRight >> 1);
					sound4ShiftIndex -= NOISE_ONE_SAMP_SCALE;
				}
			}
			else
			{
				while (sound4ShiftIndex >= NOISE_ONE_SAMP_SCALE)
				{
					sound4ShiftRight = (((sound4ShiftRight << 14) ^
					                     (sound4ShiftRight << 13)) & 0x4000) |
					                   (sound4ShiftRight >> 1);

					sound4ShiftIndex -= NOISE_ONE_SAMP_SCALE;
				}
			}

			sound4Index		 %= NOISE_ONE_SAMP_SCALE;
			sound4ShiftIndex %= NOISE_ONE_SAMP_SCALE;

			value = ((sound4ShiftRight & 1) * 2 - 1) * vol;
		}
		else
		{
			value = 0;
		}
	}

	soundBuffer[3][soundIndex] = value;

	if (sound4On)
	{
		if (sound4ATL)
		{
			sound4ATL -= soundQuality;

			if (sound4ATL <= 0 && sound4Continue)
			{
				ioMem[NR52] &= 0xfd;
				sound4On	 = 0;
			}
		}

		if (sound4EnvelopeATL)
		{
			sound4EnvelopeATL -= soundQuality;

			if (sound4EnvelopeATL <= 0)
			{
				if (sound4EnvelopeUpDown)
				{
					if (sound4EnvelopeVolume < 15)
						sound4EnvelopeVolume++;
				}
				else
				{
					if (sound4EnvelopeVolume)
						sound4EnvelopeVolume--;
				}
				sound4EnvelopeATL += sound4EnvelopeATLReload;
			}
		}
	}
}

void soundDirectSoundA()
{
	soundBuffer[4][soundIndex] = soundDSAValue;
}

void soundDirectSoundATimer()
{
	if (soundDSAEnabled)
	{
		if (soundDSFifoACount <= 16)
		{
			CPUCheckDMA(3, 2);
			if (soundDSFifoACount <= 16)
			{
				soundEvent(FIFOA_L, (u16)0);
				soundEvent(FIFOA_H, (u16)0);
				soundEvent(FIFOA_L, (u16)0);
				soundEvent(FIFOA_H, (u16)0);
				soundEvent(FIFOA_L, (u16)0);
				soundEvent(FIFOA_H, (u16)0);
				soundEvent(FIFOA_L, (u16)0);
				soundEvent(FIFOA_H, (u16)0);
			}
		}

		soundDSAValue	  = (soundDSFifoA[soundDSFifoAIndex]);
		soundDSFifoAIndex = (++soundDSFifoAIndex) & 31;
		soundDSFifoACount--;
	}
	else
		soundDSAValue = 0;
}

void soundDirectSoundB()
{
	soundBuffer[5][soundIndex] = soundDSBValue;
}

void soundDirectSoundBTimer()
{
	if (soundDSBEnabled)
	{
		if (soundDSFifoBCount <= 16)
		{
			CPUCheckDMA(3, 4);
			if (soundDSFifoBCount <= 16)
			{
				soundEvent(FIFOB_L, (u16)0);
				soundEvent(FIFOB_H, (u16)0);
				soundEvent(FIFOB_L, (u16)0);
				soundEvent(FIFOB_H, (u16)0);
				soundEvent(FIFOB_L, (u16)0);
				soundEvent(FIFOB_H, (u16)0);
				soundEvent(FIFOB_L, (u16)0);
				soundEvent(FIFOB_H, (u16)0);
			}
		}

		soundDSBValue	  = (soundDSFifoB[soundDSFifoBIndex]);
		soundDSFifoBIndex = (++soundDSFifoBIndex) & 31;
		soundDSFifoBCount--;
	}
	else
	{
		soundDSBValue = 0;
	}
}

void soundTimerOverflow(int timer)
{
	if (soundDSAEnabled && (soundDSATimer == timer))
	{
		soundDirectSoundATimer();
	}
	if (soundDSBEnabled && (soundDSBTimer == timer))
	{
		soundDirectSoundBTimer();
	}
}

void soundMix()
{
	soundBalance = (ioMem[NR51] & soundEnableFlag);

	int cgbRatio = ioMem[0x82] & 3;
	int dsaRatio = ioMem[0x82] & 4;
	int dsbRatio = ioMem[0x82] & 8;

	int cgbResL = 0;
	if (soundBalance & 16)
	{
		cgbResL += ((s8)soundBuffer[0][soundIndex]);
	}
	if (soundBalance & 32)
	{
		cgbResL += ((s8)soundBuffer[1][soundIndex]);
	}
	if (soundBalance & 64)
	{
		cgbResL += ((s8)soundBuffer[2][soundIndex]);
	}
	if (soundBalance & 128)
	{
		cgbResL += ((s8)soundBuffer[3][soundIndex]);
	}

	int cgbResR = 0;
	if (soundBalance & 1)
	{
		cgbResR += ((s8)soundBuffer[0][soundIndex]);
	}
	if (soundBalance & 2)
	{
		cgbResR += ((s8)soundBuffer[1][soundIndex]);
	}
	if (soundBalance & 4)
	{
		cgbResR += ((s8)soundBuffer[2][soundIndex]);
	}
	if (soundBalance & 8)
	{
		cgbResR += ((s8)soundBuffer[3][soundIndex]);
	}

	int dsResL = 0;
	if ((soundControl & 0x0200) && (soundEnableFlag & 0x100))
	{
		if (!dsaRatio)
			dsResL = ((s8)soundBuffer[4][soundIndex]) >> 1;
		else
			dsResL = ((s8)soundBuffer[4][soundIndex]);
	}
	if ((soundControl & 0x2000) && (soundEnableFlag & 0x200))
	{
		if (!dsbRatio)
			dsResL += ((s8)soundBuffer[5][soundIndex]) >> 1;
		else
			dsResL += ((s8)soundBuffer[5][soundIndex]);
	}

	int dsResR = 0;
	if ((soundControl & 0x0100) && (soundEnableFlag & 0x100))
	{
		if (!dsaRatio)
			dsResR = ((s8)soundBuffer[4][soundIndex]) >> 1;
		else
			dsResR = ((s8)soundBuffer[4][soundIndex]);
	}

	if ((soundControl & 0x1000) && (soundEnableFlag & 0x200))
	{
		if (!dsbRatio)
			dsResR += ((s8)soundBuffer[5][soundIndex]) >> 1;
		else
			dsResR += ((s8)soundBuffer[5][soundIndex]);
	}

	dsResL  *= 170;
	dsResR	*= 170;
	cgbResL *= 52 * soundLevel1;
	cgbResR *= 52 * soundLevel1;

	switch (cgbRatio)
	{
	case 0:
	case 3: // prohibited, but 25%
		cgbResL >>= 2;
		cgbResR >>= 2;
		break;
	case 1:
		cgbResL >>= 1;
		cgbResR >>= 1;
		break;
	case 2:
		break;
	}

	systemSoundMix(cgbResL + dsResL, cgbResR + dsResR);
}

void soundTick()
{
	if (systemSoundOn)
	{
		if (soundMasterOn && !stopState)
		{
			soundChannel1();
			soundChannel2();
			soundChannel3();
			soundChannel4();
			soundDirectSoundA();
			soundDirectSoundB();
			soundMix();
		}
		else
		{
			systemSoundMixSilence();
		}
		systemSoundNext();
	}
}

void soundReset()
{
	systemSoundMixReset();

	soundIndex		  = 0;
	soundPaused		  = 1;
	soundPlay		  = 0;
	USE_TICKS_AS	  = GBA_SOUNDTICK_AS;
	soundTickStep	  = soundQuality * USE_TICKS_AS;
	soundTicks		  = soundTickStep;
	//soundTicks		  = 0;
	soundNextPosition = 0;
	soundMasterOn	  = 1;
	soundLevel1		  = 7;
	soundLevel2		  = 7;

	sound1On = 0;
	sound1ATL = 0;
	sound1Skip = 0;
	sound1Index = 0;
	sound1Continue = 0;
	sound1EnvelopeVolume	= 0;
	sound1EnvelopeATL		= 0;
	sound1EnvelopeUpDown	= 0;
	sound1EnvelopeATLReload = 0;
	sound1SweepATL			= 0;
	sound1SweepATLReload	= 0;
	sound1SweepSteps		= 0;
	sound1SweepUpDown		= 0;
	sound1SweepStep			= 0;
	sound1Wave				= soundWavePattern[2];

	sound2On = 0;
	sound2ATL = 0;
	sound2Skip = 0;
	sound2Index = 0;
	sound2Continue = 0;
	sound2EnvelopeVolume	= 0;
	sound2EnvelopeATL		= 0;
	sound2EnvelopeUpDown	= 0;
	sound2EnvelopeATLReload = 0;
	sound2Wave				= soundWavePattern[2];

	sound3On = 0;
	sound3ATL = 0;
	sound3Skip		   = 0;
	sound3Index		   = 0;
	sound3Continue	   = 0;
	sound3OutputLevel  = 0;
	sound3Last		   = 0;
	sound3Bank		   = 0;
	sound3DataSize	   = 0;
	sound3ForcedOutput = 0;

	sound4On = 0;
	sound4Clock = 0;
	sound4ATL = 0;
	sound4Skip = 0;
	sound4Index = 0;
	sound4ShiftRight		= 0x7f;
	sound4NSteps			= 0;
	sound4CountDown			= 0;
	sound4Continue			= 0;
	sound4EnvelopeVolume	= 0;
	sound4EnvelopeATL		= 0;
	sound4EnvelopeUpDown	= 0;
	sound4EnvelopeATLReload = 0;

	soundControl = 0;

	sound1On = 0;
	sound2On = 0;
	sound3On = 0;
	sound4On = 0;

	int addr = 0x90;

	while (addr < 0xA0)
	{
		ioMem[addr++] = 0x00;
		ioMem[addr++] = 0xff;
	}

	addr = 0;
	while (addr < 0x20)
	{
		sound3WaveRam[addr++] = 0x00;
		sound3WaveRam[addr++] = 0xff;
	}
}

void soundSaveGame(gzFile gzFile)
{
	soundTicks_int32		= (int32) soundTicks;
	soundTickStep_int32		= (int32) soundTickStep;
	soundDSBEnabled_int32	= (int32) soundDSBEnabled;
	soundDSBValue_int32		= (int32) soundDSBValue;

	utilWriteData(gzFile, soundSaveStruct);
	utilWriteData(gzFile, soundSaveStructV2);

	utilGzWrite(gzFile, &soundQuality, sizeof(int32));
	//utilWriteData(gzFile, soundSaveStructV3);
}

void soundReadGame(gzFile gzFile, int version)
{
	int32 oldSoundPaused = soundPaused;
	int32 oldSoundEnableFlag = soundEnableFlag;

	utilReadData(gzFile, soundSaveStruct);
	soundPaused		 = oldSoundPaused;
	soundEnableFlag	 = oldSoundEnableFlag;
	soundBufferIndex = soundIndex * 2;

	if (version >= SAVE_GAME_VERSION_3)
	{
		utilReadData(gzFile, soundSaveStructV2);
	}
	else
	{
		sound3Bank		   = (ioMem[NR30] >> 6) & 1;
		sound3DataSize	   = (ioMem[NR30] >> 5) & 1;
		sound3ForcedOutput = (ioMem[NR32] >> 7) & 1;
		// nothing better to do here...
		memcpy(&sound3WaveRam[0x00], &ioMem[0x90], 0x10);
		memcpy(&sound3WaveRam[0x10], &ioMem[0x90], 0x10);
	}

	int quality = 1;
	utilGzRead(gzFile, &quality, sizeof(int32));
	systemSoundSetQuality(quality);

	sound1Wave = soundWavePattern[ioMem[NR11] >> 6];
	sound2Wave = soundWavePattern[ioMem[NR21] >> 6];

	//if(version >= SAVE_GAME_VERSION_14) {
	//  utilReadData(gzFile, soundSaveStructV3);
	//}
	//else {
	soundTicks		 = (soundtick_t) soundTicks_int32;
	soundTickStep	 = (soundtick_t) soundTickStep_int32;
	//}
	soundDSBEnabled	 = (u8) (soundDSBEnabled_int32 & 0xFF);
	soundDSBValue	 = (u8) (soundDSBValue_int32 & 0xFF);
}

