#include <cstring>

#include "../../common/System.h"
#include "../../common/SystemGlobals.h"
#include "../../common/Util.h"
//#include "../../Blip_Buffer.h"
#include "../gbGlobals.h"
#include "../gbSound.h"

#define SOUND_MAGIC   0x60000000
#define SOUND_MAGIC_2 0x30000000
#define NOISE_MAGIC   (2097152.0 / 44100.0)

extern u8 soundWavePattern[4][32];

extern int32 soundLevel1;
extern int32 soundLevel2;
extern int32 soundBalance;
extern int32 soundMasterOn;
int32		 soundVIN = 0;
extern int32 soundDebug;

extern int32 sound1On;
extern int32 sound1ATL;
extern int32 sound1Skip;
extern int32 sound1Index;
extern int32 sound1Continue;
extern int32 sound1EnvelopeVolume;
extern int32 sound1EnvelopeATL;
extern int32 sound1EnvelopeUpDown;
extern int32 sound1EnvelopeATLReload;
extern int32 sound1SweepATL;
extern int32 sound1SweepATLReload;
extern int32 sound1SweepSteps;
extern int32 sound1SweepUpDown;
extern int32 sound1SweepStep;
extern u8 *	 sound1Wave;

extern int32 sound2On;
extern int32 sound2ATL;
extern int32 sound2Skip;
extern int32 sound2Index;
extern int32 sound2Continue;
extern int32 sound2EnvelopeVolume;
extern int32 sound2EnvelopeATL;
extern int32 sound2EnvelopeUpDown;
extern int32 sound2EnvelopeATLReload;
extern u8 *	 sound2Wave;

extern int32 sound3On;
extern int32 sound3ATL;
extern int32 sound3Skip;
extern int32 sound3Index;
extern int32 sound3Continue;
extern int32 sound3OutputLevel;
extern int32 sound3Last;

extern int32 sound4On;
extern int32 sound4Clock;
extern int32 sound4ATL;
extern int32 sound4Skip;
extern int32 sound4Index;
extern int32 sound4ShiftRight;
extern int32 sound4ShiftSkip;
extern int32 sound4ShiftIndex;
extern int32 sound4NSteps;
extern int32 sound4CountDown;
extern int32 sound4Continue;
extern int32 sound4EnvelopeVolume;
extern int32 sound4EnvelopeATL;
extern int32 sound4EnvelopeUpDown;
extern int32 sound4EnvelopeATLReload;

extern int32 soundFreqRatio[8];
extern int32 soundShiftClock[16];

bool8 gbDigitalSound = false;

