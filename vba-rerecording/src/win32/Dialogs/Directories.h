#if !defined(AFX_DIRECTORIES_H__7ADB14C1_3C1B_4294_8D66_A4E87D6FC731__INCLUDED_)
#define AFX_DIRECTORIES_H__7ADB14C1_3C1B_4294_8D66_A4E87D6FC731__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Directories.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// Directories dialog

class Directories : public CDialog
{
	// Construction
public:
	CString initialFolderDir;
	CString browseForDir(CString title);
	Directories(CWnd*pParent = NULL);  // standard constructor

	// Dialog Data
	//{{AFX_DATA(Directories)
	enum { IDD = IDD_DIRECTORIES };
	CEdit m_romPath;
	CEdit m_gbxromPath;
	CEdit m_batteryPath;
	CEdit m_savePath;
	CEdit m_moviePath;
	CEdit m_cheatPath;
	CEdit m_ipsPath;
	CEdit m_luaPath;
	CEdit m_aviPath;
	CEdit m_wavPath;
	CEdit m_capturePath;
	CEdit m_watchPath;
//  CEdit m_pluginPath;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Directories)
protected:
	virtual void DoDataExchange(CDataExchange*pDX);   // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(Directories)
	virtual BOOL OnInitDialog();
	afx_msg void OnRomDir();
	afx_msg void OnRomDirReset();
	afx_msg void OnGBxRomDir();
	afx_msg void OnGBxRomDirReset();
	afx_msg void OnBatteryDir();
	afx_msg void OnBatteryDirReset();
	afx_msg void OnSaveDir();
	afx_msg void OnSaveDirReset();
	afx_msg void OnMovieDir();
	afx_msg void OnMovieDirReset();
	afx_msg void OnCheatDir();
	afx_msg void OnCheatDirReset();
	afx_msg void OnIpsDir();
	afx_msg void OnIpsDirReset();
	afx_msg void OnLuaDir();
	afx_msg void OnLuaDirReset();
	afx_msg void OnAviDir();
	afx_msg void OnAviDirReset();
	afx_msg void OnWavDir();
	afx_msg void OnWavDirReset();
	afx_msg void OnCaptureDir();
	afx_msg void OnCaptureDirReset();
	afx_msg void OnWatchDir();
	afx_msg void OnWatchDirReset();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DIRECTORIES_H__7ADB14C1_3C1B_4294_8D66_A4E87D6FC731__INCLUDED_)
