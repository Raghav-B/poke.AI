//#define USE_GETASYNCKEYSTATE_FOR_KEYBOARD

#include "stdafx.h"

#define DIRECTINPUT_VERSION 0x0500
#include "dinput.h"

#include "resource.h"
#include "Input.h"
#include "Reg.h"
#include "WinResUtil.h"

// master keyboard translation table
static const struct {
	int dik;
	int vk;
	int ascii;
} win_key_trans_table[] = {
	// dinput key		virtual key		ascii
	{ DIK_ESCAPE,		VK_ESCAPE,	 	27 },
	{ DIK_1,			'1',			'1' },
	{ DIK_2,			'2',			'2' },
	{ DIK_3,			'3',			'3' },
	{ DIK_4,			'4',			'4' },
	{ DIK_5,			'5',			'5' },
	{ DIK_6,			'6',			'6' },
	{ DIK_7,			'7',			'7' },
	{ DIK_8,			'8',			'8' },
	{ DIK_9,			'9',			'9' },
	{ DIK_0,			'0',			'0' },
	{ DIK_MINUS, 		VK_OEM_MINUS,	'-' },
	{ DIK_EQUALS,		VK_OEM_PLUS,	'=' },
	{ DIK_BACK, 		VK_BACK, 		8 },
	{ DIK_TAB, 			VK_TAB, 		9 },
	{ DIK_Q,			'Q',			'Q' },
	{ DIK_W,			'W',			'W' },
	{ DIK_E,			'E',			'E' },
	{ DIK_R,			'R',			'R' },
	{ DIK_T,			'T',			'T' },
	{ DIK_Y,			'Y',			'Y' },
	{ DIK_U,			'U',			'U' },
	{ DIK_I,			'I',			'I' },
	{ DIK_O,			'O',			'O' },
	{ DIK_P,			'P',			'P' },
	{ DIK_LBRACKET, 	VK_OEM_4,		'[' },
	{ DIK_RBRACKET, 	VK_OEM_6,		']' },
	{ DIK_RETURN, 		VK_RETURN, 		13 },
	{ DIK_LCONTROL, 	VK_LCONTROL, 	0 },
	{ DIK_A,			'A',			'A' },
	{ DIK_S,			'S',			'S' },
	{ DIK_D,			'D',			'D' },
	{ DIK_F,			'F',			'F' },
	{ DIK_G,			'G',			'G' },
	{ DIK_H,			'H',			'H' },
	{ DIK_J,			'J',			'J' },
	{ DIK_K,			'K',			'K' },
	{ DIK_L,			'L',			'L' },
	{ DIK_SEMICOLON,	VK_OEM_1,		';' },
	{ DIK_APOSTROPHE,	VK_OEM_7,		'\'' },
	{ DIK_GRAVE, 		VK_OEM_3,		'`' },
	{ DIK_LSHIFT, 		VK_LSHIFT, 		0 },
	{ DIK_BACKSLASH, 	VK_OEM_5,		'\\' },
	{ DIK_Z,			'Z',			'Z' },
	{ DIK_X,			'X',			'X' },
	{ DIK_C,			'C',			'C' },
	{ DIK_V,			'V',			'V' },
	{ DIK_B,			'B',			'B' },
	{ DIK_N,			'N',			'N' },
	{ DIK_M,			'M',			'M' },
	{ DIK_COMMA,		VK_OEM_COMMA,	',' },
	{ DIK_PERIOD, 		VK_OEM_PERIOD,	'.' },
	{ DIK_SLASH, 		VK_OEM_2,		'/' },
	{ DIK_RSHIFT, 		VK_RSHIFT, 		0 },
	{ DIK_MULTIPLY, 	VK_MULTIPLY,	'*' },
	{ DIK_LMENU, 		VK_LMENU, 		0 },
	{ DIK_SPACE, 		VK_SPACE,		' ' },
	{ DIK_CAPITAL, 		VK_CAPITAL, 	0 },
	{ DIK_F1,			VK_F1, 			0 },
	{ DIK_F2,			VK_F2, 			0 },
	{ DIK_F3,			VK_F3, 			0 },
	{ DIK_F4,			VK_F4, 			0 },
	{ DIK_F5,			VK_F5, 			0 },
	{ DIK_F6,			VK_F6, 			0 },
	{ DIK_F7,			VK_F7, 			0 },
	{ DIK_F8,			VK_F8, 			0 },
	{ DIK_F9,			VK_F9, 			0 },
	{ DIK_F10,			VK_F10, 		0 },
	{ DIK_NUMLOCK,		VK_NUMLOCK, 	0 },
	{ DIK_SCROLL,		VK_SCROLL, 		0 },
	{ DIK_NUMPAD7,		VK_NUMPAD7, 	0 },
	{ DIK_NUMPAD8,		VK_NUMPAD8, 	0 },
	{ DIK_NUMPAD9,		VK_NUMPAD9, 	0 },
	{ DIK_SUBTRACT,		VK_SUBTRACT, 	0 },
	{ DIK_NUMPAD4,		VK_NUMPAD4, 	0 },
	{ DIK_NUMPAD5,		VK_NUMPAD5, 	0 },
	{ DIK_NUMPAD6,		VK_NUMPAD6, 	0 },
	{ DIK_ADD,			VK_ADD, 		0 },
	{ DIK_NUMPAD1,		VK_NUMPAD1, 	0 },
	{ DIK_NUMPAD2,		VK_NUMPAD2, 	0 },
	{ DIK_NUMPAD3,		VK_NUMPAD3, 	0 },
	{ DIK_NUMPAD0,		VK_NUMPAD0, 	0 },
	{ DIK_DECIMAL,		VK_DECIMAL, 	0 },
	{ DIK_F11,			VK_F11, 		0 },
	{ DIK_F12,			VK_F12, 		0 },
	{ DIK_F13,			VK_F13, 		0 },
	{ DIK_F14,			VK_F14, 		0 },
	{ DIK_F15,			VK_F15, 		0 },
	{ DIK_NUMPADENTER,	VK_RETURN, 		0 },
	{ DIK_RCONTROL,		VK_RCONTROL, 	0 },
	{ DIK_DIVIDE,		VK_DIVIDE, 		0 },
	{ DIK_SYSRQ, 		0, 				0 },
	{ DIK_RMENU,		VK_RMENU, 		0 },
	{ DIK_HOME,			VK_HOME, 		0 },
	{ DIK_UP,			VK_UP, 			0 },
	{ DIK_PRIOR,		VK_PRIOR, 		0 },
	{ DIK_LEFT,			VK_LEFT, 		0 },
	{ DIK_RIGHT,		VK_RIGHT, 		0 },
	{ DIK_END,			VK_END, 		0 },
	{ DIK_DOWN,			VK_DOWN, 		0 },
	{ DIK_NEXT,			VK_NEXT, 		0 },
	{ DIK_INSERT,		VK_INSERT, 		0 },
	{ DIK_DELETE,		VK_DELETE, 		0 },
	{ DIK_LWIN,			VK_LWIN, 		0 },
	{ DIK_RWIN,			VK_RWIN, 		0 },
	{ DIK_APPS,			VK_APPS, 		0 },
	{ DIK_PAUSE,		VK_PAUSE,		0 },
	{ 0,				VK_CANCEL,		0 },

	// New keys introduced in Windows 2000. These have no MAME codes to
	// preserve compatibility with old config files that may refer to them
	// as e.g. FORWARD instead of e.g. KEYCODE_WEBFORWARD. They need table
	// entries anyway because otherwise they aren't recognized when
	// GetAsyncKeyState polling is used (as happens currently when MAME is
	// paused). Some codes are missing because the mapping to vkey codes
	// isn't clear, and MapVirtualKey is no help.

	{ DIK_MUTE,			VK_VOLUME_MUTE,			0 },
	{ DIK_VOLUMEDOWN,	VK_VOLUME_DOWN,			0 },
	{ DIK_VOLUMEUP,		VK_VOLUME_UP,			0 },
	{ DIK_WEBHOME,		VK_BROWSER_HOME,		0 },
	{ DIK_WEBSEARCH,	VK_BROWSER_SEARCH,		0 },
	{ DIK_WEBFAVORITES,	VK_BROWSER_FAVORITES,	0 },
	{ DIK_WEBREFRESH,	VK_BROWSER_REFRESH,		0 },
	{ DIK_WEBSTOP,		VK_BROWSER_STOP,		0 },
	{ DIK_WEBFORWARD,	VK_BROWSER_FORWARD,		0 },
	{ DIK_WEBBACK,		VK_BROWSER_BACK,		0 },
	{ DIK_MAIL,			VK_LAUNCH_MAIL,			0 },
	{ DIK_MEDIASELECT,	VK_LAUNCH_MEDIA_SELECT,	0 },
};

