#ifndef VBA_UTIL_H
#define VBA_UTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zlib.h"
#include "../Port.h"

enum IMAGE_TYPE
{
	IMAGE_UNKNOWN = -1,
	IMAGE_GBA	  = 0,
	IMAGE_GB	  = 1
};

// save game

typedef struct
{
	void *address;
	int	  size;
} variable_desc;

extern void utilWriteBMP(u8 *out, int w, int h, int dstDepth, const u8 *in);
extern bool utilWriteBMPFile(const char *, int, int, const u8 *);
extern bool utilWritePNGFile(const char *, int, int, const u8 *);
extern void utilApplyIPS(const char *ips, u8 * *rom, int *size);
extern bool utilIsGBAImage(const char *);
extern bool utilIsGBABios(const char *file);
extern bool utilIsELF(const char *file);
extern bool utilIsGBImage(const char *);
extern bool utilIsGBBios(const char *file);
extern bool utilIsZipFile(const char *);
extern bool utilIsGzipFile(const char *);
extern bool utilIsRarFile(const char *);
extern void utilGetBaseName(const char *, char *);
extern IMAGE_TYPE utilFindType(const char *);
extern u8 *	 utilLoad(const char *, bool (*)(const char *), u8 *, int &);
extern void	 utilPutDword(u8 *, u32);
extern void	 utilPutWord(u8 *, u16);
extern void	 utilWriteData(gzFile, variable_desc *);
extern void	 utilReadData(gzFile, variable_desc *);
extern void	 utilReadDataSkip(gzFile, variable_desc *);
extern int32 utilReadInt(gzFile);
extern void	 utilWriteInt(gzFile, int32);
extern gzFile utilGzOpen(const char *file, const char *mode);
extern gzFile utilGzReopen(int id, const char *mode);
extern gzFile utilMemGzOpen(char *memory, int available, char *mode);
extern int utilGzWrite(gzFile file, voidp buffer, unsigned int len);
extern int utilGzRead(gzFile file, voidp buffer, unsigned int len);
extern int utilGzClose(gzFile file);
extern z_off_t utilGzSeek(gzFile file, z_off_t offset, int whence);
extern z_off_t utilGzTell(gzFile file);
extern void utilGBAFindSave(const u8 *, const int);
extern void utilUpdateSystemColorMaps();
extern bool utilLoadBIOS(u8 *bios, const char *biosFileName, int systemType);
extern bool utilCheckBIOS(const char *biosFileName, int systemType);
extern u16 utilCalcBIOSChecksum(const u8 *bios, int systemType);
extern u16 utilCalcBIOSFileChecksum(const char *biosFileName, int systemType);

#endif // VBA_UTIL_H
