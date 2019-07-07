#include "stdafx.h"
#include <gl/GL.h>

#include "resource.h"
#include "MainWnd.h"
#include "Reg.h"
#include "VBA.h"

#include "../gba/GBAGlobals.h"
#include "../gb/gbGlobals.h"
#include "../common/SystemGlobals.h"
#include "../common/Text.h"
#include "../version.h"

#ifdef MMX
extern "C" bool cpu_mmx;

extern bool detectMMX();
#endif

extern int systemSpeed;
extern int Init_2xSaI(u32);
extern void winlog(const char *, ...);

class OpenGLDisplay : public IDisplay
{
private:
	HDC    hDC;
	HGLRC  hglrc;
	GLuint texture;
	int    width;
	int    height;
	float  size;
	u8 *   filterData;
	bool   failed;

	bool initializeTexture(int w, int h);
	void updateFiltering(int);
public:
	OpenGLDisplay();
	virtual ~OpenGLDisplay();

	virtual bool initialize();
	virtual void cleanup();
	virtual void render();
	virtual void checkFullScreen();
	virtual void renderMenu();
	virtual void clear();
	virtual bool changeRenderSize(int w, int h);
	virtual void resize(int w, int h);
	virtual DISPLAY_TYPE getType() { return OPENGL; };
	virtual void setOption(const char *, int);
	virtual int selectFullScreenMode(GUID * *);
};

OpenGLDisplay::OpenGLDisplay()
{
	hDC        = NULL;
	hglrc      = NULL;
	texture    = 0;
	width      = 0;
	height     = 0;
	size       = 0.0f;
	filterData = (u8 *)malloc(4*16*256*256); // sufficient for 4x filters @ 32bit color depth
	failed     = false;
}

OpenGLDisplay::~OpenGLDisplay()
{
	cleanup();
}

void OpenGLDisplay::cleanup()
{
	if (texture != 0)
	{
		glDeleteTextures(1, &texture);
		texture = 0;
	}
	if (hglrc != NULL)
	{
		wglDeleteContext(hglrc);
		wglMakeCurrent(NULL, NULL);
		hglrc = NULL;
	}
	if (hDC != NULL)
	{
		ReleaseDC(*theApp.m_pMainWnd, hDC);
		hDC = NULL;
	}
	if (filterData)
	{
		free(filterData);
		filterData = NULL;
	}
	width  = 0;
	height = 0;
	size   = 0.0f;
}

bool OpenGLDisplay::initialize()
{
	CWnd *pWnd = theApp.m_pMainWnd;

	theApp.mode320Available = false;
	theApp.mode640Available = false;
	theApp.mode800Available = false;

	CDC *dc  = pWnd->GetDC();
	HDC  hDC = dc->GetSafeHdc();

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), //  size of this pfd
		1, // version number
		PFD_DRAW_TO_WINDOW | // support window
		PFD_SUPPORT_OPENGL | // support OpenGL
		PFD_DOUBLEBUFFER, // double buffered
		PFD_TYPE_RGBA, // RGBA type
		16, // 16-bit color depth
		0, 0, 0, 0, 0, 0,  // color bits ignored
		0, // no alpha buffer
		0, // shift bit ignored
		0, // no accumulation buffer
		0, 0, 0, 0, // accum bits ignored
		32, // 32-bit z-buffer
		0, // no stencil buffer
		0, // no auxiliary buffer
		PFD_MAIN_PLANE, // main layer
		0, // reserved
		0, 0, 0            // layer masks ignored
	};
	int iPixelFormat;

	if (!(iPixelFormat = ChoosePixelFormat(hDC, &pfd)))
	{
		winlog("Failed ChoosePixelFormat\n");
		return false;
	}

	// obtain detailed information about
	// the device context's first pixel format
	if (!(DescribePixelFormat(hDC, iPixelFormat,
	                          sizeof(PIXELFORMATDESCRIPTOR), &pfd)))
	{
		winlog("Failed DescribePixelFormat\n");
		return false;
	}

	if (!SetPixelFormat(hDC, iPixelFormat, &pfd))
	{
		winlog("Failed SetPixelFormat\n");
		return false;
	}

	if (!(hglrc = wglCreateContext(hDC)))
	{
		winlog("Failed wglCreateContext\n");
		return false;
	}

	if (!wglMakeCurrent(hDC, hglrc))
	{
		winlog("Failed wglMakeCurrent\n");
		return false;
	}
	pWnd->ReleaseDC(dc);

	// setup 2D gl environment
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_TEXTURE_2D);

	glViewport(0, 0, theApp.surfaceSizeX, theApp.surfaceSizeY);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, (GLdouble)(theApp.surfaceSizeX), (GLdouble)(theApp.surfaceSizeY),
	        0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	systemRedShift      = 3;
	systemGreenShift    = 11;
	systemBlueShift     = 19;
	systemColorDepth    = 32;
	theApp.fsColorDepth = 32;

	Init_2xSaI(32);