extern void directXMessage(const char *);
extern void winlog(const char *msg, ...);

#define POV_UP    1
#define POV_DOWN  2
#define POV_RIGHT 4
#define POV_LEFT  8

class DirectInput : public Input
{
private:
	HINSTANCE dinputDLL;
public:
	virtual void checkDevices();
	DirectInput();
	virtual ~DirectInput();

	virtual bool initialize();
	virtual bool readDevices();
	virtual u32 readDevice(int which, bool sensor);
	virtual CString getKeyName(LONG_PTR key);
	virtual void checkKeys();
	virtual void activate();
	virtual void loadSettings();
	virtual void saveSettings();
};

struct deviceInfo
{
	LPDIRECTINPUTDEVICE device;
	BOOL isPolled;
	int  nButtons;
	int  nAxes;
	int  nPovs;
	BOOL first;
	struct
	{
		DWORD offset;
		LONG  center;
		LONG  negative;
		LONG  positive;
	} axis[8];
	int needed;
	union
	{
		UCHAR      data[256];
		DIJOYSTATE state;
	};
};

static deviceInfo *  currentDevice = NULL;
static int           numDevices    = 1;
static deviceInfo *  pDevices      = NULL;
static LPDIRECTINPUT pDirectInput  = NULL;
static int           joyDebug      = 0;
static int           axisNumber    = 0;

