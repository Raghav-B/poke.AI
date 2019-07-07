#if !defined(AFX_MODECONFIRM_H__AF9F877E_6EDF_4523_95C9_1C745ABBA796__INCLUDED_)
#define AFX_MODECONFIRM_H__AF9F877E_6EDF_4523_95C9_1C745ABBA796__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// -*- C++ -*-
// ModeConfirm.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// ModeConfirm dialog

class ModeConfirm : public CDialog
{
	// Construction
public:
	int  count;
	UINT timer;
	ModeConfirm(CWnd*pParent);  // standard constructor

	// Dialog Data
	//{{AFX_DATA(ModeConfirm)
	enum { IDD = IDD_MODE_CONFIRM };
	// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(ModeConfirm)
protected:
	virtual void DoDataExchange(CDataExchange*pDX);   // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(ModeConfirm)
	afx_msg void OnCancel();
	afx_msg void OnOk();
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MODECONFIRM_H__AF9F877E_6EDF_4523_95C9_1C745ABBA796__INCLUDED_)
