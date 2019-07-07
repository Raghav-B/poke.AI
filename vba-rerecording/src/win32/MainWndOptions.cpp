#include "stdafx.h"
#include "resource.h"
#include "MainWnd.h"
#include "WinResUtil.h"
#include "Reg.h"
#include "VBA.h"
#include "Dialogs/Associate.h"
#include "Dialogs/Directories.h"
#include "Dialogs/FileDlg.h"
#include "Dialogs/GBColorDlg.h"
#include "Dialogs/Joypad.h"
#include "Dialogs/MaxScale.h"
#include "Dialogs/ModeConfirm.h"
#include "Dialogs/RewindInterval.h"
#include "Dialogs/Throttle.h"
#include "Dialogs/TextOptions.h"

#include "../gba/GBA.h"
#include "../gba/GBAGlobals.h"
#include "../gba/Flash.h"
#include "../gba/agbprint.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
#include "../gb/gbPrinter.h"
#include "../common/System.h"
#include "../common/SystemGlobals.h"
#include "../common/inputGlobal.h"
#include "../common/movie.h"
#include "../version.h"

#define VBA_CONFIRM_MODE WM_APP + 100

void MainWnd::OnOptionsFrameskipThrottleNothrottle()
{
	systemSetThrottle(0);
}

void MainWnd::OnUpdateOptionsFrameskipThrottleNothrottle(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 0);
}

void MainWnd::OnOptionsFrameskipThrottle6()
{
	systemSetThrottle(6);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle6(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 6);
}

void MainWnd::OnOptionsFrameskipThrottle15()
{
	systemSetThrottle(15);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle15(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 15);
}

void MainWnd::OnOptionsFrameskipThrottle25()
{
	systemSetThrottle(25);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle25(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 25);
}

void MainWnd::OnOptionsFrameskipThrottle50()
{
	systemSetThrottle(50);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle50(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 50);
}

void MainWnd::OnOptionsFrameskipThrottle75()
{
	systemSetThrottle(75);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle75(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 75);
}

void MainWnd::OnOptionsFrameskipThrottle100()
{
	systemSetThrottle(100);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle100(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 100);
}

void MainWnd::OnOptionsFrameskipThrottle125()
{
	systemSetThrottle(125);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle125(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 125);
}

void MainWnd::OnOptionsFrameskipThrottle150()
{
	systemSetThrottle(150);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle150(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 150);
}

void MainWnd::OnOptionsFrameskipThrottle200()
{
	systemSetThrottle(200);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle200(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 200);
}

void MainWnd::OnOptionsFrameskipThrottle300()
{
	systemSetThrottle(300);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle300(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 300);
}

void MainWnd::OnOptionsFrameskipThrottle400()
{
	systemSetThrottle(400);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle400(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 400);
}

void MainWnd::OnOptionsFrameskipThrottle600()
{
	systemSetThrottle(600);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle600(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 600);
}

void MainWnd::OnOptionsFrameskipThrottle1000()
{
	systemSetThrottle(1000);
}

void MainWnd::OnUpdateOptionsFrameskipThrottle1000(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.throttle == 1000);
}

void MainWnd::OnOptionsFrameskipThrottleOther()
{
	Throttle dlg;
	int      v = dlg.DoModal();
	if (v)
		systemSetThrottle(v);
}

void MainWnd::OnUpdateOptionsFrameskipThrottleOther(CCmdUI*pCmdUI)
{
}

void MainWnd::OnOptionsFrameskipThrottleIncrease()
{
	systemIncreaseThrottle();
}

void MainWnd::OnUpdateOptionsFrameskipThrottleIncrease(CCmdUI*pCmdUI)
{
}

void MainWnd::OnOptionsFrameskipThrottleDecrease()
{
	systemDecreaseThrottle();
}

void MainWnd::OnUpdateOptionsFrameskipThrottleDecrease(CCmdUI*pCmdUI)
{
}

/*
   void MainWnd::OnOptionsFrameskipAutomatic()
   {
   theApp.autoFrameSkip = !theApp.autoFrameSkip;
   if(!theApp.autoFrameSkip && emulating)
    theApp.updateFrameSkip();
   }

   void MainWnd::OnUpdateOptionsFrameskipAutomatic(CCmdUI* pCmdUI)
   {
   pCmdUI->SetCheck(theApp.autoFrameSkip);
   }
 */

void MainWnd::OnOptionsFrameskipAccuratePitch()
{
	theApp.accuratePitchThrottle = true;
}

void MainWnd::OnUpdateOptionsFrameskipAccuratePitch(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.accuratePitchThrottle);
	pCmdUI->Enable(!soundOffFlag && synchronize);
}

void MainWnd::OnOptionsFrameskipAccurateSpeed()
{
	theApp.accuratePitchThrottle = false;
}

void MainWnd::OnUpdateOptionsFrameskipAccurateSpeed(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(!theApp.accuratePitchThrottle);
	pCmdUI->Enable(!soundOffFlag && synchronize);
}

BOOL MainWnd::OnOptionsFrameskip(UINT nID)
{
	switch (nID)
	{
	case ID_OPTIONS_VIDEO_FRAMESKIP_0:
	case ID_OPTIONS_VIDEO_FRAMESKIP_1:
	case ID_OPTIONS_VIDEO_FRAMESKIP_2:
	case ID_OPTIONS_VIDEO_FRAMESKIP_3:
	case ID_OPTIONS_VIDEO_FRAMESKIP_4:
	case ID_OPTIONS_VIDEO_FRAMESKIP_5:
	case ID_OPTIONS_VIDEO_FRAMESKIP_6:
	case ID_OPTIONS_VIDEO_FRAMESKIP_7:
	case ID_OPTIONS_VIDEO_FRAMESKIP_8:
	case ID_OPTIONS_VIDEO_FRAMESKIP_9:
		if (systemCartridgeType == IMAGE_GBA)
		{
			frameSkip = nID - ID_OPTIONS_VIDEO_FRAMESKIP_0;
		}
		else
		{
			gbFrameSkip = nID - ID_OPTIONS_VIDEO_FRAMESKIP_0;
		}
		if (emulating)
			theApp.updateFrameSkip();
		return TRUE;
		break;
	}
	return FALSE;
}

void MainWnd::OnUpdateOptionsVideoFrameskip0(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 0 : gbFrameSkip == 0);
}

void MainWnd::OnUpdateOptionsVideoFrameskip1(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 1 : gbFrameSkip == 1);
}

void MainWnd::OnUpdateOptionsVideoFrameskip2(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 2 : gbFrameSkip == 2);
}

void MainWnd::OnUpdateOptionsVideoFrameskip3(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 3 : gbFrameSkip == 3);
}

void MainWnd::OnUpdateOptionsVideoFrameskip4(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 4 : gbFrameSkip == 4);
}

void MainWnd::OnUpdateOptionsVideoFrameskip5(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 5 : gbFrameSkip == 5);
}

void MainWnd::OnUpdateOptionsVideoFrameskip6(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 6 : gbFrameSkip == 6);
}

void MainWnd::OnUpdateOptionsVideoFrameskip7(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 7 : gbFrameSkip == 7);
}

void MainWnd::OnUpdateOptionsVideoFrameskip8(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 8 : gbFrameSkip == 8);
}

void MainWnd::OnUpdateOptionsVideoFrameskip9(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemCartridgeType == IMAGE_GBA ? frameSkip == 9 : gbFrameSkip == 9);
}

void MainWnd::OnOptionsVideoVsync()
{
	theApp.vsync = !theApp.vsync;
}

void MainWnd::OnUpdateOptionsVideoVsync(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.vsync);
}

void MainWnd::OnUpdateOptionsVideoX1(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_1X);
}

void MainWnd::OnUpdateOptionsVideoX2(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_2X);
}

void MainWnd::OnUpdateOptionsVideoX3(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_3X);
}

void MainWnd::OnUpdateOptionsVideoX4(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_4X);
}

void MainWnd::OnUpdateOptionsVideoFullscreen320x240(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.mode320Available);
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_320x240);
}

void MainWnd::OnUpdateOptionsVideoFullscreen640x480(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.mode640Available);
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_640x480);
}

