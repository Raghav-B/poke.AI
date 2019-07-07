// GBMemoryViewerDlg.cpp : implementation file
//

#include "../stdafx.h"
#include "../resource.h"
#include "GBMemoryViewerDlg.h"
#include "FileDlg.h"
#include "MemoryViewerAddressSize.h"
#include "../Reg.h"
#include "../WinResUtil.h"
#include "../VBA.h"

#include "../../common/SystemGlobals.h"
#include "../../gb/gbGlobals.h"

GBMemoryViewer::GBMemoryViewer()
	: MemoryViewer()
{
	setAddressSize(1);
}

void GBMemoryViewer::readData(u32 address, int len, u8 *data)
{
	u16 addr = address & 0xffff;
	if (emulating && gbRom != NULL)
	{
		for (int i = 0; i < len; i++)
		{
			*data++ = gbReadMemoryQuick(addr);
			addr++;
		}
	}
	else
	{
		for (int i = 0; i < len; i++)
		{
			*data++ = 0;
			addr++;
		}
	}
}

void GBMemoryViewer::editData(u32 address, int size, int mask, u32 value)
{
	u32 oldValue;
	u16 addr = (u16)address & 0xffff;
	switch (size)
	{
	case 8:
		oldValue  = gbReadMemoryQuick(addr);
		oldValue &= mask;
		oldValue |= (u8)value;
		gbWriteMemoryQuick(addr, oldValue);
		break;
	case 16:
		oldValue = gbReadMemoryQuick(addr) |
		           (gbReadMemoryQuick(addr + 1) << 8);
		oldValue &= mask;
		oldValue |= (u16)value;
		gbWriteMemoryQuick(addr, (oldValue & 255));
		gbWriteMemoryQuick(addr+1, (oldValue >> 8));
		break;
	case 32:
		oldValue = gbReadMemoryQuick(addr) |
		           (gbReadMemoryQuick(addr + 1) << 8) |
		           (gbReadMemoryQuick(addr + 2) << 16) |
		           (gbReadMemoryQuick(addr + 3) << 24);
		oldValue &= mask;
		oldValue |= (u32)value;
		gbWriteMemoryQuick(addr, (oldValue & 255));
		gbWriteMemoryQuick(addr+1, (oldValue >> 8));
		gbWriteMemoryQuick(addr+2, (oldValue >> 16));
		gbWriteMemoryQuick(addr+3, (oldValue >> 24));
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////
// GBMemoryViewerDlg dialog

GBMemoryViewerDlg::GBMemoryViewerDlg(CWnd*pParent /*=NULL*/)
	: ResizeDlg(GBMemoryViewerDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(GBMemoryViewerDlg)
	m_size = -1;
	//}}AFX_DATA_INIT
	autoUpdate = false;
}

void GBMemoryViewerDlg::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(GBMemoryViewerDlg)
	DDX_Control(pDX, IDC_CURRENT_ADDRESS, m_current);
	DDX_Control(pDX, IDC_ADDRESS, m_address);
	DDX_Control(pDX, IDC_ADDRESSES, m_addresses);
	DDX_Radio(pDX, IDC_8_BIT, m_size);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_VIEWER, m_viewer);
}

BEGIN_MESSAGE_MAP(GBMemoryViewerDlg, CDialog)
//{{AFX_MSG_MAP(GBMemoryViewerDlg)
ON_BN_CLICKED(IDC_CLOSE, OnClose)
ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
ON_BN_CLICKED(IDC_8_BIT, On8Bit)
ON_BN_CLICKED(IDC_16_BIT, On16Bit)
ON_BN_CLICKED(IDC_32_BIT, On32Bit)
ON_BN_CLICKED(IDC_AUTO_UPDATE, OnAutoUpdate)
ON_BN_CLICKED(IDC_DECIMAL_DISPLAY, OnDecimalDisplay)
ON_BN_CLICKED(IDC_ALIGN, OnAlign)
ON_BN_CLICKED(IDC_GO, OnGo)
ON_CBN_SELCHANGE(IDC_ADDRESSES, OnSelchangeAddresses)
ON_BN_CLICKED(IDC_SAVE, OnSave)
ON_BN_CLICKED(IDC_LOAD, OnLoad)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GBMemoryViewerDlg message handlers