#ifdef MMX
	if (!theApp.disableMMX)
		cpu_mmx = theApp.detectMMX();
	else
		cpu_mmx = 0;
#endif

	if (theApp.ddrawDebug)
	{
		winlog("R shift: %d\n", systemRedShift);
		winlog("G shift: %d\n", systemGreenShift);
		winlog("B shift: %d\n", systemBlueShift);
	}

	switch (systemColorDepth)
	{
	case 16:
	{
		for (int i = 0; i < 0x10000; i++)
		{
			systemColorMap16[i] = ((i & 0x1f) << systemRedShift) |
			                      (((i & 0x3e0) >> 5) << systemGreenShift) |
			                      (((i & 0x7c00) >> 10) << systemBlueShift);
		}
		break;
	}
	case 24:
	case 32:
	{
		for (int i = 0; i < 0x10000; i++)
		{
			systemColorMap32[i] = ((i & 0x1f) << systemRedShift) |
			                      (((i & 0x3e0) >> 5) << systemGreenShift) |
			                      (((i & 0x7c00) >> 10) << systemBlueShift);
		}
		break;
	}
	}
	theApp.updateFilter();
	theApp.updateIFB();

	if (failed)
		return false;

	pWnd->DragAcceptFiles(TRUE);

	return TRUE;
}

void OpenGLDisplay::clear()
{}

void OpenGLDisplay::renderMenu()
{
	checkFullScreen();
	if (theApp.m_pMainWnd)
		theApp.m_pMainWnd->DrawMenuBar();
}

void OpenGLDisplay::checkFullScreen()
{
	//  if(tripleBuffering)
	//    pOpenGL->FlipToGDISurface();
}

