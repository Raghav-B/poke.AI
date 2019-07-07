// Directories.cpp : implementation file
//

#include "../stdafx.h"
#include <shlobj.h>
#include "../resource.h"
#include "Directories.h"
#include "../Reg.h"
#include "../WinMiscUtil.h"
#include "../WinResUtil.h"

/////////////////////////////////////////////////////////////////////////////
// Directories dialog

static int CALLBACK browseCallbackProc(HWND hWnd, UINT msg,
                                       LPARAM l, LPARAM data)
{
	char *buffer = (char *)data;
	switch (msg)
	{
	case BFFM_INITIALIZED:
		if (buffer[0])
			SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)buffer);
		break;
	default:
		break;
	}
	return 0;
}

Directories::Directories(CWnd*pParent /*=NULL*/)
	: CDialog(Directories::IDD, pParent)
{
	//{{AFX_DATA_INIT(Directories)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void Directories::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Directories)
	DDX_Control(pDX, IDC_ROM_PATH, m_romPath);
	DDX_Control(pDX, IDC_GBXROM_PATH, m_gbxromPath);
	DDX_Control(pDX, IDC_BATTERY_PATH, m_batteryPath);
	DDX_Control(pDX, IDC_SAVE_PATH, m_savePath);
	DDX_Control(pDX, IDC_MOVIE_PATH, m_moviePath);
	DDX_Control(pDX, IDC_CHEAT_PATH, m_cheatPath);
	DDX_Control(pDX, IDC_IPS_PATH, m_ipsPath);
	DDX_Control(pDX, IDC_LUA_PATH, m_luaPath);
	DDX_Control(pDX, IDC_AVI_PATH, m_aviPath);
	DDX_Control(pDX, IDC_WAV_PATH, m_wavPath);
	DDX_Control(pDX, IDC_CAPTURE_PATH, m_capturePath);
	DDX_Control(pDX, IDC_WATCH_PATH, m_watchPath);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(Directories, CDialog)
//{{AFX_MSG_MAP(Directories)
ON_BN_CLICKED(IDC_ROM_DIR, OnRomDir)
ON_BN_CLICKED(IDC_ROM_DIR_RESET, OnRomDirReset)
ON_BN_CLICKED(IDC_GBXROM_DIR, OnGBxRomDir)
ON_BN_CLICKED(IDC_GBXROM_DIR_RESET, OnGBxRomDirReset)
ON_BN_CLICKED(IDC_BATTERY_DIR, OnBatteryDir)
ON_BN_CLICKED(IDC_BATTERY_DIR_RESET, OnBatteryDirReset)
ON_BN_CLICKED(IDC_SAVE_DIR, OnSaveDir)
ON_BN_CLICKED(IDC_SAVE_DIR_RESET, OnSaveDirReset)
ON_BN_CLICKED(IDC_MOVIE_DIR, OnMovieDir)
ON_BN_CLICKED(IDC_MOVIE_DIR_RESET, OnMovieDirReset)
ON_BN_CLICKED(IDC_CHEAT_DIR, OnCheatDir)
ON_BN_CLICKED(IDC_CHEAT_DIR_RESET, OnCheatDirReset)
ON_BN_CLICKED(IDC_IPS_DIR, OnIpsDir)
ON_BN_CLICKED(IDC_IPS_DIR_RESET, OnIpsDirReset)
ON_BN_CLICKED(IDC_LUA_DIR, OnLuaDir)
ON_BN_CLICKED(IDC_LUA_DIR_RESET, OnLuaDirReset)
ON_BN_CLICKED(IDC_AVI_DIR, OnAviDir)
ON_BN_CLICKED(IDC_AVI_DIR_RESET, OnAviDirReset)
ON_BN_CLICKED(IDC_WAV_DIR, OnWavDir)
ON_BN_CLICKED(IDC_WAV_DIR_RESET, OnWavDirReset)
ON_BN_CLICKED(IDC_CAPTURE_DIR, OnCaptureDir)
ON_BN_CLICKED(IDC_CAPTURE_DIR_RESET, OnCaptureDirReset)
ON_BN_CLICKED(IDC_WATCH_DIR, OnWatchDir)
ON_BN_CLICKED(IDC_WATCH_DIR_RESET, OnWatchDirReset)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Directories message handlers

BOOL Directories::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString p = regQueryStringValue(IDS_ROM_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_ROM_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_GBXROM_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_GBXROM_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_BATTERY_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_BATTERY_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_SAVE_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_SAVE_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_MOVIE_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_MOVIE_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_CHEAT_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_CHEAT_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_IPS_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_IPS_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_LUA_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_LUA_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_AVI_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_AVI_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_WAV_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_WAV_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_CAPTURE_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_CAPTURE_PATH)->SetWindowText(p);

	p = regQueryStringValue(IDS_WATCH_DIR, NULL);
	if (!p.IsEmpty())
		GetDlgItem(IDC_WATCH_PATH)->SetWindowText(p);

	CenterWindow();

	return TRUE; // return TRUE unless you set the focus to a control
	             // EXCEPTION: OCX Property Pages should return FALSE
}

void Directories::OnRomDir()
{
	m_romPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_ROM_DIR));
	if (!p.IsEmpty())
		m_romPath.SetWindowText(p);
}

void Directories::OnRomDirReset()
{
	m_romPath.SetWindowText("");
}

void Directories::OnGBxRomDir()
{
	m_gbxromPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_GBXROM_DIR));
	if (!p.IsEmpty())
		m_gbxromPath.SetWindowText(p);
}

