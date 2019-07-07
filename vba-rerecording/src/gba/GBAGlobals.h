#ifndef VBA_GBA_GLOBALS_H
#define VBA_GBA_GLOBALS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Port.h"

#define VERBOSE_SWI                  1
#define VERBOSE_UNALIGNED_MEMORY     2
#define VERBOSE_ILLEGAL_WRITE        4
#define VERBOSE_ILLEGAL_READ         8
#define VERBOSE_DMA0                16
#define VERBOSE_DMA1                32
#define VERBOSE_DMA2                64
#define VERBOSE_DMA3               128
#define VERBOSE_UNDEFINED          256
#define VERBOSE_AGBPRINT           512
#define VERBOSE_SOUNDOUTPUT       1024

#ifdef BKPT_SUPPORT
extern int	oldreg[18];
extern char oldbuffer[10];
extern void (*dbgSignal)(int, int);
extern void (*dbgOutput)(const char *, u32);
extern bool debugger_last;
#endif

// moved from GBA.h
typedef union
{
	struct
	{
#ifdef WORDS_BIGENDIAN
		u8 B3;
		u8 B2;
		u8 B1;
		u8 B0;
#else
		u8 B0;
		u8 B1;
		u8 B2;
		u8 B3;
#endif
	} B;
	struct
	{
#ifdef WORDS_BIGENDIAN
		u16 W1;
		u16 W0;
#else
		u16 W0;
		u16 W1;
#endif
	} W;
#ifdef WORDS_BIGENDIAN
	volatile u32 I;
#else
	u32 I;
#endif
} reg_pair;

// internal...
extern reg_pair reg[45];
extern u8		biosProtected[4];
extern bool8	ioReadable[0x400];
extern bool8	N_FLAG;
extern bool8	C_FLAG;
extern bool8	Z_FLAG;
extern bool8	V_FLAG;
extern bool8	armState;
extern bool8	armIrqEnable;
extern u32		armNextPC;
extern int32	armMode;
extern int32	saveType;
extern bool8	speedHack;

#ifdef USE_GBA_CORE_V7
extern bool		sramInitFix;
#endif

#ifndef FINAL_VERSION
extern u32		armStopAddr;
#endif

extern u8 *rom;
extern u8 *internalRAM;
extern u8 *workRAM;
extern u8 *paletteRAM;
extern u8 *vram;
extern u8 *oam;
extern u8 *ioMem;

extern u16 DISPCNT;
extern u16 DISPSTAT;
extern u16 VCOUNT;
extern u16 BG0CNT;
extern u16 BG1CNT;
extern u16 BG2CNT;
extern u16 BG3CNT;
extern u16 BG0HOFS;
extern u16 BG0VOFS;
extern u16 BG1HOFS;
extern u16 BG1VOFS;
extern u16 BG2HOFS;
extern u16 BG2VOFS;
extern u16 BG3HOFS;
extern u16 BG3VOFS;
extern u16 BG2PA;
extern u16 BG2PB;
extern u16 BG2PC;
extern u16 BG2PD;
extern u16 BG2X_L;
extern u16 BG2X_H;
extern u16 BG2Y_L;
extern u16 BG2Y_H;
extern u16 BG3PA;
extern u16 BG3PB;
extern u16 BG3PC;
extern u16 BG3PD;
extern u16 BG3X_L;
extern u16 BG3X_H;
extern u16 BG3Y_L;
extern u16 BG3Y_H;
extern u16 WIN0H;
extern u16 WIN1H;
extern u16 WIN0V;
extern u16 WIN1V;
extern u16 WININ;
extern u16 WINOUT;
extern u16 MOSAIC;
extern u16 BLDMOD;
extern u16 COLEV;
extern u16 COLY;
extern u16 DM0SAD_L;
extern u16 DM0SAD_H;
extern u16 DM0DAD_L;
extern u16 DM0DAD_H;
extern u16 DM0CNT_L;
extern u16 DM0CNT_H;
extern u16 DM1SAD_L;
extern u16 DM1SAD_H;
extern u16 DM1DAD_L;
extern u16 DM1DAD_H;
extern u16 DM1CNT_L;
extern u16 DM1CNT_H;
extern u16 DM2SAD_L;
extern u16 DM2SAD_H;
extern u16 DM2DAD_L;
extern u16 DM2DAD_H;
extern u16 DM2CNT_L;
extern u16 DM2CNT_H;
extern u16 DM3SAD_L;
extern u16 DM3SAD_H;
extern u16 DM3DAD_L;
extern u16 DM3DAD_H;
extern u16 DM3CNT_L;
extern u16 DM3CNT_H;
extern u16 TM0D;
extern u16 TM0CNT;
extern u16 TM1D;
extern u16 TM1CNT;
extern u16 TM2D;
extern u16 TM2CNT;
extern u16 TM3D;
extern u16 TM3CNT;
extern u16 P1;
extern u16 IE;
extern u16 IF;
extern u16 IME;

extern const u32 frameRateDividend;
extern const u32 frameRateDivisor;
extern const double frameRate;

extern const u32 pixBufferSize;

#endif // VBA_GBA_GLOBALS_H
