#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"
#include "Reg.h"
#include "WinResUtil.h"
#include "WinMiscUtil.h"
#include "Sound.h"
#include "VBA.h"
#include "Dialogs/ExportGSASnapshot.h"
#include "Dialogs/FileDlg.h"
#include "Dialogs/GSACodeSelect.h"
#include "Dialogs/RomInfo.h"
#include "Dialogs/LuaOpenDialog.h"
#include "Dialogs/ram_search.h"
#include "Dialogs/ramwatch.h"

#include "../NLS.h"
#include "../version.h"
#include "../gba/GBA.h"
#include "../gba/GBAGlobals.h"
#include "../gba/EEprom.h"
#include "../gba/GBASound.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
#include "../common/SystemGlobals.h"
#include "../common/movie.h"
#include "../common/vbalua.h"

#include "Input.h"

void MainWnd::OnFileOpen()
{
	theApp.winCheckFullscreen();
	if (winFileOpenSelect(0))
	{
		winFileRun();
	}
}

void MainWnd::OnFileOpenGBx()
{
	theApp.winCheckFullscreen();
	if (winFileOpenSelect(1))
	{
		winFileRun();
	}
}

void MainWnd::ResetController() {
    //theApp->input.context = zmq_ctx_new();
    //theApp.input.context = zmq_ctx_new();

    //theApp.initInput();
    theApp.input->endZMQ();
    theApp.input->startZMQ();

    //context = zmq_ctx_new();

    //requester = zmq_socket(context, ZMQ_REQ);
    //int timeout = 1;
    //zmq_setsockopt(requester, ZMQ_RCVTIMEO, &timeout, sizeof(timeout));
    ///zmq_connect(requester, "tcp://localhost:5555");
}

void MainWnd::OnFilePause()
{
	systemSetPause(!theApp.paused);
}

void MainWnd::OnUpdateFilePause(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.paused);
}

void MainWnd::OnFileReset()
{
	if (emulating)
	{
		if (VBAMovieIsPlaying() && VBAMovieIsXorInput() || VBAMovieIsRecording())
		{
			VBAMovieSignalReset();
		}
		else if (VBAMovieIsPlaying())
		{
			// HACK: backward-compatibility shortcut
			VBAMovieRestart();
		}
		else
		{
			theApp.emulator.emuReset();
			systemScreenMessage(winResLoadString(IDS_RESET));
		}
	}
}

void MainWnd::OnUpdateFileReset(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnFileRecentFreeze()
{
	theApp.recentFreeze = !theApp.recentFreeze;
}

void MainWnd::OnUpdateFileRecentFreeze(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.recentFreeze);
}

void MainWnd::OnFileRecentReset()
{
	theApp.winCheckFullscreen();
	systemSoundClearBuffer();
	if (MessageBox("Really clear your recent ROMs list?", //winResLoadString(IDS_REALLY_CLEAR),
	               winResLoadString(IDS_CONFIRM_ACTION),
	               MB_YESNO | MB_DEFBUTTON2) == IDNO)
		return;

	for (int i = 0; i < 10; ++i)
		theApp.recentFiles[i] = "";
}

BOOL MainWnd::OnFileRecentFile(UINT nID)
{
	if (theApp.recentFiles[(nID & 0xFFFF) - ID_FILE_MRU_FILE1].GetLength())
	{
		theApp.romFilename = theApp.recentFiles[(nID & 0xFFFF) - ID_FILE_MRU_FILE1];
		winFileRun();
	}
	return TRUE;
}

void MainWnd::OnUpdateFileRecentFile(CCmdUI *pCmdUI)
{
	int fileID = pCmdUI->m_nID - ID_FILE_MRU_FILE1;

	bool bExist = !theApp.recentFiles[fileID].IsEmpty();

	if (pCmdUI->m_pMenu != NULL)
	{
		CString p = theApp.recentFiles[fileID];

		int index = max(p.ReverseFind('/'), max(p.ReverseFind('\\'), p.ReverseFind('|')));

		if (index != -1)
		{
			p.Delete(0, index + 1);
		}

		p.Replace("&", "&&");

		CString number("1&0 - ");
		if (fileID < 9)
			number.Format("&%d - ", fileID + 1);

		if (p.IsEmpty())
		{
			p	   = "No Recent ROM";
			bExist = false;
		}

		pCmdUI->SetText(number + p);
		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}

	pCmdUI->Enable(bExist);
}