void MainWnd::OnUpdateOptionsVideoFullscreen800x600(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.mode800Available);
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_800x600);
}

BOOL MainWnd::OnOptionVideoSize(UINT nID)
{
	theApp.updateVideoSize(nID);
	theApp.m_pMainWnd->PostMessage(VBA_CONFIRM_MODE);
	return TRUE;
}

void MainWnd::OnOptionsVideoFullscreen320x240()
{
	OnOptionVideoSize(ID_OPTIONS_VIDEO_FULLSCREEN320X240);
}

void MainWnd::OnOptionsVideoFullscreen640x480()
{
	OnOptionVideoSize(ID_OPTIONS_VIDEO_FULLSCREEN640X480);
}

void MainWnd::OnOptionsVideoFullscreen800x600()
{
	OnOptionVideoSize(ID_OPTIONS_VIDEO_FULLSCREEN800X600);
}

void MainWnd::OnOptionsVideoFullscreen()
{
	theApp.winCheckFullscreen();
	GUID *pGUID = NULL;
	int   size  = theApp.display->selectFullScreenMode(&pGUID);
	if (size != -1)
	{
		int width      = (size >> 12) & 4095;
		int height     = (size & 4095);
		int colorDepth = (size >> 24);
		if (width != theApp.fsWidth ||
		    height != theApp.fsHeight ||
		    colorDepth != theApp.fsColorDepth ||
		    pGUID != theApp.pVideoDriverGUID ||
		    theApp.videoOption != VIDEO_OTHER)
		{
			theApp.fsForceChange    = true;
			theApp.fsWidth          = width;
			theApp.fsHeight         = height;
			theApp.fsColorDepth     = colorDepth;
			theApp.pVideoDriverGUID = pGUID;
			if (pGUID)
			{
				theApp.videoDriverGUID = *pGUID;
				regSetDwordValue("defaultVideoDriver", FALSE);
				regSetBinaryValue("videoDriverGUID",
				                  (char *)pGUID, sizeof(GUID));
			}
			else
			{
				regSetDwordValue("defaultVideoDriver", TRUE);
			}
			theApp.updateVideoSize(ID_OPTIONS_VIDEO_FULLSCREEN);
			theApp.m_pMainWnd->PostMessage(VBA_CONFIRM_MODE);
		}
	}
}

void MainWnd::OnUpdateOptionsVideoFullscreen(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.videoOption == VIDEO_OTHER);
}

void MainWnd::OnOptionsVideoDisablesfx()
{
	cpuDisableSfx = !cpuDisableSfx;
	if (emulating && systemCartridgeType == IMAGE_GBA)
		CPUUpdateRender();
}

void MainWnd::OnUpdateOptionsVideoDisablesfx(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(cpuDisableSfx);
}

void MainWnd::OnOptionsVideoFullscreenstretchtofit()
{
	theApp.fullScreenStretch = !theApp.fullScreenStretch;
	theApp.updateWindowSize(theApp.videoOption);
	if (theApp.display)
		theApp.display->clear();
}

void MainWnd::OnUpdateOptionsVideoFullscreenstretchtofit(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.fullScreenStretch);
}

BOOL MainWnd::OnVideoLayer(UINT nID)
{
	layerSettings ^= 0x0100 << ((nID & 0xFFFF) - ID_OPTIONS_VIDEO_LAYERS_BG0);
	extern int32 layerEnable;
	layerEnable = DISPCNT & layerSettings;
	CPUUpdateRenderBuffers(false);
	return TRUE;
}

void MainWnd::OnUpdateVideoLayer(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck((layerSettings >> (8 + pCmdUI->m_nID - ID_OPTIONS_VIDEO_LAYERS_BG0)) & 1);
	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_VIDEO_LAYERS_BG1:
	case ID_OPTIONS_VIDEO_LAYERS_BG2:
	case ID_OPTIONS_VIDEO_LAYERS_BG3:
	case ID_OPTIONS_VIDEO_LAYERS_WIN1:
	case ID_OPTIONS_VIDEO_LAYERS_OBJWIN:
		pCmdUI->Enable(systemCartridgeType == IMAGE_GBA);
		break;
	}
}

void MainWnd::OnOptionsVideoRendermethodGdi()
{
	theApp.renderMethod = GDI;
	theApp.updateRenderMethod(false);
}

void MainWnd::OnUpdateOptionsVideoRendermethodGdi(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.renderMethod == GDI);
}

void MainWnd::OnOptionsVideoRendermethodDirectdraw()
{
	theApp.renderMethod = DIRECT_DRAW;
	theApp.updateRenderMethod(false);
}

void MainWnd::OnUpdateOptionsVideoRendermethodDirectdraw(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.renderMethod == DIRECT_DRAW);
}

void MainWnd::OnOptionsVideoRendermethodDirect3d()
{
	theApp.renderMethod = DIRECT_3D;
	theApp.updateRenderMethod(false);
}

void MainWnd::OnUpdateOptionsVideoRendermethodDirect3d(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.renderMethod == DIRECT_3D);
}

void MainWnd::OnOptionsVideoRendermethodOpengl()
{
	theApp.renderMethod = OPENGL;
	theApp.updateRenderMethod(false);
}

void MainWnd::OnUpdateOptionsVideoRendermethodOpengl(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.renderMethod == OPENGL);
}

void MainWnd::OnOptionsVideoTriplebuffering()
{
	theApp.tripleBuffering = !theApp.tripleBuffering;
}

void MainWnd::OnUpdateOptionsVideoTriplebuffering(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.tripleBuffering);
}

void MainWnd::OnOptionsVideoDdrawemulationonly()
{
	theApp.ddrawEmulationOnly = !theApp.ddrawEmulationOnly;
}

void MainWnd::OnUpdateOptionsVideoDdrawemulationonly(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.ddrawEmulationOnly);
}

void MainWnd::OnOptionsVideoDdrawusevideomemory()
{
	theApp.ddrawUseVideoMemory = !theApp.ddrawUseVideoMemory;
}

void MainWnd::OnUpdateOptionsVideoDdrawusevideomemory(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.ddrawUseVideoMemory);
}

void MainWnd::OnOptionsVideoRenderoptionsD3dnofilter()
{
	theApp.d3dFilter = 0;
	if (theApp.display)
		theApp.display->setOption("d3dFilter", 0);
}

void MainWnd::OnUpdateOptionsVideoRenderoptionsD3dnofilter(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.d3dFilter == 0);
}

void MainWnd::OnOptionsVideoRenderoptionsD3dbilinear()
{
	theApp.d3dFilter = 1;
	if (theApp.display)
		theApp.display->setOption("d3dFilter", 1);
}

void MainWnd::OnUpdateOptionsVideoRenderoptionsD3dbilinear(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.d3dFilter == 1);
}

void MainWnd::OnOptionsVideoRenderoptionsGlnearest()
{
	theApp.glFilter = 0;
	if (theApp.display)
		theApp.display->setOption("glFilter", 0);
}

void MainWnd::OnUpdateOptionsVideoRenderoptionsGlnearest(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.glFilter == 0);
}

void MainWnd::OnOptionsVideoRenderoptionsGlbilinear()
{
	theApp.glFilter = 1;
	if (theApp.display)
		theApp.display->setOption("glFilter", 1);
}

void MainWnd::OnUpdateOptionsVideoRenderoptionsGlbilinear(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.glFilter == 1);
}

void MainWnd::OnOptionsVideoRenderoptionsGltriangle()
{
	theApp.glType = 0;
	if (theApp.display)
		theApp.display->setOption("glType", 0);
}

void MainWnd::OnUpdateOptionsVideoRenderoptionsGltriangle(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.glType == 0);
}

void MainWnd::OnOptionsVideoRenderoptionsGlquads()
{
	theApp.glType = 1;
	if (theApp.display)
		theApp.display->setOption("glType", 1);
}

void MainWnd::OnUpdateOptionsVideoRenderoptionsGlquads(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.glType == 1);
}

void MainWnd::OnOptionsVideoRenderoptionsSelectskin()
{}

void MainWnd::OnUpdateOptionsVideoRenderoptionsSelectskin(CCmdUI*pCmdUI)
{}

