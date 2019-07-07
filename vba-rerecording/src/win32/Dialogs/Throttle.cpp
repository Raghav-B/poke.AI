// Throttle.cpp : implementation file
//

#include "../stdafx.h"
#include "../resource.h"
#include "Throttle.h"
#include "../../common/System.h" // for system messages

/////////////////////////////////////////////////////////////////////////////
// Throttle dialog

Throttle::Throttle(CWnd*pParent /*=NULL*/)
	: CDialog(Throttle::IDD, pParent)
{
	//{{AFX_DATA_INIT(Throttle)
	m_throttle = 100;
	//}}AFX_DATA_INIT
}

void Throttle::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(Throttle)
	DDX_Text(pDX, IDC_THROTTLE, m_throttle);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(Throttle, CDialog)
//{{AFX_MSG_MAP(Throttle)
ON_BN_CLICKED(ID_CANCEL, OnCancel)
ON_BN_CLICKED(ID_OK, OnOk)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Throttle message handlers

BOOL Throttle::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow();

	return TRUE; // return TRUE unless you set the focus to a control
	             // EXCEPTION: OCX Property Pages should return FALSE
}

void Throttle::OnCancel()
{
	EndDialog(false);
}

void Throttle::OnOk()
{
	UpdateData();

	if (m_throttle < 1 || m_throttle > 1000)
		systemMessage(IDS_INVALID_THROTTLE_VALUE, "Invalid throttle value. Please enter a number between 1 and 1000");
	else
		EndDialog(m_throttle);
}

