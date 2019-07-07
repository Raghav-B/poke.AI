#ifndef VBA_WIN32_DISASSEMBLE_H
#define VBA_WIN32_DISASSEMBLE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

enum DISPLAY_TYPE
{
	GDI         = 0,
	DIRECT_DRAW = 1,
	DIRECT_3D   = 2,
	OPENGL      = 3
};

class IDisplay
{
public:
	IDisplay() {};
	virtual ~IDisplay() {};

	virtual bool initialize() = 0;
	virtual void cleanup()    = 0;
	virtual void render()     = 0;
	virtual void checkFullScreen() { };
	virtual void renderMenu() { };
	virtual void clear() = 0;
	virtual bool changeRenderSize(int w, int h) { return true; };
	virtual void resize(int w, int h) {};
	virtual void setOption(const char *option, int value) = 0;
	virtual DISPLAY_TYPE getType() = 0;
	virtual int selectFullScreenMode(GUID * *) = 0;
};

#endif // VBA_WIN32_DISASSEMBLE_H