BOOL GBMemoryViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	autoUpdate = !regQueryDwordValue("memViewerAutoUpdate", 1);
	OnAutoUpdate(); // inverts and update dialog

	decimalDisplay = !regQueryDwordValue("memViewerDecimalDisplay", 0);
	OnDecimalDisplay();

	align = !regQueryDwordValue("memViewerAlign", 0);
	OnAlign();

	DIALOG_SIZER_START(sz)
	DIALOG_SIZER_ENTRY(IDC_VIEWER, DS_SizeX | DS_SizeY)
	DIALOG_SIZER_ENTRY(IDC_REFRESH, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_CLOSE, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_LOAD, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_SAVE, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_AUTO_UPDATE, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_DECIMAL_DISPLAY, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_ALIGN, DS_MoveY)
	DIALOG_SIZER_ENTRY(IDC_CURRENT_ADDRESS_LABEL, DS_MoveY | DS_MoveX)
	DIALOG_SIZER_ENTRY(IDC_CURRENT_ADDRESS, DS_MoveY | DS_MoveX)
	DIALOG_SIZER_END()
	SetData(sz,
	        TRUE,
	        HKEY_CURRENT_USER,
	        "Software\\Emulators\\VisualBoyAdvance\\Viewer\\GBMemoryView",
	        NULL);

	m_viewer.setDialog(this);
	m_viewer.ShowScrollBar(SB_VERT, TRUE);
	m_viewer.EnableScrollBar(SB_VERT, ESB_ENABLE_BOTH);

	LPCTSTR s[] = {
		"0x0000 - ROM",
		"0x4000 - ROM",
		"0x8000 - VRAM",
		"0xA000 - SRAM",
		"0xC000 - RAM",
		"0xD000 - WRAM",
		"0xFF00 - I/O",
		"0xFF80 - RAM"
	};

	for (int i = 0; i < 8; i++)
		m_addresses.AddString(s[i]);

	m_addresses.SetCurSel(0);

	RECT cbSize;
	int  Height;

	m_addresses.GetClientRect(&cbSize);
	Height  = m_addresses.GetItemHeight(-1);
	Height += m_addresses.GetItemHeight(0) * (9);

	// Note: The use of SM_CYEDGE assumes that we're using Windows '95
	// Now add on the height of the border of the edit box
	Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// The height of the border of the drop-down box
	Height += GetSystemMetrics(SM_CYEDGE) * 2; // top & bottom edges

	// now set the size of the window
	m_addresses.SetWindowPos(NULL,
	                         0, 0,
	                         cbSize.right, Height,
	                         SWP_NOMOVE | SWP_NOZORDER);

	m_address.LimitText(8);

	m_size = regQueryDwordValue("memViewerDataSize", 0);
	if (m_size < 0 || m_size > 2)
		m_size = 0;
	m_viewer.setSize(m_size);
	UpdateData(FALSE);

	m_current.SetFont(CFont::FromHandle((HFONT)GetStockObject(SYSTEM_FIXED_FONT)));

	return TRUE; // return TRUE unless you set the focus to a control
	             // EXCEPTION: OCX Property Pages should return FALSE
}

void GBMemoryViewerDlg::OnClose()
{
	theApp.winRemoveUpdateListener(this);

	DestroyWindow();
}

void GBMemoryViewerDlg::OnRefresh()
{
	m_viewer.Invalidate();
}

void GBMemoryViewerDlg::update()
{
	OnRefresh();
}

void GBMemoryViewerDlg::On8Bit()
{
	m_viewer.setSize(0);
	regSetDwordValue("memViewerDataSize", 0);
}

void GBMemoryViewerDlg::On16Bit()
{
	m_viewer.setSize(1);
	regSetDwordValue("memViewerDataSize", 1);
}

void GBMemoryViewerDlg::On32Bit()
{
	m_viewer.setSize(2);
	regSetDwordValue("memViewerDataSize", 2);
}

void GBMemoryViewerDlg::OnAutoUpdate()
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
	if (GetDlgItem(IDC_AUTO_UPDATE))
		((CButton *)GetDlgItem(IDC_AUTO_UPDATE))->SetCheck(autoUpdate ? TRUE : FALSE);
	regSetDwordValue("memViewerAutoUpdate", autoUpdate);
}

void GBMemoryViewerDlg::OnDecimalDisplay()
{
	decimalDisplay = !decimalDisplay;
	m_viewer.setDecimal(decimalDisplay ? true : false);
	if (GetDlgItem(IDC_DECIMAL_DISPLAY))
		((CButton *)GetDlgItem(IDC_DECIMAL_DISPLAY))->SetCheck(decimalDisplay ? TRUE : FALSE);

	regSetDwordValue("memViewerDecimalDisplay", decimalDisplay);
}

