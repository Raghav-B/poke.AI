#ifndef VBA_GB_MEMORY_H
#define VBA_GB_MEMORY_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../Port.h"

struct mapperMBC1
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperMemoryModel;
	int32 mapperROMHighAddress;
	int32 mapperRAMAddress;
#ifndef USE_GB_CORE_V7
	int32 mapperRomBank0Remapping;
#endif
};

struct mapperMBC2
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
};

struct mapperMBC3
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperRAMAddress;
	int32 mapperClockLatch;
	int32 mapperClockRegister;
	int32 mapperSeconds;
	int32 mapperMinutes;
	int32 mapperHours;
	int32 mapperDays;
	int32 mapperControl;
	int32 mapperLSeconds;
	int32 mapperLMinutes;
	int32 mapperLHours;
	int32 mapperLDays;
	int32 mapperLControl;
	u32	  mapperLastTime;
};

struct mapperMBC5
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperROMHighAddress;
	int32 mapperRAMAddress;
	int32 isRumbleCartridge;
};

struct mapperMBC7
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperRAMAddress;
	int32 cs;
	int32 sk;
	int32 state;
	int32 buffer;
	int32 idle;
	int32 count;
	int32 code;
	int32 address;
	int32 writeEnable;
	int32 value;
};

struct mapperHuC1
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperMemoryModel;
	int32 mapperROMHighAddress;
	int32 mapperRAMAddress;
};

struct mapperHuC3
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperRAMAddress;
	int32 mapperAddress;
	int32 mapperRAMFlag;
	int32 mapperRAMValue;
	int32 mapperRegister1;
	int32 mapperRegister2;
	int32 mapperRegister3;
	int32 mapperRegister4;
	int32 mapperRegister5;
	int32 mapperRegister6;
	int32 mapperRegister7;
	int32 mapperRegister8;
};

struct mapperTAMA5
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperRAMAddress;
	int32 mapperRamByteSelect;
	int32 mapperCommandNumber;
	int32 mapperLastCommandNumber;
	int32 mapperCommands[0x10];
	int32 mapperRegister;
	int32 mapperClockLatch;
	int32 mapperClockRegister;
	int32 mapperSeconds;
	int32 mapperMinutes;
	int32 mapperHours;
	int32 mapperDays;
	int32 mapperMonths;
	int32 mapperYears;
	int32 mapperControl;
	int32 mapperLSeconds;
	int32 mapperLMinutes;
	int32 mapperLHours;
	int32 mapperLDays;
	int32 mapperLMonths;
	int32 mapperLYears;
	int32 mapperLControl;
	u32	  mapperLastTime;
};

struct mapperMMM01
{
	int32 mapperRAMEnable;
	int32 mapperROMBank;
	int32 mapperRAMBank;
	int32 mapperMemoryModel;
	int32 mapperROMHighAddress;
	int32 mapperRAMAddress;
	int32 mapperRomBank0Remapping;
};

struct mapperGS3
{
	int32 mapperROMBank;
};

extern mapperMBC1  gbDataMBC1;
extern mapperMBC2  gbDataMBC2;
extern mapperMBC3  gbDataMBC3;
extern mapperMBC5  gbDataMBC5;
extern mapperHuC1  gbDataHuC1;
extern mapperHuC3  gbDataHuC3;
extern mapperTAMA5 gbDataTAMA5;
extern mapperMMM01 gbDataMMM01;
extern mapperGS3   gbDataGS3;

void mapperMBC1ROM(u16, u8);
void mapperMBC1RAM(u16, u8);
u8	 mapperMBC1ReadRAM(u16);
void mapperMBC2ROM(u16, u8);
void mapperMBC2RAM(u16, u8);
void mapperMBC3ROM(u16, u8);
void mapperMBC3RAM(u16, u8);
u8	 mapperMBC3ReadRAM(u16);
void mapperMBC5ROM(u16, u8);
void mapperMBC5RAM(u16, u8);
u8	 mapperMBC5ReadRAM(u16);
void mapperMBC7ROM(u16, u8);
void mapperMBC7RAM(u16, u8);
u8	 mapperMBC7ReadRAM(u16);
void mapperHuC1ROM(u16, u8);
void mapperHuC1RAM(u16, u8);
void mapperHuC3ROM(u16, u8);
void mapperHuC3RAM(u16, u8);
u8	 mapperHuC3ReadRAM(u16);
void mapperTAMA5RAM(u16, u8);
u8	 mapperTAMA5ReadRAM(u16);
void memoryUpdateTAMA5Clock();
void mapperMMM01ROM(u16, u8);
void mapperMMM01RAM(u16, u8);
void mapperGGROM(u16, u8);
void mapperGS3ROM(u16, u8);

//extern void (*mapper)(u16,u8);
//extern void (*mapperRAM)(u16,u8);
//extern u8 (*mapperReadRAM)(u16);

void memoryUpdateMapMBC1();
void memoryUpdateMapMBC2();
void memoryUpdateMapMBC3();
void memoryUpdateMapMBC5();
void memoryUpdateMapMBC7();
void memoryUpdateMapHuC1();
void memoryUpdateMapHuC3();
void memoryUpdateMapTAMA5();
void memoryUpdateMapMMM01();
void memoryUpdateMapGS3();

#endif // VBA_GB_MEMORY
