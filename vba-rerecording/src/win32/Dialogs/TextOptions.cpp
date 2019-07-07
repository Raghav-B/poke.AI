// TextOptions.cpp : implementation file
//

#include "../stdafx.h"
#include "../../common/Text.h"
#include "../resource.h"
#include "TextOptions.h"

// TextOptions dialog

IMPLEMENT_DYNAMIC(TextOptions, CDialog)
TextOptions::TextOptions(CWnd*pParent /*=NULL*/)
	: CDialog(TextOptions::IDD, pParent)
{}

TextOptions::~TextOptions()
{}

BOOL TextOptions::OnInitDialog()
{
	CDialog::OnInitDialog();

	CheckRadioButton(IDC_RADIO_WHITE, IDC_RADIO_BLACK, IDC_RADIO_WHITE + textColor);
	CheckRadioButton(IDC_RADIO_PREFILTER, IDC_RADIO_POSTRENDER, IDC_RADIO_PREFILTER + textMethod);
	CheckDlgButton(IDC_CHECK_OUTLINED, outlinedText);
	CheckDlgButton(IDC_CHECK_TRANSPARENT, transparentText);
	GetDlgItem(IDC_CHECK_TRANSPARENT)->EnableWindow(GetCheckedRadioButton(IDC_RADIO_PREFILTER,
	                                                                      IDC_RADIO_POSTRENDER) != IDC_RADIO_POSTRENDER);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void TextOptions::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(TextOptions, CDialog)
ON_BN_CLICKED(IDOK, OnBnClickedOk)
ON_BN_CLICKED(IDC_RADIO_PREFILTER, OnBnClickedRadioPrefilter)
ON_BN_CLICKED(IDC_RADIO_POSTFILTER, OnBnClickedRadioPostfilter)
ON_BN_CLICKED(IDC_RADIO_POSTRENDER, OnBnClickedRadioPostrender)
END_MESSAGE_MAP()

// TextOptions message handlers

void TextOptions::OnBnClickedOk()
{
	transparentText = IsDlgButtonChecked(IDC_CHECK_TRANSPARENT) != 0;
	outlinedText    = IsDlgButtonChecked(IDC_CHECK_OUTLINED) != 0;
	textMethod      = GetCheckedRadioButton(IDC_RADIO_PREFILTER, IDC_RADIO_POSTRENDER) - IDC_RADIO_PREFILTER;
	textColor       = GetCheckedRadioButton(IDC_RADIO_WHITE, IDC_RADIO_BLACK) - IDC_RADIO_WHITE;
	if (textMethod < 0)
		textMethod = 0;
	if (textMethod > 2)
		textMethod = 2;
	if (textColor < 0)
		textColor = 0;
	if (textColor > 7)
		textColor = 7;

	OnOK();
}

void TextOptions::OnBnClickedRadioPrefilter()
{
	GetDlgItem(IDC_CHECK_TRANSPARENT)->EnableWindow(TRUE);
}

void TextOptions::OnBnClickedRadioPostfilter()
{
	GetDlgItem(IDC_CHECK_TRANSPARENT)->EnableWindow(TRUE);
}

void TextOptions::OnBnClickedRadioPostrender()
{
	GetDlgItem(IDC_CHECK_TRANSPARENT)->EnableWindow(FALSE);
	CheckDlgButton(IDC_CHECK_TRANSPARENT, FALSE);
}

