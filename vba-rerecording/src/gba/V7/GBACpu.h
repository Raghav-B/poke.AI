#ifndef VBA_GBACPU_H
#define VBA_GBACPU_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern int armExecute();
extern int thumbExecute();

#ifdef __GNUC__
# define INSN_REGPARM __attribute__((regparm(1)))
# define LIKELY(x) __builtin_expect(!!(x), 1)
# define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define INSN_REGPARM /*nothing*/
# define LIKELY(x) (x)
# define UNLIKELY(x) (x)
#endif

#define UPDATE_REG(address, value) \
	{ \
		WRITE16LE(((u16 *)&ioMem[address]), value); \
	} \

extern const int32 thumbCycles[256];
extern int32 memoryWait[16];
extern int32 memoryWait32[16];
extern int32 memoryWaitSeq[16];
extern int32 memoryWaitSeq32[16];
extern int32 memoryWaitFetch[16];
extern int32 memoryWaitFetch32[16];
extern u8	 cpuBitsSet[256];
extern u8	 cpuLowestBitSet[256];

extern void CPUSwitchMode(int mode, bool saveState, bool breakLoop);
extern void CPUSwitchMode(int mode, bool saveState);
extern void CPUUpdateCPSR();
extern void CPUUpdateFlags(bool breakLoop);
extern void CPUUpdateFlags();
extern void CPUUndefinedException();
extern void CPUSoftwareInterrupt();
extern void CPUSoftwareInterrupt(int comment);

inline int CPUUpdateTicksAccess32(u32 address)
{
	return memoryWait32[(address >> 24) & 15];
}

inline int CPUUpdateTicksAccess16(u32 address)
{
	return memoryWait[(address >> 24) & 15];
}

inline int CPUUpdateTicksAccessSeq32(u32 address)
{
	return memoryWaitSeq32[(address >> 24) & 15];
}

inline int CPUUpdateTicksAccessSeq16(u32 address)
{
	return memoryWaitSeq[(address >> 24) & 15];
}

#endif // VBA_GBACPU_H
