#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

#include "../../Port.h"
#include "../../NLS.h"
#include "../GB.h"
#include "../gbCheats.h"
#include "../gbGlobals.h"
#include "../gbMemory.h"
#include "../gbSGB.h"
#include "../gbSound.h"
#include "../../common/Util.h"
#include "../../common/System.h"
#include "../../common/SystemGlobals.h"
#include "../../common/movie.h"
#include "../../common/vbalua.h"

// FIXME: constant (GB) or boolean (GBA)?!
enum
{
	C_FLAG = 0x10,
	H_FLAG = 0x20,
	N_FLAG = 0x40,
	Z_FLAG = 0x80
};

bool gbUpdateSizes();

// debugging
bool memorydebug = false;
char gbBuffer[2048];

extern u16 gbLineMix[160];

// mappers
void (*mapper)(u16, u8)	   = NULL;
void (*mapperRAM)(u16, u8) = NULL;
u8	 (*mapperReadRAM)(u16) = NULL;

// registers
gbRegister PC;
gbRegister SP;
gbRegister AF;
gbRegister BC;
gbRegister DE;
gbRegister HL;
u16		   IFF = 0;
// 0xff04
u8 register_DIV = 0;
// 0xff05
u8 register_TIMA = 0;
// 0xff06
u8 register_TMA = 0;
// 0xff07
u8 register_TAC = 0;
// 0xff0f
u8 register_IF = 0;
// 0xff40
u8 register_LCDC = 0;
// 0xff41
u8 register_STAT = 0;
// 0xff42
u8 register_SCY = 0;
// 0xff43
u8 register_SCX = 0;
// 0xff44
u8 register_LY = 0;
// 0xff45
u8 register_LYC = 0;
// 0xff46
u8 register_DMA = 0;
// 0xff4a
u8 register_WY = 0;
// 0xff4b
u8 register_WX = 0;
// 0xff4f
u8 register_VBK = 0;
// 0xff51
u8 register_HDMA1 = 0;
// 0xff52
u8 register_HDMA2 = 0;
// 0xff53
u8 register_HDMA3 = 0;
// 0xff54
u8 register_HDMA4 = 0;
// 0xff55
u8 register_HDMA5 = 0;
// 0xff70
u8 register_SVBK = 0;
// 0xffff
u8 register_IE = 0;

// ticks definition
int32 GBDIV_CLOCK_TICKS = 64;
int32 GBLCD_MODE_0_CLOCK_TICKS	 = 51;
int32 GBLCD_MODE_1_CLOCK_TICKS	 = 1140;
int32 GBLCD_MODE_2_CLOCK_TICKS	 = 20;
int32 GBLCD_MODE_3_CLOCK_TICKS	 = 43;
int32 GBLY_INCREMENT_CLOCK_TICKS = 114;
int32 GBTIMER_MODE_0_CLOCK_TICKS = 256;
int32 GBTIMER_MODE_1_CLOCK_TICKS = 4;
int32 GBTIMER_MODE_2_CLOCK_TICKS = 16;
int32 GBTIMER_MODE_3_CLOCK_TICKS = 64;
int32 GBSERIAL_CLOCK_TICKS		 = 128;
int32 GBSYNCHRONIZE_CLOCK_TICKS	 = 52920;

// state variables

// interrupt
int32 gbInterrupt	  = 0;
int32 gbInterruptWait = 0;
// serial
int32 gbSerialOn	= 0;
int32 gbSerialTicks = 0;
int32 gbSerialBits	= 0;
// timer
int32 gbTimerOn			= 0;
int32 gbTimerTicks		= 0;
int32 gbTimerClockTicks = 0;
int32 gbTimerMode		= 0;
// lcd
int32 gbLcdMode	 = 2;
int32 gbLcdTicks = GBLCD_MODE_2_CLOCK_TICKS;
int32 gbLcdLYIncrementTicks = 0;
// div
int32 gbDivTicks = GBDIV_CLOCK_TICKS;
// cgb
int32 gbVramBank		= 0;
int32 gbWramBank		= 1;
int32 gbHdmaSource		= 0x0000;
int32 gbHdmaDestination = 0x8000;
int32 gbHdmaBytes		= 0x0000;
int32 gbHdmaOn = 0;
int32 gbSpeed  = 0;
// timing
u32	  gbElapsedTime		 = 0;
u32	  gbTimeNow			 = 0;
int32 gbSynchronizeTicks = GBSYNCHRONIZE_CLOCK_TICKS;
// emulator features
int32 gbBattery	   = 0;
int32 gbJoymask[4] = { 0, 0, 0, 0 };

int32 gbRomSizes[] = { 0x00008000, // 32K
	                   0x00010000, // 64K
	                   0x00020000, // 128K
	                   0x00040000, // 256K
	                   0x00080000, // 512K
	                   0x00100000, // 1024K
	                   0x00200000, // 2048K
	                   0x00400000, // 4096K
	                   0x00800000 // 8192K
};
int32 gbRomSizesMasks[] = { 0x00007fff,
	                        0x0000ffff,
	                        0x0001ffff,
	                        0x0003ffff,
	                        0x0007ffff,
	                        0x000fffff,
	                        0x001fffff,
	                        0x003fffff,
	                        0x007fffff };

int32 gbRamSizes[6] = { 0x00000000, // 0K
	                    0x00000800, // 2K
	                    0x00002000, // 8K
	                    0x00008000, // 32K
	                    0x00020000, // 128K
	                    0x00010000 // 64K
};

int32 gbRamSizesMasks[6] = { 0x00000000,
	                         0x000007ff,
	                         0x00001fff,
	                         0x00007fff,
	                         0x0001ffff,
	                         0x0000ffff };

int32 gbCycles[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,  // 0
	1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,  // 1
	2, 3, 2, 2, 1, 1, 2, 1, 2, 2, 2, 2, 1, 1, 2, 1,  // 2
	2, 3, 2, 2, 3, 3, 3, 1, 2, 2, 2, 2, 1, 1, 2, 1,  // 3
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 4
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 5
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 6
	2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,  // 7
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 8
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // 9
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // a
	1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,  // b
	2, 3, 3, 4, 3, 4, 2, 4, 2, 4, 3, 2, 3, 6, 2, 4,  // c
	2, 3, 3, 0, 3, 4, 2, 4, 2, 4, 3, 0, 3, 0, 2, 4,  // d
	3, 3, 2, 0, 0, 4, 2, 4, 4, 1, 4, 0, 0, 0, 2, 4,  // e
	3, 3, 2, 1, 0, 4, 2, 4, 3, 2, 4, 1, 0, 0, 2, 4   // f
};

int32 gbCyclesCB[] = {
//  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 0
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 1
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 2
	2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 4, 2,  // 3
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 4
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 5
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 6
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 7
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 8
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // 9
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // a
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // b
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // c
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // d
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2,  // e
	2, 2, 2, 2, 2, 2, 3, 2, 2, 2, 2, 2, 2, 2, 3, 2   // f
};

