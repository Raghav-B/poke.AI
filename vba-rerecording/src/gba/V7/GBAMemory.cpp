#include "../GBAinline.h"
#include "../../common/System.h"
#include "../../common/SystemGlobals.h"
#include "../../common/vbalua.h"
#include "../GBAGlobals.h"
#include "../GBA.h"
#include "../GBACheats.h"
#include "../GBASound.h"
#include "../agbprint.h"
#include "../EEprom.h"
#include "../Flash.h"
#include "../RTC.h"

#ifdef BKPT_SUPPORT
#ifdef SDL
void cheatsWriteMemory(u32 *address, u32 value, u32 mask);
void cheatsWriteHalfWord(u16 *address, u16 value, u16 mask);
void cheatsWriteByte(u8 *address, u8 value);
#endif
#endif

extern bool8 stopState;
extern bool8 holdState;
extern int32 holdType;
extern bool8 cpuSramEnabled;
extern bool8 cpuFlashEnabled;
extern bool8 cpuEEPROMEnabled;
extern bool8 cpuEEPROMSensorEnabled;
extern bool8 cpuDmaHack;
extern u32	 cpuDmaLast;
extern int32 cpuDmaCount;

MemoryMap memoryMap[256];

u32 CPUReadMemoryWrapped(u32 address)
{
#ifdef GBA_LOGGING
	if (address & 3)
	{
		if (systemVerbose & VERBOSE_UNALIGNED_MEMORY)
		{
			log("Unaligned word read: %08x at %08x\n", address, armMode ?
			    armNextPC - 4 : armNextPC - 2);
		}
	}
#endif

	u32 value;
	switch (address >> 24)
	{
	case 0:
		if (reg[15].I >> 24)
		{
			if (address < 0x4000)
			{
#ifdef GBA_LOGGING
				if (systemVerbose & VERBOSE_ILLEGAL_READ)
				{
					log("Illegal word read: %08x at %08x\n", address, armMode ?
					    armNextPC - 4 : armNextPC - 2);
				}
#endif

				value = READ32LE(((u32 *)&biosProtected));
			}
			else
				goto unreadable;
		}
		else
			value = READ32LE(((u32 *)&bios[address & 0x3FFC]));
		break;
	case 2:
		value = READ32LE(((u32 *)&workRAM[address & 0x3FFFC]));
		break;
	case 3:
		value = READ32LE(((u32 *)&internalRAM[address & 0x7ffC]));
		break;
	case 4:
		if ((address < 0x4000400) && ioReadable[address & 0x3fc])
		{
			if (ioReadable[(address & 0x3fc) + 2])
			{
				if (address >= 0x400012d && address <= 0x4000131)
					systemCounters.lagged = false;
				value = READ32LE(((u32 *)&ioMem[address & 0x3fC]));
			}
			else
			{
				if (address >= 0x400012f && address <= 0x4000131)
					systemCounters.lagged = false;
				value = READ16LE(((u16 *)&ioMem[address & 0x3fc]));
			}
		}
		else
			goto unreadable;
		break;
	case 5:
		value = READ32LE(((u32 *)&paletteRAM[address & 0x3fC]));
		break;
	case 6:
		value = READ32LE(((u32 *)&vram[address & 0x1fffc]));
		break;
	case 7:
		value = READ32LE(((u32 *)&oam[address & 0x3FC]));
		break;
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		value = READ32LE(((u32 *)&rom[address & 0x1FFFFFC]));
		break;
	case 13:
		if (cpuEEPROMEnabled)
			// no need to swap this
			return eepromRead(address);
		goto unreadable;
	case 14:
		if (cpuFlashEnabled | cpuSramEnabled)
			// no need to swap this
			return flashRead(address);
	// default
	default:
unreadable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_READ)
		{
			log("Illegal word read: %08x at %08x\n", address, armMode ?
			    armNextPC - 4 : armNextPC - 2);
		}
#endif

		//    if(ioMem[0x205] & 0x40) {
		if (armState)
		{
			value = CPUReadMemoryQuick(reg[15].I);
		}
		else
		{
			value = CPUReadHalfWordQuick(reg[15].I) |
			        CPUReadHalfWordQuick(reg[15].I) << 16;
		}
		//  } else {
		//      value = *((u32 *)&bios[address & 0x3ffc]);
		//    }
		//        return 0xFFFFFFFF;
	}

	if (address & 3)
	{
#ifdef C_CORE
		int shift = (address & 3) << 3;
		value = (value >> shift) | (value << (32 - shift));
#else
#ifdef __GNUC__
		asm ("and $3, %%ecx;"
		     "shl $3 ,%%ecx;"
		     "ror %%cl, %0"
			 : "=r" (value)
			 : "r" (value), "c" (address));
#else
		__asm {
			mov ecx, address;
			and ecx, 3;
			shl ecx, 3;
			ror [dword ptr value], cl;
		}
#endif
#endif
	}
	return value;
}

