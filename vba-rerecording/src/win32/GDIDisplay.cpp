#include "stdafx.h"
#include <cstdio>

#include "resource.h"
#include "MainWnd.h"
#include "Reg.h"
#include "VBA.h"

#include "../gba/GBAGlobals.h"
#include "../gb/gbGlobals.h"
#include "../common/SystemGlobals.h"
#include "../common/Text.h"
#include "../version.h"

extern u32 RGB_LOW_BITS_MASK;
extern int systemSpeed;
extern void winlog(const char *, ...);
extern int Init_2xSaI(u32);

class GDIDisplay : public IDisplay
{
private:
	u8 *filterData;
	u8  info[sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD)];
public:
	GDIDisplay();
	virtual ~GDIDisplay();

	virtual bool initialize();
	virtual void cleanup();
	virtual void render();
	virtual void checkFullScreen();
	virtual void renderMenu();
	virtual void clear();
	virtual DISPLAY_TYPE getType() { return GDI; };
	virtual void setOption(const char *, int) {}
	virtual int selectFullScreenMode(GUID * *);
};

static int calculateShift(u32 mask)
{
	int m = 0;

	while (mask)
	{
		m++;
		mask >>= 1;
	}

	return m-5;
}

GDIDisplay::GDIDisplay()
{
	filterData = (u8 *)malloc(4*16*256*256); // sufficient for 4x filters @ 32bit color depth
}

GDIDisplay::~GDIDisplay()
{
	cleanup();
}

void GDIDisplay::cleanup()
{
	if (filterData)
	{
		free(filterData);
		filterData = NULL;
	}
}

bool GDIDisplay::initialize()
{
	CWnd *pWnd = theApp.m_pMainWnd;

	theApp.mode320Available = false;
	theApp.mode640Available = false;
	theApp.mode800Available = false;

	HDC         dc  = GetDC(NULL);
	HBITMAP     hbm = CreateCompatibleBitmap(dc, 1, 1);
	BITMAPINFO *bi  = (BITMAPINFO *)info;
	ZeroMemory(bi, sizeof(info));
	bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(dc, hbm, 0, 1, NULL, (LPBITMAPINFO)info, DIB_RGB_COLORS);
	GetDIBits(dc, hbm, 0, 1, NULL, (LPBITMAPINFO)info, DIB_RGB_COLORS);
	DeleteObject(hbm);
	ReleaseDC(NULL, dc);

	if (bi->bmiHeader.biCompression == BI_BITFIELDS)
	{
		systemColorDepth = bi->bmiHeader.biBitCount;
		if (systemColorDepth == 15)
			systemColorDepth = 16;
		systemRedShift   = calculateShift(*((DWORD *)&bi->bmiColors[0]));
		systemGreenShift = calculateShift(*((DWORD *)&bi->bmiColors[1]));
		systemBlueShift  = calculateShift(*((DWORD *)&bi->bmiColors[2]));
		if (systemColorDepth == 16)
		{
			if (systemGreenShift == 6)
			{
				Init_2xSaI(565);
				RGB_LOW_BITS_MASK = 0x821;
			}
			else
			{
				Init_2xSaI(555);
				RGB_LOW_BITS_MASK = 0x421;
			}
		}
		else if (systemColorDepth == 32)
			Init_2xSaI(32);
	}
	else
	{
		systemColorDepth = 32;
		systemRedShift   = 19;
		systemGreenShift = 11;
		systemBlueShift  = 3;

		Init_2xSaI(32);
	}
	theApp.fsColorDepth = systemColorDepth;
	if (systemColorDepth == 24)
		theApp.filterFunction = NULL;
#ifdef MMX
	if (!theApp.disableMMX)
		cpu_mmx = theApp.detectMMX();
	else
		cpu_mmx = 0;
#endif

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

	pWnd->DragAcceptFiles(TRUE);

	return TRUE;
}

void GDIDisplay::clear()
{}

void GDIDisplay::renderMenu()
{
	checkFullScreen();
	theApp.m_pMainWnd->DrawMenuBar();
}

void GDIDisplay::checkFullScreen()
{}

void GDIDisplay::render()
{
	void (*filterFunction)(u8 *, u32, u8 *, u8 *, u32, int, int) = theApp.filterFunction;
	int  filterWidth = theApp.filterWidth, filterHeight = theApp.filterHeight;

	int rectWidth = theApp.rect.right - theApp.rect.left, rectHeight = theApp.rect.bottom - theApp.rect.top;
	int filterPitch = rectWidth * (systemColorDepth / 8);

	u8 *data = pix;
	int dataPitch = theApp.filterWidth * (systemColorDepth / 8) + (systemColorDepth == 24 ? 0 : 4);

	if (textMethod != 0) // do not draw Lua HUD to a video dump
	{
		systemClonePixBuffer(osd);
		data = osd;
		systemRenderLua((u8 *)data, dataPitch);
	}

	if (filterFunction)
	{
		(*filterFunction)(data + dataPitch,
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
			drawTextTransp((u8 *)data,
				           dataPitch,
				           theApp.rect.left + 10,
				           theApp.rect.bottom - 10,
				           buffer);
		else
			drawText((u8 *)data,
				     dataPitch,
			         theApp.rect.left + 10,
			         theApp.rect.bottom - 10,
				     buffer);
	}

	if (textMethod == 1)
	{
		DrawTextMessages((u8 *)filterData, filterPitch, theApp.rect.left, theApp.rect.bottom);
	}

	POINT p;
	p.x = theApp.dest.left;
	p.y = theApp.dest.top;
	CWnd *pWnd = theApp.m_pMainWnd;
	pWnd->ScreenToClient(&p);
	POINT p2;
	p2.x = theApp.dest.right;
	p2.y = theApp.dest.bottom;
	pWnd->ScreenToClient(&p2);

	CDC *dc = pWnd->GetDC();

	BITMAPINFO *bi = (BITMAPINFO *)info;
	bi->bmiHeader.biWidth = rectWidth;
	bi->bmiHeader.biHeight = -rectHeight;

	StretchDIBits((HDC)*dc,
	              p.x,
	              p.y,
	              p2.x - p.x,
	              p2.y - p.y,
	              0,
	              0,
	              theApp.rect.right,
	              theApp.rect.bottom,
	              filterData,
	              bi,
	              DIB_RGB_COLORS,
	              SRCCOPY);

	if (textMethod == 2)
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
							dc->TextOut(p.x+10+xd[i], p2.y - 20*(slot+1)+yd[i], theApp.screenMessageBuffer[slot]);
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
					dc->TextOut(p.x+10, p2.y - 20*(slot+1), theApp.screenMessageBuffer[slot]);
				}
				else
				{
					theApp.screenMessage[slot] = false;
				}
			}
		}

	pWnd->ReleaseDC(dc);
}

int GDIDisplay::selectFullScreenMode(GUID * *)
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

IDisplay *newGDIDisplay()
{
	return new GDIDisplay();
}