u16 DAATable[] = {
	0x0080, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
	0x0800, 0x0900, 0x1020, 0x1120, 0x1220, 0x1320, 0x1420, 0x1520,
	0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
	0x1800, 0x1900, 0x2020, 0x2120, 0x2220, 0x2320, 0x2420, 0x2520,
	0x2000, 0x2100, 0x2200, 0x2300, 0x2400, 0x2500, 0x2600, 0x2700,
	0x2800, 0x2900, 0x3020, 0x3120, 0x3220, 0x3320, 0x3420, 0x3520,
	0x3000, 0x3100, 0x3200, 0x3300, 0x3400, 0x3500, 0x3600, 0x3700,
	0x3800, 0x3900, 0x4020, 0x4120, 0x4220, 0x4320, 0x4420, 0x4520,
	0x4000, 0x4100, 0x4200, 0x4300, 0x4400, 0x4500, 0x4600, 0x4700,
	0x4800, 0x4900, 0x5020, 0x5120, 0x5220, 0x5320, 0x5420, 0x5520,
	0x5000, 0x5100, 0x5200, 0x5300, 0x5400, 0x5500, 0x5600, 0x5700,
	0x5800, 0x5900, 0x6020, 0x6120, 0x6220, 0x6320, 0x6420, 0x6520,
	0x6000, 0x6100, 0x6200, 0x6300, 0x6400, 0x6500, 0x6600, 0x6700,
	0x6800, 0x6900, 0x7020, 0x7120, 0x7220, 0x7320, 0x7420, 0x7520,
	0x7000, 0x7100, 0x7200, 0x7300, 0x7400, 0x7500, 0x7600, 0x7700,
	0x7800, 0x7900, 0x8020, 0x8120, 0x8220, 0x8320, 0x8420, 0x8520,
	0x8000, 0x8100, 0x8200, 0x8300, 0x8400, 0x8500, 0x8600, 0x8700,
	0x8800, 0x8900, 0x9020, 0x9120, 0x9220, 0x9320, 0x9420, 0x9520,
	0x9000, 0x9100, 0x9200, 0x9300, 0x9400, 0x9500, 0x9600, 0x9700,
	0x9800, 0x9900, 0x00B0, 0x0130, 0x0230, 0x0330, 0x0430, 0x0530,
	0x0090, 0x0110, 0x0210, 0x0310, 0x0410, 0x0510, 0x0610, 0x0710,
	0x0810, 0x0910, 0x1030, 0x1130, 0x1230, 0x1330, 0x1430, 0x1530,
	0x1010, 0x1110, 0x1210, 0x1310, 0x1410, 0x1510, 0x1610, 0x1710,
	0x1810, 0x1910, 0x2030, 0x2130, 0x2230, 0x2330, 0x2430, 0x2530,
	0x2010, 0x2110, 0x2210, 0x2310, 0x2410, 0x2510, 0x2610, 0x2710,
	0x2810, 0x2910, 0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530,
	0x3010, 0x3110, 0x3210, 0x3310, 0x3410, 0x3510, 0x3610, 0x3710,
	0x3810, 0x3910, 0x4030, 0x4130, 0x4230, 0x4330, 0x4430, 0x4530,
	0x4010, 0x4110, 0x4210, 0x4310, 0x4410, 0x4510, 0x4610, 0x4710,
	0x4810, 0x4910, 0x5030, 0x5130, 0x5230, 0x5330, 0x5430, 0x5530,
	0x5010, 0x5110, 0x5210, 0x5310, 0x5410, 0x5510, 0x5610, 0x5710,
	0x5810, 0x5910, 0x6030, 0x6130, 0x6230, 0x6330, 0x6430, 0x6530,
	0x6010, 0x6110, 0x6210, 0x6310, 0x6410, 0x6510, 0x6610, 0x6710,
	0x6810, 0x6910, 0x7030, 0x7130, 0x7230, 0x7330, 0x7430, 0x7530,
	0x7010, 0x7110, 0x7210, 0x7310, 0x7410, 0x7510, 0x7610, 0x7710,
	0x7810, 0x7910, 0x8030, 0x8130, 0x8230, 0x8330, 0x8430, 0x8530,
	0x8010, 0x8110, 0x8210, 0x8310, 0x8410, 0x8510, 0x8610, 0x8710,
	0x8810, 0x8910, 0x9030, 0x9130, 0x9230, 0x9330, 0x9430, 0x9530,
	0x9010, 0x9110, 0x9210, 0x9310, 0x9410, 0x9510, 0x9610, 0x9710,
	0x9810, 0x9910, 0xA030, 0xA130, 0xA230, 0xA330, 0xA430, 0xA530,
	0xA010, 0xA110, 0xA210, 0xA310, 0xA410, 0xA510, 0xA610, 0xA710,
	0xA810, 0xA910, 0xB030, 0xB130, 0xB230, 0xB330, 0xB430, 0xB530,
	0xB010, 0xB110, 0xB210, 0xB310, 0xB410, 0xB510, 0xB610, 0xB710,
	0xB810, 0xB910, 0xC030, 0xC130, 0xC230, 0xC330, 0xC430, 0xC530,
	0xC010, 0xC110, 0xC210, 0xC310, 0xC410, 0xC510, 0xC610, 0xC710,
	0xC810, 0xC910, 0xD030, 0xD130, 0xD230, 0xD330, 0xD430, 0xD530,
	0xD010, 0xD110, 0xD210, 0xD310, 0xD410, 0xD510, 0xD610, 0xD710,
	0xD810, 0xD910, 0xE030, 0xE130, 0xE230, 0xE330, 0xE430, 0xE530,
	0xE010, 0xE110, 0xE210, 0xE310, 0xE410, 0xE510, 0xE610, 0xE710,
	0xE810, 0xE910, 0xF030, 0xF130, 0xF230, 0xF330, 0xF430, 0xF530,
	0xF010, 0xF110, 0xF210, 0xF310, 0xF410, 0xF510, 0xF610, 0xF710,
	0xF810, 0xF910, 0x00B0, 0x0130, 0x0230, 0x0330, 0x0430, 0x0530,
	0x0090, 0x0110, 0x0210, 0x0310, 0x0410, 0x0510, 0x0610, 0x0710,
	0x0810, 0x0910, 0x1030, 0x1130, 0x1230, 0x1330, 0x1430, 0x1530,
	0x1010, 0x1110, 0x1210, 0x1310, 0x1410, 0x1510, 0x1610, 0x1710,
	0x1810, 0x1910, 0x2030, 0x2130, 0x2230, 0x2330, 0x2430, 0x2530,
	0x2010, 0x2110, 0x2210, 0x2310, 0x2410, 0x2510, 0x2610, 0x2710,
	0x2810, 0x2910, 0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530,
	0x3010, 0x3110, 0x3210, 0x3310, 0x3410, 0x3510, 0x3610, 0x3710,
	0x3810, 0x3910, 0x4030, 0x4130, 0x4230, 0x4330, 0x4430, 0x4530,
	0x4010, 0x4110, 0x4210, 0x4310, 0x4410, 0x4510, 0x4610, 0x4710,
	0x4810, 0x4910, 0x5030, 0x5130, 0x5230, 0x5330, 0x5430, 0x5530,
	0x5010, 0x5110, 0x5210, 0x5310, 0x5410, 0x5510, 0x5610, 0x5710,
	0x5810, 0x5910, 0x6030, 0x6130, 0x6230, 0x6330, 0x6430, 0x6530,
	0x0600, 0x0700, 0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00,
	0x0E00, 0x0F00, 0x1020, 0x1120, 0x1220, 0x1320, 0x1420, 0x1520,
	0x1600, 0x1700, 0x1800, 0x1900, 0x1A00, 0x1B00, 0x1C00, 0x1D00,
	0x1E00, 0x1F00, 0x2020, 0x2120, 0x2220, 0x2320, 0x2420, 0x2520,
	0x2600, 0x2700, 0x2800, 0x2900, 0x2A00, 0x2B00, 0x2C00, 0x2D00,
	0x2E00, 0x2F00, 0x3020, 0x3120, 0x3220, 0x3320, 0x3420, 0x3520,
	0x3600, 0x3700, 0x3800, 0x3900, 0x3A00, 0x3B00, 0x3C00, 0x3D00,
	0x3E00, 0x3F00, 0x4020, 0x4120, 0x4220, 0x4320, 0x4420, 0x4520,
	0x4600, 0x4700, 0x4800, 0x4900, 0x4A00, 0x4B00, 0x4C00, 0x4D00,
	0x4E00, 0x4F00, 0x5020, 0x5120, 0x5220, 0x5320, 0x5420, 0x5520,
	0x5600, 0x5700, 0x5800, 0x5900, 0x5A00, 0x5B00, 0x5C00, 0x5D00,
	0x5E00, 0x5F00, 0x6020, 0x6120, 0x6220, 0x6320, 0x6420, 0x6520,
	0x6600, 0x6700, 0x6800, 0x6900, 0x6A00, 0x6B00, 0x6C00, 0x6D00,
	0x6E00, 0x6F00, 0x7020, 0x7120, 0x7220, 0x7320, 0x7420, 0x7520,
	0x7600, 0x7700, 0x7800, 0x7900, 0x7A00, 0x7B00, 0x7C00, 0x7D00,
	0x7E00, 0x7F00, 0x8020, 0x8120, 0x8220, 0x8320, 0x8420, 0x8520,
	0x8600, 0x8700, 0x8800, 0x8900, 0x8A00, 0x8B00, 0x8C00, 0x8D00,
	0x8E00, 0x8F00, 0x9020, 0x9120, 0x9220, 0x9320, 0x9420, 0x9520,
	0x9600, 0x9700, 0x9800, 0x9900, 0x9A00, 0x9B00, 0x9C00, 0x9D00,
	0x9E00, 0x9F00, 0x00B0, 0x0130, 0x0230, 0x0330, 0x0430, 0x0530,
	0x0610, 0x0710, 0x0810, 0x0910, 0x0A10, 0x0B10, 0x0C10, 0x0D10,
	0x0E10, 0x0F10, 0x1030, 0x1130, 0x1230, 0x1330, 0x1430, 0x1530,
	0x1610, 0x1710, 0x1810, 0x1910, 0x1A10, 0x1B10, 0x1C10, 0x1D10,
	0x1E10, 0x1F10, 0x2030, 0x2130, 0x2230, 0x2330, 0x2430, 0x2530,
	0x2610, 0x2710, 0x2810, 0x2910, 0x2A10, 0x2B10, 0x2C10, 0x2D10,
	0x2E10, 0x2F10, 0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530,
	0x3610, 0x3710, 0x3810, 0x3910, 0x3A10, 0x3B10, 0x3C10, 0x3D10,
	0x3E10, 0x3F10, 0x4030, 0x4130, 0x4230, 0x4330, 0x4430, 0x4530,
	0x4610, 0x4710, 0x4810, 0x4910, 0x4A10, 0x4B10, 0x4C10, 0x4D10,
	0x4E10, 0x4F10, 0x5030, 0x5130, 0x5230, 0x5330, 0x5430, 0x5530,
	0x5610, 0x5710, 0x5810, 0x5910, 0x5A10, 0x5B10, 0x5C10, 0x5D10,
	0x5E10, 0x5F10, 0x6030, 0x6130, 0x6230, 0x6330, 0x6430, 0x6530,
	0x6610, 0x6710, 0x6810, 0x6910, 0x6A10, 0x6B10, 0x6C10, 0x6D10,
	0x6E10, 0x6F10, 0x7030, 0x7130, 0x7230, 0x7330, 0x7430, 0x7530,
	0x7610, 0x7710, 0x7810, 0x7910, 0x7A10, 0x7B10, 0x7C10, 0x7D10,
	0x7E10, 0x7F10, 0x8030, 0x8130, 0x8230, 0x8330, 0x8430, 0x8530,
	0x8610, 0x8710, 0x8810, 0x8910, 0x8A10, 0x8B10, 0x8C10, 0x8D10,
	0x8E10, 0x8F10, 0x9030, 0x9130, 0x9230, 0x9330, 0x9430, 0x9530,
	0x9610, 0x9710, 0x9810, 0x9910, 0x9A10, 0x9B10, 0x9C10, 0x9D10,
	0x9E10, 0x9F10, 0xA030, 0xA130, 0xA230, 0xA330, 0xA430, 0xA530,
	0xA610, 0xA710, 0xA810, 0xA910, 0xAA10, 0xAB10, 0xAC10, 0xAD10,
	0xAE10, 0xAF10, 0xB030, 0xB130, 0xB230, 0xB330, 0xB430, 0xB530,
	0xB610, 0xB710, 0xB810, 0xB910, 0xBA10, 0xBB10, 0xBC10, 0xBD10,
	0xBE10, 0xBF10, 0xC030, 0xC130, 0xC230, 0xC330, 0xC430, 0xC530,
	0xC610, 0xC710, 0xC810, 0xC910, 0xCA10, 0xCB10, 0xCC10, 0xCD10,
	0xCE10, 0xCF10, 0xD030, 0xD130, 0xD230, 0xD330, 0xD430, 0xD530,
	0xD610, 0xD710, 0xD810, 0xD910, 0xDA10, 0xDB10, 0xDC10, 0xDD10,
	0xDE10, 0xDF10, 0xE030, 0xE130, 0xE230, 0xE330, 0xE430, 0xE530,
	0xE610, 0xE710, 0xE810, 0xE910, 0xEA10, 0xEB10, 0xEC10, 0xED10,
	0xEE10, 0xEF10, 0xF030, 0xF130, 0xF230, 0xF330, 0xF430, 0xF530,
	0xF610, 0xF710, 0xF810, 0xF910, 0xFA10, 0xFB10, 0xFC10, 0xFD10,
	0xFE10, 0xFF10, 0x00B0, 0x0130, 0x0230, 0x0330, 0x0430, 0x0530,
	0x0610, 0x0710, 0x0810, 0x0910, 0x0A10, 0x0B10, 0x0C10, 0x0D10,
	0x0E10, 0x0F10, 0x1030, 0x1130, 0x1230, 0x1330, 0x1430, 0x1530,
	0x1610, 0x1710, 0x1810, 0x1910, 0x1A10, 0x1B10, 0x1C10, 0x1D10,
	0x1E10, 0x1F10, 0x2030, 0x2130, 0x2230, 0x2330, 0x2430, 0x2530,
	0x2610, 0x2710, 0x2810, 0x2910, 0x2A10, 0x2B10, 0x2C10, 0x2D10,
	0x2E10, 0x2F10, 0x3030, 0x3130, 0x3230, 0x3330, 0x3430, 0x3530,
	0x3610, 0x3710, 0x3810, 0x3910, 0x3A10, 0x3B10, 0x3C10, 0x3D10,
	0x3E10, 0x3F10, 0x4030, 0x4130, 0x4230, 0x4330, 0x4430, 0x4530,
	0x4610, 0x4710, 0x4810, 0x4910, 0x4A10, 0x4B10, 0x4C10, 0x4D10,
	0x4E10, 0x4F10, 0x5030, 0x5130, 0x5230, 0x5330, 0x5430, 0x5530,
	0x5610, 0x5710, 0x5810, 0x5910, 0x5A10, 0x5B10, 0x5C10, 0x5D10,
	0x5E10, 0x5F10, 0x6030, 0x6130, 0x6230, 0x6330, 0x6430, 0x6530,
	0x00C0, 0x0140, 0x0240, 0x0340, 0x0440, 0x0540, 0x0640, 0x0740,
	0x0840, 0x0940, 0x0440, 0x0540, 0x0640, 0x0740, 0x0840, 0x0940,
	0x1040, 0x1140, 0x1240, 0x1340, 0x1440, 0x1540, 0x1640, 0x1740,
	0x1840, 0x1940, 0x1440, 0x1540, 0x1640, 0x1740, 0x1840, 0x1940,
	0x2040, 0x2140, 0x2240, 0x2340, 0x2440, 0x2540, 0x2640, 0x2740,
	0x2840, 0x2940, 0x2440, 0x2540, 0x2640, 0x2740, 0x2840, 0x2940,
	0x3040, 0x3140, 0x3240, 0x3340, 0x3440, 0x3540, 0x3640, 0x3740,
	0x3840, 0x3940, 0x3440, 0x3540, 0x3640, 0x3740, 0x3840, 0x3940,
	0x4040, 0x4140, 0x4240, 0x4340, 0x4440, 0x4540, 0x4640, 0x4740,
	0x4840, 0x4940, 0x4440, 0x4540, 0x4640, 0x4740, 0x4840, 0x4940,
	0x5040, 0x5140, 0x5240, 0x5340, 0x5440, 0x5540, 0x5640, 0x5740,
	0x5840, 0x5940, 0x5440, 0x5540, 0x5640, 0x5740, 0x5840, 0x5940,
	0x6040, 0x6140, 0x6240, 0x6340, 0x6440, 0x6540, 0x6640, 0x6740,
	0x6840, 0x6940, 0x6440, 0x6540, 0x6640, 0x6740, 0x6840, 0x6940,
	0x7040, 0x7140, 0x7240, 0x7340, 0x7440, 0x7540, 0x7640, 0x7740,
	0x7840, 0x7940, 0x7440, 0x7540, 0x7640, 0x7740, 0x7840, 0x7940,
	0x8040, 0x8140, 0x8240, 0x8340, 0x8440, 0x8540, 0x8640, 0x8740,
	0x8840, 0x8940, 0x8440, 0x8540, 0x8640, 0x8740, 0x8840, 0x8940,
	0x9040, 0x9140, 0x9240, 0x9340, 0x9440, 0x9540, 0x9640, 0x9740,
	0x9840, 0x9940, 0x3450, 0x3550, 0x3650, 0x3750, 0x3850, 0x3950,
	0x4050, 0x4150, 0x4250, 0x4350, 0x4450, 0x4550, 0x4650, 0x4750,
	0x4850, 0x4950, 0x4450, 0x4550, 0x4650, 0x4750, 0x4850, 0x4950,
	0x5050, 0x5150, 0x5250, 0x5350, 0x5450, 0x5550, 0x5650, 0x5750,
	0x5850, 0x5950, 0x5450, 0x5550, 0x5650, 0x5750, 0x5850, 0x5950,
	0x6050, 0x6150, 0x6250, 0x6350, 0x6450, 0x6550, 0x6650, 0x6750,
	0x6850, 0x6950, 0x6450, 0x6550, 0x6650, 0x6750, 0x6850, 0x6950,
	0x7050, 0x7150, 0x7250, 0x7350, 0x7450, 0x7550, 0x7650, 0x7750,
	0x7850, 0x7950, 0x7450, 0x7550, 0x7650, 0x7750, 0x7850, 0x7950,
	0x8050, 0x8150, 0x8250, 0x8350, 0x8450, 0x8550, 0x8650, 0x8750,
	0x8850, 0x8950, 0x8450, 0x8550, 0x8650, 0x8750, 0x8850, 0x8950,
	0x9050, 0x9150, 0x9250, 0x9350, 0x9450, 0x9550, 0x9650, 0x9750,
	0x9850, 0x9950, 0x9450, 0x9550, 0x9650, 0x9750, 0x9850, 0x9950,
	0xA050, 0xA150, 0xA250, 0xA350, 0xA450, 0xA550, 0xA650, 0xA750,
	0xA850, 0xA950, 0xA450, 0xA550, 0xA650, 0xA750, 0xA850, 0xA950,
	0xB050, 0xB150, 0xB250, 0xB350, 0xB450, 0xB550, 0xB650, 0xB750,
	0xB850, 0xB950, 0xB450, 0xB550, 0xB650, 0xB750, 0xB850, 0xB950,
	0xC050, 0xC150, 0xC250, 0xC350, 0xC450, 0xC550, 0xC650, 0xC750,
	0xC850, 0xC950, 0xC450, 0xC550, 0xC650, 0xC750, 0xC850, 0xC950,
	0xD050, 0xD150, 0xD250, 0xD350, 0xD450, 0xD550, 0xD650, 0xD750,
	0xD850, 0xD950, 0xD450, 0xD550, 0xD650, 0xD750, 0xD850, 0xD950,
	0xE050, 0xE150, 0xE250, 0xE350, 0xE450, 0xE550, 0xE650, 0xE750,
	0xE850, 0xE950, 0xE450, 0xE550, 0xE650, 0xE750, 0xE850, 0xE950,
	0xF050, 0xF150, 0xF250, 0xF350, 0xF450, 0xF550, 0xF650, 0xF750,
	0xF850, 0xF950, 0xF450, 0xF550, 0xF650, 0xF750, 0xF850, 0xF950,
	0x00D0, 0x0150, 0x0250, 0x0350, 0x0450, 0x0550, 0x0650, 0x0750,
	0x0850, 0x0950, 0x0450, 0x0550, 0x0650, 0x0750, 0x0850, 0x0950,
	0x1050, 0x1150, 0x1250, 0x1350, 0x1450, 0x1550, 0x1650, 0x1750,
	0x1850, 0x1950, 0x1450, 0x1550, 0x1650, 0x1750, 0x1850, 0x1950,
	0x2050, 0x2150, 0x2250, 0x2350, 0x2450, 0x2550, 0x2650, 0x2750,
	0x2850, 0x2950, 0x2450, 0x2550, 0x2650, 0x2750, 0x2850, 0x2950,
	0x3050, 0x3150, 0x3250, 0x3350, 0x3450, 0x3550, 0x3650, 0x3750,
	0x3850, 0x3950, 0x3450, 0x3550, 0x3650, 0x3750, 0x3850, 0x3950,
	0x4050, 0x4150, 0x4250, 0x4350, 0x4450, 0x4550, 0x4650, 0x4750,
	0x4850, 0x4950, 0x4450, 0x4550, 0x4650, 0x4750, 0x4850, 0x4950,
	0x5050, 0x5150, 0x5250, 0x5350, 0x5450, 0x5550, 0x5650, 0x5750,
	0x5850, 0x5950, 0x5450, 0x5550, 0x5650, 0x5750, 0x5850, 0x5950,
	0x6050, 0x6150, 0x6250, 0x6350, 0x6450, 0x6550, 0x6650, 0x6750,
	0x6850, 0x6950, 0x6450, 0x6550, 0x6650, 0x6750, 0x6850, 0x6950,
	0x7050, 0x7150, 0x7250, 0x7350, 0x7450, 0x7550, 0x7650, 0x7750,
	0x7850, 0x7950, 0x7450, 0x7550, 0x7650, 0x7750, 0x7850, 0x7950,
	0x8050, 0x8150, 0x8250, 0x8350, 0x8450, 0x8550, 0x8650, 0x8750,
	0x8850, 0x8950, 0x8450, 0x8550, 0x8650, 0x8750, 0x8850, 0x8950,
	0x9050, 0x9150, 0x9250, 0x9350, 0x9450, 0x9550, 0x9650, 0x9750,
	0x9850, 0x9950, 0x9450, 0x9550, 0x9650, 0x9750, 0x9850, 0x9950,
	0xFA60, 0xFB60, 0xFC60, 0xFD60, 0xFE60, 0xFF60, 0x00C0, 0x0140,
	0x0240, 0x0340, 0x0440, 0x0540, 0x0640, 0x0740, 0x0840, 0x0940,
	0x0A60, 0x0B60, 0x0C60, 0x0D60, 0x0E60, 0x0F60, 0x1040, 0x1140,
	0x1240, 0x1340, 0x1440, 0x1540, 0x1640, 0x1740, 0x1840, 0x1940,
	0x1A60, 0x1B60, 0x1C60, 0x1D60, 0x1E60, 0x1F60, 0x2040, 0x2140,
	0x2240, 0x2340, 0x2440, 0x2540, 0x2640, 0x2740, 0x2840, 0x2940,
	0x2A60, 0x2B60, 0x2C60, 0x2D60, 0x2E60, 0x2F60, 0x3040, 0x3140,
	0x3240, 0x3340, 0x3440, 0x3540, 0x3640, 0x3740, 0x3840, 0x3940,
	0x3A60, 0x3B60, 0x3C60, 0x3D60, 0x3E60, 0x3F60, 0x4040, 0x4140,
	0x4240, 0x4340, 0x4440, 0x4540, 0x4640, 0x4740, 0x4840, 0x4940,
	0x4A60, 0x4B60, 0x4C60, 0x4D60, 0x4E60, 0x4F60, 0x5040, 0x5140,
	0x5240, 0x5340, 0x5440, 0x5540, 0x5640, 0x5740, 0x5840, 0x5940,
	0x5A60, 0x5B60, 0x5C60, 0x5D60, 0x5E60, 0x5F60, 0x6040, 0x6140,
	0x6240, 0x6340, 0x6440, 0x6540, 0x6640, 0x6740, 0x6840, 0x6940,
	0x6A60, 0x6B60, 0x6C60, 0x6D60, 0x6E60, 0x6F60, 0x7040, 0x7140,
	0x7240, 0x7340, 0x7440, 0x7540, 0x7640, 0x7740, 0x7840, 0x7940,
	0x7A60, 0x7B60, 0x7C60, 0x7D60, 0x7E60, 0x7F60, 0x8040, 0x8140,
	0x8240, 0x8340, 0x8440, 0x8540, 0x8640, 0x8740, 0x8840, 0x8940,
	0x8A60, 0x8B60, 0x8C60, 0x8D60, 0x8E60, 0x8F60, 0x9040, 0x9140,
	0x9240, 0x9340, 0x3450, 0x3550, 0x3650, 0x3750, 0x3850, 0x3950,
	0x3A70, 0x3B70, 0x3C70, 0x3D70, 0x3E70, 0x3F70, 0x4050, 0x4150,
	0x4250, 0x4350, 0x4450, 0x4550, 0x4650, 0x4750, 0x4850, 0x4950,
	0x4A70, 0x4B70, 0x4C70, 0x4D70, 0x4E70, 0x4F70, 0x5050, 0x5150,
	0x5250, 0x5350, 0x5450, 0x5550, 0x5650, 0x5750, 0x5850, 0x5950,
	0x5A70, 0x5B70, 0x5C70, 0x5D70, 0x5E70, 0x5F70, 0x6050, 0x6150,
	0x6250, 0x6350, 0x6450, 0x6550, 0x6650, 0x6750, 0x6850, 0x6950,
	0x6A70, 0x6B70, 0x6C70, 0x6D70, 0x6E70, 0x6F70, 0x7050, 0x7150,
	0x7250, 0x7350, 0x7450, 0x7550, 0x7650, 0x7750, 0x7850, 0x7950,
	0x7A70, 0x7B70, 0x7C70, 0x7D70, 0x7E70, 0x7F70, 0x8050, 0x8150,
	0x8250, 0x8350, 0x8450, 0x8550, 0x8650, 0x8750, 0x8850, 0x8950,
	0x8A70, 0x8B70, 0x8C70, 0x8D70, 0x8E70, 0x8F70, 0x9050, 0x9150,
	0x9250, 0x9350, 0x9450, 0x9550, 0x9650, 0x9750, 0x9850, 0x9950,
	0x9A70, 0x9B70, 0x9C70, 0x9D70, 0x9E70, 0x9F70, 0xA050, 0xA150,
	0xA250, 0xA350, 0xA450, 0xA550, 0xA650, 0xA750, 0xA850, 0xA950,
	0xAA70, 0xAB70, 0xAC70, 0xAD70, 0xAE70, 0xAF70, 0xB050, 0xB150,
	0xB250, 0xB350, 0xB450, 0xB550, 0xB650, 0xB750, 0xB850, 0xB950,
	0xBA70, 0xBB70, 0xBC70, 0xBD70, 0xBE70, 0xBF70, 0xC050, 0xC150,
	0xC250, 0xC350, 0xC450, 0xC550, 0xC650, 0xC750, 0xC850, 0xC950,
	0xCA70, 0xCB70, 0xCC70, 0xCD70, 0xCE70, 0xCF70, 0xD050, 0xD150,
	0xD250, 0xD350, 0xD450, 0xD550, 0xD650, 0xD750, 0xD850, 0xD950,
	0xDA70, 0xDB70, 0xDC70, 0xDD70, 0xDE70, 0xDF70, 0xE050, 0xE150,
	0xE250, 0xE350, 0xE450, 0xE550, 0xE650, 0xE750, 0xE850, 0xE950,
	0xEA70, 0xEB70, 0xEC70, 0xED70, 0xEE70, 0xEF70, 0xF050, 0xF150,
	0xF250, 0xF350, 0xF450, 0xF550, 0xF650, 0xF750, 0xF850, 0xF950,
	0xFA70, 0xFB70, 0xFC70, 0xFD70, 0xFE70, 0xFF70, 0x00D0, 0x0150,
	0x0250, 0x0350, 0x0450, 0x0550, 0x0650, 0x0750, 0x0850, 0x0950,
	0x0A70, 0x0B70, 0x0C70, 0x0D70, 0x0E70, 0x0F70, 0x1050, 0x1150,
	0x1250, 0x1350, 0x1450, 0x1550, 0x1650, 0x1750, 0x1850, 0x1950,
	0x1A70, 0x1B70, 0x1C70, 0x1D70, 0x1E70, 0x1F70, 0x2050, 0x2150,
	0x2250, 0x2350, 0x2450, 0x2550, 0x2650, 0x2750, 0x2850, 0x2950,
	0x2A70, 0x2B70, 0x2C70, 0x2D70, 0x2E70, 0x2F70, 0x3050, 0x3150,
	0x3250, 0x3350, 0x3450, 0x3550, 0x3650, 0x3750, 0x3850, 0x3950,
	0x3A70, 0x3B70, 0x3C70, 0x3D70, 0x3E70, 0x3F70, 0x4050, 0x4150,
	0x4250, 0x4350, 0x4450, 0x4550, 0x4650, 0x4750, 0x4850, 0x4950,
	0x4A70, 0x4B70, 0x4C70, 0x4D70, 0x4E70, 0x4F70, 0x5050, 0x5150,
	0x5250, 0x5350, 0x5450, 0x5550, 0x5650, 0x5750, 0x5850, 0x5950,
	0x5A70, 0x5B70, 0x5C70, 0x5D70, 0x5E70, 0x5F70, 0x6050, 0x6150,
	0x6250, 0x6350, 0x6450, 0x6550, 0x6650, 0x6750, 0x6850, 0x6950,
	0x6A70, 0x6B70, 0x6C70, 0x6D70, 0x6E70, 0x6F70, 0x7050, 0x7150,
	0x7250, 0x7350, 0x7450, 0x7550, 0x7650, 0x7750, 0x7850, 0x7950,
	0x7A70, 0x7B70, 0x7C70, 0x7D70, 0x7E70, 0x7F70, 0x8050, 0x8150,
	0x8250, 0x8350, 0x8450, 0x8550, 0x8650, 0x8750, 0x8850, 0x8950,
	0x8A70, 0x8B70, 0x8C70, 0x8D70, 0x8E70, 0x8F70, 0x9050, 0x9150,
	0x9250, 0x9350, 0x9450, 0x9550, 0x9650, 0x9750, 0x9850, 0x9950,
};

