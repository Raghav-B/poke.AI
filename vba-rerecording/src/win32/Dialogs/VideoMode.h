#if !defined(AFX_VIDEOMODE_H__074B2426_32EA_4D69_9215_AB5E90F885D0__INCLUDED_)
#define AFX_VIDEOMODE_H__074B2426_32EA_4D69_9215_AB5E90F885D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// VideoMode.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VideoMode dialog

class VideoMode : public CDialog
{
	// Construction
public:
	VideoMode(LPDIRECTDRAW7 pDraw, CWnd *pParent = NULL); // standard constructor

	// Dialog Data
	//{{AFX_DATA(VideoMode)
	enum { IDD = IDD_MODES };
	CListBox m_modes;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VideoMode)
protected:
	virtual void DoDataExchange(CDataExchange *pDX);  // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(VideoMode)
	afx_msg void OnSelchangeModes();
	afx_msg void OnCancel();
	afx_msg void OnOk();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	LPDIRECTDRAW7 pDirectDraw;
};

/////////////////////////////////////////////////////////////////////////////
// VideoDriverSelect dialog

class VideoDriverSelect : public CDialog
{
	// Construction
public:
	VideoDriverSelect(CWnd *pParent = NULL); // standard constructor

	// Dialog Data
	//{{AFX_DATA(VideoDriverSelect)
	enum { IDD = IDD_DRIVERS };
	CListBox m_drivers;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VideoDriverSelect)
protected:
	virtual void DoDataExchange(CDataExchange *pDX);  // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(VideoDriverSelect)
	afx_msg void OnCancel();
	afx_msg void OnOk();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeDrivers();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
#endif // !defined(AFX_VIDEOMODE_H__074B2426_32EA_4D69_9215_AB5E90F885D0__INCLUDED_)