u32 CPUReadHalfWordWrapped(u32 address)
{
#ifdef GBA_LOGGING
	if (address & 1)
	{
		if (systemVerbose & VERBOSE_UNALIGNED_MEMORY)
		{
			log("Unaligned halfword read: %08x at %08x\n", address, armMode ?
			    armNextPC - 4 : armNextPC - 2);
		}
	}
#endif

	u32 value;

	switch (address >> 24)
	{
	case 0:
		if (reg[15].I >> 24)
		{
			if (address < 0x4000)
			{
#ifdef GBA_LOGGING
				if (systemVerbose & VERBOSE_ILLEGAL_READ)
				{
					log("Illegal halfword read: %08x at %08x\n", address, armMode ?
					    armNextPC - 4 : armNextPC - 2);
				}
#endif
				value = READ16LE(((u16 *)&biosProtected[address & 2]));
			}
			else
				goto unreadable;
		}
		else
			value = READ16LE(((u16 *)&bios[address & 0x3FFE]));
		break;
	case 2:
		value = READ16LE(((u16 *)&workRAM[address & 0x3FFFE]));
		break;
	case 3:
		value = READ16LE(((u16 *)&internalRAM[address & 0x7ffe]));
		break;
	case 4:
		if ((address < 0x4000400) && ioReadable[address & 0x3fe])
		{
			if (address >= 0x400012f && address <= 0x4000131)
				systemCounters.lagged = false;
			value =  READ16LE(((u16 *)&ioMem[address & 0x3fe]));
		}
		else
			goto unreadable;
		break;
	case 5:
		value = READ16LE(((u16 *)&paletteRAM[address & 0x3fe]));
		break;
	case 6:
		value = READ16LE(((u16 *)&vram[address & 0x1fffe]));
		break;
	case 7:
		value = READ16LE(((u16 *)&oam[address & 0x3fe]));
		break;
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		if (address == 0x80000c4 || address == 0x80000c6 || address == 0x80000c8)
			value = rtcRead(address);
		else
			value = READ16LE(((u16 *)&rom[address & 0x1FFFFFE]));
		break;
	case 13:
		if (cpuEEPROMEnabled)
			// no need to swap this
			return eepromRead(address);
		goto unreadable;
	case 14:
		if (cpuFlashEnabled | cpuSramEnabled)
			// no need to swap this
			return flashRead(address);
	// default
	default:
unreadable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_READ)
		{
			log("Illegal halfword read: %08x at %08x\n", address, armMode ?
			    armNextPC - 4 : armNextPC - 2);
		}
#endif
		if (cpuDmaHack && cpuDmaCount)
		{
			value = cpuDmaLast & 0xFFFF;
		}
		else
		{
			if (armState)
			{
				value = CPUReadHalfWordQuick(reg[15].I + (address & 2));
			}
			else
			{
				value = CPUReadHalfWordQuick(reg[15].I);
			}
		}
		//    return value;
		//    if(address & 1)
		//      value = (value >> 8) | ((value & 0xFF) << 24);
		//    return 0xFFFF;
		break;
	}

	if (address & 1)
	{
		value = (value >> 8) | (value << 24);
	}

	return value;
}

u16 CPUReadHalfWordSignedWrapped(u32 address)
{
	u16 value = CPUReadHalfWordWrapped(address);
	if ((address & 1))
		value = (s8)value;
	return value;
}