u8 ZeroTable[] = {
	0x80, 0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,
	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0,	0,	  0
};

#define GBSAVE_GAME_VERSION_1 1
#define GBSAVE_GAME_VERSION_2 2
#define GBSAVE_GAME_VERSION_3 3
#define GBSAVE_GAME_VERSION_4 4
#define GBSAVE_GAME_VERSION_5 5
#define GBSAVE_GAME_VERSION_6 6
#define GBSAVE_GAME_VERSION_7 7
#define GBSAVE_GAME_VERSION_8 8
#define GBSAVE_GAME_VERSION_9 9
#define GBSAVE_GAME_VERSION_10 10
#define GBSAVE_GAME_VERSION_11 11
#define GBSAVE_GAME_VERSION_12 12
#define GBSAVE_GAME_VERSION_13 13
#define GBSAVE_GAME_VERSION GBSAVE_GAME_VERSION_13

#if 1
u16 gbcGetNewBGR15(int r, int g, int b)
{
	r = (r * 13 + g * 2 + b * 1 + 8) >> 4;
	g = (r * 1 + g * 12 + b * 3 + 8) >> 4;
	b = (r * 2 + g * 2 + b * 12 + 8) >> 4;
	return (b << 10) | (g << 5) | r;
}

void gbGenFilter()
{
	for (int r = 0; r < 32; r++)
	{
		for (int g = 0; g < 32; g++)
		{
			for (int b = 0; b < 32; b++)
			{
				gbColorFilter[(b << 10) | (g << 5) | r] = gbcGetNewBGR15(r, g, b);
			}
		}
	}
}
#else
int inline gbGetValue(int min, int max, int v)
{
	return (int)(min + (float)(max - min) * (2.0 * (v / 31.0) - (v / 31.0) * (v / 31.0)));
}

void gbGenFilter()
{
	for (int r = 0; r < 32; r++)
	{
		for (int g = 0; g < 32; g++)
		{
			for (int b = 0; b < 32; b++)
			{
				int nr = gbGetValue(gbGetValue(4, 14, g),
				                    gbGetValue(24, 29, g), r) - 4;
				int ng = gbGetValue(gbGetValue(4 + gbGetValue(0, 5, r),
				                               14 + gbGetValue(0, 3, r), b),
				                    gbGetValue(24 + gbGetValue(0, 3, r),
				                               29 + gbGetValue(0, 1, r), b), g) - 4;
				int nb = gbGetValue(gbGetValue(4 + gbGetValue(0, 5, r),
				                               14 + gbGetValue(0, 3, r), g),
				                    gbGetValue(24 + gbGetValue(0, 3, r),
				                               29 + gbGetValue(0, 1, r), g), b) - 4;
				gbColorFilter[(b << 10) | (g << 5) | r] = (nb << 10) | (ng << 5) | nr;
			}
		}
	}
}
#endif

void gbCopyMemory(u16 d, u16 s, int count)
{
	while (count)
	{
		gbWriteMemoryQuick(d, gbReadMemoryQuick(s));
		s++;
		d++;
		count--;
	}
}

void gbDoHdma()
{
	gbCopyMemory(gbHdmaDestination, gbHdmaSource, 0x10);

	gbHdmaDestination += 0x10;
	gbHdmaSource	  += 0x10;

	register_HDMA2 = (register_HDMA2 + 0x10) & 0xFF;
	if (register_HDMA2 == 0x00)
		register_HDMA1++;

	register_HDMA4 = (register_HDMA4 + 0x10) & 0xFF;
	if (register_HDMA4 == 0x00)
		register_HDMA3++;

	gbHdmaBytes -= 0x10;
	register_HDMA5--;
	if (register_HDMA5 == 0xff)
		gbHdmaOn = 0;
}

// fix for Harley and Lego Racers
void gbCompareLYToLYC()
{
	if (register_LY == register_LYC)
	{
		// mark that we have a match
		register_STAT |= 4;

		// check if we need an interrupt
		if ((register_STAT & 0x40) && (register_IE & 2))
			gbInterrupt |= 2;
	}
	else   // no match
		register_STAT &= 0xfb;
}

// FIXME: horrible kludge to workaround the frame timing bug
static int32 s_gbJoymask[4] = { 0, 0, 0, 0 };