void MainWnd::OnFileExit()
{
	if (AskSave())
		SendMessage(WM_CLOSE);
}

void MainWnd::OnFileClose()
{
	// save battery file before we change the filename...
	CloseRamWindows();
	winFileClose();
}

void MainWnd::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnFileLoad()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { ".sgm", NULL };
	CString filter = winResLoadFilter(IDS_FILTER_SGM);
	CString title  = winResLoadString(IDS_SELECT_SAVE_GAME_NAME);

	CString saveName = winGetDestFilename(theApp.gameFilename, IDS_SAVE_DIR, exts[0]);
	CString saveDir	 = winGetDestDir(IDS_SAVE_DIR);

	FileDlg dlg(this, saveName, filter, 0, "SGM", exts, saveDir, title, false, true);

	if (dlg.DoModal() == IDOK)
	{
		CallRegisteredLuaFunctions(LUACALL_BEFORESTATELOAD);

		bool res = winReadSaveGame(dlg.GetPathName());

		// deleting rewinds because you loaded a save state is stupid
		///  theApp.rewindCount = 0;
		///  theApp.rewindCounter = 0;
		///  theApp.rewindSaveNeeded = false;

		if (res)
		{
			systemScreenMessage(winResLoadString(IDS_LOADED_STATE));

			theApp.frameSearching = false;
			theApp.frameSearchSkipping = false;

			CallRegisteredLuaFunctions(LUACALL_AFTERSTATELOAD);
		}
	}
}

void MainWnd::OnUpdateFileLoad(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating);
}

BOOL MainWnd::OnFileLoadSlot(UINT nID)
{
	nID = nID + 1 - ID_FILE_LOADGAME_SLOT1;

	CString filename = winGetSavestateFilename(theApp.gameFilename, nID);

	CallRegisteredLuaFunctions(LUACALL_BEFORESTATELOAD);

	bool res = winReadSaveGame(filename);

	// deleting rewinds because you loaded a save state is stupid
///  theApp.rewindCount = 0;
///  theApp.rewindCounter = 0;
///  theApp.rewindSaveNeeded = false;

	if (res)
	{
		CString format;
		if (VBAMovieIsActive())
		{
			if (!VBAMovieIsRecording())
			{
				format = winResLoadString(IDS_REPLAYED_STATE_N);
			}
			else
			{
				format = winResLoadString(IDS_RERECORDED_STATE_N);
			}
		}
		else
		{
			format = winResLoadString(IDS_LOADED_STATE_N);
		}

		CString buffer;
		buffer.Format(format, nID);
		systemScreenMessage(buffer);

		int lastSlot = theApp.currentSlot;

		if (theApp.loadMakesRecent)
		{
			// to update the file's modification date
			SYSTEMTIME st;
			FILETIME   ft;
			GetSystemTime(&st);
			SystemTimeToFileTime(&st, &ft);
			HANDLE fh = CreateFile(filename, 
									GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 
									NULL, 
									OPEN_EXISTING, 
									0, 
									NULL);
			if (fh != INVALID_HANDLE_VALUE)
				SetFileTime(fh, NULL, NULL, &ft);
			CloseHandle(fh);
		}

		if (theApp.loadMakesCurrent)
			theApp.currentSlot = nID - 1;
		else
			theApp.currentSlot = lastSlot;  // restore value in case the call to OnFileSaveSlot changed it

		theApp.frameSearching	   = false;
		theApp.frameSearchSkipping = false;

		CallRegisteredLuaFunctions(LUACALL_AFTERSTATELOAD);
	}

	return res;
}