USHORT joypad[4][13] = {
	{
		DIK_LEFT,  DIK_RIGHT,
		DIK_UP,    DIK_DOWN,
		DIK_Z,     DIK_X,
		DIK_RETURN, DIK_BACK,
		DIK_A,     DIK_S,
		DIK_SPACE, DIK_F12,
		DIK_C
	},
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

USHORT motion[4] = {
	DIK_NUMPAD4, DIK_NUMPAD6, DIK_NUMPAD8, DIK_NUMPAD2
};

static int winReadKey(char *name, int num)
{
	char buffer[80];

	sprintf(buffer, "Joy%d_%s", num, name);

	return regQueryDwordValue(buffer, (DWORD)-1);
}

void winReadKeys()
{
	int key = -1;

	for (int i = 0; i < 4; i++)
	{
		key = winReadKey("Left", i);
		if (key != -1)
			joypad[i][KEY_LEFT] = key;
		key = winReadKey("Right", i);
		if (key != -1)
			joypad[i][KEY_RIGHT] = key;
		key = winReadKey("Up", i);
		if (key != -1)
			joypad[i][KEY_UP] = key;
		key = winReadKey("Down", i);
		if (key != -1)
			joypad[i][KEY_DOWN] = key;
		key = winReadKey("A", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_A] = key;
		key = winReadKey("B", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_B] = key;
		key = winReadKey("L", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_L] = key;
		key = winReadKey("R", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_R] = key;
		key = winReadKey("Start", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_START] = key;
		key = winReadKey("Select", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_SELECT] = key;
		key = winReadKey("Speed", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_SPEED] = key;
		key = winReadKey("Capture", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_CAPTURE] = key;
		key = winReadKey("GS", i);
		if (key != -1)
			joypad[i][KEY_BUTTON_GS] = key;
	}
	key = regQueryDwordValue("Motion_Left", (DWORD)-1);
	if (key != -1)
		motion[KEY_LEFT] = key;
	key = regQueryDwordValue("Motion_Right", (DWORD)-1);
	if (key != -1)
		motion[KEY_RIGHT] = key;
	key = regQueryDwordValue("Motion_Up", (DWORD)-1);
	if (key != -1)
		motion[KEY_UP] = key;
	key = regQueryDwordValue("Motion_Down", (DWORD)-1);
	if (key != -1)
		motion[KEY_DOWN] = key;
}

static void winSaveKey(char *name, int num, USHORT value)
{
	char buffer[80];

	sprintf(buffer, "Joy%d_%s", num, name);

	regSetDwordValue(buffer, value);
}

void winSaveKeys()
{
	for (int i = 0; i < 4; i++)
	{
		winSaveKey("Left", i, joypad[i][KEY_LEFT]);
		winSaveKey("Right", i, joypad[i][KEY_RIGHT]);
		winSaveKey("Up", i, joypad[i][KEY_UP]);
		winSaveKey("Speed", i, joypad[i][KEY_BUTTON_SPEED]);
		winSaveKey("Capture", i, joypad[i][KEY_BUTTON_CAPTURE]);
		winSaveKey("GS", i, joypad[i][KEY_BUTTON_GS]);
		winSaveKey("Down", i, joypad[i][KEY_DOWN]);
		winSaveKey("A", i, joypad[i][KEY_BUTTON_A]);
		winSaveKey("B", i, joypad[i][KEY_BUTTON_B]);
		winSaveKey("L", i, joypad[i][KEY_BUTTON_L]);
		winSaveKey("R", i, joypad[i][KEY_BUTTON_R]);
		winSaveKey("Start", i, joypad[i][KEY_BUTTON_START]);
		winSaveKey("Select", i, joypad[i][KEY_BUTTON_SELECT]);
	}
	regSetDwordValue("joyVersion", 1);

	regSetDwordValue("Motion_Left",
	                 motion[KEY_LEFT]);
	regSetDwordValue("Motion_Right",
	                 motion[KEY_RIGHT]);
	regSetDwordValue("Motion_Up",
	                 motion[KEY_UP]);
	regSetDwordValue("Motion_Down",
	                 motion[KEY_DOWN]);
}

static BOOL CALLBACK EnumAxesCallback(const DIDEVICEOBJECTINSTANCE*pdidoi,
                                      VOID*pContext)
{
	DIPROPRANGE diprg;
	diprg.diph.dwSize       = sizeof(DIPROPRANGE);
	diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	diprg.diph.dwHow        = DIPH_BYOFFSET;
	diprg.diph.dwObj        = pdidoi->dwOfs; // Specify the enumerated axis

	diprg.lMin = -32768;
	diprg.lMax = 32767;
	// try to set the range
	if (FAILED(currentDevice->device->SetProperty(DIPROP_RANGE, &diprg.diph)))
	{
		// Get the range for the axis
		if (FAILED(currentDevice->device->
		           GetProperty(DIPROP_RANGE, &diprg.diph)))
		{
			return DIENUM_STOP;
		}
	}

	DIPROPDWORD didz;

	didz.diph.dwSize       = sizeof(didz);
	didz.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	didz.diph.dwHow        = DIPH_BYOFFSET;
	didz.diph.dwObj        = pdidoi->dwOfs;

	didz.dwData = 5000;

	currentDevice->device->SetProperty(DIPROP_DEADZONE, &didz.diph);

	LONG center    = (diprg.lMin + diprg.lMax)/2;
	LONG threshold = (diprg.lMax - center)/2;

	// only 8 axis supported
	if (axisNumber < 8)
	{
		currentDevice->axis[axisNumber].center   = center;
		currentDevice->axis[axisNumber].negative = center - threshold;
		currentDevice->axis[axisNumber].positive = center + threshold;
		currentDevice->axis[axisNumber].offset   = pdidoi->dwOfs;
	}
	axisNumber++;
	return DIENUM_CONTINUE;
}

static BOOL CALLBACK EnumPovsCallback(const DIDEVICEOBJECTINSTANCE*pdidoi,
                                      VOID*pContext)
{
	return DIENUM_CONTINUE;
}

static BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCE pInst,
                                           LPVOID lpvContext)
{
	ZeroMemory(&pDevices[numDevices], sizeof(deviceInfo));

	HRESULT hRet = pDirectInput->CreateDevice(pInst->guidInstance,
	                                          &pDevices[numDevices].device,
	                                          NULL);

	if (hRet != DI_OK)
		return DIENUM_STOP;

	DIDEVCAPS caps;
	caps.dwSize = sizeof(DIDEVCAPS);

	hRet = pDevices[numDevices].device->GetCapabilities(&caps);

	if (hRet == DI_OK)
	{
		if (caps.dwFlags & DIDC_POLLEDDATAFORMAT ||
		    caps.dwFlags & DIDC_POLLEDDEVICE)
			pDevices[numDevices].isPolled = TRUE;

		pDevices[numDevices].nButtons = caps.dwButtons;
		pDevices[numDevices].nAxes    = caps.dwAxes;
		pDevices[numDevices].nPovs    = caps.dwPOVs;

		for (int i = 0; i < 6; i++)
		{
			pDevices[numDevices].axis[i].center   = 0x8000;
			pDevices[numDevices].axis[i].negative = 0x4000;
			pDevices[numDevices].axis[i].positive = 0xc000;
		}
	}
	else if (joyDebug)
		winlog("Failed to get device capabilities %08x\n", hRet);

	if (joyDebug)
	{
		// don't translate. debug only
		winlog("******************************\n");
		winlog("Joystick %2d name    : %s\n", numDevices, pInst->tszProductName);
	}

	numDevices++;

	return DIENUM_CONTINUE;
}