void gbWriteMemoryWrapped(register u16 address, register u8 value)
{
	if (address < 0x8000)
	{
#ifndef FINAL_VERSION
		if (memorydebug && (address > 0x3fff || address < 0x2000))
		{
			log("Memory register write %04x=%02x PC=%04x\n",
			    address,
			    value,
			    PC.W);
		}
#endif
		if (mapper)
			(*mapper)(address, value);
		return;
	}

	if (address < 0xa000)
	{
		gbWriteMemoryQuick(address, value);
		return;
	}

	if (address < 0xc000)
	{
#ifndef FINAL_VERSION
		if (memorydebug)
		{
			log("Memory register write %04x=%02x PC=%04x\n",
			    address,
			    value,
			    PC.W);
		}
#endif

		// I believe this is a correct fix (it used to be 'if (mapper)')...
		if (mapperRAM)
			(*mapperRAM)(address, value);
		return;
	}

	if (address < 0xfe00)
	{
		gbWriteMemoryQuick(address, value);
		return;
	}

	if (address < 0xff00)
	{
		gbMemory[address] = value;
		return;
	}

	switch (address & 0x00ff)
	{
	case 0x00:
	{
		gbMemory[0xff00] = ((gbMemory[0xff00] & 0xcf) |
		                    (value & 0x30));
		if (gbSgbMode)
		{
			gbSgbDoBitTransfer(value);
		}

		return;
	}

	case 0x01:
	{
		gbMemory[0xff01] = value;
		return;
	}

	// serial control
	case 0x02:
	{
		gbSerialOn		 = (value & 0x80);
		gbMemory[0xff02] = value;
		if (gbSerialOn)
		{
			gbSerialTicks = GBSERIAL_CLOCK_TICKS;
#ifdef OLD_GB_LINK
			if (linkConnected)
			{
				if (value & 1)
				{
					linkSendByte(0x100 | gbMemory[0xFF01]);
					Sleep(5);
				}
			}
#endif
		}

		gbSerialBits = 0;
		return;
	}

	// DIV register resets on any write
	case 0x04:
	{
		register_DIV = 0;
		return;
	}
	case 0x05:
		register_TIMA = value;
		return;

	case 0x06:
		register_TMA = value;
		return;

	// TIMER control
	case 0x07:
	{
		register_TAC = value;

		gbTimerOn	= (value & 4);
		gbTimerMode = value & 3;
		//    register_TIMA = register_TMA;
		switch (gbTimerMode)
		{
		case 0:
			gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_0_CLOCK_TICKS;
			break;
		case 1:
			gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_1_CLOCK_TICKS;
			break;
		case 2:
			gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_2_CLOCK_TICKS;
			break;
		case 3:
			gbTimerClockTicks = gbTimerTicks = GBTIMER_MODE_3_CLOCK_TICKS;
			break;
		}
		return;
	}

	case 0x0f:
	{
		register_IF = value;
		gbInterrupt = value;
		return;
	}

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
	case 0x20:
	case 0x21:
	case 0x22:
	case 0x23:
	case 0x24:
	case 0x25:
	case 0x26:
	{
		SOUND_EVENT(address, value);
		return;
	}
	case 0x40:
	{
		int lcdChange = (register_LCDC & 0x80) ^ (value & 0x80);

		if (lcdChange)
		{
			if (value & 0x80)
			{
				gbLcdTicks	   = GBLCD_MODE_1_CLOCK_TICKS;
				gbLcdMode	   = 0;
				register_STAT &= 0xfc;
				register_LY	   = 0x00;
				// FIXME: horrible workaround
				if (gbNullInputHackTempEnabled && !useOldFrameTiming)
					memcpy(gbJoymask, s_gbJoymask, sizeof(gbJoymask));
			}
			else
			{
				gbLcdTicks	   = 0;
				gbLcdMode	   = 0;
				register_STAT &= 0xfc;
				register_LY	   = 0x00;
				// FIXME: horrible workaround
				memcpy(s_gbJoymask, gbJoymask, sizeof(gbJoymask));
				if (gbNullInputHackTempEnabled && !useOldFrameTiming)
					memset(gbJoymask, 0, sizeof(gbJoymask));
			}
			//      compareLYToLYC();
		}
		// don't draw the window if it was not enabled and not being drawn before
		if (!(register_LCDC & 0x20) && (value & 0x20) && gbWindowLine == -1 &&
		    register_LY > register_WY)
			gbWindowLine = 144;

		register_LCDC = value;

		return;
	}

	// STAT
	case 0x41:
	{
		//register_STAT = (register_STAT & 0x87) |
		//      (value & 0x7c);
		register_STAT = (value & 0xf8) | (register_STAT & 0x07);     // fix ?
		// GB bug from Devrs FAQ
		if (!gbCgbMode && (register_LCDC & 0x80) && gbLcdMode < 2)
			gbInterrupt |= 2;
		return;
	}

	// SCY
	case 0x42:
	{
		register_SCY = value;
		return;
	}

	// SCX
	case 0x43:
	{
		register_SCX = value;
		return;
	}

	// LY
	case 0x44:
	{
		// read only
		return;
	}

	// LYC
	case 0x45:
	{
		register_LYC = value;
		if ((register_LCDC & 0x80))
		{
			gbCompareLYToLYC();
		}
		return;
	}

	// DMA!
	case 0x46:
	{
		int source = value * 0x0100;

		gbCopyMemory(0xfe00,
		             source,
		             0xa0);
		register_DMA = value;
		return;
	}

	// BGP
	case 0x47:
	{
		gbBgp[0] = value & 0x03;
		gbBgp[1] = (value & 0x0c) >> 2;
		gbBgp[2] = (value & 0x30) >> 4;
		gbBgp[3] = (value & 0xc0) >> 6;
		break;
	}

	// OBP0
	case 0x48:
	{
		gbObp0[0] = value & 0x03;
		gbObp0[1] = (value & 0x0c) >> 2;
		gbObp0[2] = (value & 0x30) >> 4;
		gbObp0[3] = (value & 0xc0) >> 6;
		break;
	}

	// OBP1
	case 0x49:
	{
		gbObp1[0] = value & 0x03;
		gbObp1[1] = (value & 0x0c) >> 2;
		gbObp1[2] = (value & 0x30) >> 4;
		gbObp1[3] = (value & 0xc0) >> 6;
		break;
	}

	case 0x4a:
		register_WY = value;
		return;

	case 0x4b:
		register_WX = value;
		return;

	// KEY1
	case 0x4d:
	{
		if (gbCgbMode)
		{
			gbMemory[0xff4d] = (gbMemory[0xff4d] & 0x80) | (value & 1);
			return;
		}
		break;
	}

	// VBK
	case 0x4f:
	{
		if (gbCgbMode)
		{
			value = value & 1;
			if (value == gbVramBank)
				return;

			int vramAddress = value * 0x2000;
			gbMemoryMap[0x08] = &gbVram[vramAddress];
			gbMemoryMap[0x09] = &gbVram[vramAddress + 0x1000];

			gbVramBank	 = value;
			register_VBK = value;
		}
		return;
	}

	// HDMA1
	case 0x51:
	{
		if (gbCgbMode)
		{
			if (value > 0x7f && value < 0xa0)
				value = 0;

			gbHdmaSource = (value << 8) | (register_HDMA2 & 0xf0);

			register_HDMA1 = value;
			return;
		}
		break;
	}

	// HDMA2
	case 0x52:
	{
		if (gbCgbMode)
		{
			value = value & 0xf0;

			gbHdmaSource = (register_HDMA1 << 8) | (value);

			register_HDMA2 = value;
			return;
		}
		break;
	}

	// HDMA3
	case 0x53:
	{
		if (gbCgbMode)
		{
			value = value & 0x1f;
			gbHdmaDestination  = (value << 8) | (register_HDMA4 & 0xf0);
			gbHdmaDestination += 0x8000;
			register_HDMA3	   = value;
			return;
		}
		break;
	}

	// HDMA4
	case 0x54:
	{
		if (gbCgbMode)
		{
			value = value & 0xf0;
			gbHdmaDestination  = ((register_HDMA3 & 0x1f) << 8) | value;
			gbHdmaDestination += 0x8000;
			register_HDMA4	   = value;
			return;
		}
		break;
	}

	// HDMA5
	case 0x55:
	{
		if (gbCgbMode)
		{
			gbHdmaBytes = 16 + (value & 0x7f) * 16;
			if (gbHdmaOn)
			{
				if (value & 0x80)
				{
					register_HDMA5 = (value & 0x7f);
				}
				else
				{
					register_HDMA5 = 0xff;
					gbHdmaOn	   = 0;
				}
			}
			else
			{
				if (value & 0x80)
				{
					gbHdmaOn	   = 1;
					register_HDMA5 = value & 0x7f;
					if (gbLcdMode == 0)
						gbDoHdma();
				}
				else
				{
					// we need to take the time it takes to complete the transfer into
					// account... according to GB DEV FAQs, the setup time is the same
					// for single and double speed, but the actual transfer takes the
					// same time // (is that a typo?)
					if (gbDMASpeedVersion)
					{
						// I believe this is more correct
						// the lower 7 bits of FF55 specify the Transfer Length (divided by 16, minus 1)
						// and we make gbDmaTicks twice as many cycles at double speed to make the transfer take the same time
						if (gbSpeed)
							gbDmaTicks = 16 * ((value & 0x7f) + 1);
						else
							gbDmaTicks = 8 * ((value & 0x7f) + 1);
					}
					else
					{
						// here for backward compatibility
						// I think this was a guess that approximates the above in most but not all games
						if (gbSpeed)
							gbDmaTicks = 231 + 16 * (value & 0x7f);
						else
							gbDmaTicks = 231 + 8 * (value & 0x7f);
					}
					gbCopyMemory(gbHdmaDestination, gbHdmaSource, gbHdmaBytes);
					gbHdmaDestination += gbHdmaBytes;
					gbHdmaSource	  += gbHdmaBytes;

					register_HDMA3 = ((gbHdmaDestination - 0x8000) >> 8) & 0x1f;
					register_HDMA4 = gbHdmaDestination & 0xf0;
					register_HDMA1 = (gbHdmaSource >> 8) & 0xff;
					register_HDMA2 = gbHdmaSource & 0xf0;
				}
			}
			return;
		}
		break;
	}

	// BCPS
	case 0x68:
	{
		if (gbCgbMode)
		{
			int paletteIndex = (value & 0x3f) >> 1;
			int paletteHiLo	 = (value & 0x01);

			gbMemory[0xff68] = value;
			gbMemory[0xff69] = (paletteHiLo ?
			                    (gbPalette[paletteIndex] >> 8) :
			                    (gbPalette[paletteIndex] & 0x00ff));
			return;
		}
		break;
	}

	// BCPD
	case 0x69:
	{
		if (gbCgbMode)
		{
			int v = gbMemory[0xff68];
			int paletteIndex = (v & 0x3f) >> 1;
			int paletteHiLo	 = (v & 0x01);
			gbMemory[0xff69]		= value;
			gbPalette[paletteIndex] = (paletteHiLo ?
			                           ((value << 8) | (gbPalette[paletteIndex] & 0xff)) :
			                           ((gbPalette[paletteIndex] & 0xff00) | (value))) & 0x7fff;

			if (gbMemory[0xff68] & 0x80)
			{
				int index = ((gbMemory[0xff68] & 0x3f) + 1) & 0x3f;

				gbMemory[0xff68] = (gbMemory[0xff68] & 0x80) | index;
				gbMemory[0xff69] = (index & 1 ?
				                    (gbPalette[index >> 1] >> 8) :
				                    (gbPalette[index >> 1] & 0x00ff));
			}
			return;
		}
		break;
	}

	// OCPS
	case 0x6a:
	{
		if (gbCgbMode)
		{
			int paletteIndex = (value & 0x3f) >> 1;
			int paletteHiLo	 = (value & 0x01);

			paletteIndex += 32;

			gbMemory[0xff6a] = value;
			gbMemory[0xff6b] = (paletteHiLo ?
			                    (gbPalette[paletteIndex] >> 8) :
			                    (gbPalette[paletteIndex] & 0x00ff));
			return;
		}
		break;
	}

	// OCPD
	case 0x6b:
	{
		if (gbCgbMode)
		{
			int v = gbMemory[0xff6a];
			int paletteIndex = (v & 0x3f) >> 1;
			int paletteHiLo	 = (v & 0x01);

			paletteIndex += 32;

			gbMemory[0xff6b]		= value;
			gbPalette[paletteIndex] = (paletteHiLo ?
			                           ((value << 8) | (gbPalette[paletteIndex] & 0xff)) :
			                           ((gbPalette[paletteIndex] & 0xff00) | (value))) & 0x7fff;
			if (gbMemory[0xff6a] & 0x80)
			{
				int index = ((gbMemory[0xff6a] & 0x3f) + 1) & 0x3f;

				gbMemory[0xff6a] = (gbMemory[0xff6a] & 0x80) | index;

				gbMemory[0xff6b] = (index & 1 ?
				                    (gbPalette[(index >> 1) + 32] >> 8) :
				                    (gbPalette[(index >> 1) + 32] & 0x00ff));
			}
			return;
		}
		break;
	}

	// SVBK
	case 0x70:
	{
		if (gbCgbMode)
		{
			value = value & 7;

			int bank = value;
			if (value == 0)
				bank = 1;

			if (bank == gbWramBank)
				return;

			int wramAddress = bank * 0x1000;
			gbMemoryMap[0x0d] = &gbWram[wramAddress];

			gbWramBank	  = bank;
			register_SVBK = value;
			return;
		}
		break;
	}

	case 0xff:
	{
		register_IE	 = value;
		register_IF &= value;
		return;
	}
	}

	gbWriteMemoryQuick(address, value);
}

u8 gbReadOpcode(register u16 address)
{
	if (gbCheatMap[address])
		return gbCheatRead(address);

	// the following fix does more than Echo RAM fix, anyway...
	switch (gbEchoRAMFixOn ? (address >> 12) & 0x000f : address & 0xf000)
	{
	case 0x0a:
	case 0x0b:
		if (mapperReadRAM)
			return mapperReadRAM(address);
		break;
	case 0x0f:
		if (address > 0xff00)
		{
			switch (address & 0x00ff)
			{
			case 0x04:
				return register_DIV;
			case 0x05:
				return register_TIMA;
			case 0x06:
				return register_TMA;
			case 0x07:
				return (0xf8 | register_TAC);
			case 0x0f:
				return (0xe0 | register_IF);
			case 0x40:
				return register_LCDC;
			case 0x41:
				return (0x80 | register_STAT);
			case 0x42:
				return register_SCY;
			case 0x43:
				return register_SCX;
			case 0x44:
				return register_LY;
			case 0x45:
				return register_LYC;
			case 0x46:
				return register_DMA;
			case 0x4a:
				return register_WY;
			case 0x4b:
				return register_WX;
			case 0x4f:
				return (0xfe | register_VBK);
			case 0x51:
				return register_HDMA1;
			case 0x52:
				return register_HDMA2;
			case 0x53:
				return register_HDMA3;
			case 0x54:
				return register_HDMA4;
			case 0x55:
				return register_HDMA5;
			case 0x70:
				return (0xf8 | register_SVBK);
			case 0xff:
				return register_IE;
			}
		}
		break;
	}
	return gbReadMemoryQuick(address);
}

void gbWriteMemory(register u16 address, register u8 value)
{
	gbWriteMemoryWrapped(address, value);
	CallRegisteredLuaMemHook(address, 1, value, LUAMEMHOOK_WRITE);
}

u8 gbReadMemoryWrapped(register u16 address)
{
	if (gbCheatMap[address])
		return gbCheatRead(address);

	if (address < 0xa000)
		return gbReadMemoryQuick(address);

	if (address < 0xc000)
	{
#ifndef FINAL_VERSION
		if (memorydebug)
		{
			log("Memory register read %04x PC=%04x\n",
			    address,
			    PC.W);
		}
#endif

		if (mapperReadRAM)
			return mapperReadRAM(address);
		return gbReadMemoryQuick(address);
	}

	if (address >= 0xff00)
	{
		switch (address & 0x00ff)
		{
		case 0x00:
		{
			if (gbSgbMode)
			{
				gbSgbReadingController |= 4;
				gbSgbResetPacketState();
			}

			int b = gbMemory[0xff00];

			if ((b & 0x30) == 0x20)
			{
				b &= 0xf0;

				int joy = 0;
				if (gbSgbMode && gbSgbMultiplayer)
				{
					switch (gbSgbNextController)
					{
					case 0x0f:
						joy = 0;
						break;
					case 0x0e:
						joy = 1;
						break;
					case 0x0d:
						joy = 2;
						break;
					case 0x0c:
						joy = 3;
						break;
					default:
						joy = 0;
					}
				}
				int joystate = gbJoymask[joy];
				if (!(joystate & 128))
					b |= 0x08;
				if (!(joystate & 64))
					b |= 0x04;
				if (!(joystate & 32))
					b |= 0x02;
				if (!(joystate & 16))
					b |= 0x01;

				gbMemory[0xff00] = b;
			}
			else if ((b & 0x30) == 0x10)
			{
				b &= 0xf0;

				int joy = 0;
				if (gbSgbMode && gbSgbMultiplayer)
				{
					switch (gbSgbNextController)
					{
					case 0x0f:
						joy = 0;
						break;
					case 0x0e:
						joy = 1;
						break;
					case 0x0d:
						joy = 2;
						break;
					case 0x0c:
						joy = 3;
						break;
					default:
						joy = 0;
					}
				}
				int joystate = gbJoymask[joy];
				if (!(joystate & 8))
					b |= 0x08;
				if (!(joystate & 4))
					b |= 0x04;
				if (!(joystate & 2))
					b |= 0x02;
				if (!(joystate & 1))
					b |= 0x01;

				gbMemory[0xff00] = b;
			}
			else
			{
				if (gbSgbMode && gbSgbMultiplayer)
				{
					gbMemory[0xff00] = 0xf0 | gbSgbNextController;
				}
				else
				{
					gbMemory[0xff00] = 0xff;
				}
			}
		}
			systemCounters.lagged = false;
			return gbMemory[0xff00];
			break;
		case 0x01:
			return gbMemory[0xff01];
		case 0x04:
			return register_DIV;
		case 0x05:
			return register_TIMA;
		case 0x06:
			return register_TMA;
		case 0x07:
			return (0xf8 | register_TAC);
		case 0x0f:
			return (0xe0 | register_IF);
		case 0x40:
			return register_LCDC;
		case 0x41:
			return (0x80 | register_STAT);
		case 0x42:
			return register_SCY;
		case 0x43:
			return register_SCX;
		case 0x44:
			return register_LY;
		case 0x45:
			return register_LYC;
		case 0x46:
			return register_DMA;
		case 0x4a:
			return register_WY;
		case 0x4b:
			return register_WX;
		case 0x4f:
			return (0xfe | register_VBK);
		case 0x51:
			return register_HDMA1;
		case 0x52:
			return register_HDMA2;
		case 0x53:
			return register_HDMA3;
		case 0x54:
			return register_HDMA4;
		case 0x55:
			return register_HDMA5;
		case 0x70:
			return (0xf8 | register_SVBK);
		case 0xff:
			return register_IE;
		}
	}

	return gbReadMemoryQuick(address);
}

u8 gbReadMemory(register u16 address)
{
	u8 value = gbReadMemoryWrapped(address);
	CallRegisteredLuaMemHook(address, 1, value, LUAMEMHOOK_READ);
	return value;
}

void gbVblank_interrupt()
{
	if (IFF & 0x80)
	{
		PC.W++;
		IFF &= 0x7f;
	}
	gbInterrupt &= 0xfe;

	IFF			&= 0x7e;
	register_IF &= 0xfe;

	gbWriteMemory(--SP.W, PC.B.B1);
	gbWriteMemory(--SP.W, PC.B.B0);
	PC.W = 0x40;
}

void gbLcd_interrupt()
{
	if (IFF & 0x80)
	{
		PC.W++;
		IFF &= 0x7f;
	}
	gbInterrupt &= 0xfd;
	IFF			&= 0x7e;
	register_IF &= 0xfd;

	gbWriteMemory(--SP.W, PC.B.B1);
	gbWriteMemory(--SP.W, PC.B.B0);

	PC.W = 0x48;
}

void gbTimer_interrupt()
{
	if (IFF & 0x80)
	{
		PC.W++;
		IFF &= 0x7f;
	}
	IFF			&= 0x7e;
	gbInterrupt &= 0xfb;
	register_IF &= 0xfb;

	gbWriteMemory(--SP.W, PC.B.B1);
	gbWriteMemory(--SP.W, PC.B.B0);

	PC.W = 0x50;
}

void gbSerial_interrupt()
{
	if (IFF & 0x80)
	{
		PC.W++;
		IFF &= 0x7f;
	}
	IFF			&= 0x7e;
	gbInterrupt &= 0xf7;
	register_IF &= 0xf7;

	gbWriteMemory(--SP.W, PC.B.B1);
	gbWriteMemory(--SP.W, PC.B.B0);

	PC.W = 0x58;
}

void gbJoypad_interrupt()
{
	if (IFF & 0x80)
	{
		PC.W++;
		IFF &= 0x7f;
	}
	IFF			&= 0x7e;
	gbInterrupt &= 0xef;
	register_IF &= 0xef;

	gbWriteMemory(--SP.W, PC.B.B1);
	gbWriteMemory(--SP.W, PC.B.B0);

	PC.W = 0x60;
}

