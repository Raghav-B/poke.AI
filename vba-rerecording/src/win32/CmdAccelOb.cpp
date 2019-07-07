////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Thierry Maurel
// All rights reserved
//
// Distribute freely, except: don't remove my name from the source or
// documentation (don't take credit for my work), mark your changes (don't
// get me blamed for your possible bugs), don't alter or remove this
// notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    tmaurel@caramail.com   (or tmaurel@hol.fr)
//
////////////////////////////////////////////////////////////////////////////////
// File    : CmdAccelOb.cpp
// Project : AccelsEditor
////////////////////////////////////////////////////////////////////////////////
// Version : 1.0                       * Author : T.Maurel
// Date    : 17.08.98
//
// Remarks :
//
////////////////////////////////////////////////////////////////////////////////
// modified by the VBA-rr team

#include "stdafx.h"
#include "CmdAccelOb.h"

////////////////////////////////////////////////////////////////////////
//
//
MAPVIRTKEYS mapVirtKeys[] = {
	{ VK_LBUTTON,		 "LBUTTON"				},
	{ VK_RBUTTON,		 "RBUTTON"				},
	{ VK_CANCEL,		 "Cancel"				},
	{ VK_MBUTTON,		 "MBUTTON"				},
	{ VK_BACK,			 "Backspace"			},
	{ VK_TAB,			 "Tab"					},
	{ VK_CLEAR,			 "Clear"				},
	{ VK_RETURN,		 "Enter"				},
	{ VK_SHIFT,			 "Shift"				},
	{ VK_LSHIFT,		 "LShift"				},
	{ VK_RSHIFT,		 "RShift"				},
	{ VK_CONTROL,		 "Control"				},
	{ VK_LCONTROL,		 "LControl"				},
	{ VK_RCONTROL,		 "RControl"				},
	{ VK_MENU,			 "Alt"					},
	{ VK_LMENU,			 "LAlt"					},
	{ VK_RMENU,			 "RAlt"					},
	{ VK_PAUSE,			 "Pause"				},
	{ VK_CAPITAL,		 "Caps Lock"			},
	{ VK_ESCAPE,		 "Escape"				},
	{ VK_SPACE,			 "Space"				},
	{ VK_PRIOR,			 "Prior"				},
	{ VK_NEXT,			 "Next"					},
	{ VK_END,			 "End"					},
	{ VK_HOME,			 "Home"					},
	{ VK_LEFT,			 "Left"					},
	{ VK_UP,			 "Up"					},
	{ VK_RIGHT,			 "Right"				},
	{ VK_DOWN,			 "Down"					},
	{ VK_SELECT,		 "Select"				},
	{ VK_PRINT,			 "Print"				},
	{ VK_EXECUTE,		 "Execute"				},
	{ VK_SNAPSHOT,		 "Snapshot"				},
	{ VK_INSERT,		 "Insert"				},
	{ VK_DELETE,		 "Delete"				},
	{ VK_HELP,			 "Help"					},
	{ WORD('0'),		 "0"					},
	{ WORD('1'),		 "1"					},
	{ WORD('2'),		 "2"					},
	{ WORD('3'),		 "3"					},
	{ WORD('4'),		 "4"					},
	{ WORD('5'),		 "5"					},
	{ WORD('6'),		 "6"					},
	{ WORD('7'),		 "7"					},
	{ WORD('8'),		 "8"					},
	{ WORD('9'),		 "9"					},
	{ WORD('A'),		 "A"					},
	{ WORD('B'),		 "B"					},
	{ WORD('C'),		 "C"					},
	{ WORD('D'),		 "D"					},
	{ WORD('E'),		 "E"					},
	{ WORD('F'),		 "F"					},
	{ WORD('G'),		 "G"					},
	{ WORD('H'),		 "H"					},
	{ WORD('I'),		 "I"					},
	{ WORD('J'),		 "J"					},
	{ WORD('K'),		 "K"					},
	{ WORD('L'),		 "L"					},
	{ WORD('M'),		 "M"					},
	{ WORD('N'),		 "N"					},
	{ WORD('O'),		 "O"					},
	{ WORD('P'),		 "P"					},
	{ WORD('Q'),		 "Q"					},
	{ WORD('R'),		 "R"					},
	{ WORD('S'),		 "S"					},
	{ WORD('T'),		 "T"					},
	{ WORD('U'),		 "U"					},
	{ WORD('V'),		 "V"					},
	{ WORD('W'),		 "W"					},
	{ WORD('X'),		 "X"					},
	{ WORD('Y'),		 "Y"					},
	{ WORD('Z'),		 "Z"					},
	{ VK_LWIN,			 "LWIN"					},
	{ VK_RWIN,			 "RWIN"					},
	{ VK_APPS,			 "APPS"					},
	{ VK_NUMPAD0,		 "NumPad0"				},
	{ VK_NUMPAD1,		 "NumPad1"				},
	{ VK_NUMPAD2,		 "NumPad2"				},
	{ VK_NUMPAD3,		 "NumPad3"				},
	{ VK_NUMPAD4,		 "NumPad4"				},
	{ VK_NUMPAD5,		 "NumPad5"				},
	{ VK_NUMPAD6,		 "NumPad6"				},
	{ VK_NUMPAD7,		 "NumPad7"				},
	{ VK_NUMPAD8,		 "NumPad8"				},
	{ VK_NUMPAD9,		 "NumPad9"				},
	{ VK_MULTIPLY,		 "NumpadMultiply"		},
	{ VK_ADD,			 "NumpadAdd"			},
	{ VK_SEPARATOR,		 "Separator"			},
	{ VK_SUBTRACT,		 "NumpadSubtract"		},
	{ VK_DECIMAL,		 "NumpadDecimal"		},
	{ VK_DIVIDE,		 "NumpadDivide"			},
	{ VK_F1,			 "F1"					},
	{ VK_F2,			 "F2"					},
	{ VK_F3,			 "F3"					},
	{ VK_F4,			 "F4"					},
	{ VK_F5,			 "F5"					},
	{ VK_F6,			 "F6"					},
	{ VK_F7,			 "F7"					},
	{ VK_F8,			 "F8"					},
	{ VK_F9,			 "F9"					},
	{ VK_F10,			 "F10"					},
	{ VK_F11,			 "F11"					},
	{ VK_F12,			 "F12"					},
	{ VK_F13,			 "F13"					},
	{ VK_F14,			 "F14"					},
	{ VK_F15,			 "F15"					},
	{ VK_F16,			 "F16"					},
	{ VK_F17,			 "F17"					},
	{ VK_F18,			 "F18"					},
	{ VK_F19,			 "F19"					},
	{ VK_F20,			 "F20"					},
	{ VK_F21,			 "F21"					},
	{ VK_F22,			 "F22"					},
	{ VK_F23,			 "F23"					},
	{ VK_F24,			 "F24"					},
	{ VK_NUMLOCK,		 "Num Lock"				},
	{ VK_SCROLL,		 "Scroll Lock"			},
	{ VK_ATTN,			 "Attention"			},
	{ VK_CRSEL,			 "CRSEL"				},
	{ VK_EXSEL,			 "EXSEL"				},
	{ VK_EREOF,			 "EREOF"				},
	{ VK_PLAY,			 "Play"					},
	{ VK_ZOOM,			 "Zoom"					},
	{ VK_NONAME,		 "No Name"				},
	{ VK_PA1,			 "PA1"					},
	{ VK_OEM_CLEAR,		 "Clear2"				},
	{ VK_OEM_1,			 "Semicolon;"			},
	{ VK_OEM_2,			 "Slash/"				},
	{ VK_OEM_3,			 "Tilde~"				},
	{ VK_OEM_4,			 "LBracket["			},
	{ VK_OEM_5,			 "Backslash\\"			},
	{ VK_OEM_6,			 "RBracket]"			},
	{ VK_OEM_7,			 "Apostrophe'"			},
	{ VK_OEM_8,			 "OEM8"					},
	{ VK_OEM_PLUS,		 "Plus+"				},
	{ VK_OEM_MINUS,		 "Minus-"				},
	{ VK_OEM_COMMA,		 "Comma,"				},
	{ VK_OEM_PERIOD,	 "Period."				},
	{ VK_OEM_AX,		 "Apostrophe`"			},
	{ VK_OEM_102,		 "<> or \\|"			},
	{ VK_ICO_HELP,		 "ICO Help"				},
	{ VK_ICO_00,		 "ICO 00"				},
	{ VK_OEM_FJ_JISHO,	 "JISHO"				},
	{ VK_OEM_FJ_MASSHOU, "MASSHOU"				},
	{ VK_OEM_FJ_TOUROKU, "TOUROKU"				},
	{ VK_OEM_FJ_LOYA,	 "LOYA"					},
	{ VK_OEM_FJ_ROYA,	 "ROYA"					},
	{ VK_OEM_NEC_EQUAL,	 "Numpad Equals"		},
};