void gbSoundEvent(register u16 address, register int data)
{
	int freq = 0;

	gbMemory[address] = data;

#ifndef FINAL_VERSION
	if (soundDebug)
	{
		// don't translate. debug only
		log("Sound event: %08lx %02x\n", address, data);
	}
#endif
	switch (address)
	{
	case NR10:
		sound1SweepATL	  = sound1SweepATLReload = 344 * ((data >> 4) & 7);
		sound1SweepSteps  = data & 7;
		sound1SweepUpDown = data & 0x08;
		sound1SweepStep	  = 0;
		break;
	case NR11:
		sound1Wave = soundWavePattern[data >> 6];
		sound1ATL  = 172 * (64 - (data & 0x3f));
		break;
	case NR12:
		sound1EnvelopeVolume	= data >> 4;
		sound1EnvelopeUpDown	= data & 0x08;
		sound1EnvelopeATLReload = sound1EnvelopeATL = 689 * (data & 7);
		break;
	case NR13:
		freq	  = (((int)(gbMemory[NR14] & 7)) << 8) | data;
		sound1ATL = 172 * (64 - (gbMemory[NR11] & 0x3f));
		freq	  = 2048 - freq;
		if (freq)
		{
			sound1Skip = SOUND_MAGIC / freq;
		}
		else
			sound1Skip = 0;
		break;
	case NR14:
		freq = (((int)(data & 7) << 8) | gbMemory[NR13]);
		freq = 2048 - freq;
		sound1ATL	   = 172 * (64 - (gbMemory[NR11] & 0x3f));
		sound1Continue = data & 0x40;
		if (freq)
		{
			sound1Skip = SOUND_MAGIC / freq;
		}
		else
			sound1Skip = 0;
		if (data & 0x80)
		{
			gbMemory[NR52]		|= 1;
			sound1EnvelopeVolume = gbMemory[NR12] >> 4;
			sound1EnvelopeUpDown = gbMemory[NR12] & 0x08;
			sound1ATL = 172 * (64 - (gbMemory[NR11] & 0x3f));
			sound1EnvelopeATLReload = sound1EnvelopeATL = 689 * (gbMemory[NR12] & 7);
			sound1SweepATL			= sound1SweepATLReload = 344 * ((gbMemory[NR10] >> 4) & 7);
			sound1SweepSteps		= gbMemory[NR10] & 7;
			sound1SweepUpDown		= gbMemory[NR10] & 0x08;
			sound1SweepStep			= 0;

			sound1Index = 0;
			sound1On	= 1;
		}
		break;
	case NR21:
		sound2Wave = soundWavePattern[data >> 6];
		sound2ATL  = 172 * (64 - (data & 0x3f));
		break;
	case NR22:
		sound2EnvelopeVolume	= data >> 4;
		sound2EnvelopeUpDown	= data & 0x08;
		sound2EnvelopeATLReload = sound2EnvelopeATL = 689 * (data & 7);
		break;
	case NR23:
		freq	  = (((int)(gbMemory[NR24] & 7)) << 8) | data;
		sound2ATL = 172 * (64 - (gbMemory[NR21] & 0x3f));
		freq	  = 2048 - freq;
		if (freq)
		{
			sound2Skip = SOUND_MAGIC / freq;
		}
		else
			sound2Skip = 0;
		break;
	case NR24:
		freq = (((int)(data & 7) << 8) | gbMemory[NR23]);
		freq = 2048 - freq;
		sound2ATL	   = 172 * (64 - (gbMemory[NR21] & 0x3f));
		sound2Continue = data & 0x40;
		if (freq)
		{
			sound2Skip = SOUND_MAGIC / freq;
		}
		else
			sound2Skip = 0;
		if (data & 0x80)
		{
			gbMemory[NR52]		|= 2;
			sound2EnvelopeVolume = gbMemory[NR22] >> 4;
			sound2EnvelopeUpDown = gbMemory[NR22] & 0x08;
			sound2ATL = 172 * (64 - (gbMemory[NR21] & 0x3f));
			sound2EnvelopeATLReload = sound2EnvelopeATL = 689 * (gbMemory[NR22] & 7);

			sound2Index = 0;
			sound2On	= 1;
		}
		break;
	case NR30:
		if (!(data & 0x80))
		{
			gbMemory[NR52] &= 0xfb;
			sound3On		= 0;
		}
		break;
	case NR31:
		sound3ATL = 172 * (256 - data);
		break;
	case NR32:
		sound3OutputLevel = (data >> 5) & 3;
		break;
	case NR33:
		freq = 2048 - (((int)(gbMemory[NR34] & 7) << 8) | data);
		if (freq)
		{
			sound3Skip = SOUND_MAGIC_2 / freq;
		}
		else
			sound3Skip = 0;
		break;
	case NR34:
		freq = 2048 - (((data & 7) << 8) | (int)gbMemory[NR33]);
		if (freq)
		{
			sound3Skip = SOUND_MAGIC_2 / freq;
		}
		else
		{
			sound3Skip = 0;
		}
		sound3Continue = data & 0x40;
		if ((data & 0x80) && (gbMemory[NR30] & 0x80))
		{
			gbMemory[NR52] |= 4;
			sound3ATL		= 172 * (256 - gbMemory[NR31]);
			sound3Index		= 0;
			sound3On		= 1;
		}
		break;
	case NR41:
		sound4ATL = 172 * (64 - (data & 0x3f));
		break;
	case NR42:
		sound4EnvelopeVolume	= data >> 4;
		sound4EnvelopeUpDown	= data & 0x08;
		sound4EnvelopeATLReload = sound4EnvelopeATL = 689 * (data & 7);
		break;
	case NR43:
		freq		 = soundFreqRatio[data & 7];
		sound4NSteps = data & 0x08;

		sound4Skip = freq * NOISE_MAGIC;

		sound4Clock = data >> 4;

		freq = freq / soundShiftClock[sound4Clock];

		sound4ShiftSkip = freq * NOISE_MAGIC;

		break;
	case NR44:
		sound4Continue = data & 0x40;
		if (data & 0x80)
		{
			gbMemory[NR52]		|= 8;
			sound4EnvelopeVolume = gbMemory[NR42] >> 4;
			sound4EnvelopeUpDown = gbMemory[NR42] & 0x08;
			sound4ATL = 172 * (64 - (gbMemory[NR41] & 0x3f));
			sound4EnvelopeATLReload = sound4EnvelopeATL = 689 * (gbMemory[NR42] & 7);

			sound4On = 1;

			sound4Index		 = 0;
			sound4ShiftIndex = 0;

			freq = soundFreqRatio[gbMemory[NR43] & 7];

			sound4Skip = freq * NOISE_MAGIC;

			sound4NSteps = gbMemory[NR43] & 0x08;

			freq = freq / soundShiftClock[gbMemory[NR43] >> 4];

			sound4ShiftSkip = freq * NOISE_MAGIC;
			if (sound4NSteps)
				sound4ShiftRight = 0x7f;
			else
				sound4ShiftRight = 0x7fff;
		}
		break;
	case NR50:
		soundVIN	= data & 0x88;
		soundLevel1 = data & 7;
		soundLevel2 = (data >> 4) & 7;
		break;
	case NR51:
		soundBalance	  = (data & soundEnableFlag);
		gbMemory[address] = data;
		break;
	case NR52:
		soundMasterOn = data & 0x80;
		if (!(data & 0x80))
		{
			sound1On = 0;
			sound2On = 0;
			sound3On = 0;
			sound4On = 0;
		}
		break;
	}

	gbDigitalSound = true;

	if (sound1On && sound1EnvelopeVolume != 0)
		gbDigitalSound = false;
	if (sound2On && sound2EnvelopeVolume != 0)
		gbDigitalSound = false;
	if (sound3On && sound3OutputLevel != 0)
		gbDigitalSound = false;
	if (sound4On && sound4EnvelopeVolume != 0)
		gbDigitalSound = false;
}