void MainWnd::OnFileSave()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { ".sgm", NULL };
	CString filter = winResLoadFilter(IDS_FILTER_SGM);
	CString title  = winResLoadString(IDS_SELECT_SAVE_GAME_NAME);

	CString saveName = winGetDestFilename(theApp.gameFilename, IDS_SAVE_DIR, exts[0]);
	CString saveDir	 = winGetDestDir(IDS_SAVE_DIR);

	FileDlg dlg(this, saveName, filter, 0, "SGM", exts, saveDir, title, true);

	if (dlg.DoModal() == IDOK)
	{
		CallRegisteredLuaFunctions(LUACALL_BEFORESTATESAVE);

		bool res = winWriteSaveGame(dlg.GetPathName());
		if (res)
		{
			systemScreenMessage(winResLoadString(IDS_WROTE_STATE));

			CallRegisteredLuaFunctions(LUACALL_AFTERSTATESAVE);
		}
	}
}

void MainWnd::OnUpdateFileSave(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating);
}

BOOL MainWnd::OnFileSaveSlot(UINT nID)
{
	nID = nID + 1 - ID_FILE_SAVEGAME_SLOT1;

	if (theApp.saveMakesCurrent)
		theApp.currentSlot = nID - 1;

	CString filename = winGetSavestateFilename(theApp.gameFilename, nID);

	CallRegisteredLuaFunctions(LUACALL_BEFORESTATESAVE);

	bool res = winWriteSaveGame(filename);

	if (res)
	{
		CString format = winResLoadString(IDS_WROTE_STATE_N);
		CString buffer;
		buffer.Format(format, nID);

		systemScreenMessage(buffer);

		CallRegisteredLuaFunctions(LUACALL_AFTERSTATESAVE);
	}

	return res;
}

BOOL MainWnd::OnSelectSlot(UINT nID)
{
	nID -= ID_SELECT_SLOT1;
	theApp.currentSlot = nID;

	CString buffer;
	buffer.Format("Slot %d selected", nID + 1);
	systemScreenMessage(buffer, 0);

	return true;
}

void MainWnd::OnFileImportBatteryfile()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { ".sav", NULL };
	CString filter = winResLoadFilter(IDS_FILTER_SAV);
	CString title  = winResLoadString(IDS_SELECT_BATTERY_FILE);

	CString batteryName = winGetDestFilename(theApp.gameFilename, IDS_BATTERY_DIR, exts[0]);
	CString batteryDir	= winGetDestDir(IDS_BATTERY_DIR);

	FileDlg dlg(this, batteryName, filter, 0, "SAV", exts, batteryDir, title, false, true);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str1 = winResLoadString(IDS_SAVE_WILL_BE_LOST);
	CString str2 = winResLoadString(IDS_CONFIRM_ACTION);

	systemSoundClearBuffer();
	if (MessageBox(str1,
	               str2,
	               MB_OKCANCEL) == IDCANCEL)
		return;

	bool res = false;

	res = theApp.emulator.emuReadBattery(dlg.GetPathName());

	if (!res)
	{
		systemMessage(MSG_CANNOT_OPEN_FILE, "Cannot open file %s", dlg.GetPathName());
	}
	else if (VBAMovieIsRecording())
	{
		// FIXME: we just treat this as if using a cheat code for now
		VBAMovieSignalReset();
	}
	else
	{
		theApp.emulator.emuReset();
	}
}

void MainWnd::OnUpdateFileImportBatteryfile(CCmdUI *pCmdUI)
{
	// we allow this as we allow using cheats during recording
	pCmdUI->Enable(emulating /*&& !VBAMovieIsActive()*/);
}

void MainWnd::OnFileImportGamesharkcodefile()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { NULL };
	CString filter = systemCartridgeType == IMAGE_GBA ? winResLoadFilter(IDS_FILTER_SPC) : winResLoadFilter(IDS_FILTER_GCF);
	CString title  = winResLoadString(IDS_SELECT_CODE_FILE);

	FileDlg dlg(this, "", filter, 0, "", exts, "", title, false, true);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str1 = winResLoadString(IDS_CODES_WILL_BE_LOST);
	CString str2 = winResLoadString(IDS_CONFIRM_ACTION);

	systemSoundClearBuffer();
	if (MessageBox(str1,
	               str2,
	               MB_OKCANCEL) == IDCANCEL)
		return;

	CString file = dlg.GetPathName();
	bool	res	 = winImportGSACodeFile(file);
}

