#if !defined(AFX_ACCELEDITOR_H__66F5C854_E28E_40D1_B763_1850374B46A2__INCLUDED_)
#define AFX_ACCELEDITOR_H__66F5C854_E28E_40D1_B763_1850374B46A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AccelEditor.h : header file
//
#include "../AcceleratorManager.h"
#include "../Controls/KeyboardEdit.h"
#include "ResizeDlg.h"

/////////////////////////////////////////////////////////////////////////////
// AccelEditor dialog

class AccelEditor : public ResizeDlg
{
	// Construction
public:
	void InitCommands();
	void AddCommandsFromTable();
	void AddCommandsFromMenu(CMenu *pMenu, HTREEITEM hParent);
	BOOL IsModified() const;
	const CAcceleratorManager &GetResultMangager() const;
	AccelEditor(CWnd *pParent, CMenu *pMenu, CAcceleratorManager *pExtMgr = NULL);  // non-standard constructor
	virtual BOOL PreTranslateMessage(MSG *pMsg);

	// Dialog Data
	//{{AFX_DATA(AccelEditor)
protected:
	enum { IDD = IDD_ACCEL_EDITOR };
	enum { KEY_COLUMN = 0 };
	enum { AUTO_REPLACE = 0, AUTO_NEW };
	CListCtrl		  m_currents;
	CStatic			  m_alreadyAffected;
	CTreeCtrl		  m_commands;
	CKeyboardEdit	  m_key;
	CEdit			  m_timeout;
	CProgressCtrl	  m_progress;
	CList<HTREEITEM>  m_hItems;

	int m_timeoutValue;
	int m_timer;
	int m_autoMode;
	BOOL m_modified;
	CMenu *m_pMenuSrc;
	CAcceleratorManager m_mgr, m_result;
	CAcceleratorManager *m_pExtMgr;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AccelEditor)
protected:
	virtual void DoDataExchange(CDataExchange *pDX);   // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	BOOL CheckAffected();
	BOOL CheckJammed();
	BOOL CheckListSelections();

	// Generated message map functions
	//{{AFX_MSG(AccelEditor)
	virtual BOOL OnInitDialog();
	afx_msg void OnOk();
	afx_msg void OnCancel();
	afx_msg void OnApply();
	afx_msg void OnReset();
	afx_msg void OnAssign();
	afx_msg void OnRemove();
	afx_msg void OnReplace();
	afx_msg void OnNew();
	afx_msg void OnEdit();

	afx_msg void OnTvnSelchangedCommands(NMHDR *pNMHDR, LRESULT *pResult);
//	afx_msg void OnListItemChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnListDblClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyboardEditChange();
	afx_msg void OnKeyboardEditKillfocus();
	afx_msg void OnTimeoutEditSetfocus();
	afx_msg void OnTimeoutEditKillfocus();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ACCELEDITOR_H__66F5C854_E28E_40D1_B763_1850374B46A2__INCLUDED_)
