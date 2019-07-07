#ifndef VBA_FLASH_H
#define VBA_FLASH_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zlib.h"
#include "../Port.h"

extern void flashSaveGame(gzFile file);
extern void flashReadGame(gzFile file, int version);
extern void flashReadGameSkip(gzFile file, int version);
extern u8	flashRead(u32 address);
extern void flashDelayedWrite(u32 address, u8 byte);
extern void flashWrite(u32 address, u8 byte);
extern void flashSaveDecide(u32 address, u8 byte);
extern void flashInit();
extern void flashReset();
extern void flashErase();
extern void flashSetSize(int32 size);

extern int32 flashSize;
extern u8	 flashSaveMemory[0x20000];

#endif // VBA_FLASH_H
