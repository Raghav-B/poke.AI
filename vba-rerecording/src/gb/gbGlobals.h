#ifndef VBA_GB_GLOBALS_H
#define VBA_GB_GLOBALS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Port.h"

extern int32 gbRomSizeMask;
extern int32 gbRomSize;
extern int32 gbRamSize;
extern int32 gbRamSizeMask;
extern int32 gbTAMA5ramSize;

extern u8 * gbRom;
extern u8 * gbRam;
extern u8 * gbVram;
extern u8 * gbWram;
extern u8 * gbMemory;
extern u16 *gbLineBuffer;
extern u8 * gbTAMA5ram;

extern u8 *gbMemoryMap[16];

extern const u32 gbFrameRateDividend;
extern const u32 gbFrameRateDivisor;
extern const double gbFrameRate;

extern const u32 gbPixBufferSize;

#ifdef USE_GB_CORE_V7
extern bool gbEchoRAMFixOn;
extern bool gbDMASpeedVersion;
#endif

static inline u8 gbReadMemoryQuick(u16 address)
{
#ifdef USE_GB_CORE_V7
	if (gbEchoRAMFixOn)
#endif
	{
		if (address >= 0xe000 && address < 0xfe00)
		{
			address -= 0x2000;
		}
	}
	return gbMemoryMap[address >> 12][address & 0xfff];
}

static inline void gbWriteMemoryQuick(u16 address, u8 value)
{
#ifdef USE_GB_CORE_V7
	if (gbEchoRAMFixOn)
#endif
	{
		if (address >= 0xe000 && address < 0xfe00)
		{
			address -= 0x2000;
		}
	}
	gbMemoryMap[address >> 12][address & 0xfff] = value;
}

static inline u8 gbReadMemoryQuick8(u16 addr)
{
	return gbReadMemoryQuick(addr);
}

static inline void gbWriteMemoryQuick8(u16 addr, u8 b)
{
	gbWriteMemoryQuick(addr, b);
}

static inline u16 gbReadMemoryQuick16(u16 addr)
{
	return (gbReadMemoryQuick(addr + 1) << 8) | gbReadMemoryQuick(addr);
}

static inline void gbWriteMemoryQuick16(u16 addr, u16 b)
{
	gbWriteMemoryQuick(addr, b & 0xff);
	gbWriteMemoryQuick(addr + 1, (b >> 8) & 0xff);
}

static inline u32 gbReadMemoryQuick32(u16 addr)
{
	return (gbReadMemoryQuick(addr + 3) << 24) |
		   (gbReadMemoryQuick(addr + 2) << 16) |
		   (gbReadMemoryQuick(addr + 1) << 8) |
		   gbReadMemoryQuick(addr);
}

static inline void gbWriteMemoryQuick32(u16 addr, u32 b)
{
	gbWriteMemoryQuick(addr, b & 0xff);
	gbWriteMemoryQuick(addr + 1, (b >> 8) & 0xff);
	gbWriteMemoryQuick(addr + 2, (b >> 16) & 0xff);
	gbWriteMemoryQuick(addr + 1, (b >> 24) & 0xff);
}

static inline u8 gbReadROMQuick(u32 address)
{
	return gbRom[address];
}

static inline u8 gbReadROMQuick8(u32 addr)
{
	return gbReadROMQuick(addr & gbRomSizeMask);
}

static inline u8 gbReadROMQuick16(u32 addr)
{
	return (gbReadROMQuick(addr+1 & gbRomSizeMask) << 8) | gbReadROMQuick(addr & gbRomSizeMask);
}

static inline u8 gbReadROMQuick32(u32 addr)
{
	return (gbReadROMQuick(addr+3 & gbRomSizeMask) << 24) |
		(gbReadROMQuick(addr+2 & gbRomSizeMask) << 16) |
		(gbReadROMQuick(addr+1 & gbRomSizeMask) << 8) |
		gbReadROMQuick(addr & gbRomSizeMask);
}

extern u16	 gbColorFilter[32768];
extern int32 gbColorOption;
extern int32 gbPaletteOption;
extern int32 gbEmulatorType;
extern int32 gbHardware;
extern int32 gbBorderOn;
extern int32 gbBorderAutomatic;
extern int32 gbCgbMode;
extern int32 gbSgbMode;
extern int32 gbWindowLine;
extern int32 gbSpeed;
extern u8	 gbBgp[4];
extern u8	 gbObp0[4];
extern u8	 gbObp1[4];
extern u16	 gbPalette[128];

#ifdef USE_GB_CORE_V7
#else
extern bool	 genericflashcardEnable;
extern bool	 gbScreenOn;
extern u8	 oldRegister_WY;

// gbSCXLine is used for the emulation (bug) of the SX change
// found in the Artic Zone game.
extern u8 gbSCXLine[300];
extern u8 gbSCYLine[300];

// gbBgpLine is used for the emulation of the
// Prehistorik Man's title screen scroller.
extern u8 gbBgpLine[300];
extern u8 gbObp0Line [300];
extern u8 gbObp1Line [300];

// gbSpritesTicks is used for the emulation of Parodius' Laser Beam.
extern u8 gbSpritesTicks[300];
#endif

extern u8 register_LCDC;
extern u8 register_LY;
extern u8 register_SCY;
extern u8 register_SCX;
extern u8 register_WY;
extern u8 register_WX;
extern u8 register_VBK;

extern int32 gbBorderLineSkip;
extern int32 gbBorderRowSkip;
extern int32 gbBorderColumnSkip;
extern int32 gbDmaTicks;

extern void gbRenderLine();

#ifdef USE_GB_CORE_V7
extern void gbDrawSprites();
#else
extern void gbDrawSprites(bool);
#endif

extern u8 (*gbSerialFunction)(u8);

#endif // VBA_GB_GLOBALS_H
