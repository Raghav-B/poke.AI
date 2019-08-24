#if !defined(AFX_MAINWND_H__E8AD28B9_C9FB_4EC2_A2DC_DD1BBA55A275__INCLUDED_)
#define AFX_MAINWND_H__E8AD28B9_C9FB_4EC2_A2DC_DD1BBA55A275__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MainWnd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// MainWnd window

class MainWnd : public CWnd
{
	// Constructor/Destructor
public:
	MainWnd();
	virtual ~MainWnd();

	// Attributes
private:
	HCURSOR arrow;
	HACCEL m_hAccelTable;

	// Implementation
private:

	// Operations
public:
	afx_msg void OnOptionsEmulatorSelectbiosfile();
	void winFileClose(bool reopening = false);
	bool winFileRun(bool reopening = false);
	bool winFileOpenSelect(int cartridgeType);
	void winMouseOn();
	void winConfirmMode();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MainWnd)
public:
	virtual BOOL PreTranslateMessage(MSG*pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(MainWnd)
	afx_msg LRESULT OnConfirmMode(WPARAM, LPARAM);
	afx_msg LRESULT OnMySysCommand(WPARAM, LPARAM);

	afx_msg void OnMove(int x, int y);
	afx_msg void OnContextMenu(CWnd*pWnd, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnInitMenuPopup(CMenu*pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnInitMenu(CMenu*pMenu);
	afx_msg void OnPaint();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnActivate(UINT nState, CWnd*pWndOther, BOOL bMinimized);
#if _MSC_VER <= 1200
	afx_msg void OnActivateApp(BOOL bActive, HTASK hTask);
#else
	afx_msg void OnActivateApp(BOOL bActive, DWORD hTask);
#endif

	afx_msg void OnUpdateOptionsJoypadAutofire(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsJoypadAutofire(UINT nID);
	afx_msg void OnUpdateOptionsJoypadSticky(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsJoypadSticky(UINT nID);
	afx_msg void OnUpdateOptionsJoypadDefault(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsJoypadDefault(UINT nID);
	afx_msg void OnUpdateOptionsFilterIFB(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsFilterIFB(UINT nID);
	afx_msg void OnUpdateOptionsFilter(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsFilter(UINT nID);
	afx_msg void OnUpdateOptionsPriority(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsPriority(UINT nID);
	afx_msg void OnUpdateOptionsSoundVolume(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsSoundVolume(UINT nID);
	afx_msg void OnUpdateOptionsEmulatorShowSpeed(CCmdUI *pCmdUI);
	afx_msg BOOL OnOptionsEmulatorShowSpeed(UINT nID);
	afx_msg void OnUpdateVideoLayer(CCmdUI*pCmdUI);
	afx_msg BOOL OnVideoLayer(UINT nID);
	afx_msg BOOL OnOptionVideoSize(UINT nID);
	afx_msg BOOL OnOptionsFrameskip(UINT nID);
	afx_msg void OnFileTogglemenu();

	afx_msg void OnSystemMinimize();
	afx_msg void OnSystemMaximize();

	afx_msg void OnHelpAbout();
	afx_msg void OnHelpFaq();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenGBx();
    afx_msg void ResetController();
	afx_msg void OnFilePause();
	afx_msg void OnUpdateFilePause(CCmdUI*pCmdUI);
	afx_msg void OnFileReset();
	afx_msg void OnUpdateFileReset(CCmdUI*pCmdUI);
	afx_msg void OnUpdateFileRecentFreeze(CCmdUI*pCmdUI);
	afx_msg void OnFileRecentReset();
	afx_msg void OnFileRecentFreeze();
	afx_msg void OnFileExit();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileClose(CCmdUI*pCmdUI);
	afx_msg void OnFileLoad();
	afx_msg void OnUpdateFileLoad(CCmdUI*pCmdUI);
	afx_msg void OnFileSave();
	afx_msg void OnUpdateFileSave(CCmdUI*pCmdUI);
	afx_msg void OnFileImportBatteryfile();
	afx_msg void OnUpdateFileImportBatteryfile(CCmdUI*pCmdUI);
	afx_msg void OnFileImportGamesharkcodefile();
	afx_msg void OnUpdateFileImportGamesharkcodefile(CCmdUI*pCmdUI);
	afx_msg void OnFileImportGamesharksnapshot();
	afx_msg void OnUpdateFileImportGamesharksnapshot(CCmdUI*pCmdUI);
	afx_msg void OnFileExportBatteryfile();
	afx_msg void OnUpdateFileExportBatteryfile(CCmdUI*pCmdUI);
	afx_msg void OnFileExportGamesharksnapshot();
	afx_msg void OnUpdateFileExportGamesharksnapshot(CCmdUI*pCmdUI);
	afx_msg void OnFileQuickScreencapture();
	afx_msg void OnFileScreencapture();
	afx_msg void OnUpdateFileScreencapture(CCmdUI*pCmdUI);
	afx_msg void OnFileRominformation();
	afx_msg void OnUpdateFileRominformation(CCmdUI*pCmdUI);
	afx_msg void OnUpdateFileTogglemenu(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottleNothrottle(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle6(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle15(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle25(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle50(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle75(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle100(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle125(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle150(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle200(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle300(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle400(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle600(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottle1000(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottleOther(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottleIncrease(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipThrottleDecrease(CCmdUI*pCmdUI);
	afx_msg void OnOptionsFrameskipThrottleNothrottle();
	afx_msg void OnOptionsFrameskipThrottle6();
	afx_msg void OnOptionsFrameskipThrottle15();
	afx_msg void OnOptionsFrameskipThrottle25();
	afx_msg void OnOptionsFrameskipThrottle50();
	afx_msg void OnOptionsFrameskipThrottle75();
	afx_msg void OnOptionsFrameskipThrottle100();
	afx_msg void OnOptionsFrameskipThrottle125();
	afx_msg void OnOptionsFrameskipThrottle150();
	afx_msg void OnOptionsFrameskipThrottle200();
	afx_msg void OnOptionsFrameskipThrottle300();
	afx_msg void OnOptionsFrameskipThrottle400();
	afx_msg void OnOptionsFrameskipThrottle600();
	afx_msg void OnOptionsFrameskipThrottle1000();
	afx_msg void OnOptionsFrameskipThrottleOther();
	afx_msg void OnOptionsFrameskipThrottleIncrease();
	afx_msg void OnOptionsFrameskipThrottleDecrease();
	afx_msg void OnOptionsFrameskipAccuratePitch();
	afx_msg void OnOptionsFrameskipAccurateSpeed();
	afx_msg void OnUpdateOptionsFrameskipAccuratePitch(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsFrameskipAccurateSpeed(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip0(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip1(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip2(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip3(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip4(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip5(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip6(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip7(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip8(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFrameskip9(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoVsync();
	afx_msg void OnUpdateOptionsVideoVsync(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoX1(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoX2(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoX3(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoX4(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFullscreen320x240(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFullscreen640x480(CCmdUI*pCmdUI);
	afx_msg void OnUpdateOptionsVideoFullscreen800x600(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoFullscreen320x240();
	afx_msg void OnOptionsVideoFullscreen640x480();
	afx_msg void OnOptionsVideoFullscreen800x600();
	afx_msg void OnOptionsVideoFullscreen();
	afx_msg void OnUpdateOptionsVideoFullscreen(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoDisablesfx();
	afx_msg void OnUpdateOptionsVideoDisablesfx(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoFullscreenstretchtofit();
	afx_msg void OnUpdateOptionsVideoFullscreenstretchtofit(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRendermethodGdi();
	afx_msg void OnUpdateOptionsVideoRendermethodGdi(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRendermethodDirectdraw();
	afx_msg void OnUpdateOptionsVideoRendermethodDirectdraw(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRendermethodDirect3d();
	afx_msg void OnUpdateOptionsVideoRendermethodDirect3d(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRendermethodOpengl();
	afx_msg void OnUpdateOptionsVideoRendermethodOpengl(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoTriplebuffering();
	afx_msg void OnUpdateOptionsVideoTriplebuffering(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoDdrawemulationonly();
	afx_msg void OnUpdateOptionsVideoDdrawemulationonly(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoDdrawusevideomemory();
	afx_msg void OnUpdateOptionsVideoDdrawusevideomemory(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsD3dnofilter();
	afx_msg void OnUpdateOptionsVideoRenderoptionsD3dnofilter(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsD3dbilinear();
	afx_msg void OnUpdateOptionsVideoRenderoptionsD3dbilinear(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsGlnearest();
	afx_msg void OnUpdateOptionsVideoRenderoptionsGlnearest(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsGlbilinear();
	afx_msg void OnUpdateOptionsVideoRenderoptionsGlbilinear(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsGltriangle();
	afx_msg void OnUpdateOptionsVideoRenderoptionsGltriangle(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsGlquads();
	afx_msg void OnUpdateOptionsVideoRenderoptionsGlquads(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsSelectskin();
	afx_msg void OnUpdateOptionsVideoRenderoptionsSelectskin(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoRenderoptionsSkin();
	afx_msg void OnUpdateOptionsVideoRenderoptionsSkin(CCmdUI*pCmdUI);

	afx_msg void OnOptionsEmulatorAssociate();
	afx_msg void OnOptionsEmulatorDirectories();
	afx_msg void OnOptionsEmulatorFilenamePreference(UINT nID);
	afx_msg void OnUpdateOptionsEmulatorFilenamePreference(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoDisablestatusmessages();
	afx_msg void OnUpdateOptionsVideoDisablestatusmessages(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSynchronize();
	afx_msg void OnUpdateOptionsEmulatorSynchronize(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorAlwaysOnTop();
	afx_msg void OnUpdateOptionsEmulatorAlwaysOnTop(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorPausewheninactive();
	afx_msg void OnUpdateOptionsEmulatorPausewheninactive(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorEnableBackgroundInput();
	afx_msg void OnUpdateOptionsEmulatorEnableBackgroundInput(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSpeeduptoggle();
	afx_msg void OnUpdateOptionsEmulatorSpeeduptoggle(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorRemoveintrosgba();
	afx_msg void OnUpdateOptionsEmulatorRemoveintrosgba(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorAutomaticallyipspatch();
	afx_msg void OnUpdateOptionsEmulatorAutomaticallyipspatch(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorAgbprint();
	afx_msg void OnUpdateOptionsEmulatorAgbprint(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorRealtimeclock();
	afx_msg void OnUpdateOptionsEmulatorRealtimeclock(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorAutohidemenu();
	afx_msg void OnUpdateOptionsEmulatorAutohidemenu(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorRewindinterval();
	afx_msg void OnOptionsEmulatorSavetypeAutomatic();
	afx_msg void OnUpdateOptionsEmulatorSavetypeAutomatic(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSavetypeEeprom();
	afx_msg void OnUpdateOptionsEmulatorSavetypeEeprom(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSavetypeSram();
	afx_msg void OnUpdateOptionsEmulatorSavetypeSram(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSavetypeFlash();
	afx_msg void OnUpdateOptionsEmulatorSavetypeFlash(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSavetypeEepromsensor();
	afx_msg void OnUpdateOptionsEmulatorSavetypeEepromsensor(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSavetypeNone();
	afx_msg void OnUpdateOptionsEmulatorSavetypeNone(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSavetypeFlash512k();
	afx_msg void OnUpdateOptionsEmulatorSavetypeFlash512k(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSavetypeFlash1m();
	afx_msg void OnUpdateOptionsEmulatorSavetypeFlash1m(CCmdUI*pCmdUI);

#ifdef USE_GBA_CORE_V7
	afx_msg void OnOptionsEmulatorGBALag();
	afx_msg void OnUpdateOptionsEmulatorGBALag(CCmdUI*pCmdUI);
#endif
	afx_msg void OnOptionsEmulatorUseOldGBTiming();
	afx_msg void OnUpdateOptionsEmulatorUseOldGBTiming(CCmdUI*pCmdUI);
#ifdef USE_GB_CORE_V7
	afx_msg void OnOptionsEmulatorUseGBNullInputHack();
	afx_msg void OnUpdateOptionsEmulatorUseGBNullInputHack(CCmdUI*pCmdUI);
#else
	afx_msg void OnOptionsEmulatorUseV20GBFrameHack();
	afx_msg void OnUpdateOptionsEmulatorUseV20GBFrameHack(CCmdUI*pCmdUI);
#endif

	afx_msg void OnOptionsEmulatorUsebiosfile();
	afx_msg void OnUpdateOptionsEmulatorUsebiosfile(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorSkipbios();
	afx_msg void OnUpdateOptionsEmulatorSkipbios(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorPngformat();
	afx_msg void OnUpdateOptionsEmulatorPngformat(CCmdUI*pCmdUI);
	afx_msg void OnOptionsEmulatorBmpformat();
	afx_msg void OnUpdateOptionsEmulatorBmpformat(CCmdUI*pCmdUI);

	afx_msg void OnOptionsSoundDisable();
	afx_msg void OnUpdateOptionsSoundDisable(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundMute();
	afx_msg void OnUpdateOptionsSoundMute(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundOff();
	afx_msg void OnUpdateOptionsSoundOff(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundOn();
	afx_msg void OnUpdateOptionsSoundOn(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundUseoldsynchronization();
	afx_msg void OnUpdateOptionsSoundUseoldsynchronization(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundEcho();
	afx_msg void OnUpdateOptionsSoundEcho(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundLowpassfilter();
	afx_msg void OnUpdateOptionsSoundLowpassfilter(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundReversestereo();
	afx_msg void OnUpdateOptionsSoundReversestereo(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundMuteFrameAdvance();
	afx_msg void OnUpdateOptionsSoundMuteFrameAdvance(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundMuteWhenInactive();
	afx_msg void OnUpdateOptionsSoundMuteWhenInactive(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSound11khz();
	afx_msg void OnUpdateOptionsSound11khz(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSound22khz();
	afx_msg void OnUpdateOptionsSound22khz(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSound44khz();
	afx_msg void OnUpdateOptionsSound44khz(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundChannel1();
	afx_msg void OnUpdateOptionsSoundChannel1(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundChannel2();
	afx_msg void OnUpdateOptionsSoundChannel2(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundChannel3();
	afx_msg void OnUpdateOptionsSoundChannel3(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundChannel4();
	afx_msg void OnUpdateOptionsSoundChannel4(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundDirectsounda();
	afx_msg void OnUpdateOptionsSoundDirectsounda(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundDirectsoundb();
	afx_msg void OnUpdateOptionsSoundDirectsoundb(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyBorder();
	afx_msg void OnUpdateOptionsGameboyBorder(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyPrinter();
	afx_msg void OnUpdateOptionsGameboyPrinter(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyBorderAutomatic();
	afx_msg void OnUpdateOptionsGameboyBorderAutomatic(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyAutomatic();
	afx_msg void OnUpdateOptionsGameboyAutomatic(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyGba();
	afx_msg void OnUpdateOptionsGameboyGba(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyCgb();
	afx_msg void OnUpdateOptionsGameboyCgb(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboySgb();
	afx_msg void OnUpdateOptionsGameboySgb(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboySgb2();
	afx_msg void OnUpdateOptionsGameboySgb2(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyGb();
	afx_msg void OnUpdateOptionsGameboyGb(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyRealcolors();
	afx_msg void OnUpdateOptionsGameboyRealcolors(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyGameboycolors();
	afx_msg void OnUpdateOptionsGameboyGameboycolors(CCmdUI*pCmdUI);
	afx_msg void OnOptionsGameboyColors();
	afx_msg void OnOptionsFilterDisablemmx();
	afx_msg void OnUpdateOptionsFilterDisablemmx(CCmdUI*pCmdUI);
	afx_msg void OnOptionsLanguageSystem();
	afx_msg void OnUpdateOptionsLanguageSystem(CCmdUI*pCmdUI);
	afx_msg void OnOptionsLanguageEnglish();
	afx_msg void OnUpdateOptionsLanguageEnglish(CCmdUI*pCmdUI);
	afx_msg void OnOptionsLanguageOther();
	afx_msg void OnUpdateOptionsLanguageOther(CCmdUI*pCmdUI);
	afx_msg void OnOptionsJoypadConfigure1();
	afx_msg void OnUpdateOptionsJoypadConfigure1(CCmdUI*pCmdUI);
	afx_msg void OnOptionsJoypadConfigure2();
	afx_msg void OnUpdateOptionsJoypadConfigure2(CCmdUI*pCmdUI);
	afx_msg void OnOptionsJoypadConfigure3();
	afx_msg void OnUpdateOptionsJoypadConfigure3(CCmdUI*pCmdUI);
	afx_msg void OnOptionsJoypadConfigure4();
	afx_msg void OnUpdateOptionsJoypadConfigure4(CCmdUI*pCmdUI);
	afx_msg void OnOptionsJoypadMotionconfigure();
	afx_msg void OnUpdateOptionsJoypadMotionconfigure(CCmdUI*pCmdUI);
	afx_msg void OnOptionsJoypadAllowLeftRight();
	afx_msg void OnUpdateOptionsJoypadAllowLeftRight(CCmdUI*pCmdUI);
	afx_msg void OnOptionsJoypadAutofireAccountForLag();
	afx_msg void OnUpdateOptionsJoypadAutofireAccountForLag(CCmdUI*pCmdUI);
	afx_msg void OnCheatsSearchforcheats();
	afx_msg void OnUpdateCheatsSearchforcheats(CCmdUI*pCmdUI);
	afx_msg void OnCheatsCheatlist();
	afx_msg void OnUpdateCheatsCheatlist(CCmdUI*pCmdUI);
	afx_msg void OnCheatsAutomaticsaveloadcheats();
	afx_msg void OnCheatsPauseDuringCheatSearch();
	afx_msg void OnCheatsLoadcheatlist();
	afx_msg void OnUpdateCheatsLoadcheatlist(CCmdUI*pCmdUI);
	afx_msg void OnCheatsSavecheatlist();
	afx_msg void OnUpdateCheatsSavecheatlist(CCmdUI*pCmdUI);
	afx_msg void OnToolsDisassemble();
	afx_msg void OnUpdateToolsDisassemble(CCmdUI*pCmdUI);
	afx_msg void OnToolsLogging();
	afx_msg void OnUpdateToolsLogging(CCmdUI*pCmdUI);
	afx_msg void OnToolsIoviewer();
	afx_msg void OnUpdateToolsIoviewer(CCmdUI*pCmdUI);
	afx_msg void OnToolsMapview();
	afx_msg void OnUpdateToolsMapview(CCmdUI*pCmdUI);
	afx_msg void OnToolsMemoryviewer();
	afx_msg void OnUpdateToolsMemoryviewer(CCmdUI*pCmdUI);
	afx_msg void OnToolsOamviewer();
	afx_msg void OnUpdateToolsOamviewer(CCmdUI*pCmdUI);
	afx_msg void OnToolsPaletteview();
	afx_msg void OnUpdateToolsPaletteview(CCmdUI*pCmdUI);
	afx_msg void OnToolsTileviewer();
	afx_msg void OnUpdateToolsTileviewer(CCmdUI*pCmdUI);
	afx_msg void OnDebugNextframe();
	afx_msg void OnUpdateDebugNextframe(CCmdUI*pCmdUI);
	afx_msg void OnDebugNextframeAccountForLag();
	afx_msg void OnUpdateDebugNextframeAccountForLag(CCmdUI*pCmdUI);
	afx_msg void OnDebugFramesearch();
	afx_msg void OnUpdateDebugFramesearch(CCmdUI*pCmdUI);
	afx_msg void OnDebugFramesearchPrev();
	afx_msg void OnUpdateDebugFramesearchPrev(CCmdUI*pCmdUI);
	afx_msg void OnDebugFramesearchLoad();
	afx_msg void OnUpdateDebugFramesearchLoad(CCmdUI*pCmdUI);
	afx_msg void OnUpdateCheatsAutomaticsaveloadcheats(CCmdUI*pCmdUI);
	afx_msg void OnUpdateCheatsPauseDuringCheatSearch(CCmdUI*pCmdUI);
	afx_msg void OnToolsFrameCounter();
	afx_msg void OnUpdateToolsFrameCounter(CCmdUI*pCmdUI);
	afx_msg void OnToolsLagCounter();
	afx_msg void OnUpdateToolsLagCounter(CCmdUI*pCmdUI);
	afx_msg void OnToolsExtraCounter();
	afx_msg void OnUpdateToolsExtraCounter(CCmdUI*pCmdUI);
	afx_msg void OnToolsExtraCounterReset();
	afx_msg void OnToolsInputDisplay();
	afx_msg void OnUpdateToolsInputDisplay(CCmdUI*pCmdUI);
	afx_msg void OnToolsNextInputDisplay();
	afx_msg void OnUpdateToolsNextInputDisplay(CCmdUI*pCmdUI);
	afx_msg void OnToolsDebugGdb();
	afx_msg void OnUpdateToolsDebugGdb(CCmdUI*pCmdUI);
	afx_msg void OnToolsDebugLoadandwait();
	afx_msg void OnUpdateToolsDebugLoadandwait(CCmdUI*pCmdUI);
	afx_msg void OnToolsDebugBreak();
	afx_msg void OnUpdateToolsDebugBreak(CCmdUI*pCmdUI);
	afx_msg void OnToolsDebugDisconnect();
	afx_msg void OnUpdateToolsDebugDisconnect(CCmdUI*pCmdUI);
	afx_msg void OnToolsSoundStartrecording();
	afx_msg void OnToolsSoundStoprecording();
	afx_msg void OnToolsSoundRecording();
	afx_msg void OnUpdateToolsSoundRecording(CCmdUI*pCmdUI);
	afx_msg void OnToolsStartAVIRecording();
	afx_msg void OnToolsPauseAVIRecording();
	afx_msg void OnToolsStopAVIRecording();
	afx_msg void OnToolsAVIRecording();
	afx_msg void OnUpdateToolsAVIRecording(CCmdUI*pCmdUI);
	afx_msg void OnUpdateToolsPauseAVIRecording(CCmdUI*pCmdUI);

	afx_msg void OnToolsRecordMovie();
	afx_msg void OnUpdateToolsRecordMovie(CCmdUI*pCmdUI);
	afx_msg void OnToolsStopMovie();
	afx_msg void OnUpdateToolsStopMovie(CCmdUI*pCmdUI);
	afx_msg void OnToolsPlayMovie();
	afx_msg void OnUpdateToolsPlayMovie(CCmdUI*pCmdUI);
	afx_msg void OnToolsPlayReadOnly();
	afx_msg void OnUpdateToolsPlayReadOnly(CCmdUI*pCmdUI);
	afx_msg void OnAsscWithSaveState();
	afx_msg void OnUpdateAsscWithSaveState(CCmdUI*pCmdUI);
	afx_msg void OnToolsResumeRecord();
	afx_msg void OnUpdateToolsResumeRecord(CCmdUI*pCmdUI);
	afx_msg void OnToolsPlayRestart();
	afx_msg void OnUpdateToolsPlayRestart(CCmdUI*pCmdUI);

	afx_msg void OnToolsEditModePrev();
	afx_msg void OnUpdateToolsEditModePrev(CCmdUI*pCmdUI);
	afx_msg void OnToolsEditModeNext();
	afx_msg void OnUpdateToolsEditModeNext(CCmdUI*pCmdUI);
	afx_msg void OnToolsEditModeDiscard();
	afx_msg void OnUpdateToolsEditModeDiscard(CCmdUI*pCmdUI);
	afx_msg void OnToolsEditModeOverwrite();
	afx_msg void OnUpdateToolsEditModeOverwrite(CCmdUI*pCmdUI);
	afx_msg void OnToolsEditModeInsert();
	afx_msg void OnUpdateToolsEditModeInsert(CCmdUI*pCmdUI);
	afx_msg void OnToolsEditModeXor();
	afx_msg void OnUpdateToolsEditModeXor(CCmdUI*pCmdUI);

	afx_msg void OnToolsOnMovieEndPause();
	afx_msg void OnUpdateToolsOnMovieEndPause(CCmdUI*pCmdUI);
	afx_msg void OnToolsOnMovieEndStop();
	afx_msg void OnUpdateToolsOnMovieEndStop(CCmdUI*pCmdUI);
	afx_msg void OnToolsOnMovieEndRestart();
	afx_msg void OnUpdateToolsOnMovieEndRestart(CCmdUI*pCmdUI);
	afx_msg void OnToolsOnMovieEndAppend();
	afx_msg void OnUpdateToolsOnMovieEndAppend(CCmdUI*pCmdUI);
	afx_msg void OnToolsOnMovieEndKeep();
	afx_msg void OnUpdateToolsOnMovieEndKeep(CCmdUI*pCmdUI);

	afx_msg void OnToolsMovieSetPauseAt();
	afx_msg void OnUpdateToolsSetMoviePauseAt(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieConvertCurrent();
	afx_msg void OnUpdateToolsMovieConvertCurrent(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieAutoConvert();
	afx_msg void OnUpdateToolsMovieAutoConvert(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieFixHeader();
	afx_msg void OnUpdateToolsMovieFixHeader(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieInsertOneFrame();
	afx_msg void OnUpdateToolsMovieInsertOneFrame(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieDeleteOneFrame();
	afx_msg void OnUpdateToolsMovieDeleteOneFrame(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieInsertFrames();
	afx_msg void OnUpdateToolsMovieInsertFrames(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieDeleteFrames();
	afx_msg void OnUpdateToolsMovieDeleteFrames(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieTruncateAtCurrent();
	afx_msg void OnUpdateToolsMovieTruncateAtCurrent(CCmdUI*pCmdUI);
	afx_msg void OnToolsMovieExtractFromSavegame();
	afx_msg void OnUpdateToolsMovieExtractFromSavegame(CCmdUI*pCmdUI);

	afx_msg void OnToolsRewind();
	afx_msg void OnUpdateToolsRewind(CCmdUI*pCmdUI);
	afx_msg void OnToolsCustomize();
	afx_msg void OnUpdateToolsCustomize(CCmdUI*pCmdUI);
//	afx_msg void OnToolsCustomizeCommon();
//	afx_msg void OnUpdateToolsCustomizeCommon(CCmdUI*pCmdUI);
	afx_msg void OnToolsCopyVBAWatchSetting();
	afx_msg void OnToolsCopyVBxWatchSetting();
	afx_msg void OnHelpBugreport();

	afx_msg void OnFileSavegameOldestslot();
	afx_msg void OnUpdateFileSavegameOldestslot(CCmdUI*pCmdUI);
	afx_msg void OnFileLoadgameMostrecent();
	afx_msg void OnUpdateFileLoadgameMostrecent(CCmdUI*pCmdUI);
	afx_msg void OnFileLoadgameAutoloadmostrecent();
	afx_msg void OnUpdateFileLoadgameAutoloadmostrecent(CCmdUI*pCmdUI);
	afx_msg void OnFileLoadgameMakeRecent();
	afx_msg void OnUpdateFileLoadgameMakeRecent(CCmdUI*pCmdUI);

	afx_msg void OnFileSavegameCurrent();
	afx_msg void OnUpdateFileSavegameCurrent(CCmdUI*pCmdUI);
	afx_msg void OnFileLoadgameCurrent();
	afx_msg void OnUpdateFileLoadgameCurrent(CCmdUI*pCmdUI);
	afx_msg void OnFileLoadgameMakeCurrent();
	afx_msg void OnUpdateFileLoadgameMakeCurrent(CCmdUI*pCmdUI);
	afx_msg void OnFileSavegameMakeCurrent();
	afx_msg void OnUpdateFileSavegameMakeCurrent(CCmdUI*pCmdUI);
	afx_msg void OnFileSavegameIncrementSlot();
	afx_msg void OnUpdateFileSavegameIncrementSlot(CCmdUI*pCmdUI);
	afx_msg void OnFileSavegameDecrementSlot();
	afx_msg void OnUpdateFileSavegameDecrementSlot(CCmdUI*pCmdUI);
	afx_msg void OnFileSlotDisplayModificationTime();
	afx_msg void OnUpdateFileSlotDisplayModificationTime(CCmdUI*pCmdUI);

	afx_msg void OnOptionsSoundVolume25x();
	afx_msg void OnUpdateOptionsSoundVolume25x(CCmdUI*pCmdUI);
	afx_msg void OnOptionsSoundVolume5x();
	afx_msg void OnUpdateOptionsSoundVolume5x(CCmdUI*pCmdUI);
	afx_msg void OnCheatsDisablecheats();
	afx_msg void OnUpdateCheatsDisablecheats(CCmdUI*pCmdUI);
	afx_msg void OnOptionsVideoFullscreenmaxscale();
	afx_msg void OnOptionsVideoTextdisplayoptions();
	afx_msg void OnUpdateOptionsVideoTextdisplayoptions(CCmdUI *pCmdUI);

	afx_msg void OnUpdateFileRecentFile(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileLoadSlot(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSaveSlot(CCmdUI *pCmdUI);
	afx_msg void OnUpdateSelectSlot(CCmdUI *pCmdUI);
	afx_msg BOOL OnFileRecentFile(UINT nID);
	afx_msg BOOL OnFileLoadSlot(UINT nID);
	afx_msg BOOL OnFileSaveSlot(UINT nID);
	afx_msg BOOL OnSelectSlot(UINT nID);

	afx_msg void OnFileLuaOpen();
	afx_msg void OnUpdateFileLuaOpen(CCmdUI *pCmdUI);
	afx_msg void OnFileLuaCloseAll();
	afx_msg void OnUpdateFileLuaCloseAll(CCmdUI *pCmdUI);
	afx_msg void OnFileLuaReload();
	afx_msg void OnFileLuaStop();
	afx_msg void OnFileRamSearch();
	afx_msg void OnUpdateFileRamSearch(CCmdUI *pCmdUI);
	afx_msg void OnFileRamWatch();
	afx_msg void OnUpdateFileRamWatch(CCmdUI *pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINWND_H__E8AD28B9_C9FB_4EC2_A2DC_DD1BBA55A275__INCLUDED_)
