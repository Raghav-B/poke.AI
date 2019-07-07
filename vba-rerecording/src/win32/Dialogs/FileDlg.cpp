// FileDlg.cpp: implementation of the FileDlg class.
//
//////////////////////////////////////////////////////////////////////
//#define WINVER 0x0410 // windows 98 - just for this 1 file - just in case
#include "../stdafx.h"
#include <commdlg.h>
#include <dlgs.h>
#include "../resource.h"
#include "FileDlg.h"
#include "../Sound.h"
#include "../VBA.h"

static FileDlg *instance = NULL;

static UINT_PTR CALLBACK HookFunc(HWND hwnd,
                                  UINT msg,
                                  WPARAM wParam,
                                  LPARAM lParam)
{
	if (instance)
	{
		if (msg == WM_NOTIFY)
		{
			OFNOTIFY *notify = (OFNOTIFY *)lParam;
			if (notify)
			{
				if (notify->hdr.code == CDN_TYPECHANGE)
				{
					instance->OnTypeChange(hwnd);
					return 1;
				}
			}
		}
	}
	return 0;
}

static UINT_PTR CALLBACK HookFuncOldStyle(HWND hwnd,
                                          UINT msg,
                                          WPARAM wParam,
                                          LPARAM lParam)
{
	if (instance)
	{
		if (msg == WM_COMMAND)
		{
			if (HIWORD(wParam) == CBN_SELCHANGE)
			{
				if (LOWORD(wParam) == cmb1)
				{
					// call method with combobox handle to keep
					// behaviour there
					instance->OnTypeChange((HWND)lParam);
					return 1;
				}
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// FileDlg

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

FileDlg::FileDlg(CWnd *parent, LPCTSTR file, LPCTSTR filter,
                 int filterIndex, LPCTSTR ext, LPCTSTR *exts, LPCTSTR initialDir,
                 LPCTSTR title, bool save, bool noReadOnly)
{
	OSVERSIONINFO info;
	info.dwOSVersionInfoSize = sizeof(info);
	GetVersionEx(&info);
	m_file = file;
	int size = sizeof(OPENFILENAME);

	// avoid problems if OPENFILENAME is already defined with the extended fields
	// needed for the enhanced open/save dialog
#if _WIN32_WINNT < 0x0500
	if (info.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		if (info.dwMajorVersion >= 5)
			size = sizeof(OPENFILENAMEEX);
	}
#endif

	ZeroMemory(&m_ofn, sizeof(m_ofn));
	m_ofn.lpstrFile       = m_file.GetBuffer(MAX_PATH);
	m_ofn.nMaxFile        = MAX_PATH;
	m_ofn.lStructSize     = size;
	m_ofn.hwndOwner       = parent ? parent->GetSafeHwnd() : NULL;
	m_ofn.nFilterIndex    = filterIndex;
	m_ofn.lpstrInitialDir = initialDir;
	m_ofn.lpstrTitle      = title;
	m_ofn.lpstrDefExt     = ext;
	m_ofn.lpfnHook        = HookFunc;
	m_ofn.Flags  = OFN_PATHMUSTEXIST | OFN_ENABLESIZING | OFN_ENABLEHOOK;
	m_ofn.Flags |= OFN_EXPLORER;
	if (noReadOnly)
		m_ofn.Flags |= OFN_HIDEREADONLY;
	m_filter = filter;

	char *p = m_filter.GetBuffer(0);

	while ((p = strchr(p, '|')) != NULL)
		*p++ = 0;
	m_ofn.lpstrFilter = m_filter;

	if (theApp.videoOption == VIDEO_320x240)
	{
		m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENDLG);
		m_ofn.lpfnHook       = HookFuncOldStyle;
		m_ofn.Flags |= OFN_ENABLETEMPLATE;
		m_ofn.Flags &= ~OFN_EXPLORER;
	}

	isSave     = save;
	extensions = exts;

	instance = this;
}

FileDlg::~FileDlg()
{
	instance = NULL;
}

void FileDlg::OnTypeChange(HWND hwnd)
{
	HWND parent = GetParent(hwnd);

	HWND fileNameControl = ::GetDlgItem(parent, cmb13);

	if (fileNameControl == NULL)
		fileNameControl = ::GetDlgItem(parent, edt1);

	if (fileNameControl == NULL)
		return;

	CString filename;
	GetWindowText(fileNameControl, filename.GetBuffer(MAX_PATH), MAX_PATH);
	filename.ReleaseBuffer();

	HWND typeControl = ::GetDlgItem(parent, cmb1);

	ASSERT(typeControl != NULL);

	int sel = ::SendMessage(typeControl, CB_GETCURSEL, 0, 0);

	ASSERT(sel != -1);

	LPCTSTR typeName = extensions[sel];

	// sel could easily be an invalid index of extensions, so check for null guard
	for(int i = 0; i <= sel; i++)
		if(extensions[i] == NULL)
			typeName = "";

	if (filename.GetLength() == 0)
	{
		if(*typeName)
			filename.Format("*%s", typeName);
	}
	else
	{
		int index = filename.Find('.');
		if (index == -1)
		{
			filename = filename + typeName;
		}
		else
		{
			filename = filename.Left(index) + typeName;
		}
	}
	SetWindowText(fileNameControl, filename);
}

int FileDlg::getFilterIndex()
{
	return m_ofn.nFilterIndex;
}

int FileDlg::DoModal()
{
	systemSoundClearBuffer();
	BOOL res = isSave ? GetSaveFileName(&m_ofn) :
	           GetOpenFileName(&m_ofn);

	return res ? IDOK : IDCANCEL;
}

LPCTSTR FileDlg::GetPathName()
{
	return (LPCTSTR)m_file;
}