BOOL CALLBACK DIEnumDevicesCallback2(LPCDIDEVICEINSTANCE pInst,
                                     LPVOID lpvContext)
{
	numDevices++;

	return DIENUM_CONTINUE;
}

static int getPovState(DWORD value)
{
	int state = 0;
	if (LOWORD(value) != 0xFFFF)
	{
		if (value < 9000 || value > 27000)
			state |= POV_UP;
		if (value > 0 && value < 18000)
			state |= POV_RIGHT;
		if (value > 9000 && value < 27000)
			state |= POV_DOWN;
		if (value > 18000)
			state |= POV_LEFT;
	}
	return state;
}

static void checkKeys()
{
	LONG_PTR dev = 0;
	int      i;

	for (i = 0; i < numDevices; i++)
		pDevices[i].needed = 0;

	for (i = 0; i < 4; i++)
	{
		dev = joypad[i][KEY_LEFT] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_LEFT] = DIK_LEFT;

		dev = joypad[i][KEY_RIGHT] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_RIGHT] = DIK_RIGHT;

		dev = joypad[i][KEY_UP] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_UP] = DIK_UP;

		dev = joypad[i][KEY_DOWN] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_DOWN] = DIK_DOWN;

		dev = joypad[i][KEY_BUTTON_A] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_A] = DIK_Z;

		dev = joypad[i][KEY_BUTTON_B] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_B] = DIK_X;

		dev = joypad[i][KEY_BUTTON_L] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_L] = DIK_A;

		dev = joypad[i][KEY_BUTTON_R] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_R] = DIK_S;

		dev = joypad[i][KEY_BUTTON_START] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_START] = DIK_RETURN;

		dev = joypad[i][KEY_BUTTON_SELECT] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_SELECT] = DIK_BACK;

		dev = joypad[i][KEY_BUTTON_SPEED] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_SPEED] = DIK_SPACE;

		dev = joypad[i][KEY_BUTTON_CAPTURE] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_CAPTURE] = DIK_F12;

		dev = joypad[i][KEY_BUTTON_GS] >> 8;
		if (dev < numDevices && dev >= 0)
			pDevices[dev].needed = 1;
		else
			joypad[i][KEY_BUTTON_GS] = DIK_C;
	}

	dev = motion[KEY_UP] >> 8;
	if (dev < numDevices && dev >= 0)
		pDevices[dev].needed = 1;
	else
		motion[KEY_UP] = DIK_NUMPAD8;

	dev = motion[KEY_DOWN] >> 8;
	if (dev < numDevices && dev >= 0)
		pDevices[dev].needed = 1;
	else
		motion[KEY_DOWN] = DIK_NUMPAD2;

	dev = motion[KEY_LEFT] >> 8;
	if (dev < numDevices && dev >= 0)
		pDevices[dev].needed = 1;
	else
		motion[KEY_LEFT] = DIK_NUMPAD4;

	dev = motion[KEY_RIGHT] >> 8;
	if (dev < numDevices && dev >= 0)
		pDevices[dev].needed = 1;
	else
		motion[KEY_RIGHT] = DIK_NUMPAD6;
}

#define KEYDOWN(buffer, key) (buffer[key] & 0x80)

