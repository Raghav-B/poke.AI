#ifndef VBA_GBA_H
#define VBA_GBA_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zlib.h"
#include "../Port.h"

#define SAVE_GAME_VERSION_1 1
#define SAVE_GAME_VERSION_2 2
#define SAVE_GAME_VERSION_3 3
#define SAVE_GAME_VERSION_4 4
#define SAVE_GAME_VERSION_5 5
#define SAVE_GAME_VERSION_6 6
#define SAVE_GAME_VERSION_7 7
#define SAVE_GAME_VERSION_8 8
#define SAVE_GAME_VERSION_9 9
#define SAVE_GAME_VERSION_10 10
#define SAVE_GAME_VERSION_11 11
#define SAVE_GAME_VERSION_12 12
#define SAVE_GAME_VERSION_13 13
#define SAVE_GAME_VERSION_14 14

#ifdef USE_GBA_CORE_V7
#define SAVE_GAME_VERSION  SAVE_GAME_VERSION_13
#else
#define SAVE_GAME_VERSION  SAVE_GAME_VERSION_14
#endif
extern void (*cpuSaveGameFunc)(u32, u8);

#ifdef BKPT_SUPPORT
extern u8 freezeWorkRAM[0x40000];
extern u8 freezeInternalRAM[0x8000];
extern u8 freezeVRAM[0x18000];
extern u8 freezePRAM[0x400];
extern u8 freezeOAM[0x400];
#endif

extern bool CPUReadGSASnapshot(const char *);
extern bool CPUWriteGSASnapshot(const char *, const char *, const char *, const char *);
extern bool CPUWriteBatteryFile(const char *);
extern bool CPUReadBatteryFile(const char *);
extern bool CPUWriteBatteryToStream(gzFile);
extern bool CPUReadBatteryFromStream(gzFile);
extern bool CPUExportEepromFile(const char *);
extern bool CPUImportEepromFile(const char *);
extern bool CPUWritePNGFile(const char *);
extern bool CPUWriteBMPFile(const char *);
extern void CPUCleanUp();
extern void CPUUpdateRender();
extern void CPUUpdateRenderBuffers(bool force);
extern bool CPUReadMemState(char *, int);
extern bool CPUReadState(const char *);
extern bool CPUWriteMemState(char *, int);
extern bool CPUWriteState(const char *);
extern bool CPUReadStateFromStream(gzFile);
extern bool CPUWriteStateToStream(gzFile);
extern int  CPULoadRom(const char *);
extern void CPUMasterCodeCheck();
extern void CPUDoMirroring(bool);
extern void CPUUpdateRegister(u32, u16);
extern void CPULoadInternalBios();
extern void CPUInit();
extern void CPUReset();
extern void CPULoop(int);
extern void CPUCheckDMA(int, int);
#ifdef PROFILING
extern void cpuProfil(char *buffer, int, u32, int);
extern void cpuEnableProfiling(int hz);
#endif

u32 CPUReadMemory(u32 address);
u32 CPUReadHalfWord(u32 address);
u8 CPUReadByte(u32 address);
void CPUWriteMemory(u32 address, u32 value);
void CPUWriteHalfWord(u32 address, u16 value);
u16 CPUReadHalfWordSigned(u32 address);
void CPUWriteByte(u32 address, u8 b);
void CPUWriteMemoryWrapped(u32 address, u32 value);
void CPUWriteHalfWordWrapped(u32 address, u16 value);
void CPUWriteByteWrapped(u32 address, u8 b);

extern struct EmulatedSystem GBASystem;

#define R13_IRQ  18
#define R14_IRQ  19
#define SPSR_IRQ 20
#define R13_USR  26
#define R14_USR  27
#define R13_SVC  28
#define R14_SVC  29
#define SPSR_SVC 30
#define R13_ABT  31
#define R14_ABT  32
#define SPSR_ABT 33
#define R13_UND  34
#define R14_UND  35
#define SPSR_UND 36
#define R8_FIQ   37
#define R9_FIQ   38
#define R10_FIQ  39
#define R11_FIQ  40
#define R12_FIQ  41
#define R13_FIQ  42
#define R14_FIQ  43
#define SPSR_FIQ 44

#endif // VBA_GBA_H