void gbSoundChannel1()
{
	int vol = sound1EnvelopeVolume;

	int freq = 0;

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
				gbMemory[NR52] &= 0xfe;
				sound1On		= 0;
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
				freq = (((int)(gbMemory[NR14] & 7) << 8) | gbMemory[NR13]);

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
					sound1SweepATL	= 0;
					sound1On		= 0;
					gbMemory[NR52] &= 0xfe;
				}
				else
				{
					sound1SweepATL += sound1SweepATLReload;
					sound1Skip		= SOUND_MAGIC / (2048 - newfreq);

					gbMemory[NR13] = newfreq & 0xff;
					gbMemory[NR14] = (gbMemory[NR14] & 0xf8) | ((newfreq >> 8) & 7);
				}
			}
		}
	}
}

void gbSoundChannel2()
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
				gbMemory[NR52] &= 0xfd;
				sound2On		= 0;
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

void gbSoundChannel3()
{
	int value = sound3Last;

	if (sound3On && (sound3ATL || !sound3Continue))
	{
		sound3Index += soundQuality * sound3Skip;
		sound3Index &= 0x1fffffff;

		value = gbMemory[0xff30 + (sound3Index >> 25)];

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
				gbMemory[NR52] &= 0xfb;
				sound3On		= 0;
			}
		}
	}
}

void gbSoundChannel4()
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

			sound4Index		 &= (NOISE_ONE_SAMP_SCALE - 1);
			sound4ShiftIndex &= (NOISE_ONE_SAMP_SCALE - 1);
#undef NOISE_ONE_SAMP_SCALE
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
				gbMemory[NR52] &= 0xfd;
				sound4On		= 0;
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

void gbSoundMix()
{
	if (gbMemory)
		soundBalance = (gbMemory[NR51] & soundEnableFlag);

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

	if (gbDigitalSound)
	{
		cgbResL = soundLevel1 * 256;
		cgbResR = soundLevel2 * 256;
	}
	else
	{
		cgbResL *= soundLevel1 * 60;
		cgbResR *= soundLevel2 * 60;
	}

	systemSoundMix(cgbResL, cgbResR);
}

void gbSoundTick()
{
	if (systemSoundOn)
	{
		if (soundMasterOn)
		{
			gbSoundChannel1();
			gbSoundChannel2();
			gbSoundChannel3();
			gbSoundChannel4();

			gbSoundMix();
		}
		else
		{
			systemSoundMixSilence();
		}
		systemSoundNext();
	}
}