static bool IsKeyDownAsync (WORD KeyIdent)
{
	//if (KeyIdent == 0 || KeyIdent == VK_ESCAPE) // if it's the 'disabled' key, it's never pressed
	//	return false;

	//if (!GUI.BackgroundInput && GUI.hWnd != GetForegroundWindow())
	//	return false;

	// the pause key is special, need this to catch all presses of it
	// Both GetKeyState and GetAsyncKeyState cannot catch it anyway,
	// so this should be handled in WM_KEYDOWN message.
	if (KeyIdent == VK_PAUSE)
	{
		return false;
//		if(GetAsyncKeyState(VK_PAUSE)) // not &'ing this with 0x8000 is intentional and necessary
//			return true;
	}

	if (KeyIdent == VK_CAPITAL || KeyIdent == VK_NUMLOCK || KeyIdent == VK_SCROLL)
		return ((GetKeyState(KeyIdent) & 0x01) != 0);
	else
		return ((GetAsyncKeyState(KeyIdent) & 0x8000) != 0);
	//return ((GetKeyState (KeyIdent) & 0x80) != 0);
}

static bool readKeyboard()
{
#ifndef USE_GETASYNCKEYSTATE_FOR_KEYBOARD
	if (pDevices[0].needed)
	{
		HRESULT hret = pDevices[0].device->
		               GetDeviceState(256,
		                              (LPVOID)pDevices[0].data);

		if (hret == DIERR_INPUTLOST || hret == DIERR_NOTACQUIRED)
		{
			hret = pDevices[0].device->Acquire();
			if (hret != DI_OK)
				return false;
			hret = pDevices[0].device->GetDeviceState(256, (LPVOID)pDevices[0].data);
		}

		return hret == DI_OK;
	}
#else
	for (int i = 0; i < sizeof(win_key_trans_table)/sizeof(win_key_trans_table[0]); i++) {
		pDevices[0].data[win_key_trans_table[i].dik] = IsKeyDownAsync(win_key_trans_table[i].vk) ? 0x80 : 0;
	}
#endif
	return true;
}

static bool readJoystick(int joy)
{
	if (pDevices[joy].needed)
	{
		if (pDevices[joy].isPolled)
			((LPDIRECTINPUTDEVICE2)pDevices[joy].device)->Poll();

		HRESULT hret = pDevices[joy].device->
		               GetDeviceState(sizeof(DIJOYSTATE),
		                              (LPVOID)&pDevices[joy].state);

		if (hret == DIERR_INPUTLOST || hret == DIERR_NOTACQUIRED)
		{
			hret = pDevices[joy].device->Acquire();

			if (hret == DI_OK)
			{
				if (pDevices[joy].isPolled)
					((LPDIRECTINPUTDEVICE2)pDevices[joy].device)->Poll();

				hret = pDevices[joy].device->
				       GetDeviceState(sizeof(DIJOYSTATE),
				                      (LPVOID)&pDevices[joy].state);
			}
		}

		return hret == DI_OK;
	}

	return true;
}

static void checkKeyboard()
{
	// mham fix. Patch #1378104
	UCHAR   keystate[256];
	HRESULT hret = pDevices[0].device->Acquire();

	if (pDevices[0].first)
	{
		pDevices[0].device->GetDeviceState(256, (LPVOID)pDevices[0].data);
		pDevices[0].first = FALSE;
		return;
	}

	hret = pDevices[0].device->
	       GetDeviceState(256, (LPVOID)keystate);

	if (hret == DIERR_INPUTLOST || hret == DIERR_NOTACQUIRED)
	{
		return;
	}

	if (hret == DI_OK)
	{
		for (int i = 0; i < 256; i++)
		{
			if (keystate[i] == pDevices[0].data[i])
				continue;
			if (KEYDOWN(keystate, i))
			{
				SendMessage(GetFocus(), JOYCONFIG_MESSAGE, 0, i);
				break;
			}
		}
	}
	memcpy(pDevices[0].data, keystate, sizeof(UCHAR) * 256);
}

