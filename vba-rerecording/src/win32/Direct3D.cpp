#include "stdafx.h"

#pragma comment( lib, "d3d9" )
#pragma comment( lib, "d3dx9" )
#pragma comment( lib, "DxErr" )

#include "d3d9.h"
#include "d3dx9.h"

#include "resource.h"
#include "MainWnd.h"
#include "Reg.h"
#include "VBA.h"

//#include "../common/System.h"
#include "../common/SystemGlobals.h"
#include "../common/Text.h"
#include "../version.h"

#ifdef MMX
extern "C" bool cpu_mmx;

extern bool detectMMX();
#endif

extern int Init_2xSaI(u32);
extern void directXMessage(const char *);
extern void winlog(const char *, ...);

typedef struct _D3DTLVERTEX
{
	float    sx; /* Screen coordinates */
	float    sy;
	float    sz;
	float    rhw; /* Reciprocal of homogeneous w */
	D3DCOLOR color; /* Vertex color */
	float    tu; /* Texture coordinates */
	float    tv;
	_D3DTLVERTEX() { }
	_D3DTLVERTEX(const D3DVECTOR& v, float _rhw,
	             D3DCOLOR _color,
	             float _tu, float _tv)
	{
		sx    = v.x; sy = v.y; sz = v.z; rhw = _rhw;
		color = _color;
		tu    = _tu; tv = _tv;
	}
} D3DTLVERTEX, *LPD3DTLVERTEX;

#define D3DFVF_TLVERTEX D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1

class Direct3DDisplay : public IDisplay
{
public:
	Direct3DDisplay();
	virtual ~Direct3DDisplay();

	virtual bool initialize();
	virtual void cleanup();
	virtual void render();
	virtual void checkFullScreen();
	virtual void renderMenu();
	virtual void clear();
	virtual bool changeRenderSize(int w, int h);
	virtual void resize(int w, int h);
	virtual DISPLAY_TYPE getType() { return DIRECT_3D; };
	virtual void setOption(const char *, int);
	virtual int selectFullScreenMode(GUID * *);

private:
	HINSTANCE             d3dDLL;
	LPDIRECT3D9           pD3D;
	LPDIRECT3DDEVICE9     pDevice;
	LPDIRECT3DTEXTURE9    pTexture;
	D3DSURFACE_DESC       dsdBackBuffer;
	D3DPRESENT_PARAMETERS dpp;
	D3DFORMAT             screenFormat;
	int  width;
	int  height;
	bool filterDisabled;
	ID3DXFont *pFont;
	ID3DXSprite *pFontSprite;
	bool       failed;
	unsigned int textureSize;
	D3DTLVERTEX verts[4];


	void restoreDeviceObjects();
	void invalidateDeviceObjects();
	bool initializeOffscreen(int w, int h);
	void updateFiltering(int);
	void calculateVertices();
};

Direct3DDisplay::Direct3DDisplay()
{
	d3dDLL         = NULL;
	pD3D           = NULL;
	pDevice        = NULL;
	pTexture       = NULL;
	pFont          = NULL;
	pFontSprite    = NULL;
	screenFormat   = D3DFMT_R5G6B5;
	width          = 0;
	height         = 0;
	filterDisabled = false;
	failed         = false;
	textureSize    = 0;
}

Direct3DDisplay::~Direct3DDisplay()
{
	cleanup();
}

void Direct3DDisplay::cleanup()
{
	if (pD3D != NULL)
	{
		if (pFont)
		{
			pFont->Release();
			pFont = NULL;
		}
		if (pFontSprite)
		{
			pFontSprite->Release();
			pFontSprite = NULL;
		}

		if (pTexture)
		{
			pTexture->Release();
			pTexture = NULL;
		}

		if (pDevice)
		{
			pDevice->Release();
			pDevice = NULL;
		}

		pD3D->Release();
		pD3D = NULL;

		if (d3dDLL != NULL)
		{
			FreeLibrary(d3dDLL);
			d3dDLL = NULL;
		}
	}
}