void gbSpeedSwitch()
{
	if (gbSpeed == 0)
	{
		gbSpeed = 1;
		USE_TICKS_AS  = GB_SOUNDTICKS_AS * 2;
		GBLCD_MODE_0_CLOCK_TICKS   = 51 * 2; //127; //51 * 2;
		GBLCD_MODE_1_CLOCK_TICKS   = 1140 * 2;
		GBLCD_MODE_2_CLOCK_TICKS   = 20 * 2; //52; //20 * 2;
		GBLCD_MODE_3_CLOCK_TICKS   = 43 * 2; //99; //43 * 2;
		GBDIV_CLOCK_TICKS		   = 64 * 2;
		GBLY_INCREMENT_CLOCK_TICKS = 114 * 2;
		GBTIMER_MODE_0_CLOCK_TICKS = 256; //256*2;
		GBTIMER_MODE_1_CLOCK_TICKS = 4; //4*2;
		GBTIMER_MODE_2_CLOCK_TICKS = 16; //16*2;
		GBTIMER_MODE_3_CLOCK_TICKS = 64; //64*2;
		GBSERIAL_CLOCK_TICKS	   = 128 * 2;
		gbDivTicks *= 2;
		gbLcdTicks *= 2;
		gbLcdLYIncrementTicks *= 2;
		//    timerTicks *= 2;
		//    timerClockTicks *= 2;
		gbSerialTicks *= 2;
		soundTicks	  *= 2;
		//    synchronizeTicks *= 2;
		//    SYNCHRONIZE_CLOCK_TICKS *= 2;
	}
	else
	{
		gbSpeed = 0;
		USE_TICKS_AS = GB_SOUNDTICKS_AS;
		GBLCD_MODE_0_CLOCK_TICKS   = 51;
		GBLCD_MODE_1_CLOCK_TICKS   = 1140;
		GBLCD_MODE_2_CLOCK_TICKS   = 20;
		GBLCD_MODE_3_CLOCK_TICKS   = 43;
		GBDIV_CLOCK_TICKS		   = 64;
		GBLY_INCREMENT_CLOCK_TICKS = 114;
		GBTIMER_MODE_0_CLOCK_TICKS = 256;
		GBTIMER_MODE_1_CLOCK_TICKS = 4;
		GBTIMER_MODE_2_CLOCK_TICKS = 16;
		GBTIMER_MODE_3_CLOCK_TICKS = 64;
		GBSERIAL_CLOCK_TICKS	   = 128;
		gbDivTicks /= 2;
		gbLcdTicks /= 2;
		gbLcdLYIncrementTicks /= 2;
		//    timerTicks /= 2;
		//    timerClockTicks /= 2;
		gbSerialTicks /= 2;
		soundTicks	  /= 2;
		//    synchronizeTicks /= 2;
		//    SYNCHRONIZE_CLOCK_TICKS /= 2;
	}
	soundTickStep = USE_TICKS_AS * soundQuality;
}

void gbGetHardwareType()
{
	gbCgbMode = 0;
	if (gbRom[0x143] & 0x80)
	{
		if (gbEmulatorType == 0 ||
		    gbEmulatorType == 1 ||
		    gbEmulatorType == 4 ||
		    gbEmulatorType == 5 ||
		    (gbRom[0x146] != 0x03 && (gbEmulatorType == 2)))
		{
			gbCgbMode = 1;
		}
	}

	if (gbSgbMode == 2)
	{
		gbSgbMode = 0;
		return;
	}

	gbSgbMode = 0;
	if (gbRom[0x146] == 0x03)
	{
		if (gbEmulatorType == 0 ||
		    gbEmulatorType == 2 ||
		    gbEmulatorType == 5 ||
		    (!(gbRom[0x143] & 0x80) && (gbEmulatorType == 1 || gbEmulatorType == 4)))
			gbSgbMode = 1;
	}
}

void gbReset()
{
	systemReset();

	gbGetHardwareType();
	SP.W = 0xfffe;
	AF.W = 0x01b0;
	BC.W = 0x0013;
	DE.W = 0x00d8;
	HL.W = 0x014d;
	PC.W = 0x0100;
	IFF	 = 0;
	gbInterrupt		= 1;
	gbInterruptWait = 0;

	register_DIV   = 0;
	register_TIMA  = 0;
	register_TMA   = 0;
	register_TAC   = 0;
	register_IF	   = 1;
	register_LCDC  = 0x91;
	register_STAT  = 0;
	register_SCY   = 0;
	register_SCX   = 0;
	register_LY	   = 0;
	register_LYC   = 0;
	register_DMA   = 0;
	register_WY	   = 0;
	register_WX	   = 0;
	register_VBK   = 0;
	register_HDMA1 = 0;
	register_HDMA2 = 0;
	register_HDMA3 = 0;
	register_HDMA4 = 0;
	register_HDMA5 = 0;
	register_SVBK  = 0;
	register_IE	   = 0;

	if (gbCgbMode)
	{
		if (!gbVram)
			gbVram = (u8 *)RAM_MALLOC(0x4000);
		if (!gbWram)
			gbWram = (u8 *)RAM_MALLOC(0x8000);
		memset(gbVram, 0, 0x4000);
		memset(gbWram, 0, 0x8000);
	}
	else
	{
		free(gbVram);
		gbVram = NULL;
		free(gbWram);
		gbWram = NULL;
	}

	// clean LineBuffer
	if (gbLineBuffer)
		memset(gbLineBuffer, 0, 160 * sizeof(u16));
	// clean Pix
	if (pix)
		memset(pix, 0, 4 * 257 * 226);

	if (gbCgbMode)
	{
		if (gbSgbMode)
		{
			if (gbEmulatorType == 5)
				AF.W = 0xffb0;
			else
				AF.W = 0x01b0;
			BC.W = 0x0013;
			DE.W = 0x00d8;
			HL.W = 0x014d;
		}
		else
		{
			AF.W = 0x11b0;
			BC.W = 0x0000;
			DE.W = 0xff56;
			HL.W = 0x000d;
		}
		if (gbEmulatorType == 4)
			BC.B.B1 |= 0x01;

		register_HDMA5	 = 0xff;
		gbMemory[0xff68] = 0xc0;
		gbMemory[0xff6a] = 0xc0;

		for (int i = 0; i < 64; i++)
			gbPalette[i] = 0x7fff;
	}
	else
	{
		for (int i = 0; i < 8; i++)
			gbPalette[i] = systemGbPalette[gbPaletteOption * 8 + i];
	}

	if (gbSpeed)
	{
		gbSpeedSwitch();
		gbMemory[0xff4d] = 0;
	}

	gbDivTicks = GBDIV_CLOCK_TICKS;
	gbLcdMode  = 2;
	gbLcdTicks = GBLCD_MODE_2_CLOCK_TICKS;
	gbLcdLYIncrementTicks = 0;
	gbTimerTicks		  = 0;
	gbTimerClockTicks	  = 0;
	gbSerialTicks		  = 0;
	gbSerialBits		  = 0;
	gbSerialOn			  = 0;
	gbWindowLine		  = -1;
	gbTimerOn			  = 0;
	gbTimerMode			  = 0;
	//  gbSynchronizeTicks = GBSYNCHRONIZE_CLOCK_TICKS;
	gbSpeed		 = 0;
	gbJoymask[0] = gbJoymask[1] = gbJoymask[2] = gbJoymask[3] = 0;

	// FIXME: horrible kludge
	memset(s_gbJoymask, 0, sizeof(s_gbJoymask));

	if (gbCgbMode)
	{
		gbHdmaOn = 0;
		gbHdmaSource	  = 0x0000;
		gbHdmaDestination = 0x8000;
		gbVramBank		  = 0;
		gbWramBank		  = 1;
		register_LY		  = 0x90;
		gbLcdMode		  = 1;
	}

	if (gbSgbMode)
	{
		gbSgbReset();
	}

	for (int i = 0; i < 4; i++)
		gbBgp[i] = gbObp0[i] = gbObp1[i] = i;

	memset(&gbDataMBC1, 0, sizeof(gbDataMBC1));
	gbDataMBC1.mapperROMBank = 1;

	gbDataMBC2.mapperRAMEnable = 0;
	gbDataMBC2.mapperROMBank   = 1;

	memset(&gbDataMBC3, 0, 6 * sizeof(int32));
	gbDataMBC3.mapperROMBank = 1;

	memset(&gbDataMBC5, 0, sizeof(gbDataMBC5));
	gbDataMBC5.mapperROMBank = 1;
	switch (gbRom[0x147])
	{
	case 0x1c:
	case 0x1d:
	case 0x1e:
		gbDataMBC5.isRumbleCartridge = 1;
	}

	memset(&gbDataHuC1, 0, sizeof(gbDataHuC1));
	gbDataHuC1.mapperROMBank = 1;

	memset(&gbDataHuC3, 0, sizeof(gbDataHuC3));
	gbDataHuC3.mapperROMBank = 1;

	gbMemoryMap[0x00] = &gbRom[0x0000];
	gbMemoryMap[0x01] = &gbRom[0x1000];
	gbMemoryMap[0x02] = &gbRom[0x2000];
	gbMemoryMap[0x03] = &gbRom[0x3000];
	gbMemoryMap[0x04] = &gbRom[0x4000];
	gbMemoryMap[0x05] = &gbRom[0x5000];
	gbMemoryMap[0x06] = &gbRom[0x6000];
	gbMemoryMap[0x07] = &gbRom[0x7000];
	if (gbCgbMode)
	{
		gbMemoryMap[0x08] = &gbVram[0x0000];
		gbMemoryMap[0x09] = &gbVram[0x1000];
		gbMemoryMap[0x0a] = &gbMemory[0xa000];
		gbMemoryMap[0x0b] = &gbMemory[0xb000];
		gbMemoryMap[0x0c] = &gbMemory[0xc000];
		gbMemoryMap[0x0d] = &gbWram[0x1000];
		gbMemoryMap[0x0e] = &gbMemory[0xe000];
		gbMemoryMap[0x0f] = &gbMemory[0xf000];
	}
	else
	{
		gbMemoryMap[0x08] = &gbMemory[0x8000];
		gbMemoryMap[0x09] = &gbMemory[0x9000];
		gbMemoryMap[0x0a] = &gbMemory[0xa000];
		gbMemoryMap[0x0b] = &gbMemory[0xb000];
		gbMemoryMap[0x0c] = &gbMemory[0xc000];
		gbMemoryMap[0x0d] = &gbMemory[0xd000];
		gbMemoryMap[0x0e] = &gbMemory[0xe000];
		gbMemoryMap[0x0f] = &gbMemory[0xf000];
	}

	if (gbRam)
	{
		gbMemoryMap[0x0a] = &gbRam[0x0000];
		gbMemoryMap[0x0b] = &gbRam[0x1000];
	}

	gbSoundReset();
	systemRefreshScreen();
}

void gbWriteSaveMBC1(const char *name)
{
	FILE *gzFile = fopen(name, "wb");

	if (gzFile == NULL)
	{
		systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
		return;
	}

	fwrite(gbRam,
	       1,
	       gbRamSize,
	       gzFile);

	fclose(gzFile);
}

void gbWriteSaveMBC2(const char *name)
{
	FILE *file = fopen(name, "wb");

	if (file == NULL)
	{
		systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
		return;
	}

	fwrite(&gbMemory[0xa000],
	       1,
	       256,
	       file);

	fclose(file);
}

void gbWriteSaveMBC3(const char *name, bool extendedSave)
{
	FILE *gzFile = fopen(name, "wb");

	if (gzFile == NULL)
	{
		systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
		return;
	}

	fwrite(gbRam,
	       1,
	       gbRamSize,
	       gzFile);

	if (extendedSave)
	{
		fwrite(&gbDataMBC3.mapperSeconds,
		       1,
		       10 * sizeof(int32) + sizeof(u32),
		       gzFile);
	}

	fclose(gzFile);
}

void gbWriteSaveMBC5(const char *name)
{
	FILE *gzFile = fopen(name, "wb");

	if (gzFile == NULL)
	{
		systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
		return;
	}

	fwrite(gbRam,
	       1,
	       gbRamSize,
	       gzFile);

	fclose(gzFile);
}

void gbWriteSaveMBC7(const char *name)
{
	FILE *file = fopen(name, "wb");

	if (file == NULL)
	{
		systemMessage(MSG_ERROR_CREATING_FILE, N_("Error creating file %s"), name);
		return;
	}

	fwrite(&gbMemory[0xa000],
	       1,
	       256,
	       file);

	fclose(file);
}

bool gbReadSaveMBC1(const char *name)
{
	gzFile gzFile = gzopen(name, "rb");

	if (gzFile == NULL)
	{
		return false;
	}

	int read = gzread(gzFile,
	                  gbRam,
	                  gbRamSize);

	if (read != gbRamSize)
	{
		systemMessage(MSG_FAILED_TO_READ_SGM, N_("Failed to read complete save game %s (%d)"), name, read);
		gzclose(gzFile);
		return false;
	}

	gzclose(gzFile);
	return true;
}