void MainWnd::OnUpdateFileImportGamesharkcodefile(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating /*&& !VBAMovieIsActive()*/);
}

void MainWnd::OnFileImportGamesharksnapshot()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { NULL };
	CString filter = systemCartridgeType == 1 ? winResLoadFilter(IDS_FILTER_GBS) : winResLoadFilter(IDS_FILTER_SPS);
	CString title  = winResLoadString(IDS_SELECT_SNAPSHOT_FILE);

	FileDlg dlg(this, "", filter, 0, "", exts, "", title, false, true);

	if (dlg.DoModal() == IDCANCEL)
		return;

	CString str1 = winResLoadString(IDS_SAVE_WILL_BE_LOST);
	CString str2 = winResLoadString(IDS_CONFIRM_ACTION);

	systemSoundClearBuffer();
	if (MessageBox(str1,
	               str2,
	               MB_OKCANCEL) == IDCANCEL)
		return;

	if (systemCartridgeType == 1)
		gbReadGSASnapshot(dlg.GetPathName());
	else
		CPUReadGSASnapshot(dlg.GetPathName());
}

void MainWnd::OnUpdateFileImportGamesharksnapshot(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating /*&& !VBAMovieIsActive()*/);
}

void MainWnd::OnFileExportBatteryfile()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { ".sav", ".dat", NULL };
	CString filter = winResLoadFilter(IDS_FILTER_SAV);
	CString title  = winResLoadString(IDS_SELECT_BATTERY_FILE);

	CString batteryName = winGetDestFilename(theApp.gameFilename, IDS_BATTERY_DIR, exts[0]);
	CString batteryDir	= winGetDestDir(IDS_BATTERY_DIR);

	FileDlg dlg(this, batteryName, filter, 1, "SAV", exts, batteryDir, title, true);

	if (dlg.DoModal() == IDCANCEL)
	{
		return;
	}

	bool result = false;

	if (systemCartridgeType == 1)
	{
		result = gbWriteBatteryFile(dlg.GetPathName(), false);
	}
	else
		result = theApp.emulator.emuWriteBattery(dlg.GetPathName());

	if (!result)
		systemMessage(MSG_ERROR_CREATING_FILE, "Error creating file %s",
		              dlg.GetPathName());
}

void MainWnd::OnUpdateFileExportBatteryfile(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnFileExportGamesharksnapshot()
{
	theApp.winCheckFullscreen();
	if (eepromInUse)
	{
		systemMessage(IDS_EEPROM_NOT_SUPPORTED, "EEPROM saves cannot be exported");
		return;
	}

	LPCTSTR exts[] = { ".sps", NULL };

	CString filter = winResLoadFilter(IDS_FILTER_SPS);
	CString title  = winResLoadString(IDS_SELECT_SNAPSHOT_FILE);

	CString name = winGetDestFilename(theApp.gameFilename, CString(), exts[0]);

	FileDlg dlg(this, name, filter, 1, "SPS", exts, "", title, true);

	if (dlg.DoModal() == IDCANCEL)
		return;

	char buffer[16];
	strncpy(buffer, (const char *)&rom[0xa0], 12);
	buffer[12] = 0;

	ExportGSASnapshot dlg2(dlg.GetPathName(), buffer, this);
	dlg2.DoModal();
}

void MainWnd::OnUpdateFileExportGamesharksnapshot(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating && systemCartridgeType == IMAGE_GBA);
}

void MainWnd::OnFileQuickScreencapture()
{
	extern int32 captureNumber;   // GBAGlobals.cpp
	captureNumber = winScreenCapture(captureNumber);
}

