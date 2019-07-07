#ifndef VBA_EEPROM_H
#define VBA_EEPROM_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zlib.h"
#include "../Port.h"

extern void eepromSaveGame(gzFile file);
extern void eepromReadGame(gzFile file, int version);
extern void eepromReadGameSkip(gzFile file, int version);
extern int	eepromRead(u32 address);
extern void eepromWrite(u32 address, u8 value);
extern void eepromInit();
extern void eepromReset();
extern void eepromErase();
extern u8    eepromData[0x2000];
extern bool8 eepromInUse;
extern int32 eepromSize;

#define EEPROM_IDLE           0
#define EEPROM_READADDRESS    1
#define EEPROM_READDATA       2
#define EEPROM_READDATA2      3
#define EEPROM_WRITEDATA      4

#endif // VBA_EEPROM_H
