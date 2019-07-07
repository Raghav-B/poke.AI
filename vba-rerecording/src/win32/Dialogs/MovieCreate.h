#ifndef VBA_MOVIECREATE_H
#define VBA_MOVIECREATE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// MovieCreate dialog
class MovieCreate : public CDialog
{
	DECLARE_DYNAMIC(MovieCreate)

public:
	MovieCreate(CWnd* pParent = NULL);   // standard constructor
	virtual ~MovieCreate();
	virtual BOOL OnInitDialog();

// Dialog Data
  //{{AFX_DATA(MovieCreate)
	enum { IDD = IDD_MOVIECREATE };
    int    m_startOption;
    int    m_systemOption;
    int    m_biosOption;
    CEdit  m_editAuthor;
    CEdit  m_editDescription;
    CEdit  m_editFilename;
	//}}AFX_DATA

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedRecstart();
	afx_msg void OnBnClickedRecreset();
	afx_msg void OnBnClickedRecnow();
	afx_msg void OnBnClickedRecGba();
	afx_msg void OnBnClickedRecGbc();
	afx_msg void OnBnClickedRecSgb();
	afx_msg void OnBnClickedRecGb();
	afx_msg void OnBnClickedRecNobios();
	afx_msg void OnBnClickedRecEmubios();
	afx_msg void OnBnClickedRecGbabios();
	afx_msg void OnBnClickedRecGbabiosintro();
};

#endif // VBA_MOVIECREATE_H
