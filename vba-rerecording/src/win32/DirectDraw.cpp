#include "stdafx.h"

#define DIRECTDRAW_VERSION 0x0700
#include "ddraw.h"

#include "resource.h"
#include "MainWnd.h"
#include "Reg.h"
#include "VBA.h"

#include "../common/System.h"
#include "../common/SystemGlobals.h"
#include "../gba/GBAGlobals.h"
#include "../gb/gbGlobals.h"
#include "../common/Text.h"
#include "../version.h"

extern u32 RGB_LOW_BITS_MASK;
extern int systemSpeed;
extern int Init_2xSaI(u32);
extern void directXMessage(const char *);
extern void winlog(const char *, ...);
extern int winVideoModeSelect(CWnd *, GUID * *);

class DirectDrawDisplay : public IDisplay
{
private:
	HINSTANCE ddrawDLL;
	LPDIRECTDRAW7        pDirectDraw;
	LPDIRECTDRAWSURFACE7 ddsPrimary;
	LPDIRECTDRAWSURFACE7 ddsOffscreen;
	LPDIRECTDRAWSURFACE7 ddsFlip;
	LPDIRECTDRAWCLIPPER  ddsClipper;
	int  width;
	int  height;
	bool failed;

	bool initializeOffscreen(int w, int h);
public:
	DirectDrawDisplay();
	virtual ~DirectDrawDisplay();

	virtual bool initialize();
	virtual void cleanup();
	virtual void render();
	virtual void checkFullScreen();
	virtual void renderMenu();
	virtual void clear();
	virtual bool changeRenderSize(int w, int h);
	virtual DISPLAY_TYPE getType() { return DIRECT_DRAW; };
	virtual void setOption(const char *, int) {}
	virtual int selectFullScreenMode(GUID * *);
};

static HRESULT WINAPI checkModesAvailable(LPDDSURFACEDESC2 surf, LPVOID lpContext)
{
	if (surf->dwWidth == 320 &&
	    surf->dwHeight == 240 &&
	    surf->ddpfPixelFormat.dwRGBBitCount == 16)
	{
		theApp.mode320Available = TRUE;
	}
	if (surf->dwWidth == 640 &&
	    surf->dwHeight == 480 &&
	    surf->ddpfPixelFormat.dwRGBBitCount == 16)
	{
		theApp.mode640Available = TRUE;
	}
	if (surf->dwWidth == 800 &&
	    surf->dwHeight == 600 &&
	    surf->ddpfPixelFormat.dwRGBBitCount == 16)
	{
		theApp.mode800Available = TRUE;
	}
	return DDENUMRET_OK;
}

static int ffs(UINT mask)
{
	int m = 0;
	if (mask)
	{
		while (!(mask & (1 << m)))
			m++;

		return (m);
	}

	return (0);
}

DirectDrawDisplay::DirectDrawDisplay()
{
	pDirectDraw  = NULL;
	ddsPrimary   = NULL;
	ddsOffscreen = NULL;
	ddsFlip      = NULL;
	ddsClipper   = NULL;
	ddrawDLL     = NULL;
	width        = 0;
	height       = 0;
	failed       = false;
}

DirectDrawDisplay::~DirectDrawDisplay()
{
	cleanup();
}

void DirectDrawDisplay::cleanup()
{
	if (pDirectDraw != NULL)
	{
		if (ddsClipper != NULL)
		{
			ddsClipper->Release();
			ddsClipper = NULL;
		}

		if (ddsFlip != NULL)
		{
			ddsFlip->Release();
			ddsFlip = NULL;
		}

		if (ddsOffscreen != NULL)
		{
			ddsOffscreen->Release();
			ddsOffscreen = NULL;
		}

		if (ddsPrimary != NULL)
		{
			ddsPrimary->Release();
			ddsPrimary = NULL;
		}

		pDirectDraw->Release();
		pDirectDraw = NULL;
	}

	if (ddrawDLL != NULL)
	{
		/**/ ::FreeLibrary(ddrawDLL);
		ddrawDLL = NULL;
	}
	width  = 0;
	height = 0;
}