static void checkJoypads()
{
	DIDEVICEOBJECTINSTANCE di;

	ZeroMemory(&di, sizeof(DIDEVICEOBJECTINSTANCE));

	di.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);

	int i = 0;

	DIJOYSTATE joystick;

	for (i = 1; i < numDevices; i++)
	{
		HRESULT hret = pDevices[i].device->Acquire();

		if (pDevices[i].isPolled)
			((LPDIRECTINPUTDEVICE2)pDevices[i].device)->Poll();

		hret = pDevices[i].device->GetDeviceState(sizeof(joystick), &joystick);

		int j;

		if (pDevices[i].first)
		{
			memcpy(&pDevices[i].state, &joystick, sizeof(joystick));
			pDevices[i].first = FALSE;
			continue;
		}

		for (j = 0; j < pDevices[i].nButtons; j++)
		{
			if (((pDevices[i].state.rgbButtons[j] ^ joystick.rgbButtons[j])
			     & joystick.rgbButtons[j]) & 0x80)
			{
				HWND focus = GetFocus();

				SendMessage(focus, JOYCONFIG_MESSAGE, i, j+128);
			}
		}

		for (j = 0; j < pDevices[i].nAxes && j < 8; j++)
		{
			LONG value = pDevices[i].axis[j].center;
			LONG old   = 0;
			switch (pDevices[i].axis[j].offset)
			{
			case DIJOFS_X:
				value = joystick.lX;
				old   = pDevices[i].state.lX;
				break;
			case DIJOFS_Y:
				value = joystick.lY;
				old   = pDevices[i].state.lY;
				break;
			case DIJOFS_Z:
				value = joystick.lZ;
				old   = pDevices[i].state.lZ;
				break;
			case DIJOFS_RX:
				value = joystick.lRx;
				old   = pDevices[i].state.lRx;
				break;
			case DIJOFS_RY:
				value = joystick.lRy;
				old   = pDevices[i].state.lRy;
				break;
			case DIJOFS_RZ:
				value = joystick.lRz;
				old   = pDevices[i].state.lRz;
				break;
			case DIJOFS_SLIDER(0):
				value = joystick.rglSlider[0];
				old   = pDevices[i].state.rglSlider[0];
				break;
			case DIJOFS_SLIDER(1):
				value = joystick.rglSlider[1];
				old   = pDevices[i].state.rglSlider[1];
				break;
			}
			if (value != old)
			{
				if (value < pDevices[i].axis[j].negative)
					SendMessage(GetFocus(), JOYCONFIG_MESSAGE, i, (j<<1));
				else if (value > pDevices[i].axis[j].positive)
					SendMessage(GetFocus(), JOYCONFIG_MESSAGE, i, (j<<1)+1);
			}
		}

		for (j = 0; j < 4 && j < pDevices[i].nPovs; j++)
		{
			if (LOWORD(pDevices[i].state.rgdwPOV[j]) != LOWORD(joystick.rgdwPOV[j]))
			{
				int state = getPovState(joystick.rgdwPOV[j]);

				if (state & POV_UP)
					SendMessage(GetFocus(), JOYCONFIG_MESSAGE, i, (j<<2)+0x20);
				else if (state & POV_DOWN)
					SendMessage(GetFocus(), JOYCONFIG_MESSAGE, i, (j<<2)+0x21);
				else if (state & POV_RIGHT)
					SendMessage(GetFocus(), JOYCONFIG_MESSAGE, i, (j<<2)+0x22);
				else if (state & POV_LEFT)
					SendMessage(GetFocus(), JOYCONFIG_MESSAGE, i, (j<<2)+0x23);
			}
		}

		memcpy(&pDevices[i].state, &joystick, sizeof(joystick));
	}
}

BOOL checkKey(LONG_PTR key)
{
	LONG_PTR dev = (key >> 8);

	LONG_PTR k = (key & 255);

	if (dev == 0)
	{
		return KEYDOWN(pDevices[0].data, k);
	}
	else if (dev >= numDevices)
	{
		return FALSE;
	}
	else
	{
		if (k < 16)
		{
			LONG_PTR axis  = k >> 1;
			LONG     value = pDevices[dev].axis[axis].center;
			switch (pDevices[dev].axis[axis].offset)
			{
			case DIJOFS_X:
				value = pDevices[dev].state.lX;
				break;
			case DIJOFS_Y:
				value = pDevices[dev].state.lY;
				break;
			case DIJOFS_Z:
				value = pDevices[dev].state.lZ;
				break;
			case DIJOFS_RX:
				value = pDevices[dev].state.lRx;
				break;
			case DIJOFS_RY:
				value = pDevices[dev].state.lRy;
				break;
			case DIJOFS_RZ:
				value = pDevices[dev].state.lRz;
				break;
			case DIJOFS_SLIDER(0):
				value = pDevices[dev].state.rglSlider[0];
				break;
			case DIJOFS_SLIDER(1):
				value = pDevices[dev].state.rglSlider[1];
				break;
			}

			if (k & 1)
				return value > pDevices[dev].axis[axis].positive;
			return value < pDevices[dev].axis[axis].negative;
		}
		else if (k < 48)
		{
			LONG_PTR hat   = (k >> 2) & 3;
			int      state = getPovState(pDevices[dev].state.rgdwPOV[hat]);
			BOOL     res   = FALSE;
			switch (k & 3)
			{
			case 0:
				res = state & POV_UP;
				break;
			case 1:
				res = state & POV_DOWN;
				break;
			case 2:
				res = state & POV_RIGHT;
				break;
			case 3:
				res = state & POV_LEFT;
				break;
			}
			return res;
		}
		else if (k  >= 128)
		{
			return pDevices[dev].state.rgbButtons[k-128] & 0x80;
		}
	}

	return FALSE;
}

DirectInput::DirectInput()
{
	dinputDLL = NULL;
}

DirectInput::~DirectInput()
{
	saveSettings();
	if (pDirectInput != NULL)
	{
		if (pDevices)
		{
			for (int i = 0; i < numDevices; i++)
			{
				if (pDevices[i].device)
				{
					pDevices[i].device->Unacquire();
					pDevices[i].device->Release();
					pDevices[i].device = NULL;
				}
			}
			free(pDevices);
			pDevices = NULL;
		}

		pDirectInput->Release();
		pDirectInput = NULL;
	}

	if (dinputDLL)
	{
		/**/ ::FreeLibrary(dinputDLL);
		dinputDLL = NULL;
	}
}