void MainWnd::OnOptionsVideoRenderoptionsSkin()
{}

void MainWnd::OnUpdateOptionsVideoRenderoptionsSkin(CCmdUI*pCmdUI)
{}

void MainWnd::OnOptionsEmulatorAssociate()
{
	theApp.winCheckFullscreen();
	Associate dlg;
	dlg.DoModal();
}

void MainWnd::OnOptionsEmulatorDirectories()
{
	theApp.winCheckFullscreen();
	Directories dlg;
	dlg.DoModal();
}

void MainWnd::OnOptionsEmulatorFilenamePreference(UINT nID)
{
	theApp.filenamePreference = nID - ID_OPTIONS_PREFER_ARCHIVE_NAME;
}

void MainWnd::OnUpdateOptionsEmulatorFilenamePreference(CCmdUI *pCmdUI)
{
	pCmdUI->SetRadio(pCmdUI->m_nID == theApp.filenamePreference + ID_OPTIONS_PREFER_ARCHIVE_NAME);
}

void MainWnd::OnOptionsVideoDisablestatusmessages()
{
	theApp.disableStatusMessage = !theApp.disableStatusMessage;
}

void MainWnd::OnUpdateOptionsVideoDisablestatusmessages(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.disableStatusMessage);
}

void MainWnd::OnOptionsEmulatorSynchronize()
{
	synchronize = !synchronize;
}

void MainWnd::OnUpdateOptionsEmulatorSynchronize(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(synchronize);
}

void MainWnd::OnOptionsEmulatorAlwaysOnTop()
{
	theApp.alwaysOnTop = !theApp.alwaysOnTop;
	SetWindowPos((theApp.alwaysOnTop ? &wndTopMost : &wndNoTopMost), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
}

void MainWnd::OnUpdateOptionsEmulatorAlwaysOnTop(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.alwaysOnTop);
}

void MainWnd::OnOptionsEmulatorPausewheninactive()
{
	theApp.pauseWhenInactive = !theApp.pauseWhenInactive;
}

void MainWnd::OnUpdateOptionsEmulatorPausewheninactive(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.pauseWhenInactive);
}

void MainWnd::OnOptionsEmulatorEnableBackgroundInput()
{
	theApp.enableBackgroundInput = !theApp.enableBackgroundInput;
}

void MainWnd::OnUpdateOptionsEmulatorEnableBackgroundInput(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.enableBackgroundInput);
}

BOOL MainWnd::OnOptionsPriority(UINT nID)
{
	switch (nID)
	{
	case ID_OPTIONS_PRIORITY_HIGHEST:
		theApp.threadPriority = 0;
		break;
	case ID_OPTIONS_PRIORITY_ABOVENORMAL:
		theApp.threadPriority = 1;
		break;
	case ID_OPTIONS_PRIORITY_NORMAL:
		theApp.threadPriority = 2;
		break;
	case ID_OPTIONS_PRIORITY_BELOWNORMAL:
		theApp.threadPriority = 3;
		break;
	default:
		return FALSE;
	}
	theApp.updatePriority();

	return TRUE;
}

void MainWnd::OnUpdateOptionsPriority(CCmdUI *pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_PRIORITY_HIGHEST:
		pCmdUI->SetCheck(theApp.threadPriority == 0);
		break;
	case ID_OPTIONS_PRIORITY_ABOVENORMAL:
		pCmdUI->SetCheck(theApp.threadPriority == 1);
		break;
	case ID_OPTIONS_PRIORITY_NORMAL:
		pCmdUI->SetCheck(theApp.threadPriority == 2);
		break;
	case ID_OPTIONS_PRIORITY_BELOWNORMAL:
		pCmdUI->SetCheck(theApp.threadPriority == 3);
		break;
	}
}

void MainWnd::OnOptionsEmulatorSpeeduptoggle()
{
	theApp.speedupToggle = !theApp.speedupToggle;
}

void MainWnd::OnUpdateOptionsEmulatorSpeeduptoggle(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.speedupToggle);
}

void MainWnd::OnOptionsEmulatorRemoveintrosgba()
{
	//  theApp.removeIntros = !theApp.removeIntros;
}

void MainWnd::OnUpdateOptionsEmulatorRemoveintrosgba(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(false);
	//  pCmdUI->SetCheck(theApp.removeIntros);
}

void MainWnd::OnOptionsEmulatorAutomaticallyipspatch()
{
	theApp.autoIPS = !theApp.autoIPS;
}

void MainWnd::OnUpdateOptionsEmulatorAutomaticallyipspatch(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.autoIPS);
}

void MainWnd::OnOptionsEmulatorAgbprint()
{
	agbPrintEnable(!agbPrintIsEnabled());
}

void MainWnd::OnUpdateOptionsEmulatorAgbprint(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(agbPrintIsEnabled());
}

void MainWnd::OnOptionsEmulatorRealtimeclock()
{
	theApp.winRtcEnable = !theApp.winRtcEnable;
}

void MainWnd::OnUpdateOptionsEmulatorRealtimeclock(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winRtcEnable);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorAutohidemenu()
{
	theApp.autoHideMenu = !theApp.autoHideMenu;
}

void MainWnd::OnUpdateOptionsEmulatorAutohidemenu(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.autoHideMenu);
}

void MainWnd::OnOptionsEmulatorRewindinterval()
{
	RewindInterval dlg((float)theApp.rewindTimer/6.0f, theApp.rewindSlots);
	int v = dlg.DoModal();

	if (v >= 0)
	{
		int interval = v & 0x0000ffff;
		int slots    = (v & 0xffff0000) >> 16;

		int prevSlots = theApp.rewindSlots;

		theApp.rewindTimer = interval; // already converted to a multiple of 10 frames
		theApp.rewindSlots = slots;
		if (interval == 0 || slots == 0)
		{
			theApp.rewindTimer = theApp.rewindSlots = 0;
			regSetDwordValue("rewindTimer", interval);
			regSetDwordValue("rewindSlots", slots);
			if (theApp.rewindMemory)
				free(theApp.rewindMemory);
			theApp.rewindMemory     = NULL;
			theApp.rewindCount      = 0;
			theApp.rewindCounter    = 0;
			theApp.rewindSaveNeeded = false;
		}
		else
		{
			regSetDwordValue("rewindTimer", interval);
			regSetDwordValue("rewindSlots", slots);
			if (slots != prevSlots)
			{
				if (theApp.rewindMemory)
					free(theApp.rewindMemory);
				theApp.rewindMemory = NULL;
				theApp.rewindPos    = 0;
			}
			if (theApp.rewindMemory == NULL)
				theApp.rewindMemory = (char *)malloc(theApp.rewindSlots*REWIND_SIZE);
			theApp.rewindCount      = 0;
			theApp.rewindSaveNeeded = true;
		}
	}
}