bool Direct3DDisplay::initialize()
{
	CWnd *pWnd = theApp.m_pMainWnd;

	d3dDLL = LoadLibrary("D3D9.DLL");
	if (d3dDLL == NULL)
	{
		directXMessage("D3D9.DLL");
		return FALSE;
	}

	pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (pD3D == NULL)
	{
		winlog("Error creating Direct3D object\n");
		return FALSE;
	}

	theApp.mode320Available = false;
	theApp.mode640Available = false;
	theApp.mode800Available = false;

	D3DDISPLAYMODE mode;
	pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);

	switch (mode.Format)
	{
	case D3DFMT_R8G8B8:
		systemColorDepth = 24;
		systemRedShift   = 19;
		systemGreenShift = 11;
		systemBlueShift  = 3;
		break;
	case D3DFMT_X8R8G8B8:
		systemColorDepth = 32;
		systemRedShift   = 19;
		systemGreenShift = 11;
		systemBlueShift  = 3;
		Init_2xSaI(32);
		break;
	case D3DFMT_R5G6B5:
		systemColorDepth = 16;
		systemRedShift   = 11;
		systemGreenShift = 6;
		systemBlueShift  = 0;
		Init_2xSaI(565);
		break;
	case D3DFMT_X1R5G5B5:
		systemColorDepth = 16;
		systemRedShift   = 10;
		systemGreenShift = 5;
		systemBlueShift  = 0;
		Init_2xSaI(555);
		break;
	default:
		systemMessage(0, "Unsupport D3D format %d", mode.Format);
		return false;
	}
	theApp.fsColorDepth = systemColorDepth;

#ifdef MMX
	if (!theApp.disableMMX)
		cpu_mmx = theApp.detectMMX();
	else
		cpu_mmx = 0;
#endif

	screenFormat = mode.Format;

	// check for available fullscreen modes
	ZeroMemory(&dpp, sizeof(dpp));
	dpp.Windowed         = TRUE;
	dpp.BackBufferFormat = mode.Format;
	dpp.BackBufferCount  = 1;
	dpp.SwapEffect       = D3DSWAPEFFECT_DISCARD;
	dpp.BackBufferWidth  = theApp.surfaceSizeX;
	dpp.BackBufferHeight = theApp.surfaceSizeY;

	HRESULT hret = pD3D->CreateDevice(D3DADAPTER_DEFAULT,
	                                  D3DDEVTYPE_HAL,
	                                  pWnd->GetSafeHwnd(),
	                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE,
	                                  &dpp,
	                                  &pDevice);
	if (!SUCCEEDED(hret))
	{
		winlog("Error creating Direct3DDevice %08x\n", hret);
		return false;
	}

	restoreDeviceObjects();

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

bool Direct3DDisplay::initializeOffscreen(int w, int h)
{
	int size = 256;
	if (w > 512 || h > 512)
		size = 1024;
	else if (w > 256 || h > 256)
		size = 512;
	textureSize = size;

	UINT      ww     = size;
	UINT      hh     = size;
	D3DFORMAT format = screenFormat;

	if (SUCCEEDED(D3DXCheckTextureRequirements(pDevice,
	                                           &ww,
	                                           &hh,
	                                           NULL,
	                                           0,
	                                           &format,
	                                           D3DPOOL_MANAGED)))
	{
		if ((int)ww < w || (int)hh < h)
		{
			if (theApp.filterFunction)
			{
				filterDisabled        = true;
				theApp.filterFunction = NULL;
				systemMessage(0, "3D card cannot support needed texture size for filter function. Disabling it");
			}
		}
		else
			filterDisabled = false;
		if (SUCCEEDED(D3DXCreateTexture(pDevice,
		                                ww,
		                                hh,
		                                0,
		                                0,
		                                format,
		                                D3DPOOL_MANAGED,
		                                &pTexture)))
		{
			width  = w;
			height = h;
			return true;
		}
	}
	return false;
}

void Direct3DDisplay::updateFiltering(int filter)
{
	switch (filter)
	{
	default:
	case 0:
		// point filtering
		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		break;
	case 1:
		// bilinear
		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		pDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		break;
	}
}

void Direct3DDisplay::restoreDeviceObjects()
{
	// Store render target surface desc
	LPDIRECT3DSURFACE9 pBackBuffer;
	HRESULT hr;
	if (SUCCEEDED(hr = pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer)))
	{
		pBackBuffer->GetDesc(&dsdBackBuffer);
		pBackBuffer->Release();
	}
	else
		systemMessage(0, "Failed GetBackBuffer %08x", hr);

	// Set up the texture
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	int filter = regQueryDwordValue("d3dFilter", 0);
	if (filter < 0 || filter > 3)
		filter = 0;
	updateFiltering(filter);

	// Set miscellaneous render states
	pDevice->SetRenderState(D3DRS_DITHERENABLE,   TRUE);
	pDevice->SetRenderState(D3DRS_ZENABLE,        FALSE);

	// Set the projection matrix
	D3DXMATRIX matProj;
	FLOAT      fAspect = ((FLOAT)dsdBackBuffer.Width) / dsdBackBuffer.Height;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f);
	pDevice->SetTransform(D3DTS_PROJECTION, &matProj);

	// turn off lighting
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	if (pFont)
	{
		pFont->Release();
		pFont = NULL;
	}
	if (pFontSprite)
	{
		pFontSprite->Release();
		pFontSprite = NULL;
	}
	// Create a D3D font using D3DX
	D3DXCreateFont(pDevice, 14, 0, FW_BOLD, 0, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &pFont);
	if (pFont != NULL)
	{
		pFont->OnResetDevice();
	}
	// Create a font sprite
	D3DXCreateSprite(pDevice, &pFontSprite);
}

