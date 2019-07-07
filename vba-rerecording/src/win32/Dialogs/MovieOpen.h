#ifndef VBA_MOVIE_OPEN_H
#define VBA_MOVIE_OPEN_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "../../common/movie.h"

// MovieOpen dialog

class MovieOpen : public CDialog
{
	DECLARE_DYNAMIC(MovieOpen)
public:
	MovieOpen(CWnd*pParent = NULL);    // standard constructor
	virtual ~MovieOpen();

// Dialog Data
	//{{AFX_DATA(MovieOpen)
	enum { IDD = IDD_MOVIEOPEN };
	CEdit   m_editAuthor;
	CEdit   m_editDescription;
	CEdit   m_editFilename;
	CEdit   m_editPauseFrame;
	CString moviePhysicalName;
	CString movieLogicalName;
	CStatic m_warning1;
	CStatic m_warning2;
	//}}AFX_DATA

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(GBACheatList)
protected:
	virtual void DoDataExchange(CDataExchange*pDX);     // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	SMovie  movieInfo;
	int     pauseFrame;

	// Generated message map functions
	//{{AFX_MSG(GBACheatList)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd *pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedMovieRefresh();
	afx_msg void OnBnClickedReadonly();
	afx_msg void OnBnClickedCheckPauseframe();
	afx_msg void OnBnClickedHideborder();
	afx_msg void OnEnChangeMovieFilename();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif // VBA_MOVIE_OPEN_H