BOOL MainWnd::OnOptionsEmulatorShowSpeed(UINT nID)
{
	switch (nID)
	{
	case ID_OPTIONS_EMULATOR_SHOWSPEED_NONE:
		theApp.showSpeed = 0;
		systemSetTitle(VBA_NAME_AND_VERSION);
		break;
	case ID_OPTIONS_EMULATOR_SHOWSPEED_PERCENTAGE:
		theApp.showSpeed = 1;
		break;
	case ID_OPTIONS_EMULATOR_SHOWSPEED_DETAILED:
		theApp.showSpeed = 2;
		break;
	case ID_OPTIONS_EMULATOR_SHOWSPEED_TRANSPARENT:
		theApp.showSpeedTransparent = !theApp.showSpeedTransparent;
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

void MainWnd::OnUpdateOptionsEmulatorShowSpeed(CCmdUI *pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_EMULATOR_SHOWSPEED_NONE:
		pCmdUI->SetCheck(theApp.showSpeed == 0);
		break;
	case ID_OPTIONS_EMULATOR_SHOWSPEED_PERCENTAGE:
		pCmdUI->SetCheck(theApp.showSpeed == 1);
		break;
	case ID_OPTIONS_EMULATOR_SHOWSPEED_DETAILED:
		pCmdUI->SetCheck(theApp.showSpeed == 2);
		break;
	case ID_OPTIONS_EMULATOR_SHOWSPEED_TRANSPARENT:
		pCmdUI->SetCheck(theApp.showSpeedTransparent);
		break;
	}
}

void MainWnd::OnOptionsEmulatorSavetypeAutomatic()
{
	theApp.winSaveType = 0;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeAutomatic(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winSaveType == 0);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorSavetypeEeprom()
{
	theApp.winSaveType = 1;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeEeprom(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winSaveType == 1);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorSavetypeSram()
{
	theApp.winSaveType = 2;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeSram(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winSaveType == 2);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorSavetypeFlash()
{
	theApp.winSaveType = 3;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeFlash(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winSaveType == 3);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorSavetypeEepromsensor()
{
	theApp.winSaveType = 4;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeEepromsensor(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winSaveType == 4);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorSavetypeNone()
{
	theApp.winSaveType = 5;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeNone(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winSaveType == 5);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorSavetypeFlash512k()
{
	flashSetSize(0x10000);
	theApp.winFlashSize = 0x10000;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeFlash512k(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winFlashSize == 0x10000);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorSavetypeFlash1m()
{
	flashSetSize(0x20000);
	theApp.winFlashSize = 0x20000;
}

void MainWnd::OnUpdateOptionsEmulatorSavetypeFlash1m(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winFlashSize == 0x20000);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsEmulatorUsebiosfile()
{
	if (!theApp.biosFileName.IsEmpty())
		theApp.useBiosFile = !theApp.useBiosFile;
}

void MainWnd::OnUpdateOptionsEmulatorUsebiosfile(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.useBiosFile);
	pCmdUI->Enable(!theApp.biosFileName.IsEmpty() && (!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL)));
}

void MainWnd::OnOptionsEmulatorSkipbios()
{
	theApp.skipBiosIntro = !theApp.skipBiosIntro;
}

void MainWnd::OnUpdateOptionsEmulatorSkipbios(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.skipBiosIntro);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

#ifdef USE_GBA_CORE_V7
void MainWnd::OnOptionsEmulatorGBALag()
{
	extern void TogglePrefetchHack();
	TogglePrefetchHack();
	memLagEnabled = memLagTempEnabled; // memLagEnabled is only to hold the last value that the user chose, so temporary changes
                                       // don't get into the registry
}

void MainWnd::OnUpdateOptionsEmulatorGBALag(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(!memLagTempEnabled);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}
#endif

void MainWnd::OnOptionsEmulatorUseOldGBTiming()
{
	useOldFrameTiming = !useOldFrameTiming;
}

void MainWnd::OnUpdateOptionsEmulatorUseOldGBTiming(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(useOldFrameTiming);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

#ifdef USE_GB_CORE_V7
void MainWnd::OnOptionsEmulatorUseGBNullInputHack()
{
	if (VBAMovieIsActive())
		gbNullInputHackTempEnabled = !gbNullInputHackTempEnabled;
	else
		gbNullInputHackTempEnabled = gbNullInputHackEnabled = !gbNullInputHackEnabled;
}

void MainWnd::OnUpdateOptionsEmulatorUseGBNullInputHack(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbNullInputHackTempEnabled);
	pCmdUI->Enable((!VBAMovieIsActive() && !useOldFrameTiming) || GetAsyncKeyState(VK_CONTROL));
}
#else
void MainWnd::OnOptionsEmulatorUseV20GBFrameHack()
{
	if (VBAMovieIsActive())
		gbV20GBFrameTimingHackTemp = !gbV20GBFrameTimingHackTemp;
	else
		gbV20GBFrameTimingHackTemp = gbV20GBFrameTimingHack = !gbV20GBFrameTimingHack;
}

void MainWnd::OnUpdateOptionsEmulatorUseV20GBFrameHack(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbV20GBFrameTimingHackTemp);
	pCmdUI->Enable((!VBAMovieIsActive() && useOldFrameTiming) || GetAsyncKeyState(VK_CONTROL));
}
#endif

void MainWnd::OnOptionsEmulatorSelectbiosfile()
{
	theApp.winCheckFullscreen();
	LPCTSTR exts[] = { NULL };
	CString filter = winResLoadFilter(IDS_FILTER_BIOS);
	CString title  = winResLoadString(IDS_SELECT_BIOS_FILE);

	FileDlg dlg(this,
	            theApp.biosFileName,
	            filter,
	            0,
	            "BIOS",
	            exts,
	            "",
	            title,
	            false);

	if (dlg.DoModal() == IDOK)
	{
		theApp.biosFileName = dlg.GetPathName();
	}
}

void MainWnd::OnOptionsEmulatorPngformat()
{
	theApp.captureFormat = 0;
}

void MainWnd::OnUpdateOptionsEmulatorPngformat(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.captureFormat == 0);
}

void MainWnd::OnOptionsEmulatorBmpformat()
{
	theApp.captureFormat = 1;
}

void MainWnd::OnUpdateOptionsEmulatorBmpformat(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.captureFormat == 1);
}

void MainWnd::OnOptionsSoundDisable()
{
	if (soundOffFlag)
	{
		soundOffFlag = false;
		systemSoundCleanInit();
	}
	else
	{
		soundOffFlag = true;
		systemSoundShutdown();
	}
}

void MainWnd::OnUpdateOptionsSoundDisable(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundOffFlag);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsSoundMute()
{
	if ((systemSoundGetEnabledChannels() & 0x030f) == 0)
		systemSoundEnableChannels(0x030f);
	else
		systemSoundDisableChannels(0x030f);
}

void MainWnd::OnUpdateOptionsSoundMute(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck((systemSoundGetEnabledChannels() & 0x030f) == 0);
}

void MainWnd::OnOptionsSoundOff()
{
	systemSoundDisableChannels(0x030f);
}

void MainWnd::OnUpdateOptionsSoundOff(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck((systemSoundGetEnabledChannels() & 0x030f) == 0);
}

void MainWnd::OnOptionsSoundOn()
{
	systemSoundEnableChannels(0x030f);
}

void MainWnd::OnUpdateOptionsSoundOn(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemSoundGetEnabledChannels() == 0x030f);
}

void MainWnd::OnOptionsSoundUseoldsynchronization()
{
	theApp.useOldSync = !theApp.useOldSync;
	systemMessage(IDS_SETTING_WILL_BE_EFFECTIVE,
	              "Setting will be effective the next time you start the emulator");
}

void MainWnd::OnUpdateOptionsSoundUseoldsynchronization(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.useOldSync);
}

void MainWnd::OnOptionsSoundEcho()
{
	soundEcho = !soundEcho;
}

void MainWnd::OnUpdateOptionsSoundEcho(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundEcho);
}

void MainWnd::OnOptionsSoundLowpassfilter()
{
	soundLowPass = !soundLowPass;
}

void MainWnd::OnUpdateOptionsSoundLowpassfilter(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundLowPass);
}

void MainWnd::OnOptionsSoundReversestereo()
{
	soundReverse = !soundReverse;
}

void MainWnd::OnUpdateOptionsSoundReversestereo(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundReverse);
}

void MainWnd::OnOptionsSoundMuteFrameAdvance()
{
	theApp.muteFrameAdvance = !theApp.muteFrameAdvance;
}

void MainWnd::OnUpdateOptionsSoundMuteFrameAdvance(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.muteFrameAdvance);
}

void MainWnd::OnOptionsSoundMuteWhenInactive()
{
	theApp.muteWhenInactive = !theApp.muteWhenInactive;
}

void MainWnd::OnUpdateOptionsSoundMuteWhenInactive(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.muteWhenInactive);
}

void MainWnd::OnOptionsSound11khz()
{
	systemSoundSetQuality(4);
}

void MainWnd::OnUpdateOptionsSound11khz(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundQuality == 4);
	pCmdUI->Enable((!VBAMovieIsActive() ||
	                GetAsyncKeyState(VK_CONTROL)) && !(theApp.soundRecording || theApp.aviRecording || theApp.nvAudioLog));
}

void MainWnd::OnOptionsSound22khz()
{
	systemSoundSetQuality(2);
}

void MainWnd::OnUpdateOptionsSound22khz(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundQuality == 2);
	pCmdUI->Enable((!VBAMovieIsActive() ||
	                GetAsyncKeyState(VK_CONTROL)) && !(theApp.soundRecording || theApp.aviRecording || theApp.nvAudioLog));
}