void Direct3DDisplay::clear()
{}

void Direct3DDisplay::renderMenu()
{
	checkFullScreen();
	if (theApp.m_pMainWnd)
		theApp.m_pMainWnd->DrawMenuBar();
}

void Direct3DDisplay::checkFullScreen()
{
	//  if(tripleBuffering)
	//    pDirect3D->FlipToGDISurface();
}

void Direct3DDisplay::calculateVertices()
{
	// color
	D3DCOLOR col = 0xFFFFFFFF;

	// calculate rhw
	FLOAT z	  = 1.0f;
	FLOAT rhw = 1.0f / (z * 990.0f + 10.0f);

	// -0.5f is necessary in order to match texture alignment to display pixels
	FLOAT left     = -0.5f;
	FLOAT right    = dpp.BackBufferWidth - 0.5f;
	FLOAT top      = -0.5f;
	FLOAT bottom   = dpp.BackBufferHeight - 0.5f;

	FLOAT textureX = (FLOAT)theApp.rect.right / (FLOAT)textureSize;
	FLOAT textureY = (FLOAT)theApp.rect.bottom / (FLOAT)textureSize;

//#define D3D_DRAW_SINGLE_RECTANGLE
#ifdef D3D_DRAW_SINGLE_RECTANGLE
	// set up a rectangle
	verts[0] = D3DTLVERTEX(D3DXVECTOR3(left,  top,    z), rhw, col, 0.0f,     0.0f);
	verts[1] = D3DTLVERTEX(D3DXVECTOR3(right, top,    z), rhw, col, textureX, 0.0f);
	verts[2] = D3DTLVERTEX(D3DXVECTOR3(right, bottom, z), rhw, col, textureX, textureY);
	verts[3] = D3DTLVERTEX(D3DXVECTOR3(left,  bottom, z), rhw, col, 0.0f,     textureY);
#else
	// set up triangles
	verts[0] = D3DTLVERTEX(D3DXVECTOR3(left,  bottom, z), rhw, col, 0.0f,     textureY);
	verts[1] = D3DTLVERTEX(D3DXVECTOR3(left,  top,    z), rhw, col, 0.0f,     0.0f);
	verts[2] = D3DTLVERTEX(D3DXVECTOR3(right, bottom, z), rhw, col, textureX, textureY);
	verts[3] = D3DTLVERTEX(D3DXVECTOR3(right, top,    z), rhw, col, textureX, 0.0f);
#endif
}