void Directories::OnGBxRomDirReset()
{
	m_gbxromPath.SetWindowText("");
}

void Directories::OnBatteryDir()
{
	m_batteryPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_BATTERY_DIR));
	if (!p.IsEmpty())
		m_batteryPath.SetWindowText(p);
}

void Directories::OnBatteryDirReset()
{
	m_batteryPath.SetWindowText("");
}

void Directories::OnSaveDir()
{
	m_savePath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_SAVE_DIR));
	if (!p.IsEmpty())
		m_savePath.SetWindowText(p);
}

void Directories::OnSaveDirReset()
{
	m_savePath.SetWindowText("");
}

void Directories::OnMovieDir()
{
	m_moviePath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_MOVIE_DIR));
	if (!p.IsEmpty())
		m_moviePath.SetWindowText(p);
}

void Directories::OnMovieDirReset()
{
	m_moviePath.SetWindowText("");
}

void Directories::OnCheatDir()
{
	m_cheatPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_CHEAT_DIR));
	if (!p.IsEmpty())
		m_cheatPath.SetWindowText(p);
}

void Directories::OnCheatDirReset()
{
	m_cheatPath.SetWindowText("");
}

void Directories::OnLuaDir()
{
	m_luaPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_LUA_DIR));
	if (!p.IsEmpty())
		m_luaPath.SetWindowText(p);
}

void Directories::OnLuaDirReset()
{
	m_luaPath.SetWindowText("");
}

void Directories::OnAviDir()
{
	m_aviPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_AVI_DIR));
	if (!p.IsEmpty())
		m_aviPath.SetWindowText(p);
}

void Directories::OnAviDirReset()
{
	m_aviPath.SetWindowText("");
}

void Directories::OnWavDir()
{
	m_wavPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_WAV_DIR));
	if (!p.IsEmpty())
		m_wavPath.SetWindowText(p);
}

void Directories::OnWavDirReset()
{
	m_wavPath.SetWindowText("");
}

void Directories::OnCaptureDir()
{
	m_capturePath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_CAPTURE_DIR));
	if (!p.IsEmpty())
		m_capturePath.SetWindowText(p);
}

void Directories::OnCaptureDirReset()
{
	m_capturePath.SetWindowText("");
}

void Directories::OnIpsDir()
{
	m_ipsPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_IPS_DIR));
	if (!p.IsEmpty())
		m_ipsPath.SetWindowText(p);
}

void Directories::OnIpsDirReset()
{
	m_ipsPath.SetWindowText("");
}

void Directories::OnWatchDir()
{
	m_watchPath.GetWindowText(initialFolderDir);
	CString p = browseForDir(winResLoadString(IDS_SELECT_WATCH_DIR));
	if(!p.IsEmpty())
		m_watchPath.SetWindowText(p);
}

void Directories::OnWatchDirReset()
{
	m_watchPath.SetWindowText("");
}

void Directories::OnCancel()
{
	EndDialog(FALSE);
}

void Directories::OnOK()
{
	CString buffer;

	m_romPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_ROM_DIR, buffer);
	else
		regDeleteValue(IDS_ROM_DIR);

	m_gbxromPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_GBXROM_DIR, buffer);
	else
		regDeleteValue(IDS_GBXROM_DIR);

	m_batteryPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_BATTERY_DIR, buffer);
	else
		regDeleteValue(IDS_BATTERY_DIR);

	m_savePath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_SAVE_DIR, buffer);
	else
		regDeleteValue(IDS_SAVE_DIR);

	m_moviePath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_MOVIE_DIR, buffer);
	else
		regDeleteValue(IDS_MOVIE_DIR);

	m_cheatPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_CHEAT_DIR, buffer);
	else
		regDeleteValue(IDS_CHEAT_DIR);

	m_ipsPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_IPS_DIR, buffer);
	else
		regDeleteValue(IDS_IPS_DIR);

	m_luaPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_LUA_DIR, buffer);
	else
		regDeleteValue(IDS_LUA_DIR);

	m_aviPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_AVI_DIR, buffer);
	else
		regDeleteValue(IDS_AVI_DIR);

	m_wavPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_WAV_DIR, buffer);
	else
		regDeleteValue(IDS_WAV_DIR);

	m_capturePath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_CAPTURE_DIR, buffer);
	else
		regDeleteValue(IDS_CAPTURE_DIR);

	m_watchPath.GetWindowText(buffer);
	if (!buffer.IsEmpty())
		regSetStringValue(IDS_WATCH_DIR, buffer);
	else
		regDeleteValue(IDS_WATCH_DIR);

	EndDialog(TRUE);
}

CString Directories::browseForDir(CString title)
{
	static char  buffer[1024];
	LPMALLOC     pMalloc;
	LPITEMIDLIST pidl;

	CString res;

	if (SUCCEEDED(SHGetMalloc(&pMalloc)))
	{
		BROWSEINFO bi;
		ZeroMemory(&bi, sizeof(bi));
		bi.hwndOwner = m_hWnd;
		bi.lpszTitle = title;
		bi.pidlRoot  = 0;
		bi.ulFlags   = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;	// will fail if COINIT_MULTITHREADED
		bi.lpfn      = browseCallbackProc;
		bi.lParam    = (LPARAM)(LPCTSTR)initialFolderDir;

		pidl = SHBrowseForFolder(&bi);

		if (pidl)
		{
			if (SHGetPathFromIDList(pidl, buffer))
			{
				res = buffer;
			}
			pMalloc->Free(pidl);
			pMalloc->Release();
		}
	}
	return res;
}
