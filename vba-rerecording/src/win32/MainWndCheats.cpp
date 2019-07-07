#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "WinMiscUtil.h"
#include "VBA.h"
#include "Dialogs/FileDlg.h"
#include "Dialogs/GBACheatsDlg.h"
#include "Dialogs/GBCheatsDlg.h"

#include "../gba/GBA.h"
#include "../gba/GBAGlobals.h"
#include "../common/SystemGlobals.h"
#include "../gb/gbCheats.h"

GBACheatSearch gbaDlg;
GBCheatSearch  gbDlg;

void MainWnd::OnCheatsSearchforcheats()
{
	theApp.winCheckFullscreen();

	if (theApp.modelessCheatDialogIsOpen)
	{
		gbaDlg.DestroyWindow();
		gbDlg.DestroyWindow();
		theApp.modelessCheatDialogIsOpen = false;
	}

	if (systemCartridgeType == IMAGE_GBA)
	{
		if (theApp.pauseDuringCheatSearch)
		{
			gbaDlg.DoModal();
		}
		else
		{
			if (!theApp.modelessCheatDialogIsOpen)
			{
				theApp.modelessCheatDialogIsOpen = true;
				gbaDlg.Create(GBACheatSearch::IDD, theApp.m_pMainWnd);
			}
		}
	}
	else
	{
		if (theApp.pauseDuringCheatSearch)
		{
			gbDlg.DoModal();
		}
		else
		{
			if (!theApp.modelessCheatDialogIsOpen)
			{
				theApp.modelessCheatDialogIsOpen = true;
				gbDlg.Create(GBCheatSearch::IDD, theApp.m_pMainWnd);
			}
		}
	}
}

void MainWnd::OnUpdateCheatsSearchforcheats(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnCheatsCheatlist()
{
	theApp.winCheckFullscreen();
	if (systemCartridgeType == IMAGE_GBA)
	{
		GBACheatList dlg;
		dlg.DoModal();
	}
	else
	{
		GBCheatList dlg;
		dlg.DoModal();
	}
}

void MainWnd::OnUpdateCheatsCheatlist(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnCheatsAutomaticsaveloadcheats()
{
	theApp.autoSaveLoadCheatList = !theApp.autoSaveLoadCheatList;
}

void MainWnd::OnUpdateCheatsAutomaticsaveloadcheats(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.autoSaveLoadCheatList);
}

void MainWnd::OnCheatsPauseDuringCheatSearch()
{
	theApp.pauseDuringCheatSearch = !theApp.pauseDuringCheatSearch;
}

void MainWnd::OnUpdateCheatsPauseDuringCheatSearch(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.pauseDuringCheatSearch);
}

void MainWnd::OnCheatsLoadcheatlist()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { ".clt", NULL };
	CString filter = winResLoadFilter(IDS_FILTER_CHEAT_LIST);
	CString title  = winResLoadString(IDS_SELECT_CHEAT_LIST_NAME);

	CString cheatName = winGetDestFilename(theApp.gameFilename, IDS_CHEAT_DIR, exts[0]);
	CString cheatDir = winGetDestDir(IDS_CHEAT_DIR);

	FileDlg dlg(this, cheatName, filter, 0, "CLT", exts, cheatDir, title, false);

	if (dlg.DoModal() == IDOK)
	{
		winLoadCheatList(dlg.GetPathName());
	}
}

void MainWnd::OnUpdateCheatsLoadcheatlist(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnCheatsSavecheatlist()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { ".clt", NULL };
	CString filter = winResLoadFilter(IDS_FILTER_CHEAT_LIST);
	CString title  = winResLoadString(IDS_SELECT_CHEAT_LIST_NAME);

	CString cheatName = winGetDestFilename(theApp.gameFilename, IDS_CHEAT_DIR, exts[0]);
	CString cheatDir = winGetDestDir(IDS_CHEAT_DIR);

	FileDlg dlg(this, cheatName, filter, 0, "CLT", exts, cheatDir, title, true);

	if (dlg.DoModal() == IDOK)
	{
		winSaveCheatList(dlg.GetPathName());
	}
}

void MainWnd::OnUpdateCheatsSavecheatlist(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnCheatsDisablecheats()
{
	cheatsEnabled = !cheatsEnabled;
}

void MainWnd::OnUpdateCheatsDisablecheats(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(!cheatsEnabled);
}