void MainWnd::OnFileScreencapture()
{
	theApp.winCheckFullscreen();

	LPCTSTR exts[] = { ".png", ".bmp", NULL };

	CString filter = winResLoadFilter(IDS_FILTER_PNG);
	CString title  = winResLoadString(IDS_SELECT_CAPTURE_NAME);

	CString ext;

	if (theApp.captureFormat != 0)
		ext.Format(".bmp");
	else
		ext.Format(".png");

	CString captureName = winGetDestFilename(theApp.gameFilename, IDS_CAPTURE_DIR, ext);
	CString captureDir	= winGetDestDir(IDS_CAPTURE_DIR);

	FileDlg dlg(this,
	            captureName,
	            filter,
	            theApp.captureFormat ? 2 : 1,
	            theApp.captureFormat ? "BMP" : "PNG",
	            exts,
	            captureDir,
	            title,
	            true);

	if (dlg.DoModal() == IDCANCEL)
		return;

	if (dlg.getFilterIndex() == 2)
		theApp.emulator.emuWriteBMP(dlg.GetPathName());
	else
		theApp.emulator.emuWritePNG(dlg.GetPathName());

	systemScreenMessage(winResLoadString(IDS_SCREEN_CAPTURE));
}

void MainWnd::OnUpdateFileScreencapture(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnFileRominformation()
{
	theApp.winCheckFullscreen();
	if (systemCartridgeType == IMAGE_GBA)
	{
		RomInfoGBA dlg(rom);
		dlg.DoModal();
	}
	else
	{
		RomInfoGB dlg(gbRom);
		dlg.DoModal();
	}
}

void MainWnd::OnUpdateFileRominformation(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(emulating);
}

void MainWnd::OnFileTogglemenu()
{
	theApp.menuToggle = !theApp.menuToggle;

	if (theApp.menuToggle)
	{
		SetMenu(&theApp.m_menu);
		if (theApp.videoOption > VIDEO_4X && theApp.tripleBuffering)
		{
			if (theApp.display)
				theApp.display->renderMenu();
		}
	}
	else
	{
		SetMenu(NULL);
	}

	theApp.adjustDestRect();
}

void MainWnd::OnUpdateFileTogglemenu(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.menuToggle);
}

void MainWnd::OnFileSavegameOldestslot()
{
	if (!emulating)
		return;

	CFileStatus status;
	CString		str;
	time_t		time  = -1;
	int			found = -1;

	for (int i = 0; i < 10; i++)
	{
		if (CFile::GetStatus(winGetSavestateFilename(theApp.gameFilename, i + 1), status))
		{
			if (time - status.m_mtime.GetTime() > 0 || time == -1)
			{
				time  = (time_t)status.m_mtime.GetTime();
				found = i;
			}
		}
		else
		{
			found = i;
			break;
		}
	}

	OnFileSaveSlot(ID_FILE_SAVEGAME_SLOT1 + found);
}

void MainWnd::OnUpdateFileSavegameOldestslot(CCmdUI *pCmdUI)
{
	bool enabled = emulating;
	if (pCmdUI->m_pMenu != NULL)
	{
		CFileStatus status;
		time_t		time  = -1;
		int			found = -1;

		if (emulating)
		{
			for (int i = 0; i < 10; i++)
			{
				if (CFile::GetStatus(winGetSavestateFilename(theApp.gameFilename, i + 1), status))
				{
					if (time - status.m_mtime.GetTime() > 0 || time == -1)
					{
						time  = (time_t)status.m_mtime.GetTime();
						found = i;
					}
				}
				else
				{
					found = i;
					break;
				}
			}
		}

		CString str;
		enabled = (found != -1);
		if (enabled)
			str.Format("&Oldest Slot (#%d)", found + 1);
		else
			str.Format("&Oldest Slot", found + 1);

		pCmdUI->SetText(str);

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}

	pCmdUI->Enable(enabled);
}

void MainWnd::OnFileLoadgameMostrecent()
{
	if (!emulating)
		return;

	CFileStatus status;
	CString		str;
	time_t		time  = 0;
	int			found = -1;

	for (int i = 0; i < 10; i++)
	{
		if (CFile::GetStatus(winGetSavestateFilename(theApp.gameFilename, i + 1), status))
		{
			if (status.m_mtime.GetTime() > time)
			{
				time  = (time_t)status.m_mtime.GetTime();
				found = i;
			}
		}
	}

	if (found != -1)
	{
		OnFileLoadSlot(ID_FILE_LOADGAME_SLOT1 + found);
	}
}