bool DirectInput::initialize()
{
	joyDebug = GetPrivateProfileInt("config",
	                                "joyDebug",
	                                0,
	                                "VBA.ini");
	dinputDLL = /**/ ::LoadLibrary("DINPUT.DLL");
	HRESULT (WINAPI *DInputCreate)(HINSTANCE, DWORD, LPDIRECTINPUT *, IUnknown *);
	if (dinputDLL != NULL)
	{
		DInputCreate = (HRESULT (WINAPI *)(HINSTANCE, DWORD, LPDIRECTINPUT *, IUnknown *))
		               GetProcAddress(dinputDLL, "DirectInputCreateA");

		if (DInputCreate == NULL)
		{
			directXMessage("DirectInputCreateA");
			return false;
		}
	}
	else
	{
		directXMessage("DINPUT.DLL");
		return false;
	}

	HRESULT hret = DInputCreate(AfxGetInstanceHandle(),
	                            DIRECTINPUT_VERSION,
	                            &pDirectInput,
	                            NULL);
	if (hret != DI_OK)
	{
		//    errorMessage(myLoadString(IDS_ERROR_DISP_CREATE), hret);
		return false;
	}

	hret = pDirectInput->EnumDevices(DIDEVTYPE_JOYSTICK,
	                                 DIEnumDevicesCallback2,
	                                 NULL,
	                                 DIEDFL_ATTACHEDONLY);

	pDevices = (deviceInfo *)calloc(numDevices, sizeof(deviceInfo));

	hret = pDirectInput->CreateDevice(GUID_SysKeyboard, &pDevices[0].device, NULL);
	pDevices[0].isPolled = false;
	pDevices[0].needed   = true;
	pDevices[0].first    = true;

	if (hret != DI_OK)
	{
		//    errorMessage(myLoadString(IDS_ERROR_DISP_CREATEDEVICE), hret);
		return false;
	}

	numDevices = 1;

	hret = pDirectInput->EnumDevices(DIDEVTYPE_JOYSTICK,
	                                 DIEnumDevicesCallback,
	                                 NULL,
	                                 DIEDFL_ATTACHEDONLY);

	//  hret = pDevices[0].device->SetCooperativeLevel(hWindow,
	//                                             DISCL_FOREGROUND|
	//                                             DISCL_NONEXCLUSIVE);

	if (hret != DI_OK)
	{
		//    errorMessage(myLoadString(IDS_ERROR_DISP_LEVEL), hret);
		return false;
	}

	hret = pDevices[0].device->SetDataFormat(&c_dfDIKeyboard);

	if (hret != DI_OK)
	{
		//    errorMessage(myLoadString(IDS_ERROR_DISP_DATAFORMAT), hret);
		return false;
	}

	for (int i = 1; i < numDevices; i++)
	{
		pDevices[i].device->SetDataFormat(&c_dfDIJoystick);
		pDevices[i].needed = false;
		pDevices[i].first  = true;
		currentDevice      = &pDevices[i];
		axisNumber         = 0;
		currentDevice->device->EnumObjects(EnumAxesCallback, NULL, DIDFT_AXIS);
		currentDevice->device->EnumObjects(EnumPovsCallback, NULL, DIDFT_POV);
		if (joyDebug)
		{
			// don't translate. debug only
			winlog("Joystick %2d polled  : %d\n",    i, currentDevice->isPolled);
			winlog("Joystick %2d buttons : %d\n",    i, currentDevice->nButtons);
			winlog("Joystick %2d povs    : %d\n",    i, currentDevice->nPovs);
			winlog("Joystick %2d axes    : %d\n",    i, currentDevice->nAxes);
			for (int j = 0; j < currentDevice->nAxes; j++)
			{
				winlog("Axis %2d offset      : %08lx\n", j, currentDevice->axis[j].
				       offset);
				winlog("Axis %2d center      : %08lx\n", j, currentDevice->axis[j].
				       center);
				winlog("Axis %2d negative    : %08lx\n",   j, currentDevice->axis[j].
				       negative);
				winlog("Axis %2d positive    : %08lx\n",   j, currentDevice->axis[j].
				       positive);
			}
		}

		currentDevice = NULL;
	}

	for (int i = 0; i < numDevices; i++)
		pDevices[i].device->Acquire();

	return true;
}

bool DirectInput::readDevices()
{
	bool ok = true;
	for (int i = 0; i < numDevices; i++)
	{
		if (pDevices[i].needed)
		{
			ok = (i > 0 ? readJoystick(i) : readKeyboard()) || ok;
		}
	}
	return ok;
}

bool inputActive = true; // used to disable all input when the window is inactive