void MainWnd::OnOptionsSound44khz()
{
	systemSoundSetQuality(1);
}

void MainWnd::OnUpdateOptionsSound44khz(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundQuality == 1);
	pCmdUI->Enable(!(theApp.soundRecording || theApp.aviRecording || theApp.nvAudioLog));
}

BOOL MainWnd::OnOptionsSoundVolume(UINT nID)
{
	soundVolume = nID - ID_OPTIONS_SOUND_VOLUME_1X;
	return TRUE;
}

void MainWnd::OnUpdateOptionsSoundVolume(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(soundVolume == (int)(pCmdUI->m_nID - ID_OPTIONS_SOUND_VOLUME_1X));
}

void MainWnd::OnOptionsSoundVolume25x()
{
	soundVolume = 4;
}

void MainWnd::OnUpdateOptionsSoundVolume25x(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundVolume == 4);
}

void MainWnd::OnOptionsSoundVolume5x()
{
	soundVolume = 5;
}

void MainWnd::OnUpdateOptionsSoundVolume5x(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(soundVolume == 5);
}

static inline void OnSoundToggleEnabled(int c)
{
	if (systemSoundGetEnabledChannels() & c)
	{
		systemSoundDisableChannels(c);
	}
	else
	{
		systemSoundEnableChannels(c);
	}
}

void MainWnd::OnOptionsSoundChannel1()
{
	OnSoundToggleEnabled(0x01);
}

void MainWnd::OnUpdateOptionsSoundChannel1(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemSoundGetEnabledChannels() & 0x01);
}

void MainWnd::OnOptionsSoundChannel2()
{
	OnSoundToggleEnabled(0x02);
}

void MainWnd::OnUpdateOptionsSoundChannel2(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemSoundGetEnabledChannels() & 0x02);
}

void MainWnd::OnOptionsSoundChannel3()
{
	OnSoundToggleEnabled(0x04);
}

void MainWnd::OnUpdateOptionsSoundChannel3(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemSoundGetEnabledChannels() & 0x04);
}

void MainWnd::OnOptionsSoundChannel4()
{
	OnSoundToggleEnabled(0x08);
}

void MainWnd::OnUpdateOptionsSoundChannel4(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemSoundGetEnabledChannels() & 0x08);
}

void MainWnd::OnOptionsSoundDirectsounda()
{
	OnSoundToggleEnabled(0x0100);
}

void MainWnd::OnUpdateOptionsSoundDirectsounda(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemSoundGetEnabledChannels() & 0x0100);
	//pCmdUI->Enable(systemCartridgeType == IMAGE_GBA);
}

void MainWnd::OnOptionsSoundDirectsoundb()
{
	OnSoundToggleEnabled(0x0200);
}

void MainWnd::OnUpdateOptionsSoundDirectsoundb(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(systemSoundGetEnabledChannels() & 0x0200);
	//pCmdUI->Enable(systemCartridgeType == IMAGE_GBA);
}

void MainWnd::OnOptionsGameboyBorder()
{
	theApp.winGbBorderOn = !theApp.winGbBorderOn;
	gbBorderOn = theApp.winGbBorderOn;
	if (emulating && systemCartridgeType == 1 && gbBorderOn)
	{
		gbSgbRenderBorder();
	}
	theApp.updateWindowSize(theApp.videoOption);
}

void MainWnd::OnUpdateOptionsGameboyBorder(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.winGbBorderOn);
}

void MainWnd::OnOptionsGameboyPrinter()
{
	theApp.winGbPrinterEnabled = !theApp.winGbPrinterEnabled;
	if (theApp.winGbPrinterEnabled)
		gbSerialFunction = gbPrinterSend;
	else
		gbSerialFunction = NULL;
}

void MainWnd::OnUpdateOptionsGameboyPrinter(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbSerialFunction == gbPrinterSend);
}

void MainWnd::OnOptionsGameboyBorderAutomatic()
{
	gbBorderAutomatic = !gbBorderAutomatic;
	if (emulating && systemCartridgeType == 1 && gbBorderOn)
	{
		gbSgbRenderBorder();
		theApp.updateWindowSize(theApp.videoOption);
	}
}

void MainWnd::OnUpdateOptionsGameboyBorderAutomatic(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbBorderAutomatic);
}

void MainWnd::OnOptionsGameboyAutomatic()
{
	gbEmulatorType = 0;
}

void MainWnd::OnUpdateOptionsGameboyAutomatic(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbEmulatorType == 0);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsGameboyGba()
{
	gbEmulatorType = 4;
}

void MainWnd::OnUpdateOptionsGameboyGba(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbEmulatorType == 4);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsGameboyCgb()
{
	gbEmulatorType = 1;
}

void MainWnd::OnUpdateOptionsGameboyCgb(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbEmulatorType == 1);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsGameboySgb()
{
	gbEmulatorType = 2;
}

void MainWnd::OnUpdateOptionsGameboySgb(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbEmulatorType == 2);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsGameboySgb2()
{
	gbEmulatorType = 5;
}

void MainWnd::OnUpdateOptionsGameboySgb2(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbEmulatorType == 5);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsGameboyGb()
{
	gbEmulatorType = 3;
}

void MainWnd::OnUpdateOptionsGameboyGb(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbEmulatorType == 3);
	pCmdUI->Enable(!VBAMovieIsActive() || GetAsyncKeyState(VK_CONTROL));
}

void MainWnd::OnOptionsGameboyRealcolors()
{
	gbColorOption = 0;
}

void MainWnd::OnUpdateOptionsGameboyRealcolors(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbColorOption == 0);
}

void MainWnd::OnOptionsGameboyGameboycolors()
{
	gbColorOption = 1;
}

void MainWnd::OnUpdateOptionsGameboyGameboycolors(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(gbColorOption == 1);
}

void MainWnd::OnOptionsGameboyColors()
{
	theApp.winCheckFullscreen();
	GBColorDlg dlg;
	if (dlg.DoModal())
	{
		gbPaletteOption = dlg.getWhich();
		memcpy(systemGbPalette, dlg.getColors(), 24*sizeof(u16));
		if (emulating && systemCartridgeType == 1)
		{
			memcpy(gbPalette, &systemGbPalette[dlg.getWhich()*8], 8*sizeof(u16));
		}
	}
}

BOOL MainWnd::OnOptionsFilter(UINT nID)
{
	switch (nID)
	{
	case ID_OPTIONS_FILTER_NORMAL:
		theApp.filterType = 0;
		break;
	case ID_OPTIONS_FILTER_TVMODE:
		theApp.filterType = 1;
		break;
	case ID_OPTIONS_FILTER_2XSAI:
		theApp.filterType = 2;
		break;
	case ID_OPTIONS_FILTER_SUPER2XSAI:
		theApp.filterType = 3;
		break;
	case ID_OPTIONS_FILTER_SUPEREAGLE:
		theApp.filterType = 4;
		break;
	case ID_OPTIONS_FILTER16BIT_PIXELATEEXPERIMENTAL:
		theApp.filterType = 5;
		break;
	case ID_OPTIONS_FILTER16BIT_MOTIONBLUREXPERIMENTAL:
		theApp.filterType = 6;
		break;
	case ID_OPTIONS_FILTER16BIT_ADVANCEMAMESCALE2X:
		theApp.filterType = 7;
		break;
	case ID_OPTIONS_FILTER16BIT_SIMPLE2X:
		theApp.filterType = 8;
		break;
	case ID_OPTIONS_FILTER_BILINEAR:
		theApp.filterType = 9;
		break;
	case ID_OPTIONS_FILTER_BILINEARPLUS:
		theApp.filterType = 10;
		break;
	case ID_OPTIONS_FILTER_SCANLINES:
		theApp.filterType = 11;
		break;
	case ID_OPTIONS_FILTER_HQ2X2:
		theApp.filterType = 12;
		break;
	case ID_OPTIONS_FILTER_HQ2X:
		theApp.filterType = 13;
		break;
	case ID_OPTIONS_FILTER_LQ2X:
		theApp.filterType = 14;
		break;
	case ID_OPTIONS_FILTER_HQ3X2:
		theApp.filterType = 15;
		break;
	case ID_OPTIONS_FILTER_HQ3X:
		theApp.filterType = 16;
		break;
	case ID_OPTIONS_FILTER16BIT_SIMPLE3X:
		theApp.filterType = 17;
		break;
	case ID_OPTIONS_FILTER16BIT_SIMPLE4X:
		theApp.filterType = 18;
		break;
	case ID_OPTIONS_FILTER16BIT_PIXELATEEXPERIMENTAL3X:
		theApp.filterType = 19;
		break;
	case ID_OPTIONS_FILTER16BIT_PIXELATEEXPERIMENTAL4X:
		theApp.filterType = 20;
		break;
	default:
		return FALSE;
	}
	theApp.updateFilter();
	return TRUE;
}