void Direct3DDisplay::render()
{
	if (!pDevice)
		return;

	// Test the cooperative level to see if it's okay to render
	if (FAILED(pDevice->TestCooperativeLevel()))
	{
		return;
	}
	pDevice->Clear(0L, NULL, D3DCLEAR_TARGET, 0x000000ff, 1.0f, 0L);

	u8 *data = pix;
	int dataPitch = theApp.filterWidth * (systemColorDepth / 8) + (systemColorDepth == 24 ? 0 : 4);

	if (textMethod != 0) // do not draw Lua HUD to a video dump
	{
		systemClonePixBuffer(osd);
		data = osd;
		systemRenderLua((u8 *)data, dataPitch);
	}

	if (SUCCEEDED(pDevice->BeginScene()))
	{
		D3DLOCKED_RECT locked;
		if (pTexture && SUCCEEDED(pTexture->LockRect(0, &locked, NULL, 0)))
		{
			if (theApp.filterFunction)
			{
					theApp.filterFunction(data + dataPitch,
					                      dataPitch,
					                      (u8 *)theApp.delta,
					                      (u8 *)locked.pBits,
					                      locked.Pitch,
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
					mov edi, locked.pBits;
					mov edx, locked.Pitch;
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

			if (theApp.videoOption > VIDEO_4X && theApp.showSpeed)
			{
				char buffer[30];
				if (theApp.showSpeed == 1)
					sprintf(buffer, "%3d%%", systemSpeed);
				else
					sprintf(buffer, "%3d%%(%d, %d fps)", systemSpeed,
					        systemFrameSkip,
					        theApp.showRenderedFrames);
				if (theApp.showSpeedTransparent)
					drawTextTransp((u8 *)locked.pBits,
					               locked.Pitch,
					               theApp.rect.left+10,
					               theApp.rect.bottom-10,
					               buffer);
				else
					drawText((u8 *)locked.pBits,
					         locked.Pitch,
					         theApp.rect.left+10,
					         theApp.rect.bottom-10,
					         buffer);
			}

			if (textMethod == 1)
			{
				DrawTextMessages((u8 *)locked.pBits, locked.Pitch, theApp.rect.left, theApp.rect.bottom);
			}

			pTexture->UnlockRect(0);

			// set the texture
			pDevice->SetTexture(0, pTexture);

			// configure shader for vertex type
			pDevice->SetVertexShader(NULL);
			pDevice->SetFVF(D3DFVF_TLVERTEX);

#ifdef D3D_DRAW_SINGLE_RECTANGLE
			// draw the rectangle
				pDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, verts, sizeof(D3DTLVERTEX));
			//#undef D3D_DRAW_RECT
#else
			// draw the triangles
			pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(D3DTLVERTEX));
#endif
		}

		if (textMethod == 2)
		{
			for (int slot = 0; slot < SCREEN_MESSAGE_SLOTS; slot++)
			{
				if (theApp.screenMessage[slot])
				{
					if ((theApp.screenMessageDuration[slot] < 0 || 
						(int)(GetTickCount() - theApp.screenMessageTime[slot]) < theApp.screenMessageDuration[slot]) &&
					    (!theApp.disableStatusMessage || slot == 1 || slot == 2) && pFont != NULL && pFontSprite != NULL)
					{
						pFontSprite->Begin(D3DXSPRITE_ALPHABLEND | D3DXSPRITE_SORT_TEXTURE);

						RECT     r;
						D3DCOLOR color;

						r.left   = 10;
						r.top    = dpp.BackBufferHeight - 20*(slot+1);
						r.right  = dpp.BackBufferWidth - 10;
						r.bottom = r.top + 20;

						if (outlinedText)
						{
							color = textColor != 7 ? D3DCOLOR_ARGB(255, 0, 0, 0) : D3DCOLOR_ARGB(255, 255, 255, 255);

							// draw outline
							const static int xd [8] = {-1, 0, 1, 1, 1, 0, -1, -1};
							const static int yd [8] = {-1, -1, -1, 0, 1, 1, 1, 0};
							for (int i = 0; i < 8; i++)
							{
								RECT r2 = r;
								r2.left += xd[i]; r2.right += xd[i];
								r2.top  += yd[i]; r2.bottom += yd[i];

								pFont->DrawText(pFontSprite, theApp.screenMessageBuffer[slot], -1, &r2, 0, color);
							}
						}

						// draw center text
						switch (textColor)
						{
						case 0:
							color = D3DCOLOR_ARGB(255, 255, 255, 255); break;
						case 1:
							color = D3DCOLOR_ARGB(255, 255, 0, 0); break;
						case 2:
							color = D3DCOLOR_ARGB(255, 255, 255, 0); break;
						case 3:
							color = D3DCOLOR_ARGB(255, 0, 255, 0); break;
						case 4:
							color = D3DCOLOR_ARGB(255, 0, 255, 255); break;
						case 5:
							color = D3DCOLOR_ARGB(255, 0, 0, 255); break;
						case 6:
							color = D3DCOLOR_ARGB(255, 255, 0, 255); break;
						case 7:
							color = D3DCOLOR_ARGB(255, 0, 0, 0); break;
						}
						pFont->DrawText(pFontSprite, theApp.screenMessageBuffer[slot], -1, &r, 0, color);

						pFontSprite->End();
					}
					else
					{
						theApp.screenMessage[slot] = false;
					}
				}
			}
		}

		pDevice->EndScene();

		pDevice->Present(NULL, NULL, NULL, NULL);
	}
}

void Direct3DDisplay::invalidateDeviceObjects()
{
	if (pFontSprite)
	{
		pFontSprite->Release();
	}
	pFontSprite = NULL;

	if (pFont)
	{
		pFont->OnLostDevice();
		pFont->Release();
	}
	pFont = NULL;
}

void Direct3DDisplay::resize(int w, int h)
{
	if (pDevice && w > 0 && h > 0)
	{
		dpp.BackBufferWidth  = w;
		dpp.BackBufferHeight = h;
		HRESULT hr;
		invalidateDeviceObjects();
		if (SUCCEEDED(hr = pDevice->Reset(&dpp)))
		{
			restoreDeviceObjects();
		}
		else
			systemMessage(0, "Failed device reset %08x", hr);
	}
	calculateVertices();
}

bool Direct3DDisplay::changeRenderSize(int w, int h)
{
	if (w != width || h != height)
	{
		if (pTexture)
		{
			pTexture->Release();
			pTexture = NULL;
		}
		if (!initializeOffscreen(w, h))
		{
			failed = true;
			return false;
		}
	}
	calculateVertices();

	return true;
}

void Direct3DDisplay::setOption(const char *option, int value)
{
	if (!strcmp(option, "d3dFilter"))
		updateFiltering(value);
}

int Direct3DDisplay::selectFullScreenMode(GUID * *)
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

IDisplay *newDirect3DDisplay()
{
	return new Direct3DDisplay();
}

