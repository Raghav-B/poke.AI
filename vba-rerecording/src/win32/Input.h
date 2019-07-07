#ifndef VBA_WIN32_INPUT_H
#define VBA_WIN32_INPUT_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../common/inputGlobal.h"

#define JOYCONFIG_MESSAGE (WM_USER + 1000)

class Input
{
public:
	Input() {};
	virtual ~Input() {};

	virtual bool initialize() = 0;

	virtual bool readDevices() = 0;
	virtual u32 readDevice(int which, bool sensor) = 0;
	virtual CString getKeyName(LONG_PTR key) = 0;
	virtual void checkKeys()    = 0;
	virtual void checkDevices() = 0;
	virtual void activate()     = 0;
	virtual void loadSettings() = 0;
	virtual void saveSettings() = 0;
};

#endif