bool DirectDrawDisplay::initialize()
{
	CWnd *pWnd = theApp.m_pMainWnd;

	GUID *guid = NULL;
	if (theApp.ddrawEmulationOnly)
		guid = (GUID *)DDCREATE_EMULATIONONLY;

	if (theApp.pVideoDriverGUID)
		guid = theApp.pVideoDriverGUID;

	ddrawDLL = /**/ ::LoadLibrary("DDRAW.DLL");
	HRESULT (WINAPI *DDrawCreateEx)(GUID *, LPVOID *, REFIID, IUnknown *);
	if (ddrawDLL != NULL)
	{
		DDrawCreateEx = (HRESULT (WINAPI *)(GUID *, LPVOID *, REFIID, IUnknown *))
		                GetProcAddress(ddrawDLL, "DirectDrawCreateEx");

		if (DDrawCreateEx == NULL)
		{
			directXMessage("DirectDrawCreateEx");
			return FALSE;
		}
	}
	else
	{
		directXMessage("DDRAW.DLL");
		return FALSE;
	}

	theApp.ddrawUsingEmulationOnly = theApp.ddrawEmulationOnly;

	HRESULT hret = DDrawCreateEx(guid,
	                             (void * *)&pDirectDraw,
	                             IID_IDirectDraw7,
	                             NULL);

	if (hret != DD_OK)
	{
		winlog("Error creating DirectDraw object %08x\n", hret);
		if (theApp.ddrawEmulationOnly)
		{
			// disable emulation only setting in case of failure
			regSetDwordValue("ddrawEmulationOnly", 0);
		}
		//    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWCREATE), hret);
		return FALSE;
	}

	if (theApp.ddrawDebug)
	{
		DDCAPS driver;
		DDCAPS hel;
		ZeroMemory(&driver, sizeof(driver));
		ZeroMemory(&hel, sizeof(hel));
		driver.dwSize = sizeof(driver);
		hel.dwSize    = sizeof(hel);
		pDirectDraw->GetCaps(&driver, &hel);
		int    i;
		DWORD *p = (DWORD *)&driver;
		for (i = 0; i < (int)driver.dwSize; i += 4)
			winlog("Driver CAPS %2d: %08x\n", i>>2, *p++);
		p = (DWORD *)&hel;
		for (i = 0; i < (int)hel.dwSize; i += 4)
			winlog("HEL CAPS %2d: %08x\n", i>>2, *p++);
	}

	theApp.mode320Available = false;
	theApp.mode640Available = false;
	theApp.mode800Available = false;

	// check for available fullscreen modes
	pDirectDraw->EnumDisplayModes(DDEDM_STANDARDVGAMODES, NULL, NULL, checkModesAvailable);

	DWORD flags = DDSCL_NORMAL;

	if (theApp.videoOption >= VIDEO_320x240)
		flags = DDSCL_ALLOWMODEX |
		        DDSCL_ALLOWREBOOT |
		        DDSCL_EXCLUSIVE |
		        DDSCL_FULLSCREEN;

	hret = pDirectDraw->SetCooperativeLevel(pWnd->m_hWnd, flags);

	if (hret != DD_OK)
	{
		winlog("Error SetCooperativeLevel %08x\n", hret);
		//    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWLEVEL), hret);
		return FALSE;
	}

	if (theApp.videoOption > VIDEO_4X)
	{
		hret = pDirectDraw->SetDisplayMode(theApp.fsWidth,
		                                   theApp.fsHeight,
		                                   theApp.fsColorDepth,
		                                   0,
		                                   0);
		if (hret != DD_OK)
		{
			winlog("Error SetDisplayMode %08x\n", hret);
			//      errorMessage(myLoadString(IDS_ERROR_DISP_DRAWSET), hret);
			return FALSE;
		}
	}

	DDSURFACEDESC2 ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize         = sizeof(ddsd);
	ddsd.dwFlags        = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

	if (theApp.videoOption > VIDEO_4X && theApp.tripleBuffering)
	{
		// setup triple buffering
		ddsd.dwFlags		  |= DDSD_BACKBUFFERCOUNT;
		ddsd.ddsCaps.dwCaps   |= DDSCAPS_COMPLEX | DDSCAPS_FLIP;
		ddsd.dwBackBufferCount = 2;
	}

	hret = pDirectDraw->CreateSurface(&ddsd, &ddsPrimary, NULL);
	if (hret != DD_OK)
	{
		winlog("Error primary CreateSurface %08x\n", hret);
		//    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWSURFACE), hret);
		return FALSE;
	}

	if (theApp.ddrawDebug)
	{
		DDSCAPS2 caps;
		ZeroMemory(&caps, sizeof(caps));
		ddsPrimary->GetCaps(&caps);

		winlog("Primary CAPS 1: %08x\n", caps.dwCaps);
		winlog("Primary CAPS 2: %08x\n", caps.dwCaps2);
		winlog("Primary CAPS 3: %08x\n", caps.dwCaps3);
		winlog("Primary CAPS 4: %08x\n", caps.dwCaps4);
	}

	if (theApp.videoOption > VIDEO_4X && theApp.tripleBuffering)
	{
		DDSCAPS2 caps;
		ZeroMemory(&caps, sizeof(caps));
		// this gets the third surface. The front one is the primary,
		// the second is the backbuffer and the third is the flip
		// surface
		caps.dwCaps = DDSCAPS_BACKBUFFER;

		hret = ddsPrimary->GetAttachedSurface(&caps, &ddsFlip);
		if (hret != DD_OK)
		{
			winlog("Failed to get attached surface %08x", hret);
			return FALSE;
		}
		ddsFlip->AddRef();
		clear();
	}

	// create clipper in all modes to avoid paint problems
	//  if(videoOption <= VIDEO_4X) {
	hret = pDirectDraw->CreateClipper(0, &ddsClipper, NULL);
	if (hret == DD_OK)
	{
		ddsClipper->SetHWnd(0, pWnd->m_hWnd);
		if (theApp.videoOption > VIDEO_4X)
		{
			if (theApp.tripleBuffering)
				ddsFlip->SetClipper(ddsClipper);
			else
				ddsPrimary->SetClipper(ddsClipper);
		}
		else
			ddsPrimary->SetClipper(ddsClipper);
	}
	//  }

	DDPIXELFORMAT px;

	px.dwSize = sizeof(px);

	hret = ddsPrimary->GetPixelFormat(&px);

	switch (px.dwRGBBitCount)
	{
	case 15:
	case 16:
		systemColorDepth = 16;
		break;
	case 24:
		systemColorDepth      = 24;
		theApp.filterFunction = NULL;
		break;
	case 32:
		systemColorDepth = 32;
		break;
	default:
		systemMessage(
		    IDS_ERROR_DISP_COLOR,
		    "Unsupported display setting for color depth: %d bits. \nWindows desktop must be in either 16-bit, 24-bit or 32-bit mode for this program to work in window mode.",
		    px.dwRGBBitCount);
		return FALSE;
	}
	theApp.updateFilter();
	theApp.updateIFB();

	if (failed)
		return false;

	pWnd->DragAcceptFiles(TRUE);

	return true;
}

