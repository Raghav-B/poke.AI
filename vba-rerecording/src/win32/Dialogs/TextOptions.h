#ifndef VBA_WIN32_TEXT_OPTIONS_H
#define VBA_WIN32_TEXT_OPTIONS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// TextOptions dialog

class TextOptions : public CDialog
{
	DECLARE_DYNAMIC(TextOptions)

public:
	TextOptions(CWnd* pParent = NULL);   // standard constructor
	virtual ~TextOptions();

	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_TEXTCONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedRadioPrefilter();
	afx_msg void OnBnClickedRadioPostfilter();
	afx_msg void OnBnClickedRadioPostrender();
};

#endif
