// IOViewer.cpp : implementation file
//

#include "../stdafx.h"
#include "../resource.h"
#include "IOViewer.h"
#include "../VBA.h"

#include "../../gba/GBA.h"
#include "../../gba/GBAinline.h" // CPUWriteHalfWord
#include "../../gba/GBAGlobals.h"

#include "IOViewerRegs.h"

/////////////////////////////////////////////////////////////////////////////
// IOViewer dialog

IOViewer::IOViewer(CWnd*pParent /*=NULL*/)
	: ResizeDlg(IOViewer::IDD, pParent)
{
	//{{AFX_DATA_INIT(IOViewer)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	selected   = 0;
	autoUpdate = false;
}

void IOViewer::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(IOViewer)
	DDX_Control(pDX, IDC_VALUE, m_value);
	DDX_Control(pDX, IDC_ADDRESSES, m_address);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(IOViewer, CDialog)
//{{AFX_MSG_MAP(IOViewer)
ON_BN_CLICKED(IDC_CLOSE, OnClose)
ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
ON_CBN_SELCHANGE(IDC_ADDRESSES, OnSelchangeAddresses)
ON_BN_CLICKED(IDC_APPLY, OnApply)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// IOViewer message handlers

void IOViewer::OnClose()
{
	theApp.winRemoveUpdateListener(this);

	DestroyWindow();
}

void IOViewer::OnRefresh()
{
	// TODO: Add your control notification handler code here
}

void IOViewer::OnAutoUpdate()
{
	autoUpdate = !autoUpdate;
	if (autoUpdate)
	{
		theApp.winAddUpdateListener(this);
	}
	else
	{
		theApp.winRemoveUpdateListener(this);
	}
}

void IOViewer::OnSelchangeAddresses()
{
	selected = m_address.GetCurSel();

	update();
}

void IOViewer::PostNcDestroy()
{
	delete this;
}

BOOL IOViewer::OnInitDialog()
{
	CDialog::OnInitDialog();

	// winCenterWindow(getHandle());
	DIALOG_SIZER_START(sz)
	DIALOG_SIZER_END()
	SetData(sz,
	        TRUE,
	        HKEY_CURRENT_USER,
	        "Software\\Emulators\\VisualBoyAdvance\\Viewer\\IOView",
	        NULL);

	CFont *font = CFont::FromHandle((HFONT)GetStockObject(SYSTEM_FIXED_FONT));
	int    i;
	for (i = 0; i < sizeof(ioViewRegisters)/sizeof(IOData); i++)
	{
		m_address.AddString(ioViewRegisters[i].name);
	}
	m_address.SetFont(font);
	for (i = 0; i < 16; i++)
	{
		GetDlgItem(IDC_BIT_0+i)->SetFont(font);
	}

	RECT cbSize;
	int  Height;

	m_address.GetClientRect(&cbSize);
	Height  = m_address.GetItemHeight(0);
	Height += m_address.GetItemHeight(0) * (10);

	// Note: The use of SM_CYEDGE assumes that we're using Windows '95
	// Now add on the height of the border of the edit box
	Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// The height of the border of the drop-down box
	Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// now set the size of the window
	m_address.SetWindowPos(NULL,
	                       0, 0,
	                       cbSize.right, Height,
	                       SWP_NOMOVE | SWP_NOZORDER);

	m_address.SetCurSel(0);
	update();

	return TRUE; // return TRUE unless you set the focus to a control
	             // EXCEPTION: OCX Property Pages should return FALSE
}

void IOViewer::update()
{
	CString buffer;

	const IOData *sel = &ioViewRegisters[selected];
	u16 data = sel->address ? *sel->address :
	           (ioMem ? *((u16 *)&ioMem[sel->offset]) : 0);

	for (int i = 0; i < 16; i++)
	{
		CButton *pWnd = (CButton *)GetDlgItem(IDC_BIT_0 + i);

		if (pWnd)
		{
			if (!(sel->write & (1 << i)))
				pWnd->EnableWindow(FALSE);
			else
				pWnd->EnableWindow(TRUE);
			pWnd->SetCheck(((data & (1 << i)) >> i));
			buffer.Format("%2d %s", i, sel->bits[i]);
			pWnd->SetWindowText(buffer);
		}
	}

	buffer.Format("%04X", data);
	m_value.SetWindowText(buffer);
}

void IOViewer::OnApply()
{
	const IOData *sel = &ioViewRegisters[selected];
	u16 res = 0;
	for (int i = 0; i < 16; i++)
	{
		CButton *pWnd = (CButton *)GetDlgItem(IDC_BIT_0 + i);

		if (pWnd)
		{
			if (pWnd->GetCheck())
				res |= (1 << i);
		}
	}
	CPUWriteHalfWord(0x4000000+sel->offset, res);
	update();
}

