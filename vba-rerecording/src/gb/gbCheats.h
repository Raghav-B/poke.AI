#ifndef VBA_GB_CHEATS_H
#define VBA_GB_CHEATS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct gbXxCheat
{
	char cheatDesc[100];
	char cheatCode[20];
};

struct gbCheat
{
	char cheatCode[20];
	char cheatDesc[32];
	u16	 address;
	int	 code;
	u8	 compare;
	u8	 value;
	bool enabled;
};

void gbCheatsSaveGame(gzFile);
void gbCheatsReadGame(gzFile, int);
void gbCheatsReadGameSkip(gzFile, int);
void gbCheatsSaveCheatList(const char *);
bool gbCheatsLoadCheatList(const char *);
bool gbCheatReadGSCodeFile(const char *);

bool gbAddGsCheat(const char *, const char *);
bool gbAddGgCheat(const char *, const char *);
void gbCheatRemove(int);
void gbCheatRemoveAll();
void gbCheatEnable(int);
void gbCheatDisable(int);
u8 gbCheatRead(u16);
void gbCheatWrite(bool);
bool gbVerifyGsCode(const char *code);
bool gbVerifyGgCode(const char *code);

#if !defined (MAX_CHEATS)
#define MAX_CHEATS 100
#endif

extern int	   gbCheatNumber;
extern gbCheat gbCheatList[MAX_CHEATS];
extern bool	   gbCheatMap[0x10000];

#endif // VBA_GB_CHEATS_H