bool gbReadSaveMBC2(const char *name)
{
	FILE *file = fopen(name, "rb");

	if (file == NULL)
	{
		return false;
	}

	int read = fread(&gbMemory[0xa000],
	                 1,
	                 256,
	                 file);

	if (read != 256)
	{
		systemMessage(MSG_FAILED_TO_READ_SGM,
		              N_("Failed to read complete save game %s (%d)"), name, read);
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}

bool gbReadSaveMBC3(const char *name)
{
	gzFile gzFile = gzopen(name, "rb");

	if (gzFile == NULL)
	{
		return false;
	}

	int read = gzread(gzFile,
	                  gbRam,
	                  gbRamSize);

	bool res = true;

	if (read != gbRamSize)
	{
		systemMessage(MSG_FAILED_TO_READ_SGM,
		              N_("Failed to read complete save game %s (%d)"), name, read);
	}
	else
	{
		read = gzread(gzFile,
		              &gbDataMBC3.mapperSeconds,
		              sizeof(int32) * 10 + sizeof(u32));

		if (read != (sizeof(int32) * 10 + sizeof(u32)) && read != 0)
		{
			systemMessage(MSG_FAILED_TO_READ_RTC,
			              N_("Failed to read RTC from save game %s (continuing)"),
			              name);
			res = false;
		}
	}

	gzclose(gzFile);
	return res;
}

bool gbReadSaveMBC5(const char *name)
{
	gzFile gzFile = gzopen(name, "rb");

	if (gzFile == NULL)
	{
		return false;
	}

	int read = gzread(gzFile,
	                  gbRam,
	                  gbRamSize);

	if (read != gbRamSize)
	{
		systemMessage(MSG_FAILED_TO_READ_SGM,
		              N_("Failed to read complete save game %s (%d)"), name, read);
		gzclose(gzFile);
		return false;
	}

	gzclose(gzFile);
	return true;
}

bool gbReadSaveMBC7(const char *name)
{
	FILE *file = fopen(name, "rb");

	if (file == NULL)
	{
		return false;
	}

	int read = fread(&gbMemory[0xa000],
	                 1,
	                 256,
	                 file);

	if (read != 256)
	{
		systemMessage(MSG_FAILED_TO_READ_SGM,
		              N_("Failed to read complete save game %s (%d)"), name, read);
		fclose(file);
		return false;
	}

	fclose(file);
	return true;
}

void gbLoadInternalBios()
{
	// clear BIOS
	memset(bios, 0, 0x100);
}

void gbInit()
{
	gbGenFilter();
	gbSgbInit();    // calls gbSgbReset()... whatever

	gbMemory = (u8 *)RAM_MALLOC(0x10000);

	pix = (u8 *)PIX_CALLOC(4 * 257 * 226);

	gbLineBuffer = (u16 *)malloc(160 * sizeof(u16));

	free(bios);
	bios = (u8 *)RAM_MALLOC(0x100);

	memset(gbMemory, 0, 0x10000);
	memset(gbPalette, 0, 2 * 128);
}

bool gbWriteBatteryFile(const char *file, bool extendedSave)
{
	if (gbBattery)
	{
		int type = gbRom[0x147];

		switch (type)
		{
		case 0x03:
			gbWriteSaveMBC1(file);
			break;
		case 0x06:
			gbWriteSaveMBC2(file);
			break;
		case 0x0f:
		case 0x10:
		case 0x13:
			gbWriteSaveMBC3(file, extendedSave);
			break;
		case 0x1b:
		case 0x1e:
			gbWriteSaveMBC5(file);
			break;
		case 0x22:
			gbWriteSaveMBC7(file);
			break;
		case 0xff:
			gbWriteSaveMBC1(file);
			break;
		}
	}
	return true;
}

bool gbWriteBatteryFile(const char *file)
{
	gbWriteBatteryFile(file, true);
	return true;
}

bool gbWriteBatteryToStream(gzFile gzfile)
{
	// the GB save code is ugly, so rather than convert it all to use gzFiles, just save it to a temp file...
#define TEMP_SAVE_FNAME ("tempvbawrite.sav")
	bool retVal = gbWriteBatteryFile(TEMP_SAVE_FNAME, true);

	// ...open the temp file and figure out its size...
	FILE *fileTemp = fopen(TEMP_SAVE_FNAME, "rb");
	if (fileTemp == NULL)
		return false;
	fseek(fileTemp, 0, SEEK_END);
	int len = (int) ftell(fileTemp);

	// ...copy over the temp file...
	char *temp = new char [len];
	fseek(fileTemp, 0, SEEK_SET);
	if (fread(temp, len, 1, fileTemp) != 1)
	{
		delete [] temp;
		fclose(fileTemp);
		return false;
	}
	fclose(fileTemp);
	utilGzWrite(gzfile, temp, len);
	delete [] temp;

	// ... and delete the temp file
	remove(TEMP_SAVE_FNAME);
#undef TEMP_SAVE_FNAME

	return retVal;
}

bool gbReadBatteryFile(const char *file)
{
	bool res = false;
	if (gbBattery)
	{
		int type = gbRom[0x147];
		switch (type)
		{
		case 0x03:
			res = gbReadSaveMBC1(file);
			break;
		case 0x06:
			res = gbReadSaveMBC2(file);
			break;
		case 0x0f:
		case 0x10:
		case 0x13:
			if (!gbReadSaveMBC3(file))
			{
				struct tm *lt;
				time_t tmp; //64 bit kludge
				if (VBAMovieIsActive() || VBAMovieIsLoading())
				{
					tmp = time_t(VBAMovieGetId() + VBAMovieGetFrameCounter() / VBAMovieGetFrameRate());
					lt  = gmtime(&tmp);
				}
				else
				{
					tmp = time(NULL);
					lt  = localtime(&tmp);
				}
				systemScreenMessage(ctime(&tmp), 4);
				gbDataMBC3.mapperLastTime = u32(tmp);

				gbDataMBC3.mapperSeconds = lt->tm_sec;
				gbDataMBC3.mapperMinutes = lt->tm_min;
				gbDataMBC3.mapperHours	 = lt->tm_hour;
				gbDataMBC3.mapperDays	 = lt->tm_yday & 255;
				gbDataMBC3.mapperControl = (gbDataMBC3.mapperControl & 0xfe) |
				                           (lt->tm_yday > 255 ? 1 : 0);
				res = false;
				break;
			}
			time_t tmp;
			systemScreenMessage(ctime(&tmp), 4);
			gbDataMBC3.mapperLastTime=(u32)tmp;
			res = true;
			break;
		case 0x1b:
		case 0x1e:
			res = gbReadSaveMBC5(file);
			break;
		case 0x22:
			res = gbReadSaveMBC7(file);
			break;
		case 0xff:
			res = gbReadSaveMBC1(file);
			break;
		}
	}
	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;
	return res;
}

bool gbReadBatteryFromStream(gzFile gzfile)
{
	// the GB save code is ugly, so rather than convert it all to use gzFiles, just copy it to temp RAM...
#define TEMP_SAVE_FNAME ("tempvbaread.sav")
	int	  pos	 = gztell(gzfile);
	int	  buflen = 1024;
	// ...make a temp file and write it there...
	FILE *fileTemp = fopen(TEMP_SAVE_FNAME, "wb");
	if (fileTemp == NULL)
		return false;
	int gzDeflated;
	char *temp	 = new char [buflen];
	while ((gzDeflated = utilGzRead(gzfile, temp, buflen)) != 0)
	{
		if (gzDeflated == -1 || fwrite(temp, gzDeflated, 1, fileTemp) != 1)
		{
			delete [] temp;
			fclose(fileTemp);
			gzseek(gzfile, pos, SEEK_SET); /// FIXME: leaves pos in gzfile before save instead of after it (everything that
			                               // calls this right now does a seek afterwards so it doesn't matter for now, but it's
			                               // still bad)
			return false;
		}
	}
	gzseek(gzfile, pos, SEEK_SET); /// FIXME: leaves pos in gzfile before save instead of after it (everything that calls this
	                               // right now does a seek afterwards so it doesn't matter for now, but it's still bad)
	fclose(fileTemp);
	delete [] temp;

	// ... load from the temp file...
	bool retVal = gbReadBatteryFile(TEMP_SAVE_FNAME);

	// ... and delete the temp file
	remove(TEMP_SAVE_FNAME);
#undef TEMP_SAVE_FNAME

	return retVal;
}

bool gbReadGSASnapshot(const char *fileName)
{
	FILE *file = fopen(fileName, "rb");

	if (!file)
	{
		systemMessage(MSG_CANNOT_OPEN_FILE, N_("Cannot open file %s"), fileName);
		return false;
	}

	fseek(file, 0x4, SEEK_SET);
	char buffer[16];
	char buffer2[16];
	fread(buffer, 1, 15, file);
	buffer[15] = 0;
	memcpy(buffer2, &gbRom[0x134], 15);
	buffer2[15] = 0;
	if (memcmp(buffer, buffer2, 15))
	{
		systemMessage(MSG_CANNOT_IMPORT_SNAPSHOT_FOR,
		              N_("Cannot import snapshot for %s. Current game is %s"),
		              buffer,
		              buffer2);
		fclose(file);
		return false;
	}
	fseek(file, 0x13, SEEK_SET);
	int read   = 0;
	int toRead = 0;
	switch (gbRom[0x147])
	{
	case 0x03:
	case 0x0f:
	case 0x10:
	case 0x13:
	case 0x1b:
	case 0x1e:
	case 0xff:
		read   = fread(gbRam, 1, gbRamSize, file);
		toRead = gbRamSize;
		break;
	case 0x06:
	case 0x22:
		read   = fread(&gbMemory[0xa000], 1, 256, file);
		toRead = 256;
		break;
	default:
		systemMessage(MSG_UNSUPPORTED_SNAPSHOT_FILE,
		              N_("Unsupported snapshot file %s"),
		              fileName);
		fclose(file);
		return false;
	}
	fclose(file);
	gbReset();
	return true;
}

variable_desc gbSaveGameStruct[] =
{
	{ &PC.W,					   sizeof(u16)						 },
	{ &SP.W,					   sizeof(u16)						 },
	{ &AF.W,					   sizeof(u16)						 },
	{ &BC.W,					   sizeof(u16)						 },
	{ &DE.W,					   sizeof(u16)						 },
	{ &HL.W,					   sizeof(u16)						 },
	{ &IFF,						   sizeof(u8)						 },
	{ &GBLCD_MODE_0_CLOCK_TICKS,   sizeof(int32)					 },
	{ &GBLCD_MODE_1_CLOCK_TICKS,   sizeof(int32)					 },
	{ &GBLCD_MODE_2_CLOCK_TICKS,   sizeof(int32)					 },
	{ &GBLCD_MODE_3_CLOCK_TICKS,   sizeof(int32)					 },
	{ &GBDIV_CLOCK_TICKS,		   sizeof(int32)					 },
	{ &GBLY_INCREMENT_CLOCK_TICKS, sizeof(int32)					 },
	{ &GBTIMER_MODE_0_CLOCK_TICKS, sizeof(int32)					 },
	{ &GBTIMER_MODE_1_CLOCK_TICKS, sizeof(int32)					 },
	{ &GBTIMER_MODE_2_CLOCK_TICKS, sizeof(int32)					 },
	{ &GBTIMER_MODE_3_CLOCK_TICKS, sizeof(int32)					 },
	{ &GBSERIAL_CLOCK_TICKS,	   sizeof(int32)					 },
	{ &GBSYNCHRONIZE_CLOCK_TICKS,  sizeof(int32)					 },
	{ &gbDivTicks,				   sizeof(int32)					 },
	{ &gbLcdMode,				   sizeof(int32)					 },
	{ &gbLcdTicks,				   sizeof(int32)					 },
	{ &gbLcdLYIncrementTicks,	   sizeof(int32)					 },
	{ &gbTimerTicks,			   sizeof(int32)					 },
	{ &gbTimerClockTicks,		   sizeof(int32)					 },
	{ &gbSerialTicks,			   sizeof(int32)					 },
	{ &gbSerialBits,			   sizeof(int32)					 },
	{ &gbInterrupt,				   sizeof(int32)					 },
	{ &gbInterruptWait,			   sizeof(int32)					 },
	{ &gbSynchronizeTicks,		   sizeof(int32)					 },
	{ &gbTimerOn,				   sizeof(int32)					 },
	{ &gbTimerMode,				   sizeof(int32)					 },
	{ &gbSerialOn,				   sizeof(int32)					 },
	{ &gbWindowLine,			   sizeof(int32)					 },
	{ &gbCgbMode,				   sizeof(int32)					 },
	{ &gbVramBank,				   sizeof(int32)					 },
	{ &gbWramBank,				   sizeof(int32)					 },
	{ &gbHdmaSource,			   sizeof(int32)					 },
	{ &gbHdmaDestination,		   sizeof(int32)					 },
	{ &gbHdmaBytes,				   sizeof(int32)					 },
	{ &gbHdmaOn,				   sizeof(int32)					 },
	{ &gbSpeed,					   sizeof(int32)					 },
	{ &gbSgbMode,				   sizeof(int32)					 },
	{ &register_DIV,			   sizeof(u8)						 },
	{ &register_TIMA,			   sizeof(u8)						 },
	{ &register_TMA,			   sizeof(u8)						 },
	{ &register_TAC,			   sizeof(u8)						 },
	{ &register_IF,				   sizeof(u8)						 },
	{ &register_LCDC,			   sizeof(u8)						 },
	{ &register_STAT,			   sizeof(u8)						 },
	{ &register_SCY,			   sizeof(u8)						 },
	{ &register_SCX,			   sizeof(u8)						 },
	{ &register_LY,				   sizeof(u8)						 },
	{ &register_LYC,			   sizeof(u8)						 },
	{ &register_DMA,			   sizeof(u8)						 },
	{ &register_WY,				   sizeof(u8)						 },
	{ &register_WX,				   sizeof(u8)						 },
	{ &register_VBK,			   sizeof(u8)						 },
	{ &register_HDMA1,			   sizeof(u8)						 },
	{ &register_HDMA2,			   sizeof(u8)						 },
	{ &register_HDMA3,			   sizeof(u8)						 },
	{ &register_HDMA4,			   sizeof(u8)						 },
	{ &register_HDMA5,			   sizeof(u8)						 },
	{ &register_SVBK,			   sizeof(u8)						 },
	{ &register_IE,				   sizeof(u8)						 },
	{ &gbBgp[0],				   sizeof(u8)						 },
	{ &gbBgp[1],				   sizeof(u8)						 },
	{ &gbBgp[2],				   sizeof(u8)						 },
	{ &gbBgp[3],				   sizeof(u8)						 },
	{ &gbObp0[0],				   sizeof(u8)						 },
	{ &gbObp0[1],				   sizeof(u8)						 },
	{ &gbObp0[2],				   sizeof(u8)						 },
	{ &gbObp0[3],				   sizeof(u8)						 },
	{ &gbObp1[0],				   sizeof(u8)						 },
	{ &gbObp1[1],				   sizeof(u8)						 },
	{ &gbObp1[2],				   sizeof(u8)						 },
	{ &gbObp1[3],				   sizeof(u8)						 },
	{ NULL,						   0								 }
};

bool gbWriteSaveStateToStream(gzFile gzFile)
{
	utilWriteInt(gzFile, GBSAVE_GAME_VERSION);

	utilGzWrite(gzFile, &gbRom[0x134], 15);

	utilWriteData(gzFile, gbSaveGameStruct);

	utilGzWrite(gzFile, &IFF, 2);

	if (gbSgbMode)
	{
		gbSgbSaveGame(gzFile);
	}

	utilGzWrite(gzFile, &gbDataMBC1, sizeof(gbDataMBC1));
	utilGzWrite(gzFile, &gbDataMBC2, sizeof(gbDataMBC2));
	utilGzWrite(gzFile, &gbDataMBC3, sizeof(gbDataMBC3));
	utilGzWrite(gzFile, &gbDataMBC5, sizeof(gbDataMBC5));
	utilGzWrite(gzFile, &gbDataHuC1, sizeof(gbDataHuC1));
	utilGzWrite(gzFile, &gbDataHuC3, sizeof(gbDataHuC3));

	// yes, this definitely needs to be saved, or loading paused games will show a black screen
	// this is also necessary to be consistent with what the GBA saving does
	utilGzWrite(gzFile, pix, 4 * 257 * 226);

	utilGzWrite(gzFile, gbPalette, 128 * sizeof(u16));
	// todo: remove
	utilGzWrite(gzFile, gbPalette, 128 * sizeof(u16));

	utilGzWrite(gzFile, &gbMemory[0x8000], 0x8000);

	if (gbRamSize && gbRam)
	{
		utilGzWrite(gzFile, gbRam, gbRamSize);
	}

	if (gbCgbMode)
	{
		utilGzWrite(gzFile, gbVram, 0x4000);
		utilGzWrite(gzFile, gbWram, 0x8000);
	}

	gbSoundSaveGame(gzFile);
	gbCheatsSaveGame(gzFile);

	// new to re-recording version:
	{
		utilGzWrite(gzFile, &sensorX, sizeof(sensorX));
		utilGzWrite(gzFile, &sensorY, sizeof(sensorY));
		utilGzWrite(gzFile, gbJoymask, 4 * sizeof(*gbJoymask)); // this has to be saved or old input will incorrectly get
		                                                        // carried
		                                                        // back on loading a snapshot!

		bool8 movieActive = VBAMovieIsActive();
		utilGzWrite(gzFile, &movieActive, sizeof(movieActive));
		if (movieActive)
		{
			uint8 *movie_freeze_buf	 = NULL;
			uint32 movie_freeze_size = 0;

			int code = VBAMovieFreeze(&movie_freeze_buf, &movie_freeze_size);
			if (movie_freeze_buf)
			{
				utilGzWrite(gzFile, &movie_freeze_size, sizeof(movie_freeze_size));
				utilGzWrite(gzFile, movie_freeze_buf, movie_freeze_size);
				delete [] movie_freeze_buf;
			}
			else
			{
				if (code == MOVIE_UNRECORDED_INPUT)
				{
					systemMessage(0, N_("Cannot make a movie snapshot as long as there are unrecorded input changes."));
				}
				else
				{
					systemMessage(0, N_("Failed to save movie snapshot."));
				}
				return false;
			}
		}
		utilGzWrite(gzFile, &systemCounters.frameCount, sizeof(systemCounters.frameCount));
	}

	// new to rerecording 19.4 wip (svn r22+):
	{
		utilGzWrite(gzFile, &systemCounters.lagCount, sizeof(systemCounters.lagCount));
		utilGzWrite(gzFile, &systemCounters.lagged, sizeof(systemCounters.lagged));
		utilGzWrite(gzFile, &systemCounters.laggedLast, sizeof(systemCounters.laggedLast));
	}

	return true;
}

bool gbWriteMemSaveState(char *memory, int available)
{
	gzFile gzFile = utilMemGzOpen(memory, available, "w");
	if (gzFile == NULL)
	{
		return false;
	}

	bool res = gbWriteSaveStateToStream(gzFile);

	long pos = utilGzTell(gzFile) + 8;

	if (pos >= (available))
		res = false;

	utilGzClose(gzFile);

	return res;
}

bool gbWriteSaveState(const char *name)
{
	gzFile gzFile = utilGzOpen(name, "wb");

	if (gzFile == NULL)
		return false;

	bool res = gbWriteSaveStateToStream(gzFile);

	utilGzClose(gzFile);
	return res;
}

bool gbReadSaveStateFromStream(gzFile gzFile)
{
	char tempBackupName[128];
	if (tempSaveSafe)
	{
		sprintf(tempBackupName, "gbtempsave%d.sav", tempSaveID++);
		gbWriteSaveState(tempBackupName);
	}

	int version = utilReadInt(gzFile);
	if (version > GBSAVE_GAME_VERSION || version < 0)
	{
		systemMessage(MSG_UNSUPPORTED_VB_SGM,
		              N_("Unsupported VisualBoy save game version %d"), version);
		goto failedLoadGB;
	}

	u8 romname[16];
	utilGzRead(gzFile, romname, 15);
	if (memcmp(&gbRom[0x134], romname, 15) != 0)
	{
		u8 curname[16];
		for (int i = 0; i < 15; ++i)
		{
			if (!romname[i])
			{
				romname[i] = ' ';
			}
			u8 c = gbRom[0x134 + i];
			curname[i] = c ? c : ' ';
		}
		romname[15] = 0;
		curname[15] = 0;
		systemMessage(MSG_CANNOT_LOAD_SGM_FOR,
		              N_("Cannot load save game for '%s'\nPlaying '%s'"),
		              romname, curname);
		goto failedLoadGB;
	}

	utilReadData(gzFile, gbSaveGameStruct);

	if (version >= GBSAVE_GAME_VERSION_7)
	{
		utilGzRead(gzFile, &IFF, 2);
	}

	if (gbSgbMode)
	{
		gbSgbReadGame(gzFile, version);
	}
	else
	{
		gbSgbMask = 0; // loading a game at the wrong time causes no display
	}

	utilGzRead(gzFile, &gbDataMBC1, sizeof(gbDataMBC1));
	utilGzRead(gzFile, &gbDataMBC2, sizeof(gbDataMBC2));
	if (version < GBSAVE_GAME_VERSION_4)
		// prior to version 4, there was no adjustment for the time the game
		// was last played, so we have less to read. This needs update if the
		// structure changes again.
		utilGzRead(gzFile, &gbDataMBC3, sizeof(int32) * 10);
	else
	{
		utilGzRead(gzFile, &gbDataMBC3, sizeof(gbDataMBC3));
	}
	utilGzRead(gzFile, &gbDataMBC5, sizeof(gbDataMBC5));
	utilGzRead(gzFile, &gbDataHuC1, sizeof(gbDataHuC1));
	utilGzRead(gzFile, &gbDataHuC3, sizeof(gbDataHuC3));

	if (version < GBSAVE_GAME_VERSION_5)
	{
		utilGzRead(gzFile, pix, 256 * 224 * sizeof(u16));
	}
	if (version >= GBSAVE_GAME_VERSION_12)
	{
		utilGzRead(gzFile, pix, 4 * 257 * 226);
	}
	else
	{
		memset(pix, 0, 257 * 226 * sizeof(u32));
//		if(version < GBSAVE_GAME_VERSION_5)
//			utilGzRead(gzFile, pix, 256*224*sizeof(u16));
	}

	if (version < GBSAVE_GAME_VERSION_6)
	{
		utilGzRead(gzFile, gbPalette, 64 * sizeof(u16));
	}
	else
		utilGzRead(gzFile, gbPalette, 128 * sizeof(u16));

	// todo: remove
	utilGzRead(gzFile, gbPalette, 128 * sizeof(u16));

	if (version < GBSAVE_GAME_VERSION_10)
	{
		if (!gbCgbMode && !gbSgbMode)
		{
			for (int i = 0; i < 8; i++)
				gbPalette[i] = systemGbPalette[gbPaletteOption * 8 + i];
		}
	}

	utilGzRead(gzFile, &gbMemory[0x8000], 0x8000);

	if (gbRamSize && gbRam)
	{
		utilGzRead(gzFile, gbRam, gbRamSize);
	}

	gbMemoryMap[0x00] = &gbRom[0x0000];
	gbMemoryMap[0x01] = &gbRom[0x1000];
	gbMemoryMap[0x02] = &gbRom[0x2000];
	gbMemoryMap[0x03] = &gbRom[0x3000];
	gbMemoryMap[0x04] = &gbRom[0x4000];
	gbMemoryMap[0x05] = &gbRom[0x5000];
	gbMemoryMap[0x06] = &gbRom[0x6000];
	gbMemoryMap[0x07] = &gbRom[0x7000];
	gbMemoryMap[0x08] = &gbMemory[0x8000];
	gbMemoryMap[0x09] = &gbMemory[0x9000];
	gbMemoryMap[0x0a] = &gbMemory[0xa000];
	gbMemoryMap[0x0b] = &gbMemory[0xb000];
	gbMemoryMap[0x0c] = &gbMemory[0xc000];
	gbMemoryMap[0x0d] = &gbMemory[0xd000];
	gbMemoryMap[0x0e] = &gbMemory[0xe000];
	gbMemoryMap[0x0f] = &gbMemory[0xf000];

	u8 type = gbRom[0x147];

	switch (type)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		// MBC 1
		memoryUpdateMapMBC1();
		break;
	case 0x05:
	case 0x06:
		// MBC2
		memoryUpdateMapMBC2();
		break;
	case 0x0f:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		// MBC 3
		memoryUpdateMapMBC3();
		break;
	case 0x19:
	case 0x1a:
	case 0x1b:
		// MBC5
		memoryUpdateMapMBC5();
		break;
	case 0x1c:
	case 0x1d:
	case 0x1e:
		// MBC 5 Rumble
		memoryUpdateMapMBC5();
		break;
	case 0x22:
		// MBC 7
		memoryUpdateMapMBC7();
		break;
	case 0xfe:
		// HuC3
		memoryUpdateMapHuC3();
		break;
	case 0xff:
		// HuC1
		memoryUpdateMapHuC1();
		break;
	}

	if (gbCgbMode)
	{
		if (!gbVram)
			gbVram = (u8 *)RAM_MALLOC(0x4000);
		if (!gbWram)
			gbWram = (u8 *)RAM_MALLOC(0x8000);
		utilGzRead(gzFile, gbVram, 0x4000);
		utilGzRead(gzFile, gbWram, 0x8000);

		int value = register_SVBK;
		if (value == 0)
			value = 1;

		gbMemoryMap[0x08] = &gbVram[register_VBK * 0x2000];
		gbMemoryMap[0x09] = &gbVram[register_VBK * 0x2000 + 0x1000];
		gbMemoryMap[0x0d] = &gbWram[value * 0x1000];
	}
	else
	{
		free(gbVram);
		gbVram = NULL;
		free(gbWram);
		gbWram = NULL;
	}

	gbSoundReadGame(version, gzFile);

#if 0
	if (gbBorderOn)
	{
		gbSgbRenderBorder();
	}

	systemRefreshScreen();
#endif

	if (version > GBSAVE_GAME_VERSION_1)
		gbCheatsReadGame(gzFile, version);

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	bool wasPlayingMovie = VBAMovieIsActive() && VBAMovieIsPlaying();
	if (version >= GBSAVE_GAME_VERSION_11) // new to re-recording version:
	{
		utilGzRead(gzFile, &sensorX, sizeof(sensorX));
		utilGzRead(gzFile, &sensorY, sizeof(sensorY));
		utilGzRead(gzFile, gbJoymask, 4 * sizeof(*gbJoymask)); // this has to be saved or old input will incorrectly get carried
		                                                       // back on loading a snapshot!

		bool8 movieSnapshot;
		utilGzRead(gzFile, &movieSnapshot, sizeof(movieSnapshot));
		if (VBAMovieIsActive() && !movieSnapshot)
		{
			systemMessage(0, N_("Can't load a non-movie snapshot while a movie is active."));
			goto failedLoadGB;
		}

		if (movieSnapshot) // even if a movie isn't active we still want to parse through this
						   // because we have already got stuff added in this save format
		{
			uint32 movieInputDataSize = 0;
			utilGzRead(gzFile, &movieInputDataSize, sizeof(movieInputDataSize));
			uint8 *local_movie_data = new uint8 [movieInputDataSize];
			int	   readBytes		= utilGzRead(gzFile, local_movie_data, movieInputDataSize);
			if (readBytes != movieInputDataSize)
			{
				systemMessage(0, N_("Corrupt movie snapshot."));
				delete [] local_movie_data;
				goto failedLoadGB;
			}
			int code = VBAMovieUnfreeze(local_movie_data, movieInputDataSize);
			delete [] local_movie_data;
			if (code != MOVIE_SUCCESS && VBAMovieIsActive())
			{
				char errStr [1024];
				strcpy(errStr, "Failed to load movie snapshot");
				switch (code)
				{
				case MOVIE_NOT_FROM_THIS_MOVIE:
					strcat(errStr, ";\nSnapshot not from this movie"); break;
				case MOVIE_NOT_FROM_A_MOVIE:
					strcat(errStr, ";\nNot a movie snapshot"); break;                    // shouldn't get here...
				case MOVIE_UNVERIFIABLE_POST_END:
					sprintf(errStr, "%s;\nSnapshot unverifiable with movie after End Frame %u", errStr, VBAMovieGetLastErrorInfo()); break;
				case MOVIE_TIMELINE_INCONSISTENT_AT:
					sprintf(errStr, "%s;\nSnapshot inconsistent with movie at Frame %u", errStr, VBAMovieGetLastErrorInfo()); break;
				case MOVIE_WRONG_FORMAT:
					strcat(errStr, ";\nWrong format"); break;
				}
				strcat(errStr, ".");
				systemMessage(0, N_(errStr));
				goto failedLoadGB;
			}
		}
		utilGzRead(gzFile, &systemCounters.frameCount, sizeof(systemCounters.frameCount));
	}

	if (version >= GBSAVE_GAME_VERSION_13)   // new to rerecording 19.4 wip (svn r22+):
	{
		utilGzRead(gzFile, &systemCounters.lagCount, sizeof(systemCounters.lagCount));
		utilGzRead(gzFile, &systemCounters.lagged, sizeof(systemCounters.lagged));
		utilGzRead(gzFile, &systemCounters.laggedLast, sizeof(systemCounters.laggedLast));
	}

	for (int i = 0; i < 4; ++i)
		systemSetJoypad(i, gbJoymask[i] & 0xFFFF);

	// FIXME: horrible kludge
	memcpy(s_gbJoymask, gbJoymask, sizeof(gbJoymask));

	VBAUpdateButtonPressDisplay();
	VBAUpdateFrameCountDisplay();
	systemRefreshScreen();

	if (tempSaveSafe)
	{
		remove(tempBackupName);
		tempSaveAttempts = 0;
	}
	return true;

failedLoadGB:
	if (tempSaveSafe)
	{
		tempSaveAttempts++;
		if (tempSaveAttempts < 3) // fail no more than 2 times in a row
			gbReadSaveState(tempBackupName);
		remove(tempBackupName);
	}
	if (wasPlayingMovie && VBAMovieIsRecording())
	{
		VBAMovieSwitchToPlaying();
	}
	return false;
}

