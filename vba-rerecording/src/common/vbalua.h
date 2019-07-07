#ifndef VBA_LUA_H
#define VBA_LUA_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum LuaCallID
{
	LUACALL_BEFOREEMULATION,
	LUACALL_AFTEREMULATION,
	LUACALL_BEFOREEXIT,
	LUACALL_AFTERPOWERON,
	LUACALL_BEFOREPOWEROFF,
	LUACALL_BEFORESTATELOAD,
	LUACALL_AFTERSTATELOAD,
	LUACALL_BEFORESTATESAVE,
	LUACALL_AFTERSTATESAVE,

	LUACALL_COUNT
};
void CallRegisteredLuaFunctions(LuaCallID calltype);

enum LuaMemHookType
{
	LUAMEMHOOK_WRITE,
	LUAMEMHOOK_READ,
	LUAMEMHOOK_EXEC,
	LUAMEMHOOK_WRITE_SUB,
	LUAMEMHOOK_READ_SUB,
	LUAMEMHOOK_EXEC_SUB,

	LUAMEMHOOK_COUNT
};
void CallRegisteredLuaMemHook(unsigned int address, int size, unsigned int value, LuaMemHookType hookType);

// Just forward function declarations

void VBALuaFrameBoundary();
int VBALoadLuaCode(const char *filename);
int VBAReloadLuaCode();
void VBALuaStop();
int VBALuaRunning();

int VBALuaUsingJoypad(int);
int VBALuaReadJoypad(int);
int VBALuaSpeed();
bool8 VBALuaRerecordCountSkip();

void VBALuaGui(uint8 *screen, int ppl, int width, int height);
void VBALuaClearGui();

char* VBAGetLuaScriptName();

// And some interesting REVERSE declarations!
char *VBAGetFreezeFilename(int slot);

#endif // VBA_LUA_H