void MainWnd::OnUpdateOptionsFilter(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(systemColorDepth == 16 || systemColorDepth == 32);
	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_FILTER_NORMAL:
		pCmdUI->SetCheck(theApp.filterType == 0);
		break;
	case ID_OPTIONS_FILTER_TVMODE:
		pCmdUI->SetCheck(theApp.filterType == 1);
		break;
	case ID_OPTIONS_FILTER_2XSAI:
		pCmdUI->SetCheck(theApp.filterType == 2);
		break;
	case ID_OPTIONS_FILTER_SUPER2XSAI:
		pCmdUI->SetCheck(theApp.filterType == 3);
		break;
	case ID_OPTIONS_FILTER_SUPEREAGLE:
		pCmdUI->SetCheck(theApp.filterType == 4);
		break;
	case ID_OPTIONS_FILTER16BIT_PIXELATEEXPERIMENTAL:
		pCmdUI->SetCheck(theApp.filterType == 5);
		break;
	case ID_OPTIONS_FILTER16BIT_MOTIONBLUREXPERIMENTAL:
		pCmdUI->SetCheck(theApp.filterType == 6);
		break;
	case ID_OPTIONS_FILTER16BIT_ADVANCEMAMESCALE2X:
		pCmdUI->SetCheck(theApp.filterType == 7);
		break;
	case ID_OPTIONS_FILTER16BIT_SIMPLE2X:
		pCmdUI->SetCheck(theApp.filterType == 8);
		break;
	case ID_OPTIONS_FILTER_BILINEAR:
		pCmdUI->SetCheck(theApp.filterType == 9);
		break;
	case ID_OPTIONS_FILTER_BILINEARPLUS:
		pCmdUI->SetCheck(theApp.filterType == 10);
		break;
	case ID_OPTIONS_FILTER_SCANLINES:
		pCmdUI->SetCheck(theApp.filterType == 11);
		break;
	case ID_OPTIONS_FILTER_HQ2X2:
		pCmdUI->SetCheck(theApp.filterType == 12);
		break;
	case ID_OPTIONS_FILTER_HQ2X:
		pCmdUI->SetCheck(theApp.filterType == 13);
		break;
	case ID_OPTIONS_FILTER_LQ2X:
		pCmdUI->SetCheck(theApp.filterType == 14);
		break;
	case ID_OPTIONS_FILTER_HQ3X2:
		pCmdUI->SetCheck(theApp.filterType == 15);
		break;
	case ID_OPTIONS_FILTER_HQ3X:
		pCmdUI->SetCheck(theApp.filterType == 16);
		break;
	case ID_OPTIONS_FILTER16BIT_SIMPLE3X:
		pCmdUI->SetCheck(theApp.filterType == 17);
		break;
	case ID_OPTIONS_FILTER16BIT_SIMPLE4X:
		pCmdUI->SetCheck(theApp.filterType == 18);
		break;
	case ID_OPTIONS_FILTER16BIT_PIXELATEEXPERIMENTAL3X:
		pCmdUI->SetCheck(theApp.filterType == 19);
		break;
	case ID_OPTIONS_FILTER16BIT_PIXELATEEXPERIMENTAL4X:
		pCmdUI->SetCheck(theApp.filterType == 20);
		break;
	}
}

BOOL MainWnd::OnOptionsFilterIFB(UINT nID)
{
	switch (nID)
	{
	case ID_OPTIONS_FILTER_INTERFRAMEBLENDING_NONE:
		theApp.ifbType = 0;
		break;
	case ID_OPTIONS_FILTER_INTERFRAMEBLENDING_MOTIONBLUR:
		theApp.ifbType = 1;
		break;
	case ID_OPTIONS_FILTER_INTERFRAMEBLENDING_SMART:
		theApp.ifbType = 2;
		break;
	default:
		return FALSE;
	}
	theApp.updateIFB();
	return TRUE;
}

void MainWnd::OnUpdateOptionsFilterIFB(CCmdUI *pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_FILTER_INTERFRAMEBLENDING_NONE:
		pCmdUI->SetCheck(theApp.ifbType == 0);
		break;
	case ID_OPTIONS_FILTER_INTERFRAMEBLENDING_MOTIONBLUR:
		pCmdUI->SetCheck(theApp.ifbType == 1);
		break;
	case ID_OPTIONS_FILTER_INTERFRAMEBLENDING_SMART:
		pCmdUI->SetCheck(theApp.ifbType == 2);
		break;
	}
}

void MainWnd::OnOptionsFilterDisablemmx()
{
	theApp.disableMMX = !theApp.disableMMX;
	if (!theApp.disableMMX)
		cpu_mmx = theApp.detectMMX();
	else
		cpu_mmx = 0;
}

void MainWnd::OnUpdateOptionsFilterDisablemmx(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.disableMMX);
}

void MainWnd::OnOptionsLanguageSystem()
{
	theApp.winSetLanguageOption(0, false);
}

void MainWnd::OnUpdateOptionsLanguageSystem(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.languageOption == 0);
}

void MainWnd::OnOptionsLanguageEnglish()
{
	theApp.winSetLanguageOption(1, false);
}

void MainWnd::OnUpdateOptionsLanguageEnglish(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.languageOption == 1);
}

void MainWnd::OnOptionsLanguageOther()
{
	theApp.winCheckFullscreen();
	theApp.winSetLanguageOption(2, false);
}

void MainWnd::OnUpdateOptionsLanguageOther(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.languageOption == 2);
}

void MainWnd::OnOptionsJoypadConfigure1()
{
	theApp.winCheckFullscreen();
	JoypadConfig dlg(0);
	dlg.DoModal();
}

void MainWnd::OnUpdateOptionsJoypadConfigure1(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.videoOption != VIDEO_320x240);
}

void MainWnd::OnOptionsJoypadConfigure2()
{
	theApp.winCheckFullscreen();
	JoypadConfig dlg(1);
	dlg.DoModal();
}

void MainWnd::OnUpdateOptionsJoypadConfigure2(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.videoOption != VIDEO_320x240);
}

void MainWnd::OnOptionsJoypadConfigure3()
{
	theApp.winCheckFullscreen();
	JoypadConfig dlg(2);
	dlg.DoModal();
}

void MainWnd::OnUpdateOptionsJoypadConfigure3(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.videoOption != VIDEO_320x240);
}

void MainWnd::OnOptionsJoypadConfigure4()
{
	theApp.winCheckFullscreen();
	JoypadConfig dlg(3);
	dlg.DoModal();
}

void MainWnd::OnUpdateOptionsJoypadConfigure4(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.videoOption != VIDEO_320x240);
}

BOOL MainWnd::OnOptionsJoypadDefault(UINT nID)
{
	theApp.joypadDefault = nID - ID_OPTIONS_JOYPAD_DEFAULTJOYPAD_1;
	return TRUE;
}

void MainWnd::OnUpdateOptionsJoypadDefault(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(theApp.joypadDefault == (int)(pCmdUI->m_nID - ID_OPTIONS_JOYPAD_DEFAULTJOYPAD_1));
}

void MainWnd::OnOptionsJoypadMotionconfigure()
{
	theApp.winCheckFullscreen();
	MotionConfig dlg;
	dlg.DoModal();
}

void MainWnd::OnUpdateOptionsJoypadMotionconfigure(CCmdUI*pCmdUI)
{
	pCmdUI->Enable(theApp.videoOption != VIDEO_320x240);
}