void MainWnd::OnUpdateFileLoadgameMostrecent(CCmdUI *pCmdUI)
{
	bool enabled = emulating;
	if (pCmdUI->m_pMenu != NULL)
	{
		CFileStatus status;
		int			found = -1;

		time_t time = 0;
		if (emulating)
		{
			for (int i = 0; i < 10; i++)
			{
				if (CFile::GetStatus(winGetSavestateFilename(theApp.gameFilename, i + 1), status))
				{
					if (status.m_mtime.GetTime() > time)
					{
						time  = (time_t)status.m_mtime.GetTime();
						found = i;
					}
				}
			}
		}

		CString str;
		enabled = (found != -1);
		if (enabled)
			str.Format("Most &Recent Slot (#%d)", found + 1);
		else
			str.Format("Most &Recent Slot", found + 1);

		pCmdUI->SetText(str);

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}

	pCmdUI->Enable(enabled);
}

void MainWnd::OnUpdateFileLoadSlot(CCmdUI *pCmdUI)
{
	int slotID = pCmdUI->m_nID - ID_FILE_LOADGAME_SLOT1 + 1;

	if (pCmdUI->m_pMenu != NULL)
	{
		pCmdUI->SetText(winGetSavestateMenuString(theApp.gameFilename, slotID));

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}

	pCmdUI->Enable(emulating && winFileExists(winGetSavestateFilename(theApp.gameFilename, slotID)));
}

void MainWnd::OnUpdateFileSaveSlot(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_pMenu != NULL)
	{
		int slotID = pCmdUI->m_nID - ID_FILE_SAVEGAME_SLOT1 + 1;

		pCmdUI->SetText(winGetSavestateMenuString(theApp.gameFilename, slotID));

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}

	pCmdUI->Enable(emulating);
}

void MainWnd::OnUpdateSelectSlot(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_pMenu != NULL)
	{
		int slot = pCmdUI->m_nID - ID_SELECT_SLOT1;

		pCmdUI->SetText(winGetSavestateMenuString(theApp.gameFilename, slot + 1));

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());

		pCmdUI->SetCheck(slot == theApp.currentSlot);
	}
}

void MainWnd::OnFileLoadgameAutoloadmostrecent()
{
	theApp.autoLoadMostRecent = !theApp.autoLoadMostRecent;
}

void MainWnd::OnUpdateFileLoadgameAutoloadmostrecent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.autoLoadMostRecent);
}

void MainWnd::OnFileLoadgameMakeRecent()
{
	theApp.loadMakesRecent = !theApp.loadMakesRecent;
}

void MainWnd::OnUpdateFileLoadgameMakeRecent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.loadMakesRecent);
}

void MainWnd::OnFileSavegameCurrent()
{
	OnFileSaveSlot(ID_FILE_SAVEGAME_SLOT1 + theApp.currentSlot);
}

void MainWnd::OnUpdateFileSavegameCurrent(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_pMenu != NULL)
	{
		int slotID = theApp.currentSlot + 1;

		CString str;
		str.Format("&Current Slot (#%d)", slotID);

		pCmdUI->SetText(str);

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}

	pCmdUI->Enable(emulating);
}

void MainWnd::OnFileLoadgameCurrent()
{
	OnFileLoadSlot(ID_FILE_LOADGAME_SLOT1 + theApp.currentSlot);
}

void MainWnd::OnUpdateFileLoadgameCurrent(CCmdUI *pCmdUI)
{
	int slotID = theApp.currentSlot + 1;

	if (pCmdUI->m_pMenu != NULL)
	{
		CString str;
		str.Format("&Current Slot (#%d)", slotID);

		pCmdUI->SetText(str);

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}

	CFileStatus status;
	pCmdUI->Enable(emulating && CFile::GetStatus(winGetSavestateFilename(theApp.gameFilename, slotID), status));
}

void MainWnd::OnFileLoadgameMakeCurrent()
{
	theApp.loadMakesCurrent = !theApp.loadMakesCurrent;
}

void MainWnd::OnUpdateFileLoadgameMakeCurrent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.loadMakesCurrent);
}

void MainWnd::OnFileSavegameMakeCurrent()
{
	theApp.saveMakesCurrent = !theApp.saveMakesCurrent;
}

void MainWnd::OnUpdateFileSavegameMakeCurrent(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.saveMakesCurrent);
}

