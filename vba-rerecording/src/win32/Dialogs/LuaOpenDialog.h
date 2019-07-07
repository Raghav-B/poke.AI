#if !defined(VBA_WIN32_LUAOPENDIALOG_H_INCLUDED)
#define VBA_WIN32_LUAOPENDIALOG_H_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// -*- C++ -*-
// LuaOpenDialog.h : header file
//

#include "../stdafx.h"
#include "../resource.h"

extern HWND LuaConsoleHWnd;

void PrintToWindowConsole(int hDlgAsInt, const char* str);
void WinLuaOnStart(int hDlgAsInt);
void WinLuaOnStop(int hDlgAsInt);
INT_PTR CALLBACK DlgLuaScriptDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

#endif // !defined(VBA_WIN32_LUAOPENDIALOG_H_INCLUDED)