bool DirectDrawDisplay::changeRenderSize(int w, int h)
{
	if (w != width || h != height)
	{
		if (ddsOffscreen)
		{
			ddsOffscreen->Release();
			ddsOffscreen = NULL;
		}
		if (!initializeOffscreen(w, h))
		{
			failed = true;
			return false;
		}
	}
	return true;
}

bool DirectDrawDisplay::initializeOffscreen(int w, int h)
{
	DDSURFACEDESC2 ddsd;

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize         = sizeof(ddsd);
	ddsd.dwFlags        = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	if (theApp.ddrawUseVideoMemory)
		ddsd.ddsCaps.dwCaps |= (DDSCAPS_LOCALVIDMEM|DDSCAPS_VIDEOMEMORY);
	ddsd.dwWidth  = w;
	ddsd.dwHeight = h;

	HRESULT hret = pDirectDraw->CreateSurface(&ddsd, &ddsOffscreen, NULL);

	if (hret != DD_OK)
	{
		winlog("Error offscreen CreateSurface %08x\n", hret);
		if (theApp.ddrawUseVideoMemory)
		{
			regSetDwordValue("ddrawUseVideoMemory", 0);
		}
		//    errorMessage(myLoadString(IDS_ERROR_DISP_DRAWSURFACE2), hret);
		return false;
	}

	if (theApp.ddrawDebug)
	{
		DDSCAPS2 caps;
		ZeroMemory(&caps, sizeof(caps));
		ddsOffscreen->GetCaps(&caps);

		winlog("Offscreen CAPS 1: %08x\n", caps.dwCaps);
		winlog("Offscreen CAPS 2: %08x\n", caps.dwCaps2);
		winlog("Offscreen CAPS 3: %08x\n", caps.dwCaps3);
		winlog("Offscreen CAPS 4: %08x\n", caps.dwCaps4);
	}

	DDPIXELFORMAT px;

	px.dwSize = sizeof(px);

	hret = ddsOffscreen->GetPixelFormat(&px);

	if (theApp.ddrawDebug)
	{
		DWORD *pdword = (DWORD *)&px;
		for (int ii = 0; ii < 8; ii++)
		{
			winlog("Pixel format %d %08x\n", ii, pdword[ii]);
		}
	}

	switch (px.dwRGBBitCount)
	{
	case 15:
	case 16:
		systemColorDepth = 16;
		break;
	case 24:
		systemColorDepth      = 24;
		theApp.filterFunction = NULL;
		break;
	case 32:
		systemColorDepth = 32;
		break;
	default:
		systemMessage(
		    IDS_ERROR_DISP_COLOR,
		    "Unsupported display setting for color depth: %d bits. \nWindows desktop must be in either 16-bit, 24-bit or 32-bit mode for this program to work in window mode.",
		    px.dwRGBBitCount);
		return FALSE;
	}
	if (theApp.ddrawDebug)
	{
		winlog("R Mask: %08x\n", px.dwRBitMask);
		winlog("G Mask: %08x\n", px.dwGBitMask);
		winlog("B Mask: %08x\n", px.dwBBitMask);
	}

	systemRedShift   = ffs(px.dwRBitMask);
	systemGreenShift = ffs(px.dwGBitMask);
	systemBlueShift  = ffs(px.dwBBitMask);

#ifdef MMX
	if (!theApp.disableMMX)
		cpu_mmx = theApp.detectMMX();
	else
		cpu_mmx = 0;
#endif

	if ((px.dwFlags&DDPF_RGB) != 0 &&
	    px.dwRBitMask == 0xF800 &&
	    px.dwGBitMask == 0x07E0 &&
	    px.dwBBitMask == 0x001F)
	{
		systemGreenShift++;
		Init_2xSaI(565);
		RGB_LOW_BITS_MASK = 0x821;
	}
	else if ((px.dwFlags&DDPF_RGB) != 0 &&
	         px.dwRBitMask == 0x7C00 &&
	         px.dwGBitMask == 0x03E0 &&
	         px.dwBBitMask == 0x001F)
	{
		Init_2xSaI(555);
		RGB_LOW_BITS_MASK = 0x421;
	}
	else if ((px.dwFlags&DDPF_RGB) != 0 &&
	         px.dwRBitMask == 0x001F &&
	         px.dwGBitMask == 0x07E0 &&
	         px.dwBBitMask == 0xF800)
	{
		systemGreenShift++;
		Init_2xSaI(565);
		RGB_LOW_BITS_MASK = 0x821;
	}
	else if ((px.dwFlags&DDPF_RGB) != 0 &&
	         px.dwRBitMask == 0x001F &&
	         px.dwGBitMask == 0x03E0 &&
	         px.dwBBitMask == 0x7C00)
	{
		Init_2xSaI(555);
		RGB_LOW_BITS_MASK = 0x421;
	}
	else
	{
		// 32-bit or 24-bit
		if (systemColorDepth == 32 || systemColorDepth == 24)
		{
			systemRedShift   += 3;
			systemGreenShift += 3;
			systemBlueShift  += 3;
			if (systemColorDepth == 32)
				Init_2xSaI(32);
		}
	}

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
	width  = w;
	height = h;
	return true;
}

