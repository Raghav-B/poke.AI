#if !defined(AFX_BUGREPORT_H__DE7BC381_E45D_4200_910C_E5378E6364C9__INCLUDED_)
#define AFX_BUGREPORT_H__DE7BC381_E45D_4200_910C_E5378E6364C9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BugReport.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// BugReport dialog

class BugReport : public CDialog
{
	// Construction
public:
	BugReport(CWnd*pParent = NULL);  // standard constructor

	// Dialog Data
	//{{AFX_DATA(BugReport)
	enum { IDD = IDD_BUG_REPORT };
	CEdit m_report;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(BugReport)
protected:
	virtual void DoDataExchange(CDataExchange*pDX);   // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	CString createReport();

	// Generated message map functions
	//{{AFX_MSG(BugReport)
	afx_msg void OnCopy();
	afx_msg void OnOk();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BUGREPORT_H__DE7BC381_E45D_4200_910C_E5378E6364C9__INCLUDED_)