void MainWnd::OnOptionsJoypadAllowLeftRight()
{
	theApp.allowLeftRight = !theApp.allowLeftRight;
}

void MainWnd::OnUpdateOptionsJoypadAllowLeftRight(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.allowLeftRight);
}

void MainWnd::OnOptionsJoypadAutofireAccountForLag()
{
	theApp.autofireAccountForLag = !theApp.autofireAccountForLag;
}

void MainWnd::OnUpdateOptionsJoypadAutofireAccountForLag(CCmdUI*pCmdUI)
{
	pCmdUI->SetCheck(theApp.autofireAccountForLag);
}

BOOL MainWnd::OnOptionsJoypadAutofire(UINT nID)
{
	int & autoFire  = (theApp.autoFireToggle ? theApp.autoFire : theApp.autoFire2);
	int & autoFire2 = (theApp.autoFireToggle ? theApp.autoFire2 : theApp.autoFire);
	int   autoFires = (theApp.autoFire | theApp.autoFire2);

	switch (nID)
	{
	case ID_OPTIONS_JOYPAD_AUTOFIRE_A:
		if (autoFires & BUTTON_MASK_A)
		{
			autoFire &= ~BUTTON_MASK_A;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_A_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_A;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_A));
		}
		autoFire2       &= ~BUTTON_MASK_A;
		theApp.autoHold &= ~BUTTON_MASK_A;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_B:
		if (autoFires & BUTTON_MASK_B)
		{
			autoFire &= ~BUTTON_MASK_B;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_B_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_B;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_B));
		}
		autoFire2       &= ~BUTTON_MASK_B;
		theApp.autoHold &= ~BUTTON_MASK_B;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_L:
		if (autoFires & BUTTON_MASK_L)
		{
			autoFire &= ~BUTTON_MASK_L;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_L_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_L;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_L));
		}
		autoFire2       &= ~BUTTON_MASK_L;
		theApp.autoHold &= ~BUTTON_MASK_L;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_R:
		if (autoFires & BUTTON_MASK_R)
		{
			autoFire &= ~BUTTON_MASK_R;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_R_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_R;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_R));
		}
		autoFire2       &= ~BUTTON_MASK_R;
		theApp.autoHold &= ~BUTTON_MASK_R;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_START:
		if (autoFires & BUTTON_MASK_START)
		{
			autoFire &= ~BUTTON_MASK_START;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_START_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_START;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_START));
		}
		autoFire2       &= ~BUTTON_MASK_START;
		theApp.autoHold &= ~BUTTON_MASK_START;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_SELECT:
		if (autoFires & BUTTON_MASK_SELECT)
		{
			autoFire &= ~BUTTON_MASK_SELECT;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_SELECT_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_SELECT;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_SELECT));
		}
		autoFire2       &= ~BUTTON_MASK_SELECT;
		theApp.autoHold &= ~BUTTON_MASK_SELECT;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_UP:
		if (autoFires & BUTTON_MASK_UP)
		{
			autoFire &= ~BUTTON_MASK_UP;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_UP_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_UP;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_UP));
		}
		autoFire2       &= ~BUTTON_MASK_UP;
		theApp.autoHold &= ~BUTTON_MASK_UP;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_DOWN:
		if (autoFires & BUTTON_MASK_DOWN)
		{
			autoFire &= ~BUTTON_MASK_DOWN;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_DOWN_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_DOWN;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_DOWN));
		}
		autoFire2       &= ~BUTTON_MASK_DOWN;
		theApp.autoHold &= ~BUTTON_MASK_DOWN;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_LEFT:
		if (autoFires & BUTTON_MASK_LEFT)
		{
			autoFire &= ~BUTTON_MASK_LEFT;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_LEFT_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_LEFT;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_LEFT));
		}
		autoFire2       &= ~BUTTON_MASK_LEFT;
		theApp.autoHold &= ~BUTTON_MASK_LEFT;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_RIGHT:
		if (autoFires & BUTTON_MASK_RIGHT)
		{
			autoFire &= ~BUTTON_MASK_RIGHT;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_RIGHT_DISABLED));
		}
		else
		{
			autoFire |= BUTTON_MASK_RIGHT;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_RIGHT));
		}
		autoFire2       &= ~BUTTON_MASK_RIGHT;
		theApp.autoHold &= ~BUTTON_MASK_RIGHT;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_CLEAR:
		if (autoFires != 0)
		{
			theApp.autoFire = theApp.autoFire2 = 0;
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_DISABLED));
		}
		else
		{
///      systemScreenMessage(winResLoadString(IDS_AUTOFIRE_ALREADY_DISABLED));
			systemScreenMessage("already cleared");
		}
		break;
	default:
		return FALSE;
	}

	extern void VBAUpdateButtonPressDisplay(); VBAUpdateButtonPressDisplay();

	return TRUE;
}

void MainWnd::OnUpdateOptionsJoypadAutofire(CCmdUI *pCmdUI)
{
///  pCmdUI->Enable(emulating); // FIXME: this is right, but disabling menu items screws up accelerators until you view the
// menu!
	pCmdUI->Enable(TRUE); // TEMP

	int autoFires = (theApp.autoFire | theApp.autoFire2);

	bool check = true;
	switch (pCmdUI->m_nID)
	{
	case ID_OPTIONS_JOYPAD_AUTOFIRE_A:
		check = (autoFires & BUTTON_MASK_A) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_B:
		check = (autoFires & BUTTON_MASK_B) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_L:
		check = (autoFires & BUTTON_MASK_L) != 0;
///	extern int gbSgbMode; // from gbSGB.cpp
///	if(emulating && systemCartridgeType != IMAGE_GBA && !gbSgbMode) // regular GB has no L button
///      pCmdUI->Enable(false); // FIXME: this is right, but disabling menu items screws up accelerators until you view the
// menu!
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_R:
		check = (autoFires & BUTTON_MASK_R) != 0;
///	extern int gbSgbMode; // from gbSGB.cpp
///	if(emulating && systemCartridgeType != IMAGE_GBA && !gbSgbMode) // regular GB has no R button
///      pCmdUI->Enable(false); // FIXME: this is right, but disabling menu items screws up accelerators until you view the
// menu!
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_START:
		check = (autoFires & BUTTON_MASK_START) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_SELECT:
		check = (autoFires & BUTTON_MASK_SELECT) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_UP:
		check = (autoFires & BUTTON_MASK_UP) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_DOWN:
		check = (autoFires & BUTTON_MASK_DOWN) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_LEFT:
		check = (autoFires & BUTTON_MASK_LEFT) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_RIGHT:
		check = (autoFires & BUTTON_MASK_RIGHT) != 0;
		break;
	case ID_OPTIONS_JOYPAD_AUTOFIRE_CLEAR:
		check = (autoFires == 0);
///    pCmdUI->Enable(!check); // FIXME: this is right, but disabling menu items screws up accelerators until you view the menu!
		break;
	}
	pCmdUI->SetCheck(check);
}