void DirectDrawDisplay::clear()
{
	if (theApp.videoOption <= VIDEO_4X || !theApp.tripleBuffering || ddsFlip == NULL)
		return;

	DDBLTFX fx;
	ZeroMemory(&fx, sizeof(fx));
	fx.dwSize      = sizeof(fx);
	fx.dwFillColor = 0;
	ddsFlip->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx);
	ddsPrimary->Flip(NULL, 0);
	ddsFlip->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx);
	ddsPrimary->Flip(NULL, 0);
	ddsFlip->Blt(NULL, NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &fx);
	ddsPrimary->Flip(NULL, 0);
}

void DirectDrawDisplay::renderMenu()
{
	checkFullScreen();
	theApp.m_pMainWnd->DrawMenuBar();
}

void DirectDrawDisplay::checkFullScreen()
{
	if (theApp.tripleBuffering)
		pDirectDraw->FlipToGDISurface();
}

void DirectDrawDisplay::render()
{
	HRESULT hret;

	if (pDirectDraw == NULL ||
	    ddsOffscreen == NULL ||
	    ddsPrimary == NULL)
		return;

	bool fastForward = speedup;
#if (defined(WIN32) && !defined(SDL))
	if (theApp.frameSearchSkipping)
	{
		if (theApp.frameSearchFirstStep)
			fastForward = true;
		else
			return; // don't render skipped frame search frames
	}
#endif

	DDSURFACEDESC2 ddsDesc;

	ZeroMemory(&ddsDesc, sizeof(ddsDesc));

	ddsDesc.dwSize = sizeof(ddsDesc);

	hret = ddsOffscreen->Lock(NULL,
	                          &ddsDesc,
#ifndef FINAL_VERSION
	                          DDLOCK_NOSYSLOCK |
#endif
	                          DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
	                          NULL);

	if (hret == DDERR_SURFACELOST)
	{
		hret = ddsPrimary->Restore();
		if (hret == DD_OK)
		{
			hret = ddsOffscreen->Restore();

			if (hret == DD_OK)
			{
				hret = ddsOffscreen->Lock(NULL,
				                          &ddsDesc,
#ifndef FINAL_VERSION
				                          DDLOCK_NOSYSLOCK |
#endif
				                          DDLOCK_WRITEONLY | DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT,
				                          NULL);
			}
		}
	}

	u8 *data = pix;
	int dataPitch = theApp.filterWidth * (systemColorDepth / 8) + (systemColorDepth == 24 ? 0 : 4);

	if (textMethod != 0) // do not draw Lua HUD to a video dump
	{
		systemClonePixBuffer(osd);
		data = osd;
		systemRenderLua((u8 *)data, dataPitch);
	}

	if (hret == DD_OK)
	{
		if (theApp.filterFunction)
		{
			(*theApp.filterFunction)(data + dataPitch,
			                         dataPitch,
			                         (u8 *)theApp.delta,
			                         (u8 *)ddsDesc.lpSurface,
			                         ddsDesc.lPitch,
			                         theApp.filterWidth,
			                         theApp.filterHeight);
		}
		else
		{
			int copyX, copyY;
			systemGetLCDResolution(copyX, copyY);
			
			// MMX doesn't seem to be faster to copy the data
			__asm {
				mov eax, copyX;
				mov ebx, copyY;

				mov esi, data;
				mov edi, ddsDesc.lpSurface;
				mov edx, ddsDesc.lPitch;
				cmp systemColorDepth, 16;
				jnz gbaOtherColor;
				sub edx, eax;
				sub edx, eax;
				lea esi, [esi+2*eax+4];
				shr eax, 1;
gbaLoop16bit:
				mov ecx, eax;
				repz movsd;
				inc  esi;
				inc  esi;
				inc  esi;
				inc  esi;
				add  edi, edx;
				dec  ebx;
				jnz  gbaLoop16bit;
				jmp  gbaLoopEnd;
gbaOtherColor:
				cmp systemColorDepth, 32;
				jnz gbaOtherColor2;

				sub edx, eax;
				sub edx, eax;
				sub edx, eax;
				sub edx, eax;
				lea esi, [esi+4*eax+4];
gbaLoop32bit:
				mov ecx, eax;
				repz movsd;
				add  esi, 4;
				add  edi, edx;
				dec  ebx;
				jnz  gbaLoop32bit;
				jmp  gbaLoopEnd;
gbaOtherColor2:
				lea eax, [eax+2*eax];
				sub edx, eax;
gbaLoop24bit:
				mov ecx, eax;
				shr  ecx, 2;
				repz movsd;
				add  edi, edx;
				dec  ebx;
				jnz  gbaLoop24bit;
gbaLoopEnd:
			}
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
				drawTextTransp((u8 *)ddsDesc.lpSurface,
				               ddsDesc.lPitch,
				               theApp.rect.left+10,
				               theApp.rect.bottom-10,
				               buffer);
			else
				drawText((u8 *)ddsDesc.lpSurface,
				         ddsDesc.lPitch,
				         theApp.rect.left+10,
				         theApp.rect.bottom-10,
				         buffer);
		}

		if (textMethod == 1)
		{
			DrawTextMessages((u8 *)ddsDesc.lpSurface, ddsDesc.lPitch, theApp.rect.left, theApp.rect.bottom);
		}
	}
	else if (theApp.ddrawDebug)
		winlog("Error during lock: %08x\n", hret);

	hret = ddsOffscreen->Unlock(NULL);

	if (hret == DD_OK)
	{
		// the correct point where to wait
		if (theApp.vsync && !fastForward)
		{
			hret = pDirectDraw->WaitForVerticalBlank(DDWAITVB_BLOCKBEGIN, 0);
		}

		ddsOffscreen->PageLock(0);
		if (theApp.tripleBuffering && theApp.videoOption > VIDEO_4X)
		{
			hret = ddsFlip->Blt(&theApp.dest, ddsOffscreen, NULL, DDBLT_WAIT, NULL);
			if (hret == DD_OK)
			{
				if (theApp.menuToggle || !theApp.active)
				{
					pDirectDraw->FlipToGDISurface();
					ddsPrimary->SetClipper(ddsClipper);
					hret = ddsPrimary->Blt(&theApp.dest, ddsFlip, NULL, DDBLT_ASYNC, NULL);
					// if using emulation only, then we have to redraw the menu
					// everytime. It seems like a bug in DirectDraw to me as we not
					// overwritting the menu area at all.
					if (theApp.ddrawUsingEmulationOnly)
						theApp.m_pMainWnd->DrawMenuBar();
				}
				else
					hret = ddsPrimary->Flip(NULL, 0);
			}
		}
		else
		{
			hret = ddsPrimary->Blt(&theApp.dest, ddsOffscreen, NULL, DDBLT_ASYNC, NULL);

			if (hret == DDERR_SURFACELOST)
			{
				hret = ddsPrimary->Restore();

				if (hret == DD_OK)
				{
					hret = ddsPrimary->Blt(&theApp.dest, ddsOffscreen, NULL, DDBLT_ASYNC, NULL);
				}
			}
		}
		ddsOffscreen->PageUnlock(0);
	}
	else if (theApp.ddrawDebug)
		winlog("Error during unlock: %08x\n", hret);

	bool textMessageStarted = false;

	if (textMethod == 2)
	{
		HDC hdc;

		for (int slot = 0; slot < SCREEN_MESSAGE_SLOTS; slot++)
		{
			if (theApp.screenMessage[slot])
			{
				if ((theApp.screenMessageDuration[slot] < 0 || 
					(int)(GetTickCount() - theApp.screenMessageTime[slot]) < theApp.screenMessageDuration[slot]) &&
				    (!theApp.disableStatusMessage || slot == 1 || slot == 2))
				{
					if (!textMessageStarted)
					{
						textMessageStarted = true;
						ddsPrimary->SetClipper(ddsClipper);
						ddsPrimary->GetDC(&hdc);
						SetBkMode(hdc, TRANSPARENT);
						SetTextColor(hdc, textColor != 7 ? RGB(0, 0, 0) : RGB(255, 255, 255));
					}

					if (outlinedText)
					{
						// draw black outline
						const static int xd [8] = {-1, 0, 1, 1, 1, 0, -1, -1};
						const static int yd [8] = {-1, -1, -1, 0, 1, 1, 1, 0};
						for (int i = 0; i < 8; i++)
						{
							TextOut(hdc,
							        theApp.dest.left+10+xd[i],
							        theApp.dest.bottom - 20*(slot+1)+yd[i],
							        theApp.screenMessageBuffer[slot],
							        strlen(theApp.screenMessageBuffer[slot]));
						}
					}
				}
				else
				{
					theApp.screenMessage[slot] = false;
				}
			}
		}

		if (textMessageStarted)
		{
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
			SetTextColor(hdc, color);

			// draw center text
			for (int slot = 0; slot < SCREEN_MESSAGE_SLOTS; slot++)
			{
				if (theApp.screenMessage[slot])
				{
					if ((theApp.screenMessageDuration[slot] < 0 || 
						(int)(GetTickCount() - theApp.screenMessageTime[slot]) < theApp.screenMessageDuration[slot]) &&
					    (!theApp.disableStatusMessage || slot == 1 || slot == 2))
					{
						TextOut(hdc, theApp.dest.left+10, theApp.dest.bottom - 20*(slot+1), theApp.screenMessageBuffer[slot],
						        strlen(theApp.screenMessageBuffer[slot]));
					}
				}
			}
		}

		if (textMessageStarted)
		{
			ddsPrimary->ReleaseDC(hdc);
		}
	}

	if (hret != DD_OK)
	{
		if (theApp.ddrawDebug)
			winlog("Error on update screen: %08x\n", hret);
	}
}

int DirectDrawDisplay::selectFullScreenMode(GUID **pGUID)
{
	return winVideoModeSelect(theApp.m_pMainWnd, pGUID);
}

IDisplay *newDirectDrawDisplay()
{
	return new DirectDrawDisplay();
}

