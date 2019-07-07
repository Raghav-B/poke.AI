#if !defined(AFX_ABOUTDIALOG_H__48D787B2_0699_4F03_827D_404EC70DDDB2__INCLUDED_)
#define AFX_ABOUTDIALOG_H__48D787B2_0699_4F03_827D_404EC70DDDB2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// -*- C++ -*-
// AboutDialog.h : header file
//

#include "../Controls/Hyperlink.h"

/////////////////////////////////////////////////////////////////////////////
// AboutDialog dialog

class AboutDialog : public CDialog
{
	Hyperlink m_link;
	Hyperlink m_translator;
	// Construction
public:
	AboutDialog(CWnd*pParent = NULL);  // standard constructor

	// Dialog Data
	//{{AFX_DATA(AboutDialog)
	enum { IDD = IDD_ABOUT };
	CString m_version;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AboutDialog)
protected:
	virtual void DoDataExchange(CDataExchange*pDX);   // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(AboutDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ABOUTDIALOG_H__48D787B2_0699_4F03_827D_404EC70DDDB2__INCLUDED_)