BOOL MainWnd::OnOptionsJoypadSticky(UINT nID)
{
	switch (nID)
	{
	case ID_STICKY_A:
		if (theApp.autoHold & BUTTON_MASK_A)
		{
			theApp.autoHold &= ~BUTTON_MASK_A;
///      systemScreenMessage(winResLoadString(IDS_STICKY_A_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_A;
///      systemScreenMessage(winResLoadString(IDS_STICKY_A));
		}
		theApp.autoFire  &= ~BUTTON_MASK_A;
		theApp.autoFire2 &= ~BUTTON_MASK_A;
		break;
	case ID_STICKY_B:
		if (theApp.autoHold & BUTTON_MASK_B)
		{
			theApp.autoHold &= ~BUTTON_MASK_B;
///      systemScreenMessage(winResLoadString(IDS_STICKY_B_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_B;
///      systemScreenMessage(winResLoadString(IDS_STICKY_B));
		}
		theApp.autoFire  &= ~BUTTON_MASK_B;
		theApp.autoFire2 &= ~BUTTON_MASK_B;
		break;
	case ID_STICKY_L:
		if (theApp.autoHold & BUTTON_MASK_L)
		{
			theApp.autoHold &= ~BUTTON_MASK_L;
///      systemScreenMessage(winResLoadString(IDS_STICKY_L_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_L;
///      systemScreenMessage(winResLoadString(IDS_STICKY_L));
		}
		theApp.autoFire  &= ~BUTTON_MASK_L;
		theApp.autoFire2 &= ~BUTTON_MASK_L;
		break;
	case ID_STICKY_R:
		if (theApp.autoHold & BUTTON_MASK_R)
		{
			theApp.autoHold &= ~BUTTON_MASK_R;
///      systemScreenMessage(winResLoadString(IDS_STICKY_R_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_R;
///      systemScreenMessage(winResLoadString(IDS_STICKY_R));
		}
		theApp.autoFire  &= ~BUTTON_MASK_R;
		theApp.autoFire2 &= ~BUTTON_MASK_R;
		break;
	case ID_STICKY_START:
		if (theApp.autoHold & BUTTON_MASK_START)
		{
			theApp.autoHold &= ~BUTTON_MASK_START;
///      systemScreenMessage(winResLoadString(IDS_STICKY_START_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_START;
///      systemScreenMessage(winResLoadString(IDS_STICKY_START));
		}
		theApp.autoFire  &= ~BUTTON_MASK_START;
		theApp.autoFire2 &= ~BUTTON_MASK_START;
		break;
	case ID_STICKY_SELECT:
		if (theApp.autoHold & BUTTON_MASK_SELECT)
		{
			theApp.autoHold &= ~BUTTON_MASK_SELECT;
///      systemScreenMessage(winResLoadString(IDS_STICKY_SELECT_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_SELECT;
///      systemScreenMessage(winResLoadString(IDS_STICKY_SELECT));
		}
		theApp.autoFire  &= ~BUTTON_MASK_SELECT;
		theApp.autoFire2 &= ~BUTTON_MASK_SELECT;
		break;
	case ID_STICKY_UP:
		if (theApp.autoHold & BUTTON_MASK_UP)
		{
			theApp.autoHold &= ~BUTTON_MASK_UP;
///      systemScreenMessage(winResLoadString(IDS_STICKY_UP_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_UP;
///      systemScreenMessage(winResLoadString(IDS_STICKY_UP));
		}
		theApp.autoFire  &= ~BUTTON_MASK_UP;
		theApp.autoFire2 &= ~BUTTON_MASK_UP;
		if (!theApp.allowLeftRight)
			theApp.autoHold &= ~BUTTON_MASK_DOWN;
		break;
	case ID_STICKY_DOWN:
		if (theApp.autoHold & BUTTON_MASK_DOWN)
		{
			theApp.autoHold &= ~BUTTON_MASK_DOWN;
///      systemScreenMessage(winResLoadString(IDS_STICKY_DOWN_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_DOWN;
///      systemScreenMessage(winResLoadString(IDS_STICKY_DOWN));
		}
		theApp.autoFire  &= ~BUTTON_MASK_DOWN;
		theApp.autoFire2 &= ~BUTTON_MASK_DOWN;
		if (!theApp.allowLeftRight)
			theApp.autoHold &= ~BUTTON_MASK_UP;
		break;
	case ID_STICKY_LEFT:
		if (theApp.autoHold & BUTTON_MASK_LEFT)
		{
			theApp.autoHold &= ~BUTTON_MASK_LEFT;
///      systemScreenMessage(winResLoadString(IDS_STICKY_LEFT_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_LEFT;
///      systemScreenMessage(winResLoadString(IDS_STICKY_LEFT));
		}
		theApp.autoFire  &= ~BUTTON_MASK_LEFT;
		theApp.autoFire2 &= ~BUTTON_MASK_LEFT;
		if (!theApp.allowLeftRight)
			theApp.autoHold &= ~BUTTON_MASK_RIGHT;
		break;
	case ID_STICKY_RIGHT:
		if (theApp.autoHold & BUTTON_MASK_RIGHT)
		{
			theApp.autoHold &= ~BUTTON_MASK_RIGHT;
///      systemScreenMessage(winResLoadString(IDS_STICKY_RIGHT_DISABLED));
		}
		else
		{
			theApp.autoHold |= BUTTON_MASK_RIGHT;
///      systemScreenMessage(winResLoadString(IDS_STICKY_RIGHT));
		}
		theApp.autoFire  &= ~BUTTON_MASK_RIGHT;
		theApp.autoFire2 &= ~BUTTON_MASK_RIGHT;
		if (!theApp.allowLeftRight)
			theApp.autoHold &= ~BUTTON_MASK_LEFT;
		break;
	case ID_STICKY_CLEAR:
		if (theApp.autoHold != 0)
		{
			theApp.autoHold = 0;
///      systemScreenMessage(winResLoadString(IDS_STICKY_DISABLED));
		}
		else
		{
///      systemScreenMessage(winResLoadString(IDS_STICKY_ALREADY_DISABLED));
			systemScreenMessage("already cleared");
		}
		break;
	default:
		return FALSE;
	}

	extern void VBAUpdateButtonPressDisplay(); VBAUpdateButtonPressDisplay();

	return TRUE;
}

void MainWnd::OnUpdateOptionsJoypadSticky(CCmdUI *pCmdUI)
{
///  pCmdUI->Enable(emulating); // FIXME: this is right, but disabling menu items screws up accelerators until you view the
// menu!
	pCmdUI->Enable(TRUE); // TEMP

	bool check = true;
	switch (pCmdUI->m_nID)
	{
	case ID_STICKY_A:
		check = (theApp.autoHold & BUTTON_MASK_A) != 0;
		break;
	case ID_STICKY_B:
		check = (theApp.autoHold & BUTTON_MASK_B) != 0;
		break;
	case ID_STICKY_L:
		check = (theApp.autoHold & BUTTON_MASK_L) != 0;
///	extern int gbSgbMode; // from gbSGB.cpp
///	if(emulating && systemCartridgeType != IMAGE_GBA && !gbSgbMode) // regular GB has no L button
///      pCmdUI->Enable(false); // FIXME: this is right, but disabling menu items screws up accelerators until you view the
// menu!
		break;
	case ID_STICKY_R:
		check = (theApp.autoHold & BUTTON_MASK_R) != 0;
///	extern int gbSgbMode; // from gbSGB.cpp
///	if(emulating && systemCartridgeType != IMAGE_GBA && !gbSgbMode) // regular GB has no R button
///      pCmdUI->Enable(false); // FIXME: this is right, but disabling menu items screws up accelerators until you view the
// menu!
		break;
	case ID_STICKY_START:
		check = (theApp.autoHold & BUTTON_MASK_START) != 0;
		break;
	case ID_STICKY_SELECT:
		check = (theApp.autoHold & BUTTON_MASK_SELECT) != 0;
		break;
	case ID_STICKY_UP:
		check = (theApp.autoHold & BUTTON_MASK_UP) != 0;
		break;
	case ID_STICKY_DOWN:
		check = (theApp.autoHold & BUTTON_MASK_DOWN) != 0;
		break;
	case ID_STICKY_LEFT:
		check = (theApp.autoHold & BUTTON_MASK_LEFT) != 0;
		break;
	case ID_STICKY_RIGHT:
		check = (theApp.autoHold & BUTTON_MASK_RIGHT) != 0;
		break;
	case ID_STICKY_CLEAR:
		check = (theApp.autoHold == 0);
///    pCmdUI->Enable(!check); // FIXME: this is right, but disabling menu items screws up accelerators until you view the menu!
		break;
	}
	pCmdUI->SetCheck(check);
}

LRESULT MainWnd::OnConfirmMode(WPARAM, LPARAM)
{
	// we need to do this separately or the window will not have the right
	// parent. must be related to the way MFC does modal dialogs
	winConfirmMode();
	return 0;
}

void MainWnd::OnOptionsVideoFullscreenmaxscale()
{
	MaxScale dlg;

	theApp.winCheckFullscreen();

	dlg.DoModal();
}

void MainWnd::OnOptionsVideoTextdisplayoptions()
{
	TextOptions dlg;

	theApp.winCheckFullscreen();

	dlg.DoModal();
}

void MainWnd::OnUpdateOptionsVideoTextdisplayoptions(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