u8 CPUReadByteWrapped(u32 address)
{
	switch (address >> 24)
	{
	case 0:
		if (reg[15].I >> 24)
		{
			if (address < 0x4000)
			{
#ifdef GBA_LOGGING
				if (systemVerbose & VERBOSE_ILLEGAL_READ)
				{
					log("Illegal byte read: %08x at %08x\n", address, armMode ?
					    armNextPC - 4 : armNextPC - 2);
				}
#endif
				return biosProtected[address & 3];
			}
			else
				goto unreadable;
		}
		return bios[address & 0x3FFF];
	case 2:
		return workRAM[address & 0x3FFFF];
	case 3:
		return internalRAM[address & 0x7fff];
	case 4:
		if ((address < 0x4000400) && ioReadable[address & 0x3ff])
		{
			if (address == 0x4000130 || address == 0x4000131)
				systemCounters.lagged = false;
			return ioMem[address & 0x3ff];
		}
		else
			goto unreadable;
	case 5:
		return paletteRAM[address & 0x3ff];
	case 6:
		return vram[address & 0x1ffff];
	case 7:
		return oam[address & 0x3ff];
	case 8:
	case 9:
	case 10:
	case 11:
	case 12:
		return rom[address & 0x1FFFFFF];
	case 13:
		if (cpuEEPROMEnabled)
			return eepromRead(address);
		goto unreadable;
	case 14:
		if (cpuSramEnabled | cpuFlashEnabled)
			return flashRead(address);
		if (cpuEEPROMSensorEnabled)
		{
			switch (address & 0x00008f00)
			{
			case 0x8200:
				return systemGetSensorX() & 255;
			case 0x8300:
				return (systemGetSensorX() >> 8) | 0x80;
			case 0x8400:
				return systemGetSensorY() & 255;
			case 0x8500:
				return systemGetSensorY() >> 8;
			}
		}
	// default
	default:
unreadable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_READ)
		{
			log("Illegal byte read: %08x at %08x\n", address, armMode ?
			    armNextPC - 4 : armNextPC - 2);
		}
#endif

		if (armState)
		{
			return CPUReadByteQuick(reg[15].I+(address & 3));
		}
		else
		{
			return CPUReadByteQuick(reg[15].I+(address & 1));
		}
		//    return 0xFF;
		break;
	}
}

