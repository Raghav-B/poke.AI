// BugReport.cpp : implementation file
//

#include "../stdafx.h"
#include "../resource.h"
#include "BugReport.h"
#include "../VBA.h"

#include "../../version.h"
#include "../../gba/agbprint.h"
#include "../../gba/Flash.h"
#include "../../gba/GBACheats.h"
#include "../../gba/GBAGlobals.h"
#include "../../gb/gbCheats.h"
#include "../../gb/gbGlobals.h"
#include "../../gba/RTC.h"
#include "../../common/System.h"
#include "../../common/vbalua.h"
#include "../../common/SystemGlobals.h"

/////////////////////////////////////////////////////////////////////////////
// BugReport dialog

BugReport::BugReport(CWnd*pParent /*=NULL*/)
	: CDialog(BugReport::IDD, pParent)
{
	//{{AFX_DATA_INIT(BugReport)
	// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void BugReport::DoDataExchange(CDataExchange*pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BugReport)
	DDX_Control(pDX, IDC_BUG_REPORT, m_report);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(BugReport, CDialog)
//{{AFX_MSG_MAP(BugReport)
ON_BN_CLICKED(IDC_COPY, OnCopy)
ON_BN_CLICKED(ID_OK, OnOk)
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// BugReport message handlers

void BugReport::OnCopy()
{
	OpenClipboard();

	EmptyClipboard();
	CString report;
	m_report.GetWindowText(report);

	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE,
	                               (report.GetLength() + 1) * sizeof(CHAR));
	if (hglbCopy == NULL)
	{
		CloseClipboard();
		return;
	}

	// Lock the handle and copy the text to the buffer.

	LPSTR lptstrCopy = (LPSTR)GlobalLock(hglbCopy);
	memcpy(lptstrCopy, (const char *)report,
	       report.GetLength() * sizeof(CHAR));
	lptstrCopy[report.GetLength()] = (TCHAR) 0;  // null character
	GlobalUnlock(hglbCopy);

	// Place the handle on the clipboard.

	SetClipboardData(CF_TEXT, hglbCopy);
	CloseClipboard();

	systemMessage(IDS_BUG_REPORT, "Bug report has been copied to the Clipboard");
}

void BugReport::OnOk()
{
	EndDialog(TRUE);
}