u32 DirectInput::readDevice(int i, bool sensor)
{
	// this old hack is evil
	extern int systemGetDefaultJoypad();
	extern int gbSgbMode, gbSgbMultiplayer;
	if (!(gbSgbMode && gbSgbMultiplayer))
		i = systemGetDefaultJoypad();

	u32 res = 0;

	// manual input
	if (inputActive)
	{
		if (checkKey(joypad[i][KEY_BUTTON_A]))
			res |= BUTTON_MASK_A;
		if (checkKey(joypad[i][KEY_BUTTON_B]))
			res |= BUTTON_MASK_B;
		if (checkKey(joypad[i][KEY_BUTTON_SELECT]))
			res |= BUTTON_MASK_SELECT;
		if (checkKey(joypad[i][KEY_BUTTON_START]))
			res |= BUTTON_MASK_START;
		if (checkKey(joypad[i][KEY_RIGHT]))
			res |= BUTTON_MASK_RIGHT;
		if (checkKey(joypad[i][KEY_LEFT]))
			res |= BUTTON_MASK_LEFT;
		if (checkKey(joypad[i][KEY_UP]))
			res |= BUTTON_MASK_UP;
		if (checkKey(joypad[i][KEY_DOWN]))
			res |= BUTTON_MASK_DOWN;
		if (checkKey(joypad[i][KEY_BUTTON_R]))
			res |= BUTTON_MASK_R;
		if (checkKey(joypad[i][KEY_BUTTON_L]))
			res |= BUTTON_MASK_L;

		// unused
		if (checkKey(motion[KEY_LEFT]))
			res |= BUTTON_MASK_LEFT_MOTION;
		else if (checkKey(motion[KEY_RIGHT]))
			res |= BUTTON_MASK_RIGHT_MOTION;
		if (checkKey(motion[KEY_UP]))
			res |= BUTTON_MASK_UP_MOTION;
		else if (checkKey(motion[KEY_DOWN]))
			res |= BUTTON_MASK_DOWN_MOTION;
	}

	u32 hackedButtons = 0;
	if (inputActive)
	{
		// the "non-button" buttons (what a hack!)
		if (checkKey(joypad[i][KEY_BUTTON_SPEED]))
			hackedButtons |= BUTTON_MASK_SPEED;
		if (checkKey(joypad[i][KEY_BUTTON_CAPTURE]))
			hackedButtons |= BUTTON_MASK_CAPTURE;
		if (checkKey(joypad[i][KEY_BUTTON_GS]))
			hackedButtons |= BUTTON_MASK_GAMESHARK;
	}

	extern bool systemIsSpedUp();
	if (systemIsSpedUp())
		hackedButtons |= BUTTON_MASK_SPEED;

	return res | hackedButtons;
}

CString DirectInput::getKeyName(LONG_PTR key)
{
	LONG_PTR d = (key >> 8);
	LONG_PTR k = key & 255;

	DIDEVICEOBJECTINSTANCE di;

	ZeroMemory(&di, sizeof(DIDEVICEOBJECTINSTANCE));

	di.dwSize = sizeof(DIDEVICEOBJECTINSTANCE);

	CString winBuffer = winResLoadString(IDS_ERROR);

	if (d == 0)
	{
		pDevices[0].device->GetObjectInfo(&di, (DWORD)key, DIPH_BYOFFSET);
		winBuffer = di.tszName;
	}
	else if (d < numDevices)
	{
		if (k < 16)
		{
			if (k < 4)
			{
				switch (k)
				{
				case 0:
					winBuffer.Format(winResLoadString(IDS_JOY_LEFT), d);
					break;
				case 1:
					winBuffer.Format(winResLoadString(IDS_JOY_RIGHT), d);
					break;
				case 2:
					winBuffer.Format(winResLoadString(IDS_JOY_UP), d);
					break;
				case 3:
					winBuffer.Format(winResLoadString(IDS_JOY_DOWN), d);
					break;
				}
			}
			else
			{
				pDevices[d].device->GetObjectInfo(&di,
				                                  pDevices[d].axis[k>>1].offset,
				                                  DIPH_BYOFFSET);
				if (k & 1)
					winBuffer.Format("Joy %d %s +", d, di.tszName);
				else
					winBuffer.Format("Joy %d %s -", d, di.tszName);
			}
		}
		else if (k < 48)
		{
			LONG_PTR hat = (k >> 2) & 3;
			pDevices[d].device->GetObjectInfo(&di,
			                                  (DWORD)DIJOFS_POV(hat),
			                                  DIPH_BYOFFSET);
			char *   dir = "up";
			LONG_PTR dd  = k & 3;
			if (dd == 1)
				dir = "down";
			else if (dd == 2)
				dir = "right";
			else if (dd == 3)
				dir = "left";
			winBuffer.Format("Joy %d %s %s", d, di.tszName, dir);
		}
		else
		{
			pDevices[d].device->GetObjectInfo(&di,
			                                  (DWORD)DIJOFS_BUTTON(k-128),
			                                  DIPH_BYOFFSET);
			winBuffer.Format(winResLoadString(IDS_JOY_BUTTON), d, di.tszName);
		}
	}
	else
	{
		// Joystick isn't plugged in.  We can't decipher k, so just show its value.
		winBuffer.Format("Joy %d (%d)", d, k);
	}

	return winBuffer;
}

void DirectInput::checkKeys()
{
	::checkKeys();
}

Input *newDirectInput()
{
	return new DirectInput;
}

void DirectInput::checkDevices()
{
	checkJoypads();
	checkKeyboard();
}

void DirectInput::activate()
{
	for (int i = 0; i < numDevices; i++)
	{
		if (pDevices != NULL && pDevices[i].device != NULL)
			pDevices[i].device->Acquire();
	}
}

void DirectInput::loadSettings()
{
	winReadKeys();
}

void DirectInput::saveSettings()
{
	winSaveKeys();
}