void MainWnd::OnFileSavegameIncrementSlot()
{
	theApp.currentSlot = (theApp.currentSlot + 1) % 10;

	char str [32];
	sprintf(str, "Current Slot: %d", theApp.currentSlot + 1);
	systemScreenMessage(str, 0);
}

void MainWnd::OnUpdateFileSavegameIncrementSlot(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_pMenu != NULL)
	{
		int slotID = theApp.currentSlot + 1;

		CString str;
		str.Format("&Increase Current Slot (#%d -> #%d)", slotID, slotID % 10 + 1);

		pCmdUI->SetText(str);

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}
}

void MainWnd::OnFileSavegameDecrementSlot()
{
	theApp.currentSlot = (theApp.currentSlot + 9) % 10;

	char str [32];
	sprintf(str, "Current Slot: %d", theApp.currentSlot + 1);
	systemScreenMessage(str, 0);
}

void MainWnd::OnUpdateFileSavegameDecrementSlot(CCmdUI *pCmdUI)
{
	if (pCmdUI->m_pMenu != NULL)
	{
		int slotID = theApp.currentSlot + 1;

		CString str;
		str.Format("&Decrease Current Slot (#%d -> #%d)", slotID, (slotID + 8) % 10 + 1);

		pCmdUI->SetText(str);

		theApp.winAccelMgr.UpdateMenu(pCmdUI->m_pMenu->GetSafeHmenu());
	}
}

void MainWnd::OnFileSlotDisplayModificationTime()
{
	theApp.showSlotTime = !theApp.showSlotTime;
}

void MainWnd::OnUpdateFileSlotDisplayModificationTime(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.showSlotTime);
}

void MainWnd::OnFileLuaOpen()
{
	theApp.winCheckFullscreen();

	if (!LuaConsoleHWnd)
	{
		LuaConsoleHWnd = ::CreateDialog(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_LUA), AfxGetMainWnd()->GetSafeHwnd(), (DLGPROC)DlgLuaScriptDialog);
	}
	else
	{
		if (::IsIconic(LuaConsoleHWnd))
			::ShowWindow(LuaConsoleHWnd, SW_RESTORE);
		::SetForegroundWindow(LuaConsoleHWnd);
	}
}

void MainWnd::OnUpdateFileLuaOpen(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(LuaConsoleHWnd != NULL);
	pCmdUI->Enable(true);
}

void MainWnd::OnFileLuaCloseAll()
{
	if (LuaConsoleHWnd)
		::PostMessage(LuaConsoleHWnd, WM_CLOSE, 0, 0);
}

void MainWnd::OnFileLuaReload()
{
	VBAReloadLuaCode();
}

void MainWnd::OnFileLuaStop()
{
	VBALuaStop();
}

void MainWnd::OnFileRamSearch()
{
	theApp.winCheckFullscreen();

	if (!RamSearchHWnd)
	{
		reset_address_info();
		LRESULT CALLBACK RamSearchProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
		::CreateDialog(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_RAMSEARCH), AfxGetMainWnd()->GetSafeHwnd(), (DLGPROC) RamSearchProc);
	}
	else
	{
		if (::IsIconic(RamSearchHWnd))
			::ShowWindow(RamSearchHWnd, SW_RESTORE);
		::SetForegroundWindow(RamSearchHWnd);
	}
}

void MainWnd::OnUpdateFileRamSearch(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void MainWnd::OnFileRamWatch()
{
	theApp.winCheckFullscreen();

	if (!RamWatchHWnd)
	{
		LRESULT CALLBACK RamWatchProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
		RamWatchHWnd = ::CreateDialog(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDD_RAMWATCH), AfxGetMainWnd()->GetSafeHwnd(), (DLGPROC) RamWatchProc);
	}
	else
	{
		if (::IsIconic(RamWatchHWnd))
			::ShowWindow(RamWatchHWnd, SW_RESTORE);
		::SetForegroundWindow(RamWatchHWnd);
	}
}

void MainWnd::OnUpdateFileRamWatch(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void MainWnd::OnUpdateFileLuaCloseAll(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(LuaConsoleHWnd != NULL);
}

