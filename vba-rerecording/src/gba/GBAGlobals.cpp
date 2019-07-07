#include "GBAGlobals.h"

#ifdef BKPT_SUPPORT
int	 oldreg[18];
char oldbuffer[10];
#endif

// internals
reg_pair reg[45];
bool8	 ioReadable[0x400];
bool8	 N_FLAG		  = 0;
bool8	 C_FLAG		  = 0;
bool8	 Z_FLAG		  = 0;
bool8	 V_FLAG		  = 0;
bool8	 armState	  = true;
bool8	 armIrqEnable = true;
u32		 armNextPC	  = 0x00000000;
int32	 armMode	  = 0x1f;
int32	 saveType	  = 0;
bool8	 speedHack	  = false;

#ifdef USE_GBA_CORE_V7
bool	 sramInitFix  = true;
#endif

#ifndef FINAL_VERSION
u32		 armStopAddr  = 0x08000568;
#endif

u8 *rom			= NULL;
u8 *internalRAM = NULL;
u8 *workRAM		= NULL;
u8 *paletteRAM	= NULL;
u8 *vram		= NULL;
u8 *oam			= NULL;
u8 *ioMem		= NULL;

u16 DISPCNT	 = 0x0080;
u16 DISPSTAT = 0x0000;
u16 VCOUNT	 = 0x0000;
u16 BG0CNT	 = 0x0000;
u16 BG1CNT	 = 0x0000;
u16 BG2CNT	 = 0x0000;
u16 BG3CNT	 = 0x0000;
u16 BG0HOFS	 = 0x0000;
u16 BG0VOFS	 = 0x0000;
u16 BG1HOFS	 = 0x0000;
u16 BG1VOFS	 = 0x0000;
u16 BG2HOFS	 = 0x0000;
u16 BG2VOFS	 = 0x0000;
u16 BG3HOFS	 = 0x0000;
u16 BG3VOFS	 = 0x0000;
u16 BG2PA	 = 0x0100;
u16 BG2PB	 = 0x0000;
u16 BG2PC	 = 0x0000;
u16 BG2PD	 = 0x0100;
u16 BG2X_L	 = 0x0000;
u16 BG2X_H	 = 0x0000;
u16 BG2Y_L	 = 0x0000;
u16 BG2Y_H	 = 0x0000;
u16 BG3PA	 = 0x0100;
u16 BG3PB	 = 0x0000;
u16 BG3PC	 = 0x0000;
u16 BG3PD	 = 0x0100;
u16 BG3X_L	 = 0x0000;
u16 BG3X_H	 = 0x0000;
u16 BG3Y_L	 = 0x0000;
u16 BG3Y_H	 = 0x0000;
u16 WIN0H	 = 0x0000;
u16 WIN1H	 = 0x0000;
u16 WIN0V	 = 0x0000;
u16 WIN1V	 = 0x0000;
u16 WININ	 = 0x0000;
u16 WINOUT	 = 0x0000;
u16 MOSAIC	 = 0x0000;
u16 BLDMOD	 = 0x0000;
u16 COLEV	 = 0x0000;
u16 COLY	 = 0x0000;
u16 DM0SAD_L = 0x0000;
u16 DM0SAD_H = 0x0000;
u16 DM0DAD_L = 0x0000;
u16 DM0DAD_H = 0x0000;
u16 DM0CNT_L = 0x0000;
u16 DM0CNT_H = 0x0000;
u16 DM1SAD_L = 0x0000;
u16 DM1SAD_H = 0x0000;
u16 DM1DAD_L = 0x0000;
u16 DM1DAD_H = 0x0000;
u16 DM1CNT_L = 0x0000;
u16 DM1CNT_H = 0x0000;
u16 DM2SAD_L = 0x0000;
u16 DM2SAD_H = 0x0000;
u16 DM2DAD_L = 0x0000;
u16 DM2DAD_H = 0x0000;
u16 DM2CNT_L = 0x0000;
u16 DM2CNT_H = 0x0000;
u16 DM3SAD_L = 0x0000;
u16 DM3SAD_H = 0x0000;
u16 DM3DAD_L = 0x0000;
u16 DM3DAD_H = 0x0000;
u16 DM3CNT_L = 0x0000;
u16 DM3CNT_H = 0x0000;
u16 TM0D	 = 0x0000;
u16 TM0CNT	 = 0x0000;
u16 TM1D	 = 0x0000;
u16 TM1CNT	 = 0x0000;
u16 TM2D	 = 0x0000;
u16 TM2CNT	 = 0x0000;
u16 TM3D	 = 0x0000;
u16 TM3CNT	 = 0x0000;
u16 P1		 = 0xFFFF;
u16 IE		 = 0x0000;
u16 IF		 = 0x0000;
u16 IME		 = 0x0000;

// exact FPS of GBA = 16777216 / 280896 = 59.727500569605832763727500569606...
extern const u32 frameRateDividend = 262144; // 16777216;
extern const u32 frameRateDivisor = 4389;    // 280896;
extern const double frameRate = double(frameRateDividend) / double(frameRateDivisor);

extern const u32 pixBufferSize = 4 * 241 * 162;
