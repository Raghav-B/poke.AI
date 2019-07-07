// AboutDialog.cpp : implementation file
//

#include "../stdafx.h"
#include "../resource.h"
#include "AboutDialog.h"

#include "../../version.h"

/////////////////////////////////////////////////////////////////////////////
// AboutDialog dialog

AboutDialog::AboutDialog(CWnd*pParent /*=NULL*/)
	: CDialog(AboutDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(AboutDialog)
	m_version = _T(VBA_VERSION_STRING);
	//}}AFX_DATA_INIT
}

void AboutDialog::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(AboutDialog)
	DDX_Text(pDX, IDC_VERSION, m_version);
	DDX_Control(pDX, IDC_URL, m_link);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(AboutDialog, CDialog)
//{{AFX_MSG_MAP(AboutDialog)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// AboutDialog message handlers

BOOL AboutDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd *p = GetDlgItem(IDC_TRANSLATOR_URL);
	if (p)
	{
		m_translator.SubclassDlgItem(IDC_TRANSLATOR_URL, this);
	}

	m_link.SetWindowText(VBA_RR_SITE);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void AboutDialog::OnOK()
{
	// TODO: Add extra validation here

	CDialog::OnOK();
}