bool gbReadMemSaveState(char *memory, int available)
{
	gzFile gzFile = utilMemGzOpen(memory, available, "r");

	tempSaveSafe = false;
	bool res = gbReadSaveStateFromStream(gzFile);
	tempSaveSafe = true;

	utilGzClose(gzFile);

	return res;
}

bool gbReadSaveState(const char *name)
{
	gzFile gzFile = utilGzOpen(name, "rb");

	if (gzFile == NULL)
	{
		return false;
	}

	bool res = gbReadSaveStateFromStream(gzFile);

	utilGzClose(gzFile);

	return res;
}

bool gbWritePNGFile(const char *fileName)
{
	if (gbBorderOn)
		return utilWritePNGFile(fileName, 256, 224, pix);
	return utilWritePNGFile(fileName, 160, 144, pix);
}

bool gbWriteBMPFile(const char *fileName)
{
	if (gbBorderOn)
		return utilWriteBMPFile(fileName, 256, 224, pix);
	return utilWriteBMPFile(fileName, 160, 144, pix);
}

void gbCleanUp()
{
	PIX_FREE(pix);
	pix = NULL;

	free(bios);
	bios = NULL;

	free(gbRam);
	gbRam = NULL;

	free(gbRom);
	gbRom = NULL;

	free(gbMemory);
	gbMemory = NULL;

	free(gbVram);
	gbVram = NULL;

	free(gbWram);
	gbWram = NULL;

	gbSgbShutdown();

	free(gbLineBuffer);
	gbLineBuffer = NULL;

	memset(gbJoymask, 0, sizeof(gbJoymask));
	// FIXME: horrible kludge
	memset(s_gbJoymask, 0, sizeof(s_gbJoymask));

	systemCleanUp();
	systemRefreshScreen();
}

bool gbLoadRom(const char *szFile)
{
	int size = 0;

	if (gbRom != NULL)
	{
		gbCleanUp();
	}

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	gbRom = utilLoad(szFile,
	                 utilIsGBImage,
	                 NULL,
	                 size);
	if (!gbRom)
		return false;

	gbRomSize = size;

	return gbUpdateSizes();
}

bool gbUpdateSizes()
{
	if (gbRom[0x148] > 8)
	{
		systemMessage(MSG_UNSUPPORTED_ROM_SIZE,
		              N_("Unsupported rom size %02x"), gbRom[0x148]);
		return false;
	}

	if (gbRomSize < gbRomSizes[gbRom[0x148]])
	{
		void *tmp = realloc(gbRom, gbRomSizes[gbRom[0x148]]);
		if (!tmp) free(gbRom);
		gbRom = (u8 *)tmp;
	}
	gbRomSize	  = gbRomSizes[gbRom[0x148]];
	gbRomSizeMask = gbRomSizesMasks[gbRom[0x148]];

	if (gbRom[0x149] > 5)
	{
		systemMessage(MSG_UNSUPPORTED_RAM_SIZE,
		              N_("Unsupported ram size %02x"), gbRom[0x149]);
		return false;
	}

	gbRamSize	  = gbRamSizes[gbRom[0x149]];
	gbRamSizeMask = gbRamSizesMasks[gbRom[0x149]];

	if (gbRamSize)
	{
		gbRam = (u8 *)RAM_MALLOC(gbRamSize);
		memset(gbRam, 0xFF, gbRamSize);
	}

	int type = gbRom[0x147];

	mapperReadRAM = NULL;

	switch (type)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		// MBC 1
		mapper	  = mapperMBC1ROM;
		mapperRAM = mapperMBC1RAM;
		break;
	case 0x05:
	case 0x06:
		// MBC2
		mapper		  = mapperMBC2ROM;
		mapperRAM	  = mapperMBC2RAM;
		gbRamSize	  = 0x200;
		gbRamSizeMask = 0x1ff;
		break;
	case 0x0f:
	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
		// MBC 3
		mapper		  = mapperMBC3ROM;
		mapperRAM	  = mapperMBC3RAM;
		mapperReadRAM = mapperMBC3ReadRAM;
		break;
	case 0x19:
	case 0x1a:
	case 0x1b:
		// MBC5
		mapper	  = mapperMBC5ROM;
		mapperRAM = mapperMBC5RAM;
		break;
	case 0x1c:
	case 0x1d:
	case 0x1e:
		// MBC 5 Rumble
		mapper	  = mapperMBC5ROM;
		mapperRAM = mapperMBC5RAM;
		break;
	case 0x22:
		// MBC 7
		mapper		  = mapperMBC7ROM;
		mapperRAM	  = mapperMBC7RAM;
		mapperReadRAM = mapperMBC7ReadRAM;
		break;
	case 0xfe:
		// HuC3
		mapper		  = mapperHuC3ROM;
		mapperRAM	  = mapperHuC3RAM;
		mapperReadRAM = mapperHuC3ReadRAM;
		break;
	case 0xff:
		// HuC1
		mapper	  = mapperHuC1ROM;
		mapperRAM = mapperHuC1RAM;
		break;
	default:
		systemMessage(MSG_UNKNOWN_CARTRIDGE_TYPE,
		              N_("Unknown cartridge type %02x"), type);
		return false;
	}

	switch (type)
	{
	case 0x03:
	case 0x06:
	case 0x0f:
	case 0x10:
	case 0x13:
	case 0x1b:
	case 0x1d:
	case 0x1e:
	case 0x22:
	case 0xff:
		gbBattery = 1;
		break;
	}

	gbInit();
	gbReset();

	return true;
}