////////////////////////////////////////////////////////////////////////
//
//
MAPVIRTKEYS mapVirtSysKeys[] = {
	{ FCONTROL, "Ctrl"	  },
	{ FALT,		"Alt"	  },
	{ FSHIFT,	"Shift"	  },
};

////////////////////////////////////////////////////////////////////////
// helper fct for external access
////////////////////////////////////////////////////////////////////////
//
//
const TCHAR *mapVirtKeysStringFromWORD(WORD wKey)
{
	for (int index = 0; index < sizeof(mapVirtKeys) / sizeof(mapVirtKeys[0]); index++)
	{
		if (mapVirtKeys[index].wKey == wKey)
		{
			return mapVirtKeys[index].szKey;
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////
//
#define DEFAULT_ACCEL   0x01
#define USER_ACCEL      0x02

#ifndef _countof
#define sizetable(table) (sizeof(table)/sizeof(table[0]))
#else
#define sizetable(table) _countof(table)
#endif

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
//
//
CAccelsOb::CAccelsOb()
{
	m_cVirt	  = 0;
	m_wKey	  = 0;
	m_bLocked = false;
}

////////////////////////////////////////////////////////////////////////
//
//
CAccelsOb::CAccelsOb(CAccelsOb *pFrom)
{
	ASSERT(pFrom != NULL);

	m_cVirt	  = pFrom->m_cVirt;
	m_wKey	  = pFrom->m_wKey;
	m_bLocked = pFrom->m_bLocked;
}

////////////////////////////////////////////////////////////////////////
//
//
CAccelsOb::CAccelsOb(BYTE cVirt, WORD wKey, bool bLocked)
{
	m_cVirt	  = cVirt;
	m_wKey	  = wKey;
	m_bLocked = bLocked;
}

////////////////////////////////////////////////////////////////////////
//
//
CAccelsOb::CAccelsOb(LPACCEL pACCEL)
{
	ASSERT(pACCEL != NULL);

	m_cVirt	  = pACCEL->fVirt;
	m_wKey	  = pACCEL->key;
	m_bLocked = false;
}

////////////////////////////////////////////////////////////////////////
//
//
CAccelsOb & CAccelsOb::operator=(const CAccelsOb &from)
{
	m_cVirt	  = from.m_cVirt;
	m_wKey	  = from.m_wKey;
	m_bLocked = from.m_bLocked;

	return *this;
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
//
//
void CAccelsOb::GetString(CString &szBuffer)
{
	szBuffer = "";

	// modifiers part
	for (int i = 0; i < sizetable(mapVirtSysKeys); i++)
	{
		if (m_cVirt & mapVirtSysKeys[i].wKey)
		{
			szBuffer += mapVirtSysKeys[i].szKey;
			if (m_wKey)
				szBuffer += "+";
		}
	}

	// in case of the object is not assigned, we avoid error messages
	if (m_wKey == 0)
		return;

	// and virtual key part
	for (int i = 0; i < sizetable(mapVirtKeys); i++)
	{
		if (m_wKey == mapVirtKeys[i].wKey)
		{
			szBuffer += mapVirtKeys[i].szKey;
			return;
		}
	}

//	systemMessage(0, "%d", m_wKey);
	//AfxMessageBox("Internal error : (CAccelsOb::GetString) m_wKey invalid");
}

////////////////////////////////////////////////////////////////////////
//
//
bool CAccelsOb::IsEqual(WORD wKey, bool bCtrl, bool bAlt, bool bShift)
{
	bool m_bCtrl  = (m_cVirt & FCONTROL) ? true : false;
	bool m_bAlt	  = (m_cVirt & FALT) ? true : false;
	bool m_bShift = (m_cVirt & FSHIFT) ? true : false;

	bool bRet = (bCtrl == m_bCtrl) && (bAlt == m_bAlt) && (bShift == m_bShift) && (m_wKey == wKey);

	return bRet;
}

////////////////////////////////////////////////////////////////////////
//
//
DWORD CAccelsOb::GetData()
{
	BYTE cLocalCodes = 0;
	if (m_bLocked)
		cLocalCodes = DEFAULT_ACCEL;
	else
		cLocalCodes = USER_ACCEL;

	WORD bCodes = MAKEWORD(m_cVirt, cLocalCodes);
	return MAKELONG(m_wKey, bCodes);
}

////////////////////////////////////////////////////////////////////////
//
//
bool CAccelsOb::SetData(DWORD dwDatas)
{
	m_wKey = LOWORD(dwDatas);

	WORD bCodes = HIWORD(dwDatas);
	m_cVirt = LOBYTE(bCodes);

	BYTE cLocalCodes = HIBYTE(bCodes);
	m_bLocked = (cLocalCodes == DEFAULT_ACCEL);

	return true;
}

////////////////////////////////////////////////////////////////////////
//
#ifdef _DEBUG
////////////////////////////////////////////////////////////////////////
//
//
void CAccelsOb::AssertValid() const
{
	CObject::AssertValid();
}

////////////////////////////////////////////////////////////////////////
//
//
void CAccelsOb::Dump(CDumpContext &dc) const
{
	dc << "\t\t";
	CObject::Dump(dc);
	dc << "\t\tlocked=" << m_bLocked << ", cVirt=" << m_cVirt << ", wKey=" << m_wKey << "\n\n";
}

#endif

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
//
//
CCmdAccelOb::CCmdAccelOb()
{}

////////////////////////////////////////////////////////////////////////
//
//
CCmdAccelOb::CCmdAccelOb(WORD wIDCommand, LPCTSTR szCommand)
{
	ASSERT(szCommand != NULL);

	m_wIDCommand = wIDCommand;
	m_szCommand	 = szCommand;
}

////////////////////////////////////////////////////////////////////////
//
//
CCmdAccelOb::CCmdAccelOb(BYTE cVirt, WORD wIDCommand, WORD wKey, LPCTSTR szCommand, bool bLocked)
{
	ASSERT(szCommand != NULL);

	m_wIDCommand = wIDCommand;
	m_szCommand	 = szCommand;

	CAccelsOb *pAccel = new CAccelsOb(cVirt, wKey, bLocked);
	ASSERT(pAccel != NULL);
	m_Accels.AddTail(pAccel);
}

////////////////////////////////////////////////////////////////////////
//
//
CCmdAccelOb::~CCmdAccelOb()
{
	POSITION pos = m_Accels.GetHeadPosition();
	while (pos != NULL)
		delete m_Accels.GetNext(pos);
	m_Accels.RemoveAll();
}

////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////
//
//
void CCmdAccelOb::Add(BYTE cVirt, WORD wKey, bool bLocked)
{
	CAccelsOb *pAccel = new CAccelsOb(cVirt, wKey, bLocked);
	ASSERT(pAccel != NULL);
	m_Accels.AddTail(pAccel);
}

////////////////////////////////////////////////////////////////////////
//
//
void CCmdAccelOb::Add(CAccelsOb *pAccel)
{
	ASSERT(pAccel != NULL);
	m_Accels.AddTail(pAccel);
}

////////////////////////////////////////////////////////////////////////
//
//
CCmdAccelOb & CCmdAccelOb::operator=(const CCmdAccelOb &from)
{
	Reset();

	m_wIDCommand = from.m_wIDCommand;
	m_szCommand	 = from.m_szCommand;

	CAccelsOb *pAccel;
	POSITION   pos = from.m_Accels.GetHeadPosition();
	while (pos != NULL)
	{
		pAccel = new CAccelsOb(from.m_Accels.GetNext(pos));
		ASSERT(pAccel != NULL);
		m_Accels.AddTail(pAccel);
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////
//
//
void CCmdAccelOb::DeleteUserAccels()
{
	CAccelsOb *pAccel;
	POSITION   prevPos;
	POSITION   pos = m_Accels.GetHeadPosition();
	while (pos != NULL)
	{
		prevPos = pos;
		pAccel	= m_Accels.GetNext(pos);
		if (!pAccel->m_bLocked)
		{
			delete pAccel;
			m_Accels.RemoveAt(prevPos);
		}
	}
}

////////////////////////////////////////////////////////////////////////
//
//
void CCmdAccelOb::Reset()
{
	m_wIDCommand = 0;
	m_szCommand	 = "Empty command";

	CAccelsOb *pAccel;
	POSITION   pos = m_Accels.GetHeadPosition();
	while (pos != NULL)
	{
		pAccel = m_Accels.GetNext(pos);
		delete pAccel;
	}
}

////////////////////////////////////////////////////////////////////////
//
#ifdef _DEBUG
////////////////////////////////////////////////////////////////////////
//
//
void CCmdAccelOb::AssertValid() const
{
	// call base class function first
	CObject::AssertValid();
}

////////////////////////////////////////////////////////////////////////
//
//
void CCmdAccelOb::Dump(CDumpContext &dc) const
{
	// call base class function first
	dc << "\t";
	CObject::Dump(dc);

	// now do the stuff for our specific class
	dc << "\tIDCommand = " << m_wIDCommand;
	dc << "\n\tszCommand = " << m_szCommand;
	dc << "\n\tAccelerators = {\n";

	CAccelsOb *pAccel;
	POSITION   pos = m_Accels.GetHeadPosition();
	while (pos != NULL)
	{
		pAccel = m_Accels.GetNext(pos);
		dc << pAccel;
	}
	dc << "\t}\n";
}

#endif