void GBMemoryViewerDlg::OnAlign()
{
	align = !align;
	if (GetDlgItem(IDC_ALIGN))
		((CButton *)GetDlgItem(IDC_ALIGN))->SetCheck(align ? TRUE : FALSE);

	regSetDwordValue("memViewerAlign", align);
}

void GBMemoryViewerDlg::OnGo()
{
	CString buffer;

	m_address.GetWindowText(buffer);

	u32 address;
	sscanf(buffer, "%x", &address);
	if (align)
		address &= ~0xF;
	else
	{
		if (m_viewer.getSize() == 1)
			address &= ~1;
		else if (m_viewer.getSize() == 2)
			address &= ~3;
	}
	m_viewer.setAddress(address);
}

void GBMemoryViewerDlg::OnSelchangeAddresses()
{
	int cur = m_addresses.GetCurSel();

	switch (cur)
	{
	case 0:
		m_viewer.setAddress(0x0000);
		break;
	case 1:
		m_viewer.setAddress(0x4000);
		break;
	case 2:
		m_viewer.setAddress(0x8000);
		break;
	case 3:
		m_viewer.setAddress(0xa000);
		break;
	case 4:
		m_viewer.setAddress(0xc000);
		break;
	case 5:
		m_viewer.setAddress(0xd000);
		break;
	case 6:
		m_viewer.setAddress(0xff00);
		break;
	case 7:
		m_viewer.setAddress(0xff80);
		break;
	}
}

void GBMemoryViewerDlg::setCurrentAddress(u32 address)
{
	CString buffer;

	buffer.Format("0x%08X", address);
	m_current.SetWindowText(buffer);
}

void GBMemoryViewerDlg::OnSave()
{
	MemoryViewerAddressSize dlg;

	dlg.setAddress(m_viewer.getCurrentAddress());

	LPCTSTR exts[] = { ".dmp", NULL };

	CString filter = winResLoadFilter(IDS_FILTER_DUMP);
	CString title  = winResLoadString(IDS_SELECT_DUMP_FILE);

	if (dlg.DoModal() == IDOK)
	{
		CString buffer;
		FileDlg file(this,
		             buffer,
		             filter,
		             0,
		             "DMP",
		             exts,
		             "",
		             title,
		             true);
		if (file.DoModal() == IDOK)
		{
			buffer = file.GetPathName();
			FILE *f = fopen(buffer, "wb");

			if (f == NULL)
			{
				systemMessage(IDS_ERROR_CREATING_FILE, buffer);
				return;
			}

			int size = dlg.getSize();
			u16 addr = dlg.getAddress() & 0xffff;

			for (int i = 0; i < size; i++)
			{
				fputc(gbReadMemoryQuick(addr), f);
				addr++;
			}

			fclose(f);
		}
	}
}

void GBMemoryViewerDlg::OnLoad()
{
	CString buffer;
	LPCTSTR exts[] = { ".dmp", NULL };
	CString filter = winResLoadFilter(IDS_FILTER_DUMP);
	CString title  = winResLoadString(IDS_SELECT_DUMP_FILE);

	FileDlg file(this,
	             buffer,
	             filter,
	             0,
	             "DMP",
	             exts,
	             "",
	             title,
	             false);

	if (file.DoModal() == IDOK)
	{
		buffer = file.GetPathName();
		FILE *f = fopen(buffer, "rb");
		if (f == NULL)
		{
			systemMessage(IDS_CANNOT_OPEN_FILE,
			              "Cannot open file %s",
			              buffer);
			return;
		}

		MemoryViewerAddressSize dlg;

		fseek(f, 0, SEEK_END);
		int size = ftell(f);

		fseek(f, 0, SEEK_SET);

		dlg.setAddress(m_viewer.getCurrentAddress());
		dlg.setSize(size);

		if (dlg.DoModal() == IDOK)
		{
			int size = dlg.getSize();
			u16 addr = dlg.getAddress() & 0xffff;

			for (int i = 0; i < size; i++)
			{
				int c = fgetc(f);
				if (c == -1)
					break;
				gbWriteMemoryQuick(addr, c);
				addr++;
			}
			OnRefresh();
		}
		fclose(f);
	}
}

void GBMemoryViewerDlg::PostNcDestroy()
{
	delete this;
}