BOOL BugReport::OnInitDialog()
{
	CDialog::OnInitDialog();

	CenterWindow();

	CString report = createReport();

	m_report.SetFont(CFont::FromHandle((HFONT)GetStockObject(SYSTEM_FIXED_FONT)));

	m_report.SetWindowText(report);

	return TRUE; // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

static void AppendFormat(CString& report, const char *format, ...)
{
	CString buffer;
	va_list valist;

	va_start(valist, format);
	buffer.FormatV(format, valist);
	va_end(valist);
	report += buffer;
}

CString BugReport::createReport()
{
	theApp.winCheckFullscreen();

	CString report = "";
	AppendFormat(report, "Emu version  : %s\r\n", VBA_VERSION_STRING);
AppendFormat(report, "Emu type     : %s\r\n", VBA_BUILDTYPE_STRING);

	if (systemIsEmulating())
	{
		AppendFormat(report, "Game         : %s\r\n", theApp.gameFilename);

		char buffer[20];
		if (systemCartridgeType == IMAGE_GBA)
		{
			u32 check = 0;
			for (int i = 0; i < 0x4000; i += 4)
			{
				check += *((u32 *)&bios[i]);
			}
			AppendFormat(report, "BIOS checksum: %08X\r\n", check);

			strncpy(buffer, (const char *)&rom[0xa0], 12);
			buffer[12] = 0;
			AppendFormat(report, "Internal name: %s\r\n", buffer);

			strncpy(buffer, (const char *)&rom[0xac], 4);
			buffer[4] = 0;
			AppendFormat(report, "Game code    : %s\r\n", buffer);

			CString res = "";
			u32 *   p   = (u32 *)rom;
			u32 *   end = (u32 *)((char *)rom+theApp.romSize);
			while (p  < end)
			{
				u32 d = READ32LE(p);

				if (d == 0x52504545)
				{
					if (memcmp(p, "EEPROM_", 7) == 0)
					{
						res += (const char *)p;
						res += ' ';
					}
				}
				else if (d == 0x4D415253)
				{
					if (memcmp(p, "SRAM_", 5) == 0)
					{
						res += (const char *)p;
						res += ' ';
					}
				}
				else if (d == 0x53414C46)
				{
					if (memcmp(p, "FLASH1M_", 8) == 0)
					{
						res += (const char *)p;
						res += ' ';
					}
				}
				else if (memcmp(p, "FLASH", 5) == 0)
				{
					res += (const char *)p;
					res += ' ';
				}
				else if (d == 0x52494953)
				{
					if (memcmp(p, "SIIRTC_V", 8) == 0)
					{
						res += (const char *)p;
						res += ' ';
					}
				}
				p++;
			}
			if (res.GetLength() > 0)
				AppendFormat(report, "Cart Save    : %s\r\n", res);
		}
		else if (systemCartridgeType == IMAGE_GB)
		{
			strncpy(buffer, (const char *)&gbRom[0x134], 15);
			buffer[15] = 0;
			AppendFormat(report, "Game title   : %s\r\n", buffer);
		}
	}

	AppendFormat(report, "Using BIOS   : %d\r\n", useBios);
	AppendFormat(report, "Skip BIOS    : %d\r\n", theApp.skipBiosIntro);
	AppendFormat(report, "Disable SFX  : %d\r\n", cpuDisableSfx);
///  AppendFormat(report, "Skip intro   : %d\r\n", theApp.removeIntros);
	AppendFormat(report, "Throttle     : %d\r\n", theApp.throttle);
	AppendFormat(report, "Rewind       : %d\r\n", theApp.rewindTimer);
	AppendFormat(report, "Lua          : %d\r\n", VBALuaRunning());
///  AppendFormat(report, "Auto frame   : %d\r\n", theApp.autoFrameSkip);
	AppendFormat(report, "Video option : %d\r\n", theApp.videoOption);
	AppendFormat(report, "Render type  : %d\r\n", theApp.renderMethod);
	AppendFormat(report, "Color depth  : %d\r\n", systemColorDepth);
	AppendFormat(report, "Red shift    : %08x\r\n", systemRedShift);
	AppendFormat(report, "Green shift  : %08x\r\n", systemGreenShift);
	AppendFormat(report, "Blue shift   : %08x\r\n", systemBlueShift);
	AppendFormat(report, "Layer setting: %04X\r\n", layerSettings);
	AppendFormat(report, "Save type    : %d (%d)\r\n",
	             theApp.winSaveType, cpuSaveType);
	AppendFormat(report, "Flash size   : %08X (%08x)\r\n",
	             theApp.winFlashSize, flashSize);
	AppendFormat(report, "RTC          : %d (%d)\r\n", theApp.winRtcEnable,
	             rtcIsEnabled());
	AppendFormat(report, "AGBPrint     : %d\r\n", agbPrintIsEnabled());
	AppendFormat(report, "Turbo Mode   : %d\r\n", theApp.speedupToggle);
	AppendFormat(report, "Synchronize  : %d\r\n", synchronize);
	AppendFormat(report, "Sound OFF    : %d\r\n", soundOffFlag);
	AppendFormat(report, "Channels     : %04x\r\n", systemSoundGetEnabledChannels() & 0x30f);
	AppendFormat(report, "Old Sync     : %d\r\n", theApp.useOldSync);
	AppendFormat(report, "Priority     : %d\r\n", theApp.threadPriority);
	AppendFormat(report, "Filters      : %d (%d)\r\n", theApp.filterType, theApp.ifbType);
	AppendFormat(report, "Cheats       : %d\r\n", cheatsNumber);
	AppendFormat(report, "GB Cheats    : %d\r\n", gbCheatNumber);
	AppendFormat(report, "GB Emu Type  : %d\r\n", gbEmulatorType);

	return report;
}