void CPUWriteMemoryWrapped(u32 address, u32 value)
{
#ifdef GBA_LOGGING
	if (address & 3)
	{
		if (systemVerbose & VERBOSE_UNALIGNED_MEMORY)
		{
			log("Unaligned word write: %08x to %08x from %08x\n",
			    value,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
	}
#endif

	switch (address >> 24)
	{
	case 0x02:
#ifdef BKPT_SUPPORT
#ifdef SDL
		if (*((u32 *)&freezeWorkRAM[address & 0x3FFFC]))
			cheatsWriteMemory((u32 *)&workRAM[address & 0x3FFFC],
			                  value,
			                  *((u32 *)&freezeWorkRAM[address & 0x3FFFC]));
		else
#endif
#endif
		WRITE32LE(((u32 *)&workRAM[address & 0x3FFFC]), value);
		break;
	case 0x03:
#ifdef BKPT_SUPPORT
#ifdef SDL
		if (*((u32 *)&freezeInternalRAM[address & 0x7ffc]))
			cheatsWriteMemory((u32 *)&internalRAM[address & 0x7FFC],
			                  value,
			                  *((u32 *)&freezeInternalRAM[address & 0x7ffc]));
		else
#endif
#endif
		WRITE32LE(((u32 *)&internalRAM[address & 0x7ffC]), value);
		break;
	case 0x04:
		CPUUpdateRegister((address & 0x3FC), value & 0xFFFF);
		CPUUpdateRegister((address & 0x3FC) + 2, (value >> 16));
		break;
	case 0x05:
		WRITE32LE(((u32 *)&paletteRAM[address & 0x3FC]), value);
		break;
	case 0x06:
		if (address & 0x10000)
			WRITE32LE(((u32 *)&vram[address & 0x17ffc]), value);
		else
			WRITE32LE(((u32 *)&vram[address & 0x1fffc]), value);
		break;
	case 0x07:
		WRITE32LE(((u32 *)&oam[address & 0x3fc]), value);
		break;
	case 0x0D:
		if (cpuEEPROMEnabled)
		{
			eepromWrite(address, value);
			break;
		}
		goto unwritable;
	case 0x0E:
		if (!eepromInUse || cpuSramEnabled || cpuFlashEnabled)
		{
			(*cpuSaveGameFunc)(address, (u8)value);
			break;
		}
	// default
	default:
unwritable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_WRITE)
		{
			log("Illegal word write: %08x to %08x from %08x\n",
			    value,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
#endif
		break;
	}
}

void CPUWriteHalfWordWrapped(u32 address, u16 value)
{
#ifdef GBA_LOGGING
	if (address & 1)
	{
		if (systemVerbose & VERBOSE_UNALIGNED_MEMORY)
		{
			log("Unaligned halfword write: %04x to %08x from %08x\n",
			    value,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
	}
#endif

	switch (address >> 24)
	{
	case 2:
#ifdef BKPT_SUPPORT
#ifdef SDL
		if (*((u16 *)&freezeWorkRAM[address & 0x3FFFE]))
			cheatsWriteHalfWord((u16 *)&workRAM[address & 0x3FFFE],
			                    value,
			                    *((u16 *)&freezeWorkRAM[address & 0x3FFFE]));
		else
#endif
#endif
		WRITE16LE(((u16 *)&workRAM[address & 0x3FFFE]), value);
		break;
	case 3:
#ifdef BKPT_SUPPORT
#ifdef SDL
		if (*((u16 *)&freezeInternalRAM[address & 0x7ffe]))
			cheatsWriteHalfWord((u16 *)&internalRAM[address & 0x7ffe],
			                    value,
			                    *((u16 *)&freezeInternalRAM[address & 0x7ffe]));
		else
#endif
#endif
		WRITE16LE(((u16 *)&internalRAM[address & 0x7ffe]), value);
		break;
	case 4:
		CPUUpdateRegister(address & 0x3fe, value);
		break;
	case 5:
		WRITE16LE(((u16 *)&paletteRAM[address & 0x3fe]), value);
		break;
	case 6:
		if (address & 0x10000)
			WRITE16LE(((u16 *)&vram[address & 0x17ffe]), value);
		else
			WRITE16LE(((u16 *)&vram[address & 0x1fffe]), value);
		break;
	case 7:
		WRITE16LE(((u16 *)&oam[address & 0x3fe]), value);
		break;
	case 8:
	case 9:
		if (address == 0x80000c4 || address == 0x80000c6 || address == 0x80000c8)
		{
			if (!rtcWrite(address, value))
				goto unwritable;
		}
		else if (!agbPrintWrite(address, value))
			goto unwritable;
		break;
	case 13:
		if (cpuEEPROMEnabled)
		{
			eepromWrite(address, (u8)(value & 0xFF));
			break;
		}
		goto unwritable;
	case 14:
		if (!eepromInUse || cpuSramEnabled || cpuFlashEnabled)
		{
			(*cpuSaveGameFunc)(address, (u8)(value & 0xFF));
			break;
		}
		goto unwritable;
	default:
unwritable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_WRITE)
		{
			log("Illegal halfword write: %04x to %08x from %08x\n",
			    value,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
#endif
		break;
	}
}

void CPUWriteByteWrapped(u32 address, u8 b)
{
	switch (address >> 24)
	{
	case 2:
#ifdef BKPT_SUPPORT
#ifdef SDL
		if (freezeWorkRAM[address & 0x3FFFF])
			cheatsWriteByte(&workRAM[address & 0x3FFFF], b);
		else
#endif
#endif
		workRAM[address & 0x3FFFF] = b;
		break;
	case 3:
#ifdef BKPT_SUPPORT
#ifdef SDL
		if (freezeInternalRAM[address & 0x7fff])
			cheatsWriteByte(&internalRAM[address & 0x7fff], b);
		else
#endif
#endif
		internalRAM[address & 0x7fff] = b;
		break;
	case 4:
		switch (address & 0x3FF)
		{
		case 0x301:
			if (b == 0x80)
				stopState = true;
			holdState = true;
			holdType  = -1;
			break;
		case 0x60:
		case 0x61:
		case 0x62:
		case 0x63:
		case 0x64:
		case 0x65:
		case 0x68:
		case 0x69:
		case 0x6c:
		case 0x6d:
		case 0x70:
		case 0x71:
		case 0x72:
		case 0x73:
		case 0x74:
		case 0x75:
		case 0x78:
		case 0x79:
		case 0x7c:
		case 0x7d:
		case 0x80:
		case 0x81:
		case 0x84:
		case 0x85:
		case 0x90:
		case 0x91:
		case 0x92:
		case 0x93:
		case 0x94:
		case 0x95:
		case 0x96:
		case 0x97:
		case 0x98:
		case 0x99:
		case 0x9a:
		case 0x9b:
		case 0x9c:
		case 0x9d:
		case 0x9e:
		case 0x9f:
			soundEvent(address & 0xFF, b);
			break;
		default:
			//      if(address & 1) {
			//        CPUWriteHalfWord(address-1, (CPUReadHalfWord(address-1)&0x00FF)|((int)b<<8));
			//      } else
			if (address & 1)
				CPUUpdateRegister(address & 0x3fe,
				                  ((READ16LE(((u16 *)&ioMem[address & 0x3fe])))
				                   & 0x00FF) |
				                  b << 8);
			else
				CPUUpdateRegister(address & 0x3fe,
				                  ((READ16LE(((u16 *)&ioMem[address & 0x3fe])) & 0xFF00) | b));
		}
		break;
	case 5:
		// no need to switch
		*((u16 *)&paletteRAM[address & 0x3FE]) = (b << 8) | b;
		break;
	case 6:
		// no need to switch
		if (address & 0x10000)
			*((u16 *)&vram[address & 0x17FFE]) = (b << 8) | b;
		else
			*((u16 *)&vram[address & 0x1FFFE]) = (b << 8) | b;
		break;
	case 7:
		// no need to switch
		*((u16 *)&oam[address & 0x3FE]) = (b << 8) | b;
		break;
	case 13:
		if (cpuEEPROMEnabled)
		{
			eepromWrite(address, b);
			break;
		}
		goto unwritable;
	case 14:
		if (!eepromInUse || cpuSramEnabled || cpuFlashEnabled)
		{
			(*cpuSaveGameFunc)(address, b);
			break;
		}
	// default
	default:
unwritable:
#ifdef GBA_LOGGING
		if (systemVerbose & VERBOSE_ILLEGAL_WRITE)
		{
			log("Illegal byte write: %02x to %08x from %08x\n",
			    b,
			    address,
			    armMode ? armNextPC - 4 : armNextPC - 2);
		}
#endif
		break;
	}
}

void CPUWriteMemory(u32 address, u32 value)
{
	CPUWriteMemoryWrapped(address, value);
	CallRegisteredLuaMemHook(address, 4, value, LUAMEMHOOK_WRITE);
}

void CPUWriteHalfWord(u32 address, u16 value)
{
	CPUWriteHalfWordWrapped(address, value);
	CallRegisteredLuaMemHook(address, 2, value, LUAMEMHOOK_WRITE);
}

void CPUWriteByte(u32 address, u8 b)
{
	CPUWriteByteWrapped(address, b);
	CallRegisteredLuaMemHook(address, 1, b, LUAMEMHOOK_WRITE);
}

u32 CPUReadMemory(u32 address)
{
	u32 value = CPUReadMemoryWrapped(address);
	CallRegisteredLuaMemHook(address, 4, value, LUAMEMHOOK_READ);
	return value;
}

u32 CPUReadHalfWord(u32 address)
{
	u32 value = CPUReadHalfWordWrapped(address);
	CallRegisteredLuaMemHook(address, 2, value, LUAMEMHOOK_READ);
	return value;
}

u16 CPUReadHalfWordSigned(u32 address)
{
	u16 value = CPUReadHalfWordSignedWrapped(address);
	CallRegisteredLuaMemHook(address, 2, value, LUAMEMHOOK_READ);
	return value;
}

u8 CPUReadByte(u32 address)
{
	u8 value = CPUReadByteWrapped(address);
	CallRegisteredLuaMemHook(address, 1, value, LUAMEMHOOK_READ);
	return value;
}