void gbSoundReset()
{
	systemSoundMixReset();

	soundIndex		  = 0;
	soundPaused		  = 1;
	soundPlay		  = 0;
	USE_TICKS_AS	  = GB_SOUNDTICKS_AS;
	soundTickStep	  = soundQuality * USE_TICKS_AS;
//  soundTicks = soundTickStep;
	soundTicks		  = 0;
	soundNextPosition = 0;
	soundMasterOn	  = 1;
	soundLevel1		  = 7;
	soundLevel2		  = 7;
	soundVIN = 0;

	sound1On	   = 0;
	sound1ATL	   = 0;
	sound1Skip	   = 0;
	sound1Index	   = 0;
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

	sound2On	   = 0;
	sound2ATL	   = 0;
	sound2Skip	   = 0;
	sound2Index	   = 0;
	sound2Continue = 0;
	sound2EnvelopeVolume	= 0;
	sound2EnvelopeATL		= 0;
	sound2EnvelopeUpDown	= 0;
	sound2EnvelopeATLReload = 0;
	sound2Wave				= soundWavePattern[2];

	sound3On = 0;
	sound3ATL		  = 0;
	sound3Skip		  = 0;
	sound3Index		  = 0;
	sound3Continue	  = 0;
	sound3OutputLevel = 0;

	sound4On	= 0;
	sound4Clock = 0;
	sound4ATL	= 0;
	sound4Skip	= 0;
	sound4Index = 0;
	sound4ShiftRight		= 0x7f;
	sound4NSteps			= 0;
	sound4CountDown			= 0;
	sound4Continue			= 0;
	sound4EnvelopeVolume	= 0;
	sound4EnvelopeATL		= 0;
	sound4EnvelopeUpDown	= 0;
	sound4EnvelopeATLReload = 0;

	// don't translate
	if (soundDebug)
	{
		log("*** Sound Init ***\n");
	}

	gbSoundEvent(0xff10, 0x80);
	gbSoundEvent(0xff11, 0xbf);
	gbSoundEvent(0xff12, 0xf3);
	gbSoundEvent(0xff14, 0xbf);
	gbSoundEvent(0xff16, 0x3f);
	gbSoundEvent(0xff17, 0x00);
	gbSoundEvent(0xff19, 0xbf);

	gbSoundEvent(0xff1a, 0x7f);
	gbSoundEvent(0xff1b, 0xff);
	gbSoundEvent(0xff1c, 0xbf);
	gbSoundEvent(0xff1e, 0xbf);

	gbSoundEvent(0xff20, 0xff);
	gbSoundEvent(0xff21, 0x00);
	gbSoundEvent(0xff22, 0x00);
	gbSoundEvent(0xff23, 0xbf);
	gbSoundEvent(0xff24, 0x77);
	gbSoundEvent(0xff25, 0xf3);

	gbSoundEvent(0xff26, 0xf0);

	// don't translate
	if (soundDebug)
	{
		log("*** Sound Init Complete ***\n");
	}

	sound1On = 0;
	sound2On = 0;
	sound3On = 0;
	sound4On = 0;

	int addr = 0xff30;

	while (addr < 0xff40)
	{
		gbMemory[addr++] = 0x00;
		gbMemory[addr++] = 0xff;
	}
}

// dummy
static int32 soundTicks_int32;
static int32 soundTickStep_int32;
variable_desc gbSoundSaveStruct[] = {
	{ &soundPaused,				sizeof(int32) },
	{ &soundPlay,				sizeof(int32) },
	{ &soundTicks_int32,		sizeof(int32) },
	{ &soundTickStep_int32,		sizeof(int32) },
	{ &soundLevel1,				sizeof(int32) },
	{ &soundLevel2,				sizeof(int32) },
	{ &soundBalance,			sizeof(int32) },
	{ &soundMasterOn,			sizeof(int32) },
	{ &soundIndex,				sizeof(int32) },
	{ &soundVIN,				sizeof(int32) },
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
	{ NULL,						0			  }
};

//variable_desc gbSoundSaveStructV2[] = {
//  { &soundTicks, sizeof(soundtick_t) },
//  { &soundTickStep, sizeof(soundtick_t) },
//  { &USE_TICKS_AS, sizeof(soundtick_t) },
//  { NULL, 0 }
//};

void gbSoundSaveGame(gzFile gzFile)
{
	soundTicks_int32	= (int32) soundTicks;
	soundTickStep_int32 = (int32) soundTickStep;

	utilWriteData(gzFile, gbSoundSaveStruct);

	utilGzWrite(gzFile, soundBuffer, 4 * 735);
	utilGzWrite(gzFile, soundFinalWave, 2 * 735);
	utilGzWrite(gzFile, &soundQuality, sizeof(int32));

	//utilWriteData(gzFile, gbSoundSaveStructV2);
}

void gbSoundReadGame(int version, gzFile gzFile)
{
	int32 oldSoundPaused = soundPaused;
	int32 oldSoundEnableFlag = soundEnableFlag;

	utilReadData(gzFile, gbSoundSaveStruct);
	soundPaused = oldSoundPaused;
	soundEnableFlag = oldSoundEnableFlag;
	soundBufferIndex = soundIndex * 2;

	utilGzRead(gzFile, soundBuffer, 4 * 735);
	utilGzRead(gzFile, soundFinalWave, 2 * 735);

	// note: this resets the value of soundIndex to 0
	if (version >= 7)
	{
		int quality = 1;
		utilGzRead(gzFile, &quality, sizeof(int32));
		systemSoundSetQuality(quality);
	}
	else
	{
		soundQuality = -1;
		systemSoundSetQuality(1);
	}

	sound1Wave = soundWavePattern[gbMemory[NR11] >> 6];
	sound2Wave = soundWavePattern[gbMemory[NR21] >> 6];

	//if(version >= 14) {
	//  utilReadData(gzFile, gbSoundSaveStructV2);
	//}
	//else {
	soundTicks	  = (int32) soundTicks_int32;
	soundTickStep = (int32) soundTickStep_int32;
	//}
}