void OpenGLDisplay::render()
{
	void (*filterFunction)(u8 *, u32, u8 *, u8 *, u32, int, int) = theApp.filterFunction;
	int  filterWidth = theApp.filterWidth, filterHeight = theApp.filterHeight;

	int rectWidth = theApp.rect.right - theApp.rect.left, dataHeight = theApp.rect.bottom - theApp.rect.top;
	int filterPitch = rectWidth * 4;

	u8 *data = pix;
	int dataPitch = filterWidth * 4 + 4;

	if (textMethod != 0) // do not draw Lua HUD to a video dump
	{
		systemClonePixBuffer(osd);
		data = osd;
		systemRenderLua((u8 *)data, dataPitch);
	}

	if (filterFunction)
	{
		filterFunction(data + dataPitch,
		               dataPitch,
		               (u8 *)theApp.delta,
		               (u8 *)filterData,
		               filterPitch,
		               filterWidth,
		               filterHeight);

		data = filterData;
		dataPitch = filterPitch;
	}

	if (theApp.showSpeed && theApp.videoOption > VIDEO_4X)
	{
		char buffer[30];
		if (theApp.showSpeed == 1)
			sprintf(buffer, "%3d%%", systemSpeed);
		else
			sprintf(buffer, "%3d%%(%d, %d fps)", systemSpeed,
			        systemFrameSkip,
			        theApp.showRenderedFrames);

		if (theApp.showSpeedTransparent)
			drawTextTransp(data,
				dataPitch,
				theApp.rect.left + 10,
				theApp.rect.bottom - 10,
				buffer);
		else
			drawText(data,
				dataPitch,
				theApp.rect.left + 10,
				theApp.rect.bottom - 10,
				buffer);
	}

	if (textMethod == 1)
	{
		DrawTextMessages((u8 *)data, dataPitch, theApp.rect.left, theApp.rect.bottom);
	}

	// Texturemap complete texture to surface so we have free scaling
	// and antialiasing
	if (filterFunction)
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, rectWidth);
	}
	else
	{
		glPixelStorei(GL_UNPACK_ROW_LENGTH, theApp.sizeX + 1);
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0,
					theApp.rect.left, theApp.rect.top, rectWidth, dataHeight,
	                GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (theApp.glType == 0)
	{
		glBegin(GL_TRIANGLE_STRIP);
		glTexCoord2f(theApp.rect.left / size, theApp.rect.top / size); glVertex3i(0, 0, 0);
		glTexCoord2f(theApp.rect.right / size, theApp.rect.top / size); glVertex3i(theApp.surfaceSizeX, 0, 0);
		glTexCoord2f(theApp.rect.left / size, theApp.rect.bottom / size); glVertex3i(0, theApp.surfaceSizeY, 0);
		glTexCoord2f(theApp.rect.right / size, theApp.rect.bottom / size); glVertex3i(theApp.surfaceSizeX, theApp.surfaceSizeY, 0);
		glEnd();
	}
	else
	{
		glBegin(GL_QUADS);
		glTexCoord2f(theApp.rect.left / size, theApp.rect.top / size); glVertex3i(0, 0, 0);
		glTexCoord2f(theApp.rect.right / size, theApp.rect.top / size); glVertex3i(theApp.surfaceSizeX, 0, 0);
		glTexCoord2f(theApp.rect.right / size, theApp.rect.bottom / size); glVertex3i(theApp.surfaceSizeX, theApp.surfaceSizeY, 0);
		glTexCoord2f(theApp.rect.left / size, theApp.rect.bottom / size); glVertex3i(0, theApp.surfaceSizeY, 0);
		glEnd();
	}

	CDC *dc = theApp.m_pMainWnd->GetDC();

	if (textMethod == 2)
	{
		for (int slot = 0; slot < SCREEN_MESSAGE_SLOTS; slot++)
		{
			if (theApp.screenMessage[slot])
			{
				if ((theApp.screenMessageDuration[slot] < 0 || 
					(int)(GetTickCount() - theApp.screenMessageTime[slot]) < theApp.screenMessageDuration[slot]) &&
				    (!theApp.disableStatusMessage || slot == 1 || slot == 2))
				{
					dc->SetBkMode(TRANSPARENT);

					if (outlinedText)
					{
						dc->SetTextColor(textColor != 7 ? RGB(0, 0, 0) : RGB(255, 255, 255));
						// draw black outline
						const static int xd [8] = {-1, 0, 1, 1, 1, 0, -1, -1};
						const static int yd [8] = {-1, -1, -1, 0, 1, 1, 1, 0};
						for (int i = 0; i < 8; i++)
						{
							dc->TextOut(10+xd[i], theApp.surfaceSizeY - 20*(slot+1)+yd[i], theApp.screenMessageBuffer[slot]);
						}
					}

					COLORREF color;
					switch (textColor)
					{
					case 0:
						color = RGB(255, 255, 255); break;
					case 1:
						color = RGB(255, 0, 0); break;
					case 2:
						color = RGB(255, 255, 0); break;
					case 3:
						color = RGB(0, 255, 0); break;
					case 4:
						color = RGB(0, 255, 255); break;
					case 5:
						color = RGB(0, 0, 255); break;
					case 6:
						color = RGB(255, 0, 255); break;
					case 7:
						color = RGB(0, 0, 0); break;
					}
					dc->SetTextColor(color);

					// draw center text
					dc->TextOut(10, theApp.surfaceSizeY - 20*(slot+1), theApp.screenMessageBuffer[slot]);
				}
				else
				{
					theApp.screenMessage[slot] = false;
				}
			}
		}
	}

	SwapBuffers(dc->GetSafeHdc());

	theApp.m_pMainWnd->ReleaseDC(dc);
}

void OpenGLDisplay::resize(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0, (GLdouble)(w), (GLdouble)(h), 0.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGLDisplay::updateFiltering(int value)
{
	switch (value)
	{
	case 0:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	case 1:
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	}
}

bool OpenGLDisplay::initializeTexture(int w, int h)
{
	int mySize = 256;
	size = 256.0f;
	if (w > 511 || h > 511)
	{
		size   = 1024.0f;
		mySize = 1024;
	}
	else if (w > 255 || h > 255)
	{
		size   = 512.0f;
		mySize = 512;
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	int filter = regQueryDwordValue("glFilter", 0);
	if (filter < 0 || filter > 1)
		filter = 0;
	updateFiltering(filter);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mySize, mySize, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, NULL);
	width  = w;
	height = h;

	return true;
}

bool OpenGLDisplay::changeRenderSize(int w, int h)
{
	if (width != w || height != h)
	{
		if (texture != 0)
		{
			glDeleteTextures(1, &texture);
			texture = 0;
		}
		if (!initializeTexture(w, h))
		{
			failed = true;
			return false;
		}
	}
	return true;
}

void OpenGLDisplay::setOption(const char *option, int value)
{
	if (!strcmp(option, "glFilter"))
		updateFiltering(value);
}

int OpenGLDisplay::selectFullScreenMode(GUID * *)
{
	HWND wnd = GetDesktopWindow();
	RECT r;
	GetWindowRect(wnd, &r);
	int w  = (r.right - r.left) & 4095;
	int h  = (r.bottom - r.top) & 4095;
	HDC dc = GetDC(wnd);
	int c  = GetDeviceCaps(dc, BITSPIXEL);
	ReleaseDC(wnd, dc);

	return (c << 24) | (w << 12) | h;
}

IDisplay *newOpenGLDisplay()
{
	return new OpenGLDisplay();
}

