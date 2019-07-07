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
// File    : KeyboardEdit.cpp
// Project : AccelsEditor
////////////////////////////////////////////////////////////////////////////////
// Version : 1.0                       * Authors : A.Lebatard + T.Maurel
// Date    : 17.08.98
//
// Remarks : implementation file
//

////////////////////////////////////////////////////////////////////////////////
// modified by aquanull

#include "../stdafx.h"
#include "KeyboardEdit.h"

extern const TCHAR *mapVirtKeysStringFromWORD(WORD wKey);

IMPLEMENT_DYNAMIC(CKeyboardEdit, CEdit)

/////////////////////////////////////////////////////////////////////////////
// CKeyboardEdit

CKeyboardEdit::CKeyboardEdit()
{
	ResetKey();
}

CKeyboardEdit::~CKeyboardEdit()
{}

BEGIN_MESSAGE_MAP(CKeyboardEdit, CEdit)
//{{AFX_MSG_MAP(CKeyboardEdit)
ON_CONTROL_REFLECT_EX(EN_CHANGE, &CKeyboardEdit::OnEnChange)
ON_CONTROL_REFLECT_EX(EN_SETFOCUS, &CKeyboardEdit::OnEnSetfocus)
ON_CONTROL_REFLECT_EX(EN_KILLFOCUS, &CKeyboardEdit::OnEnKillfocus)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CKeyboardEdit message handlers
BOOL CKeyboardEdit::PreTranslateMessage(MSG *pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
	{
		int key = pMsg->wParam;
		m_keys[key] = true;
		switch (key)
		{
		case VK_CONTROL:
		case VK_MENU:
		case VK_SHIFT:
			m_bCtrlPressed	= m_keys[VK_CONTROL];
			m_bAltPressed	= m_keys[VK_MENU];;
			m_bShiftPressed = m_keys[VK_SHIFT];
			if (!m_keys[m_wVirtKey])
			{
				if (m_keys[m_wJamKey])
				{
					m_wVirtKey = m_wJamKey;
					m_wJamKey  = 0;
				}
				else
					m_wVirtKey = 0;
			}
			if (!m_keys[m_wJamKey])
				m_wJamKey = 0;
			break;
		default:
			m_bCtrlPressed	= m_keys[VK_CONTROL];
			m_bAltPressed	= m_keys[VK_MENU];
			m_bShiftPressed = m_keys[VK_SHIFT];
			if (m_wVirtKey != key)
			{
				if (m_keys[m_wVirtKey])
					m_wJamKey = m_wVirtKey;
				else
					m_wJamKey = 0;
				m_wVirtKey = key;
			}
			else if (!m_keys[m_wJamKey])
				m_wJamKey = 0;
			break;
		}
		DisplayKeyboardString();
		return TRUE;
	}
	else if (pMsg->message == WM_KEYUP || pMsg->message == WM_SYSKEYUP)
	{
		int key = pMsg->wParam;
		m_keys[key] = false;
		switch (key)
		{
		case VK_CONTROL:
		case VK_MENU:
		case VK_SHIFT:
			break;
		default:
			m_bCtrlPressed	= m_keys[VK_CONTROL];
			m_bAltPressed	= m_keys[VK_MENU];
			m_bShiftPressed = m_keys[VK_SHIFT];
			if (m_wJamKey)
			{
				if (!m_keys[m_wVirtKey])
				{
					m_wVirtKey = m_wJamKey;
					m_wJamKey  = 0;
				}
				if (!m_keys[m_wJamKey])
					m_wJamKey = 0;
			}
			break;
		}
		DisplayKeyboardString();
		return TRUE;
	}

	return CEdit::PreTranslateMessage(pMsg);
}

BOOL CKeyboardEdit::OnEnChange()
{
	return FALSE;
}

BOOL CKeyboardEdit::OnEnSetfocus()
{
	//SetSel(0, -1, TRUE);	// mouse click makes this in vain, so we use the method below instead
	PostMessage(EM_SETSEL, 0, -1);
	m_bForceUpdate = true;
	return FALSE;
}

BOOL CKeyboardEdit::OnEnKillfocus()
{
	AllKeyUp();
	return FALSE;
}

////////////////////////////////////////////////////////////////////////
//
void CKeyboardEdit::DisplayKeyboardString()
{
	CString strKbd;

	// modifiers
	if (m_bCtrlPressed)
		strKbd = "Ctrl";

	if (m_bAltPressed)
	{
		if (strKbd.GetLength() > 0)
			strKbd += '+';
		strKbd += "Alt";
	}
	if (m_bShiftPressed)
	{
		if (strKbd.GetLength() > 0)
			strKbd += '+';
		strKbd += "Shift";
	}
	// virtual key
	LPCTSTR szVirtKey = mapVirtKeysStringFromWORD(m_wVirtKey);
	if (szVirtKey != NULL)
	{
		if (strKbd.GetLength() > 0)
			strKbd += '+';
		strKbd += szVirtKey;
	}
	// jammed key
	LPCTSTR szJamKey = mapVirtKeysStringFromWORD(m_wJamKey);
	if (szJamKey != NULL)
	{
		strKbd += '(';
		strKbd += szJamKey;
		strKbd += ')';
	}

	if (m_bForceUpdate)
	{
		m_bForceUpdate = false;
		SetWindowText(strKbd);
	}
	else
	{
		CString oldString;
		GetWindowText(oldString);
		if (oldString.Compare(strKbd))
			SetWindowText(strKbd);
	}
}

////////////////////////////////////////////////////////////////////////
//
void CKeyboardEdit::ResetKey()
{
	AllKeyUp();

	m_bForceUpdate		= true;
	m_bCtrlPressed	= false;
	m_bAltPressed	= false;
	m_bShiftPressed = false;
	m_wVirtKey		= 0;
	m_wJamKey		= 0;

	if (m_hWnd != NULL)
	{
		CString oldString;
		GetWindowText(oldString);
		if (!oldString.IsEmpty())
			SetWindowText(_T(""));
	}
}

void CKeyboardEdit::AllKeyUp()
{
	for (int i = 0; i < 256; ++i)
		m_keys[i] = 0;
}

////////////////////////////////////////////////////////////////////////
//
bool CKeyboardEdit::GetAccelKey(WORD &wVirtKey, bool &bCtrl, bool &bAlt, bool &bShift) const
{
	if (!IsDefined())
		return false;

	wVirtKey = m_wVirtKey;
	bAlt	 = m_bAltPressed;
	bCtrl	 = m_bCtrlPressed;
	bShift	 = m_bShiftPressed;
	return true;
}

bool CKeyboardEdit::GetJamKey(WORD &wJamKey) const
{
	if (m_wJamKey != 0)
		wJamKey = m_wJamKey;
	return m_wJamKey != 0;
}

bool CKeyboardEdit::IsDefined() const
{
	return bool(m_wVirtKey || m_bAltPressed || m_bCtrlPressed || m_bShiftPressed);
}

bool CKeyboardEdit::IsFinished() const
{
	bool finished = true;
	for (int i = 0; i < 256; ++i)
		if (m_keys[i])
			finished = false;
	return finished;
}