static void gbDrawPixLine()
{
	switch (systemColorDepth)
	{
	case 16:
	{
		u16 *dest = (u16 *)pix +
		            (gbBorderLineSkip + 2) * (register_LY + gbBorderRowSkip + 1)
		            + gbBorderColumnSkip;
		for (int x = 0; x < 160; )
		{
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];

			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];

			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];

			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
			*dest++ = systemColorMap16[gbLineMix[x++]];
		}
		if (gbBorderOn)
			dest += gbBorderColumnSkip;
		*dest++ = 0; // for filters that read one pixel more
		break;
	}
	case 24:
	{
		u8 *dest = (u8 *)pix +
		           3 * (gbBorderLineSkip * (register_LY + gbBorderRowSkip) +
		                gbBorderColumnSkip);
		for (int x = 0; x < 160; )
		{
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;

			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;

			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;

			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
			*((u32 *)dest) = systemColorMap32[gbLineMix[x++]];
			dest += 3;
		}
		break;
	}
	case 32:
	{
		u32 *dest = (u32 *)pix +
		            (gbBorderLineSkip + 1) * (register_LY + gbBorderRowSkip + 1)
		            + gbBorderColumnSkip;
		for (int x = 0; x < 160; )
		{
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];

			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];

			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];

			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
			*dest++ = systemColorMap32[gbLineMix[x++]];
		}
		break;
	}
	}
}

static inline void gbGetUserInput()
{
	// update joystick information
	systemReadJoypads();

	bool sensor = (gbRom[0x147] == 0x22);
	if (gbSgbMode && gbSgbMultiplayer)
	{
		if (gbSgbFourPlayers)
		{
			gbJoymask[0] = systemGetJoypad(0, sensor);
			gbJoymask[1] = systemGetJoypad(1, false);
			gbJoymask[2] = systemGetJoypad(2, false);
			gbJoymask[3] = systemGetJoypad(3, false);
		}
		else
		{
			gbJoymask[0] = systemGetJoypad(0, sensor);
			gbJoymask[1] = systemGetJoypad(1, false);
		}
	}
	else
	{
		gbJoymask[0] = systemGetJoypad(0, sensor);
	}

	// FIXME: horrible kludge
	memcpy(s_gbJoymask, gbJoymask, sizeof(gbJoymask));

	if (gbJoymask[0] & 0xFF)
	{
		gbMemory[0xff0f] |= 16;
	}

	extButtons = (gbJoymask[0] >> 18);
	speedup	   = (extButtons & 1) != 0;
}

static inline void gbBeforeEmulation()
{
	if (newFrame)
	{
		CallRegisteredLuaFunctions(LUACALL_BEFOREEMULATION);

		gbGetUserInput();

		if (gbRom[0x147] == 0x22)
		{
			//systemUpdateMotionSensor();
		}

		VBAMovieResetIfRequested();

		newFrame = false;
	}
}

static inline void gbFrameBoundaryWork()
{
	if (!gbSgbMask)
	{
		if (gbBorderOn)
			gbSgbRenderBorder();
	}

	systemFrameBoundaryWork();
}

void gbEmulate(int ticksToStop)
{
	gbBeforeEmulation();

	gbRegister tempRegister;
	u8		   tempValue;
	s8		   offset;

	int gbClockTicks = 0;
	gbDmaTicks = 0;
	bool newVideoFrame = false;

	for (;;)
	{
#ifndef FINAL_VERSION
		if (systemDebug)
		{
			if (!(IFF & 0x80))
			{
				if (systemDebug > 1)
				{
					log("PC=%04x AF=%04x BC=%04x DE=%04x HL=%04x SP=%04x I=%04x\n",
					    PC.W, AF.W, BC.W, DE.W, HL.W, SP.W, IFF);
				}
				else
				{
					log("PC=%04x I=%02x\n", PC.W, IFF);
				}
			}
		}
#endif
		if (IFF & 0x80)
		{
			if (register_LCDC & 0x80)
			{
				gbClockTicks = gbLcdTicks;
			}
			else
				gbClockTicks = 100;

			if (gbLcdMode == 1 && (gbLcdLYIncrementTicks < gbClockTicks))
				gbClockTicks = gbLcdLYIncrementTicks;

			if (gbSerialOn && (gbSerialTicks < gbClockTicks))
				gbClockTicks = gbSerialTicks;

			if (gbTimerOn && (gbTimerTicks < gbClockTicks))
				gbClockTicks = gbTimerTicks;

			if (soundTicks && (soundTicks < gbClockTicks))
				gbClockTicks = soundTicks;
		}
		else
		{
			register int opcode = gbReadOpcode(PC.W);
			CallRegisteredLuaMemHook(PC.W, 1, opcode, LUAMEMHOOK_EXEC);
			PC.W++;

			if (IFF & 0x100)
			{
				IFF &= 0xff;
				PC.W--;
			}

			gbClockTicks = gbCycles[opcode];

#include "gbCodes.h"
		}

		if (!emulating)
			break;

		if (gbDmaTicks)
		{
			gbClockTicks += gbDmaTicks;
			gbDmaTicks	= 0;
		}

		if (gbSgbMode)
		{
			if (gbSgbPacketTimeout)
			{
				gbSgbPacketTimeout -= gbClockTicks;

				if (gbSgbPacketTimeout <= 0)
					gbSgbResetPacketState();
			}
		}

		ticksToStop -= gbClockTicks;

		// DIV register emulation
		gbDivTicks -= gbClockTicks;
		while (gbDivTicks <= 0)
		{
			register_DIV++;
			gbDivTicks += GBDIV_CLOCK_TICKS;
		}

		if (register_LCDC & 0x80)
		{
			// LCD stuff
			gbLcdTicks -= gbClockTicks;
			if (gbLcdMode == 1)
			{
				// during V-BLANK,we need to increment LY at the same rate!
				gbLcdLYIncrementTicks -= gbClockTicks;
				while (gbLcdLYIncrementTicks <= 0)
				{
					gbLcdLYIncrementTicks += GBLY_INCREMENT_CLOCK_TICKS;

					if (register_LY < 153)
					{
						register_LY++;

						gbCompareLYToLYC();

						if (register_LY >= 153)
							gbLcdLYIncrementTicks = 6;
					}
					else
					{
						register_LY = 0x00;
						// reset the window line
						gbWindowLine = -1;
						gbLcdLYIncrementTicks = GBLY_INCREMENT_CLOCK_TICKS * 2;
						gbCompareLYToLYC();
					}
				}
			}

			// our counter is off, see what we need to do
			while (gbLcdTicks <= 0)
			{
				int framesToSkip = systemFramesToSkip();

				switch (gbLcdMode)
				{
				case 0:
					// H-Blank
					register_LY++;

					gbCompareLYToLYC();

					// check if we reached the V-Blank period
					if (register_LY == 144)
					{
						// Yes, V-Blank
						// set the LY increment counter
						gbLcdLYIncrementTicks = gbLcdTicks + GBLY_INCREMENT_CLOCK_TICKS;
						gbLcdTicks += GBLCD_MODE_1_CLOCK_TICKS;
						gbLcdMode	= 1;
						if (register_LCDC & 0x80)
						{
							gbInterrupt	   |= 1; // V-Blank interrupt
							gbInterruptWait = 6;
							if (register_STAT & 0x10)
								gbInterrupt |= 2;
						}

						newVideoFrame = true;
					}
					else
					{
						// go the the OAM being accessed mode
						gbLcdTicks += GBLCD_MODE_2_CLOCK_TICKS;
						gbLcdMode	= 2;

						// only one LCD interrupt per line. may need to generalize...
						if (!(register_STAT & 0x40) ||
						    (register_LY != register_LYC))
						{
							if ((register_STAT & 0x28) == 0x20)
								gbInterrupt |= 2;
						}
					}

					break;
				case 1:
					// V-Blank
					// next mode is OAM being accessed mode
					gbLcdTicks += GBLCD_MODE_2_CLOCK_TICKS;
					gbLcdMode	= 2;
					if (!(register_STAT & 0x40) ||
					    (register_LY != register_LYC))
					{
						if ((register_STAT & 0x28) == 0x20)
							gbInterrupt |= 2;
					}
					break;
				case 2:
					// OAM being accessed mode

					// next mode is OAM and VRAM in use
					gbLcdTicks += GBLCD_MODE_3_CLOCK_TICKS;
					gbLcdMode	= 3;
					break;
				case 3:
					// OAM and VRAM in use
					// next mode is H-Blank
					if (register_LY < 144)
					{
						if (!gbSgbMask)
						{
							if (systemFrameDrawingRequired())
							{
								gbRenderLine();
								gbDrawSprites();

								gbDrawPixLine();
							}
						}
					}
					gbLcdTicks += GBLCD_MODE_0_CLOCK_TICKS;
					gbLcdMode	= 0;
					// only one LCD interrupt per line. may need to generalize...
					if (!(register_STAT & 0x40) ||
					    (register_LY != register_LYC))
					{
						if (register_STAT & 0x08)
							gbInterrupt |= 2;
					}
					if (gbHdmaOn)
					{
						gbDoHdma();
					}
					break;
				}
				// mark the correct lcd mode on STAT register
				register_STAT = (register_STAT & 0xfc) | gbLcdMode;
			}
		}

		// serial emulation
		if (gbSerialOn)
		{
#ifdef OLD_GB_LINK
			if (linkConnected)
			{
				gbSerialTicks -= gbClockTicks;

				while (gbSerialTicks <= 0)
				{
					// increment number of shifted bits
					gbSerialBits++;
					linkProc();
					if (gbSerialOn && (gbMemory[0xff02] & 1))
					{
						if (gbSerialBits == 8)
						{
							gbSerialBits	  = 0;
							gbMemory[0xff01]  = 0xff;
							gbMemory[0xff02] &= 0x7f;
							gbSerialOn		  = 0;
							gbInterrupt		 |= 8;
							gbSerialTicks	  = 0;
						}
					}
					gbSerialTicks += GBSERIAL_CLOCK_TICKS;
				}
			}
			else
			{
#endif
			if (gbMemory[0xff02] & 1)
			{
				gbSerialTicks -= gbClockTicks;

				// overflow
				while (gbSerialTicks <= 0)
				{
					// shift serial byte to right and put a 1 bit in its place
					//      gbMemory[0xff01] = 0x80 | (gbMemory[0xff01]>>1);
					// increment number of shifted bits
					gbSerialBits++;
					if (gbSerialBits == 8)
					{
						// end of transmission
						if (gbSerialFunction) // external device
							gbMemory[0xff01] = gbSerialFunction(gbMemory[0xff01]);
						else
							gbMemory[0xff01] = 0xff;
						gbSerialTicks	  = 0;
						gbMemory[0xff02] &= 0x7f;
						gbSerialOn		  = 0;
						gbInterrupt		 |= 8;
						gbSerialBits	  = 0;
					}
					else
						gbSerialTicks += GBSERIAL_CLOCK_TICKS;
				}
			}
#ifdef OLD_GB_LINK
		}
#endif
		}

		soundTicks -= gbClockTicks;
		while (soundTicks < 0)
		{
			gbSoundTick();
			soundTicks += soundTickStep;
		}

		// timer emulation
		if (gbTimerOn)
		{
			gbTimerTicks -= gbClockTicks;

			while (gbTimerTicks <= 0)
			{
				register_TIMA++;

				if (register_TIMA == 0)
				{
					// timer overflow!

					// reload timer modulo
					register_TIMA = register_TMA;

					// flag interrupt
					gbInterrupt |= 4;
				}

				gbTimerTicks += gbTimerClockTicks;
			}
		}

		/*
		   if(soundOffFlag)
		   {
		   if(synchronize && !speedup)
		   {
		   synchronizeTicks -= gbClockTicks;

		   while(synchronizeTicks < 0)
		   {
		   synchronizeTicks += SYNCHRONIZE_CLOCK_TICKS;

		   DWORD now = timeGetTime();
		   gbElapsedTime += (now - timeNow);

		   if(gbElapsedTime < 50)
		   {
		   DWORD diff = 50 - gbElapsedTime;
		   Sleep(diff);
		   timeNow = timeGetTime();
		   elapsedTime = timeNow - now - diff;
		   if((int)elapsedTime < 0)
		   elapsedTime = 0;
		   } else
		   {
		   timeNow = timeGetTime();
		   elapsedTime = 0;
		   }
		   }
		   }
		   }
		 */

		register_IF = gbInterrupt;

		if (IFF & 0x20)
		{
			IFF &= 0xdf;
			IFF |= 0x01;
			gbInterruptWait = 0;
		}
		else if (gbInterrupt)
		{
			if (gbInterruptWait == 0)
			{
				//        gbInterruptWait = 0;

				if (IFF & 0x01)
				{
					if ((gbInterrupt & 1) && (register_IE & 1))
					{
						gbVblank_interrupt();
						continue;
					}

					if ((gbInterrupt & 2) && (register_IE & 2))
					{
						gbLcd_interrupt();
						continue;
					}

					if ((gbInterrupt & 4) && (register_IE & 4))
					{
						gbTimer_interrupt();
						continue;
					}

					if ((gbInterrupt & 8) && (register_IE & 8))
					{
						gbSerial_interrupt();
						continue;
					}

					if ((gbInterrupt & 16) && (register_IE & 16))
					{
						gbJoypad_interrupt();
						continue;
					}
				}
			}
			else
			{
				gbInterruptWait -= gbClockTicks;
				if (gbInterruptWait < 0)
					gbInterruptWait = 0;
			}
		}

		if (newVideoFrame)
		{
			newVideoFrame = false;
			gbFrameBoundaryWork();
		}

		if (useOldFrameTiming)
		{
			// old timing code
			if (ticksToStop > 0)
				continue;

			break;
		}
		else
		{
			if (!(register_LCDC & 0x80))
			{
				// FIXME: since register_LY can be reset to 0 by some games, frame length is variable
				// and infinite loops can occurr
				// for now, it IS necessary to do something on this condition or games like
				// Megaman would freeze upon low-level restart interrupt sequence (Start+Select+A+B).
				// the only sensible way to fix this issue is to implement the RIGHT frame timing
#ifdef WANTS_INCOMPLETE_WORKAROUND
				if (systemReadJoypads())
				{
					if (gbSgbMode && gbSgbMultiplayer)
					{
						if (gbSgbFourPlayers)
						{
							gbJoymask[0] = systemGetJoypad(0, false);
							gbJoymask[1] = systemGetJoypad(1, false);
							gbJoymask[2] = systemGetJoypad(2, false);
							gbJoymask[3] = systemGetJoypad(3, false);
						}
						else
						{
							gbJoymask[0] = systemGetJoypad(0, false);
							gbJoymask[1] = systemGetJoypad(1, false);
						}
					}
					else
					{
						gbJoymask[0] = systemGetJoypad(0, false);
					}
				}
				else
				{
					gbJoymask[0] = gbJoymask[1] = gbJoymask[2] = gbJoymask[3] = 0;
				}
#endif
			}
		}

		// makes sure frames are really divided across input sampling boundaries which occur at a constant rate
		if (newFrame)
		{
			break;
		}
	}
}

struct EmulatedSystem GBSystem =
{
	// emuMain
	gbEmulate,
	// emuReset
	gbReset,
	// emuCleanUp
	gbCleanUp,
	// emuReadBattery
	gbReadBatteryFile,
	// emuWriteBattery
	gbWriteBatteryFile,
	// emuReadBatteryFromStream
	gbReadBatteryFromStream,
	// emuWriteBatteryToStream
	gbWriteBatteryToStream,
	// emuReadState
	gbReadSaveState,
	// emuWriteState
	gbWriteSaveState,
	// emuReadStateFromStream
	gbReadSaveStateFromStream,
	// emuWriteStateToStream
	gbWriteSaveStateToStream,
	// emuReadMemState
	gbReadMemSaveState,
	// emuWriteMemState
	gbWriteMemSaveState,
	// emuWritePNG
	gbWritePNGFile,
	// emuWriteBMP
	gbWriteBMPFile,
	// emuUpdateCPSR
	NULL,
	// emuHasDebugger
	false,
	// emuCount
#ifdef FINAL_VERSION
	70000 / 4,
#else
	7000,
#endif
};
