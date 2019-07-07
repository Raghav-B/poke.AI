// VBA.cpp : Defines the class behaviors for the application.
//
#include "stdafx.h"
#include <mmsystem.h>
#include <cassert>

#include "resource.h"
#include "VBA.h"
#include "AVIWrite.h"
#include "Input.h"
#include "IUpdate.h"
#include "MainWnd.h"
#include "Reg.h"
#include "WavWriter.h"
#include "WinResUtil.h"
#include "WinMiscUtil.h"
#include "Dialogs/LangSelect.h"
#include "Dialogs/ramwatch.h"

#include "../gba/GBA.h"
#include "../gba/GBAGlobals.h"
#include "../gba/agbprint.h"
#include "../gb/GB.h"
#include "../gb/gbGlobals.h"
#include "../gb/gbPrinter.h"
#include "../common/CheatSearch.h"
#include "../gba/RTC.h"
#include "../common/SystemGlobals.h"
#include "../common/Util.h"
#include "../common/Text.h"
#include "../common/movie.h"
#include "../common/nesvideos-piece.h"
#include "../common/vbalua.h"
#include "../filters/filters.h"
#include "../version.h"

extern IDisplay *newGDIDisplay();
extern IDisplay *newDirectDrawDisplay();
extern IDisplay *newDirect3DDisplay();
extern IDisplay *newOpenGLDisplay();

extern Input *newDirectInput();

extern void remoteStubSignal(int, int);
extern void remoteOutput(char *, u32);
extern void remoteStubMain();
extern void remoteSetProtocol(int);
extern void remoteCleanUp();

extern void winlog(const char *msg, ...);

bool debugger = false;

char movieFileToPlay[1024];
bool playMovieFile		   = false;
bool playMovieFileReadOnly = false;
char wavFileToOutput [1024];
bool outputWavFile	= false;
bool outputAVIFile	= false;
bool flagHideMenu	= false;
int	 quitAfterTime	= -1;
int	 pauseAfterTime = -1;

#ifdef MMX
extern "C" bool cpu_mmx;
#endif

// nowhere good to put them to

void DrawTextMessages(u8 *dest, int pitch, int left, int bottom)
{
	for (int slot = 0; slot < SCREEN_MESSAGE_SLOTS; slot++)
	{
		if (theApp.screenMessage[slot])
		{
			if ((theApp.screenMessageDuration[slot] < 0 ||
			     (int)(GetTickCount() - theApp.screenMessageTime[slot]) < theApp.screenMessageDuration[slot]) &&
			    (!theApp.disableStatusMessage || slot == 1 || slot == 2))
			{
				drawText(dest,
				         pitch,
				         left,
				         bottom - 10 * (slot + 1),
				         theApp.screenMessageBuffer[slot],
				         theApp.screenMessageColorBuffer[slot]);
			}
			else
			{
				theApp.screenMessage[slot] = false;
			}
		}
	}
}

void directXMessage(const char *msg)
{
	systemMessage(
	    IDS_DIRECTX_7_REQUIRED,
	    "DirectX 7.0 or greater is required to run.\nDownload at http://www.microsoft.com/directx.\n\nError found at: %s",
	    msg);
}

void winlog(const char *msg, ...)
{
	CString buffer;
	va_list valist;

	va_start(valist, msg);
	buffer.FormatV(msg, valist);

	FILE *winout = fopen("vba-trace.log", "w");

	fputs(buffer, winout);

	fclose(winout);

	va_end(valist);
}

// code from SDL_main.c for Windows
/* Parse a command line buffer into arguments */

static int parseCommandLine(char *cmdline, char * *argv)
{
	char *bufp;
	int	  argc;

	argc = 0;
	for (bufp = cmdline; *bufp; )
	{
		/* Skip leading whitespace */
		while (isspace(*bufp))
		{
			++bufp;
		}
		/* Skip over argument */
		if (*bufp == '"')
		{
			++bufp;
			if (*bufp)
			{
				if (argv)
				{
					argv[argc] = bufp;
				}
				++argc;
			}
			/* Skip over word */
			while (*bufp && (*bufp != '"'))
			{
				++bufp;
			}
		}
		else
		{
			if (*bufp)
			{
				if (argv)
				{
					argv[argc] = bufp;
				}
				++argc;
			}
			/* Skip over word */
			while (*bufp && !isspace(*bufp))
			{
				++bufp;
			}
		}
		if (*bufp)
		{
			if (argv)
			{
				*bufp = '\0';
			}
			++bufp;
		}
	}
	if (argv)
	{
		argv[argc] = NULL;
	}
	return(argc);
}

static void debugSystemScreenMessage1(const char *msg)
{
	systemScreenMessage(msg, 3);
}

static void debugSystemScreenMessage2(const char *msg)
{
	systemScreenMessage(msg, 4);
}

typedef BOOL (WINAPI * GETMENUBARINFO)(HWND, LONG, LONG, PMENUBARINFO);

static int winGetMenuBarHeight()
{
	HINSTANCE hinstDll = /**/ ::LoadLibrary("USER32.DLL");

	if (hinstDll)
	{
		GETMENUBARINFO func = (GETMENUBARINFO)GetProcAddress(hinstDll, "GetMenuBarInfo");

		if (func)
		{
			MENUBARINFO info;
			info.cbSize = sizeof(info);

			func(AfxGetMainWnd()->GetSafeHwnd(), OBJID_MENU, 0, &info);

			/**/ ::FreeLibrary(hinstDll);

			return info.rcBar.bottom - info.rcBar.top + 1;
		}
	}

	return GetSystemMetrics(SM_CYMENU);
}

/////////////////////////////////////////////////////////////////////////////
// VBA

BEGIN_MESSAGE_MAP(VBA, CWinApp)
//{{AFX_MSG_MAP(VBA)
// NOTE - the ClassWizard will add and remove mapping macros here.
//    DO NOT EDIT what you see in these blocks of generated code!
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// The one and only VBA object

VBA theApp;

/////////////////////////////////////////////////////////////////////////////
// VBA construction

VBA::VBA() : emulator(::theEmulator)
{
	// important
	{
#ifdef MULTITHREAD_STDLOCALE_WORKAROUND
		// Note: there's a known threading bug regarding std::locale with MSVC according to
		// http://connect.microsoft.com/VisualStudio/feedback/details/492128/std-locale-constructor-modifies-global-locale-via-setlocale
		int iPreviousFlag = ::_configthreadlocale(_ENABLE_PER_THREAD_LOCALE);
#endif
		using std::locale;
		locale::global(locale(locale::classic(), "", locale::collate | locale::ctype));

#ifdef MULTITHREAD_STDLOCALE_WORKAROUND
		if (iPreviousFlag > 0 )
			::_configthreadlocale(iPreviousFlag);
#endif
	}

	mode320Available	 = false;
	mode640Available	 = false;
	mode800Available	 = false;
	windowPositionX		 = 0;
	windowPositionY		 = 0;
	filterFunction		 = NULL;
	ifbFunction			 = NULL;
	ifbType				 = 0;
	filterType			 = 0;
	filterWidth			 = 0;
	filterHeight		 = 0;
	fsWidth				 = 0;
	fsHeight			 = 0;
	fsColorDepth		 = 0;
	fsForceChange		 = false;
	surfaceSizeX		 = 0;
	surfaceSizeY		 = 0;
	sizeX				 = 0;
	sizeY				 = 0;
	scale				 = 1.0;
	videoOption			 = 0;
	fullScreenStretch	 = false;
	disableStatusMessage = false;
	showSpeed			 = 1;
	showSpeedTransparent = true;
	showRenderedFrames	 = 0;
	for (int j = 0; j < SCREEN_MESSAGE_SLOTS; j++)
	{
		screenMessage[j]		 = false;
		screenMessageTime[j]	 = 0;
		screenMessageDuration[j] = 0;
	}
	menuToggle		 = true;
	display			 = NULL;
	menu			 = NULL;
	popup			 = NULL;
	soundInitialized = false;
	useBiosFile		 = false;
	skipBiosIntro	 = false;
	active			 = true;
	paused			 = false;
	recentFreeze	 = false;
	autoSaveLoadCheatList	  = false;
	pauseDuringCheatSearch	  = false;
	modelessCheatDialogIsOpen = false;
//	winout						= NULL;
//	removeIntros				= false;
	autoIPS = true;
	winGbBorderOn	  = 0;
	hideMovieBorder	  = false;
	winFlashSize	  = 0x10000;
	winRtcEnable	  = false;
	winSaveType		  = 0;
	rewindMemory	  = NULL;
	frameSearchMemory = NULL;
	rewindPos		  = 0;
	rewindTopPos	  = 0;
	rewindCounter	  = 0;
	rewindCount		  = 0;
	rewindSaveNeeded  = false;
	rewindTimer		  = 0;
	captureFormat	  = 0;
	tripleBuffering	  = true;
	autoHideMenu	  = false;
	throttle		  = 100;
	throttleLastTime  = 0;
///  autoFrameSkipLastTime		= 0;
///  autoFrameSkip				= false;
	vsync = false;
	changingVideoSize = false;
	pVideoDriverGUID  = NULL;
	renderMethod	  = DIRECT_DRAW;
	iconic = false;
	ddrawEmulationOnly		= false;
	ddrawUsingEmulationOnly = false;
	ddrawDebug				= false;
	ddrawUseVideoMemory		= false;
	d3dFilter				= 0;
	glFilter				= 0;
	glType					= 0;
	regEnabled				= false;
	pauseWhenInactive		= true;
	muteWhenInactive		= true;
	enableBackgroundInput	= false;
	alwaysOnTop				= false;
	filenamePreference		= true;
	frameCounter			= false;
	lagCounter				= false;
	extraCounter			= false;
	inputDisplay			= false;
	nextInputDisplay		= false;
	speedupToggle			= false;
	useOldSync				= false;
	allowLeftRight			= false;
	autofireAccountForLag	= false;
	nextframeAccountForLag	= false;
	muteFrameAdvance		= false;
	muteWhenInactive		= false;
	winMuteForNow		= false;
	winGbPrinterEnabled		= false;
	threadPriority			= 2;
	disableMMX				= false;
	languageOption			= 0;
	languageModule			= NULL;
	languageName			= "";
	renderedFrames			= 0;
	input					= NULL;
	joypadDefault			= 0;
	autoFire				= 0;
	autoFire2				= 0;
	autoHold				= 0;
	autoFireToggle			= false;
	winPauseNextFrame		= false;
	soundRecording			= false;
	soundRecorder			= NULL;
	sound					= NULL;
	aviRecording			= false;
	aviRecorder				= NULL;
	painting				= false;
	mouseCounter			= 0;
	movieReadOnly			= true;
	movieEditMode			= false;
	movieOnEndPause			= false;
	movieOnEndBehavior		= 0;
	wasPaused				= false;
	fsMaxScale				= 0;
	romSize					= 0;
	autoLoadMostRecent		= false;
	loadMakesRecent			= false;
	loadMakesCurrent		= false;
	saveMakesCurrent		= false;
	currentSlot				= 0;
	showSlotTime			= false;
	frameSearchLoadValid	= false;
	frameSearching			= false;
	frameSearchSkipping		= false;
	nvVideoLog				= false;
	nvAudioLog				= false;
	LoggingEnabled			= 0;

	updateCount = 0;

	systemSaveUpdateCounter = SYSTEM_SAVE_NOT_UPDATED;

	ZeroMemory(&emulator, sizeof(emulator));

	hAccel = NULL;

	for (int i = 0; i < 24; )
	{
		systemGbPalette[i++] = (0x1f) | (0x1f << 5) | (0x1f << 10);
		systemGbPalette[i++] = (0x15) | (0x15 << 5) | (0x15 << 10);
		systemGbPalette[i++] = (0x0c) | (0x0c << 5) | (0x0c << 10);
		systemGbPalette[i++] = 0;
	}

	VBAMovieInit();

	TIMECAPS tc;
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR)
	{
		wmTimerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
		timeBeginPeriod(wmTimerRes);
	}
	else
	{
		wmTimerRes = 5;
		timeBeginPeriod(wmTimerRes);
	}
}

VBA::~VBA()
{
	if (VBAMovieIsActive())
		VBAMovieStop(true);

	saveSettings();

	InterframeCleanup();

	if (aviRecorder)
	{
		delete aviRecorder;
		aviRecorder	 = NULL;
		aviRecording = false;
	}

	if (soundRecorder)
	{
		delete soundRecorder;
		soundRecorder = NULL;
	}
	soundRecording = false;
	systemSoundPause();
	systemSoundShutdown();

	((MainWnd *)(m_pMainWnd))->winFileClose();

	if (input)
		delete input;

	shutdownDisplay();

	if (rewindMemory)
		free(rewindMemory);

	if (frameSearchMemory)
		free(frameSearchMemory);

	timeEndPeriod(wmTimerRes);
}

/////////////////////////////////////////////////////////////////////////////
// VBA initialization

#include <afxdisp.h>

BOOL VBA::InitInstance()
{
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

//#ifdef _AFXDLL
//  Enable3dControls();      // Call this when using MFC in a shared DLL
//#else
//  Enable3dControlsStatic();  // Call this when linking to MFC statically
//#endif

	SetRegistryKey(_T("VBA"));

	remoteSetProtocol(0);

	systemVerbose = GetPrivateProfileInt("config", "verbose", 0, "VBA.ini");
	systemDebug = GetPrivateProfileInt("config", "debug", 0, "VBA.ini");
	ddrawDebug = GetPrivateProfileInt("config", "ddrawDebug", 0, "VBA.ini") ? true : false;

	wndClass = AfxRegisterWndClass(0, LoadCursor(IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), LoadIcon(IDI_ICON));

	char winBuffer[2048];
	GetModuleFileName(NULL, winBuffer, 2048);
	char *p = strrchr(winBuffer, '\\');
	if (p)
		*p = 0;
	exeDir = winBuffer;

	regInit(winBuffer);

	loadSettings();
	theApp.LuaFastForward = -1;
	if (!initInput())
		return FALSE;

	if (!initDisplay())
	{
		if (videoOption >= VIDEO_320x240)
		{
			regSetDwordValue("video", VIDEO_1X);
			if (pVideoDriverGUID)
				regSetDwordValue("defaultVideoDriver", TRUE);
		}
		return FALSE;
	}

	hAccel = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR));

	winAccelMgr.Connect((MainWnd *)m_pMainWnd);

	extern void winAccelAddCommandsFromMenu(CAcceleratorManager & mgr, CMenu * pMenu, const CString &parentStr);
	extern void winAccelAddCommandsFromTable(CAcceleratorManager & mgr);

	winAccelAddCommandsFromMenu(winAccelMgr, &m_menu, CString());
	winAccelAddCommandsFromTable(winAccelMgr);

	winAccelMgr.CreateDefaultTable();
	winAccelMgr.Load();
	winAccelMgr.UpdateWndTable();
	winAccelMgr.UpdateMenu(menu);

	if (m_lpCmdLine[0])
	{
		int		argc = parseCommandLine(m_lpCmdLine, NULL);
		char * *argv = (char * *)malloc((argc + 1) * sizeof(char *));
		parseCommandLine(m_lpCmdLine, argv);

		bool gotFlag = false, enoughArgs = false;
		for (int i = 0; i < argc; i++)
		{
			if (argv[i][0] == '-' || gotFlag)
			{
				if (!gotFlag)
					loadSettings();
				gotFlag = true;
				if (_stricmp(argv[i], "-rom") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					romFilename = argv[++i];
					winCorrectPath(romFilename);
					gameFilename = romFilename;
				}
				else if (_stricmp(argv[i], "-bios") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					biosFileName = argv[++i];
					winCorrectPath(biosFileName);

					//systemLoadBIOS();
				}
				else if (_stricmp(argv[i], "-frameskip") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					frameSkip = atoi(argv[++i]);
					if (frameSkip < 0)
						frameSkip = 0;
					if (frameSkip > 9)
						frameSkip = 9;
					gbFrameSkip = frameSkip;
				}
				else if (_stricmp(argv[i], "-throttle") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					throttle = atoi(argv[++i]);
					if (throttle < 5)
						throttle = 5;
					if (throttle > 1000)
						throttle = 1000;
				}
				else if (_stricmp(argv[i], "-throttleKeepPitch") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					accuratePitchThrottle = atoi(argv[++i]) != 0;
				}
				else if (_stricmp(argv[i], "-synchronize") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					synchronize = atoi(argv[++i]) != 0;
				}
				else if (_stricmp(argv[i], "-hideborder") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					hideMovieBorder = atoi(argv[++i]) != 0;
				}
				else if (_stricmp(argv[i], "-play") == 0)
				{
					playMovieFile = true;
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					strcpy(movieFileToPlay, argv[++i]);
					winCorrectPath(movieFileToPlay);
					if (i + 1 >= argc || argv[i + 1][0] == '-') { --i; goto invalidArgument; }
					playMovieFileReadOnly = atoi(argv[++i]) != 0;
				}
				else if (_stricmp(argv[i], "-videoLog") == 0)
				{
					nvVideoLog	   = true;
					nvAudioLog	   = true;
					LoggingEnabled = 2;
					if (i + 1 >= argc || argv[i + 1][0] == '-') {}
					else
						NESVideoSetVideoCmd(argv[++i]);
				}
				else if (_stricmp(argv[i], "-logDebug") == 0)
				{
					NESVideoEnableDebugging(debugSystemScreenMessage1, debugSystemScreenMessage2);
				}
				else if (_stricmp(argv[i], "-logToFile") == 0)
				{
					NESVideoSetFileFuncs(fopen, fclose);
				}
				else if (_stricmp(argv[i], "-outputWAV") == 0)
				{
					outputWavFile = true;
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					strcpy(wavFileToOutput, argv[++i]);
				}
				else if (_stricmp(argv[i], "-outputAVI") == 0)
				{
					outputAVIFile = true;
				}
				else if (_stricmp(argv[i], "-quitAfter") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					quitAfterTime = atoi(argv[++i]);
				}
				else if (_stricmp(argv[i], "-pauseAt") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					pauseAfterTime = atoi(argv[++i]);
				}
				else if (_stricmp(argv[i], "-videoScale") == 0)
				{
					if (i + 1 >= argc || argv[i + 1][0] == '-')
						goto invalidArgument;
					int size = atoi(argv[++i]);
					if (size < 1)
						size = 1;
					if (size > 4)
						size = 4;
					switch (size)
					{
					case 1:
						videoOption = VIDEO_1X; break;
					case 2:
						videoOption = VIDEO_2X; break;
					case 3:
						videoOption = VIDEO_3X; break;
					case 4:
						videoOption = VIDEO_4X; break;
					}
				}
				else if (_stricmp(argv[i], "-hideMenu") == 0)
				{
					flagHideMenu = true;
				}
				else
				{
					enoughArgs = true;
invalidArgument:
					char str [2048];    // the string is larger than 1024 bytes
					strcpy(str, "");
					if (_stricmp(argv[i], "-h") != 0)
						if (enoughArgs)
							sprintf(str, "Invalid commandline argument %d: %s\n", i, argv[i]);
						else
							sprintf(str, "Not enough arguments for arg %d: %s\n", i, argv[i]);
					strcat(str, "Valid commands:\n"
					            "-h \t\t\t displays this help\n"
					            "-rom filename \t\t opens the given ROM\n"
					            "-bios filename \t\t use the given GBA BIOS\n"
					            "-play filename val \t\t plays the given VBM movie (val: 1 = read-only, 0 = editable)\n"
					            "-outputWAV filename \t outputs WAV audio to the given file\n"
					            "-outputAVI \t\t outputs an AVI (you are prompted for location and codec)\n"
					            "-frameskip val \t\t sets the frameskip amount to the given value\n"
					            "-synchronize val \t\t limits running speed to sound playing speed, (0 = off, 1 = on)\n"
					            "-throttle val \t\t sets the throttle speed to the given percentage\n"
					            "-hideborder val \t\t hides SGB border, if any (0 = show, 1 = hide)\n"
					            "-throttleKeepPitch val \t if throttle and synch, don't change sound freq (0 = off, 1 = on)\n"
					            "-quitAfter val \t\t close program when frame counter == val\n"
					            "-pauseAt val \t\t pause (movie) once when frame counter == val\n"
					            "-videoScale val \t\t sets the video size (val = 1 for 1X, 2 for 2X, 3 for 3X, or 4 for 4X)\n"
					            "-hideMenu \t\t hides the menu until program exit\n"
					            "\n"
					            "-videoLog args \t does (nesvideos) video+audio logging with the given arguments\n"
					            "-logToFile \t tells logging to use fopen/fclose of args, if logging is enabled\n"
					            "-logDebug  \t tells logging to output debug info to screen, if logging is enabled\n"
					       );
					theApp.winCheckFullscreen();
					AfxGetApp()->m_pMainWnd->MessageBox(str, "Commandline Help", MB_OK | MB_ICONINFORMATION);
					exit(0);
				}
			}
			else
			{
				// assume anything else is a ROM, for backward compatibility
				romFilename	 = argv[i++];
				gameFilename = romFilename;
				loadSettings();
			}
		}

/*
        int index = filename.ReverseFind('.');

        if (index != -1)
            filename = filename.Left(index);
 */
		if (romFilename.GetLength() > 0)
		{
			((MainWnd *)theApp.m_pMainWnd)->winFileRun();
		}
		free(argv);
	}

	return TRUE;
}

void VBA::adjustDestRect()
{
	POINT point;

	point.x = 0;
	point.y = 0;

	m_pMainWnd->ClientToScreen(&point);
	dest.top  = point.y;
	dest.left = point.x;

	point.x = surfaceSizeX;
	point.y = surfaceSizeY;

	m_pMainWnd->ClientToScreen(&point);
	dest.bottom = point.y;
	dest.right	= point.x;

	if (videoOption > VIDEO_4X)
	{
		int menuSkip = 0;
		if (menuToggle)
		{
			menuSkip = winGetMenuBarHeight();
		}

		if (fullScreenStretch)
		{
			dest.top	= menuSkip;
			dest.left	= 0;
			dest.right	= fsWidth;
			dest.bottom = fsHeight;
		}
		else
		{
			int top	 = (fsHeight - surfaceSizeY) / 2;
			int left = (fsWidth - surfaceSizeX) / 2;
			dest.top	+= top - menuSkip * 2;
			dest.bottom += top;
			dest.left	+= left;
			dest.right	+= left;
		}
	}
}

void VBA::updateIFB()
{
	if (systemColorDepth == 16)
	{
		switch (ifbType)
		{
		case 0:
		default:
			ifbFunction = NULL;
			break;
		case 1:
			ifbFunction = MotionBlurIB;
			break;
		case 2:
			ifbFunction = SmartIB;
			break;
		}
	}
	else if (systemColorDepth == 32)
	{
		switch (ifbType)
		{
		case 0:
		default:
			ifbFunction = NULL;
			break;
		case 1:
			ifbFunction = MotionBlurIB32;
			break;
		case 2:
			ifbFunction = SmartIB32;
			break;
		}
	}
	else
		ifbFunction = NULL;
}

void VBA::updateFilter()
{
	filterWidth	 = sizeX;
	filterHeight = sizeY;

	if (systemColorDepth == 16 && (videoOption > VIDEO_1X &&
	                               videoOption != VIDEO_320x240))
	{
		switch (filterType)
		{
		default:
		case 0:
			filterFunction = NULL;
			break;
		case 1:
			filterFunction = ScanlinesTV;
			break;
		case 2:
			filterFunction = _2xSaI;
			break;
		case 3:
			filterFunction = Super2xSaI;
			break;
		case 4:
			filterFunction = SuperEagle;
			break;
		case 5:
			filterFunction = Pixelate2x16;
			break;
		case 6:
			filterFunction = MotionBlur;
			break;
		case 7:
			filterFunction = AdMame2x;
			break;
		case 8:
			filterFunction = Simple2x16;
			break;
		case 9:
			filterFunction = Bilinear;
			break;
		case 10:
			filterFunction = BilinearPlus;
			break;
		case 11:
			filterFunction = Scanlines;
			break;
		case 12:
			filterFunction = hq2xS;
			break;
		case 13:
			filterFunction = hq2x;
			break;
		case 14:
			filterFunction = lq2x;
			break;
		case 15:
			filterFunction = hq3xS;
			break;
		case 16:
			filterFunction = hq3x;
			break;
		case 17:
			filterFunction = Simple3x16;
			break;
		case 18:
			filterFunction = Simple4x16;
			break;
		case 19:
			filterFunction = Pixelate3x16;
			break;
		case 20:
			filterFunction = Pixelate4x16;
			break;
		}
		switch (filterType)
		{
		case 0: // normal -> 1x texture
			rect.right	= sizeX;
			rect.bottom = sizeY;
			break;
		default: // other -> 2x texture
			rect.right	= sizeX * 2;
			rect.bottom = sizeY * 2;
			memset(delta, 255, sizeof(delta));
			break;
		case 15: // hq3x -> 3x texture
		case 16:
		case 17:
		case 19:
			rect.right	= sizeX * 3;
			rect.bottom = sizeY * 3;
			memset(delta, 255, sizeof(delta));
			break;
		case 18: // Simple4x -> 4x texture
		case 20:
			rect.right	= sizeX * 4;
			rect.bottom = sizeY * 4;
			memset(delta, 255, sizeof(delta));
			break;
		}
	}
	else
	{
		if (systemColorDepth == 32 && videoOption > VIDEO_1X &&
		    videoOption != VIDEO_320x240)
		{
			switch (filterType)
			{
			default:
			case 0:
				filterFunction = NULL;
				break;
			case 1:
				filterFunction = ScanlinesTV32;
				break;
			case 2:
				filterFunction = _2xSaI32;
				break;
			case 3:
				filterFunction = Super2xSaI32;
				break;
			case 4:
				filterFunction = SuperEagle32;
				break;
			case 5:
				filterFunction = Pixelate2x32;
				break;
			case 6:
				filterFunction = MotionBlur32;
				break;
			case 7:
				filterFunction = AdMame2x32;
				break;
			case 8:
				filterFunction = Simple2x32;
				break;
			case 9:
				filterFunction = Bilinear32;
				break;
			case 10:
				filterFunction = BilinearPlus32;
				break;
			case 11:
				filterFunction = Scanlines32;
				break;
			case 12:
				filterFunction = hq2xS32;
				break;
			case 13:
				filterFunction = hq2x32;
				break;
			case 14:
				filterFunction = lq2x32;
				break;
			case 15:
				filterFunction = hq3xS32;
				break;
			case 16:
				filterFunction = hq3x32;
				break;
			case 17:
				filterFunction = Simple3x32;
				break;
			case 18:
				filterFunction = Simple4x32;
				break;
			case 19:
				filterFunction = Pixelate3x32;
				break;
			case 20:
				filterFunction = Pixelate4x32;
				break;
			}
			switch (filterType)
			{
			case 0: // normal -> 1x texture
				rect.right	= sizeX;
				rect.bottom = sizeY;
				memset(delta, 255, sizeof(delta));
				break;
			default: // other -> 2x texture
				rect.right	= sizeX * 2;
				rect.bottom = sizeY * 2;
				memset(delta, 255, sizeof(delta));
				break;
			case 15: // hq3x -> 3x texture
			case 16:
			case 17:
			case 19:
				rect.right	= sizeX * 3;
				rect.bottom = sizeY * 3;
				memset(delta, 255, sizeof(delta));
				break;
			case 18: // Simple4x -> 4x texture
			case 20:
				rect.right	= sizeX * 4;
				rect.bottom = sizeY * 4;
				memset(delta, 255, sizeof(delta));
				break;
			}
		}
		else
			filterFunction = NULL;
	}

	if (display)
		display->changeRenderSize(rect.right, rect.bottom);
}

void VBA::recreateMenuBar()
{
	m_menu.Detach();
	m_menu.Attach(winResLoadMenu(MAKEINTRESOURCE(IDR_MENU)));

	if (m_pMainWnd && menuToggle)   // assuming that whether the menu has been set is always kept tracked
	{
		m_pMainWnd->SetMenu(&m_menu);
	}

	if (menu != NULL)
	{
		DestroyMenu(menu);
	}

	menu = m_menu.GetSafeHmenu();
}

void VBA::updateMenuBar()
{
	//if (flagHideMenu)
	//	return;

	recreateMenuBar();

	if (popup != NULL)
	{
		// force popup recreation if language changed
		DestroyMenu(popup);
		popup = NULL;
	}
}

void VBA::saveRewindStateIfNecessary()
{
	if (rewindSaveNeeded && rewindMemory && emulator.emuWriteMemState)
	{
		rewindCount++;
		if (rewindCount > rewindSlots)
			rewindCount = rewindSlots;
		assert(rewindPos >= 0 && rewindPos < rewindSlots);
		if (emulator.emuWriteMemState(&rewindMemory[rewindPos * REWIND_SIZE], REWIND_SIZE))
		{
			rewindPos = ++rewindPos % rewindSlots;
			assert(rewindPos >= 0 && rewindPos < rewindSlots);
			if (rewindCount == rewindSlots)
				rewindTopPos = ++rewindTopPos % rewindSlots;
		}
	}

	// also update/cache some frame search stuff
	if (frameSearching)
	{
		extern SMovie Movie;
		int curFrame = (Movie.state == MOVIE_STATE_NONE) ? systemCounters.frameCount : Movie.currentFrame;
		int endFrame = theApp.frameSearchStart + theApp.frameSearchLength;
		frameSearchSkipping	 = (curFrame < endFrame);
		frameSearchFirstStep = false;

		if (curFrame == endFrame)
		{
			// cache intermediate state to speed up searching forward
			emulator.emuWriteMemState(&frameSearchMemory[REWIND_SIZE * 1], REWIND_SIZE);
		}

		if (curFrame == endFrame + 1)
		{
			emulator.emuWriteMemState(&frameSearchMemory[REWIND_SIZE * 2], REWIND_SIZE);
			frameSearchLoadValid = true;
		}
	}
	else
	{
		frameSearchFirstStep = false;

		assert(!frameSearchSkipping);
		// just in case
		frameSearchSkipping = false;
	}
}

BOOL VBA::OnIdle(LONG lCount)
{
	if (emulating && debugger)
	{
		MSG msg;
		remoteStubMain();
		if (debugger)
			return TRUE;  // continue loop
		return !::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE);
	}
	else if (emulating && active && !paused)
	{
///    for(int i = 0; i < 2; i++)
		{
			emulator.emuMain(emulator.emuCount);

			// save the state for rewinding, if necessary
			saveRewindStateIfNecessary();

			rewindSaveNeeded = false;
		}

		if (mouseCounter)
		{
			if (--mouseCounter == 0)
			{
				SetCursor(NULL);
			}
		}
		return TRUE;
	}
	else if (emulating) // this fixes display if resetting while paused
	{
		VBAUpdateButtonPressDisplay();
		VBAUpdateFrameCountDisplay();
		systemRefreshScreen();
	}

	return FALSE;

	//  return CWinApp::OnIdle(lCount);
}

void VBA::addRecentFile(const CString &file)
{
	// Do not change recent list if frozen
	if (recentFreeze)
		return;
	int i = 0;
	for (i = 0; i < 10; ++i)
	{
		if (recentFiles[i].GetLength() == 0)
			break;

		if (recentFiles[i].Compare(file) == 0)
		{
			if (i == 0)
				return;
			CString p = recentFiles[i];
			for (int j = i; j > 0; --j)
			{
				recentFiles[j] = recentFiles[j - 1];
			}
			recentFiles[0] = p;
			return;
		}
	}
	int num = 0;
	for (i = 0; i < 10; ++i)
	{
		if (recentFiles[i].GetLength() != 0)
			++num;
	}
	if (num == 10)
	{
		--num;
	}

	for (i = num; i >= 1; --i)
	{
		recentFiles[i] = recentFiles[i - 1];
	}
	recentFiles[0] = file;
}

void VBA::updateFrameSkip()
{
	switch (systemCartridgeType)
	{
	case 0:
		systemFrameSkip = frameSkip;
		break;
	case 1:
		systemFrameSkip = gbFrameSkip;
		break;
	}
}

void VBA::updateVideoSize(UINT id)
{
	int	 value		 = 0;
	bool forceUpdate = false;

	switch (id)
	{
	case ID_OPTIONS_VIDEO_X1:
		value		= VIDEO_1X;
		forceUpdate = true;
		break;
	case ID_OPTIONS_VIDEO_X2:
		value		= VIDEO_2X;
		forceUpdate = true;
		break;
	case ID_OPTIONS_VIDEO_X3:
		value		= VIDEO_3X;
		forceUpdate = true;
		break;
	case ID_OPTIONS_VIDEO_X4:
		value		= VIDEO_4X;
		forceUpdate = true;
		break;
	case ID_OPTIONS_VIDEO_FULLSCREEN320X240:
		value		 = VIDEO_320x240;
		fsWidth		 = 320;
		fsHeight	 = 240;
		fsColorDepth = 16;
		break;
	case ID_OPTIONS_VIDEO_FULLSCREEN640X480:
		value		 = VIDEO_640x480;
		fsWidth		 = 640;
		fsHeight	 = 480;
		fsColorDepth = 16;
		break;
	case ID_OPTIONS_VIDEO_FULLSCREEN800X600:
		value		 = VIDEO_800x600;
		fsWidth		 = 800;
		fsHeight	 = 600;
		fsColorDepth = 16;
		break;
	case ID_OPTIONS_VIDEO_FULLSCREEN:
		value		= VIDEO_OTHER;
		forceUpdate = true;
		break;
	}

	if (videoOption != value || forceUpdate)
		updateWindowSize(value);
}

void VBA::updateWindowSize(int value)
{
	regSetDwordValue("video", value);

	if (value == VIDEO_OTHER)
	{
		regSetDwordValue("fsWidth", fsWidth);
		regSetDwordValue("fsHeight", fsHeight);
		regSetDwordValue("fsColorDepth", fsColorDepth);
	}

	if (display &&
	    (((value >= VIDEO_320x240 || videoOption >= VIDEO_320x240) && videoOption != value) ||
	     fsForceChange))
	{
		fsForceChange = false;
		videoOption	  = value;
		initDisplay();
	}

	videoOption = value;

	systemGetLCDResolution(sizeX, sizeY);
	systemGetLCDBaseOffset(gbBorderColumnSkip, gbBorderRowSkip);
	gbBorderLineSkip = sizeX;

	switch (videoOption)
	{
	case VIDEO_1X:
		scale = 1;
		surfaceSizeX = sizeX;
		surfaceSizeY = sizeY;
		break;
	case VIDEO_2X:
		scale = 2;
		surfaceSizeX = sizeX * 2;
		surfaceSizeY = sizeY * 2;
		break;
	case VIDEO_3X:
		scale = 3;
		surfaceSizeX = sizeX * 3;
		surfaceSizeY = sizeY * 3;
		break;
	case VIDEO_4X:
		scale = 4;
		surfaceSizeX = sizeX * 4;
		surfaceSizeY = sizeY * 4;
		break;
	case VIDEO_320x240:
	case VIDEO_640x480:
	case VIDEO_800x600:
	case VIDEO_OTHER:
		// Need to fix this code later. For now, Fullscreen takes the whole screen.
		if (fullScreenStretch)
		{
			surfaceSizeX = fsWidth;
			surfaceSizeY = fsHeight;
		}
		else
		{
			double scaleX	= (double)fsWidth / (double)sizeX;
			double scaleY	= (double)fsHeight / (double)sizeY;
			double scaleMin = scaleX < scaleY ? scaleX : scaleY;
			if (fsMaxScale)
				scaleMin = scaleMin > fsMaxScale ? fsMaxScale : scaleMin;
			scale = scaleMin;
			surfaceSizeX = (int)(scaleMin * sizeX);
			surfaceSizeY = (int)(scaleMin * sizeY);
		}
		break;
	}

	rect.left	= 0;
	rect.top	= 0;
	rect.right	= sizeX;
	rect.bottom = sizeY;

	int winSizeX = 0;
	int winSizeY = 0;
	int x		 = 0;
	int y		 = 0;

	DWORD style	  = WS_POPUP | WS_VISIBLE;
	DWORD styleEx = alwaysOnTop ? WS_EX_TOPMOST : 0;

	if (videoOption <= VIDEO_4X)
	{
		style |= WS_OVERLAPPEDWINDOW;

		dest.left	= 0;
		dest.top	= 0;
		dest.right	= surfaceSizeX;
		dest.bottom = surfaceSizeY;

		RECT workAreaRect;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
		if (windowPositionX + surfaceSizeX < workAreaRect.left || windowPositionX > workAreaRect.right)
			windowPositionX = (workAreaRect.left + workAreaRect.right - surfaceSizeX) / 2;
		if (windowPositionY + surfaceSizeY < workAreaRect.top || windowPositionY > workAreaRect.bottom)
			windowPositionY = (workAreaRect.top + workAreaRect.bottom - surfaceSizeY) / 2;

		x = windowPositionX;
		y = windowPositionY;
	}
	else
	{
		dest.left	= 0;
		dest.top	= 0;
		dest.right	= fsWidth;
		dest.bottom = fsHeight;
	}

	AdjustWindowRectEx(&dest, style, flagHideMenu ? FALSE : TRUE, styleEx);
	winSizeX = dest.right  - dest.left;
	winSizeY = dest.bottom - dest.top;

	if (m_pMainWnd == NULL)
	{
		// Create a new window
		m_pMainWnd = new MainWnd;
		m_pMainWnd->CreateEx(styleEx,
		                     theApp.wndClass,
		                     VBA_NAME_AND_VERSION,
		                     style,
		                     x, y, winSizeX, winSizeY,
		                     NULL,
		                     0);

		if (!(HWND)*m_pMainWnd)
		{
			winlog("Error creating Window %08x\n", GetLastError());
			AfxPostQuitMessage(0);
			return;
		}
	}
	else
	{
		m_pMainWnd->SetWindowPos(0, //HWND_TOPMOST,
								 x,
								 y,
								 winSizeX,
								 winSizeY,
								 SWP_NOMOVE | SWP_SHOWWINDOW);
	}

	updateMenuBar(); // add menubar first of all, or winGetMenuBarHeight() will get random height.
	winAccelMgr.UpdateMenu(menu);

	if (videoOption <= VIDEO_4X)
	{
		RECT clientRect;
		m_pMainWnd->GetClientRect(&clientRect);
		winSizeX += int(sizeX * scale) - clientRect.right;
		winSizeY += int(sizeY * scale) - clientRect.bottom;
	}

	m_pMainWnd->SetWindowPos(0, //HWND_TOPMOST,
							x,
							y,
							winSizeX,
							winSizeY,
							SWP_NOMOVE | SWP_SHOWWINDOW);

	adjustDestRect();

	updateIFB();
	updateFilter();

	m_pMainWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);
}

bool VBA::initDisplay()
{
	if (display)
	{
		changingVideoSize = true;
		shutdownDisplay();
		if (input)
		{
			delete input;
			input = NULL;
		}
		CWnd *pWnd = m_pMainWnd;

		m_pMainWnd = NULL;
		pWnd->DragAcceptFiles(FALSE);
		pWnd->DestroyWindow();
		delete pWnd;

		display = NULL;
	}

	if (display == NULL)
	{
		updateWindowSize(videoOption);

		switch (renderMethod)
		{
		case GDI:
			display = newGDIDisplay();
			break;
		case DIRECT_DRAW:
			display = newDirectDrawDisplay();
			break;
		case DIRECT_3D:
			display = newDirect3DDisplay();
			break;
		case OPENGL:
			display = newOpenGLDisplay();
			break;
		}

		if (display->initialize())
		{
			if (input == NULL)
			{
				if (!initInput())
				{
					changingVideoSize = false;
					AfxPostQuitMessage(0);
					return false;
				}
			}

			input->checkKeys();

			changingVideoSize = false;
		}
		else
		{
			if (videoOption == VIDEO_320x240 ||
			    videoOption == VIDEO_640x480 ||
			    videoOption == VIDEO_800x600 ||
			    videoOption == VIDEO_OTHER)
			{
				regSetDwordValue("video", VIDEO_1X);
				if (pVideoDriverGUID)
					regSetDwordValue("defaultVideoDriver", TRUE);
			}
			changingVideoSize = false;
			return false;
		}
	}
	changingVideoSize = false;
	return true;
}

bool VBA::updateRenderMethod(bool force)
{
	bool res = true;
	if (force || (display && display->getType() != renderMethod))
	{
		res = initDisplay();

		while (!res && renderMethod > 0)
		{
			if (renderMethod == OPENGL)
				renderMethod = DIRECT_3D;
			else if (renderMethod == DIRECT_3D)
				renderMethod = DIRECT_DRAW;
			else if (renderMethod == DIRECT_DRAW)
				renderMethod = GDI;

			res = initDisplay();
		}
	}

	updateIFB();
	updateFilter();

	m_pMainWnd->RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_ALLCHILDREN);

	regSetDwordValue("renderMethod", renderMethod);

	return res;
}

void VBA::winCheckFullscreen()
{
	if (videoOption > VIDEO_4X && tripleBuffering)
	{
		if (display)
			display->checkFullScreen();
	}
}

void VBA::shutdownDisplay()
{
	if (display != NULL)
	{
		display->cleanup();
		delete display;
		display = NULL;
	}
}

void VBA::updatePriority()
{
	switch (threadPriority)
	{
	case 0:
		SetThreadPriority(THREAD_PRIORITY_HIGHEST);
		break;
	case 1:
		SetThreadPriority(THREAD_PRIORITY_ABOVE_NORMAL);
		break;
	case 3:
		SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
		break;
	default:
		SetThreadPriority(THREAD_PRIORITY_NORMAL);
	}
}

#ifdef MMX
bool VBA::detectMMX()
{
	bool support = false;
	char brand[13];

	// check for Intel chip
	__try {
		__asm {
			mov eax, 0;
			cpuid;
			mov [dword ptr brand + 0], ebx;
			mov [dword ptr brand + 4], edx;
			mov [dword ptr brand + 8], ecx;
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		if (_exception_code() == STATUS_ILLEGAL_INSTRUCTION)
		{
			return false;
		}
		return false;
	}
	// Check for Intel or AMD CPUs
	if (strncmp(brand, "GenuineIntel", 12))
	{
		if (strncmp(brand, "AuthenticAMD", 12))
		{
			return false;
		}
	}

	__asm {
		mov eax, 1;
		cpuid;
		test edx, 00800000h;
		jz	 NotFound;
		mov [support], 1;
NotFound:
	}
	return support;
}

#endif

void VBA::winSetLanguageOption(int option, bool force)
{
	if (((option == languageOption) && option != 2) && !force)
		return;
	switch (option)
	{
	case 0:
	{
		char lbuffer[10];

		if (GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SABBREVLANGNAME,
		                  lbuffer, 10))
		{
			HINSTANCE l = winLoadLanguage(lbuffer);
			if (l == NULL)
			{
				LCID locIdBase = MAKELCID(MAKELANGID(PRIMARYLANGID(GetSystemDefaultLangID()), SUBLANG_NEUTRAL), SORT_DEFAULT);
				if (GetLocaleInfo(locIdBase, LOCALE_SABBREVLANGNAME,
				                  lbuffer, 10))
				{
					l = winLoadLanguage(lbuffer);
					if (l == NULL)
					{
						systemMessage(IDS_FAILED_TO_LOAD_LIBRARY,
						              "Failed to load library %s",
						              lbuffer);
						return;
					}
				}
			}
			AfxSetResourceHandle(l);
			if (languageModule != NULL)
				/**/ ::FreeLibrary(languageModule);
			languageModule = l;
		}
		else
		{
			systemMessage(IDS_FAILED_TO_GET_LOCINFO,
			              "Failed to get locale information");
			return;
		}
		break;
	}
	case 1:
		if (languageModule != NULL)
			/**/ ::FreeLibrary(languageModule);
		languageModule = NULL;
		AfxSetResourceHandle(AfxGetInstanceHandle());
		break;
	case 2:
	{
		if (!force)
		{
			LangSelect dlg;
			if (dlg.DoModal())
			{
				HINSTANCE l = winLoadLanguage(languageName);
				if (l == NULL)
				{
					systemMessage(IDS_FAILED_TO_LOAD_LIBRARY,
					              "Failed to load library %s",
					              languageName);
					return;
				}
				AfxSetResourceHandle(l);
				if (languageModule != NULL)
					/**/ ::FreeLibrary(languageModule);
				languageModule = l;
			}
		}
		else
		{
			if (languageName.IsEmpty())
				return;
			HINSTANCE l = winLoadLanguage(languageName);
			if (l == NULL)
			{
				systemMessage(IDS_FAILED_TO_LOAD_LIBRARY,
				              "Failed to load library %s",
				              languageName);
				return;
			}
			AfxSetResourceHandle(l);
			if (languageModule != NULL)
				FreeLibrary(languageModule);
			languageModule = l;
		}
		break;
	}
	}
	languageOption = option;
	updateMenuBar();
	theApp.winAccelMgr.UpdateMenu(theApp.menu);
}

HINSTANCE VBA::winLoadLanguage(const char *name)
{
	CString buffer;

	buffer.Format("vba_%s.dll", name);

	HINSTANCE l = /**/ ::LoadLibrary(buffer);

	if (l == NULL)
	{
		if (strlen(name) == 3)
		{
			char buffer2[3];
			buffer2[0] = name[0];
			buffer2[1] = name[1];
			buffer2[2] = 0;
			buffer.Format("vba_%s.dll", buffer2);

			return /**/ ::LoadLibrary(buffer);
		}
	}
	return l;
}

bool VBA::initInput()
{
	if (input)
		delete input;
	input = newDirectInput();
	if (input->initialize())
	{
		input->loadSettings();
		input->checkKeys();
		return true;
	}
	delete input;
	return false;
}

void VBA::winAddUpdateListener(IUpdateListener *l)
{
	updateList.AddTail(l);
	updateCount++;
}

void VBA::winRemoveUpdateListener(IUpdateListener *l)
{
	POSITION pos = updateList.Find(l);
	if (pos)
	{
		updateList.RemoveAt(pos);
		updateCount--;
		if (updateCount < 0)
			updateCount = 0;
	}
}

void VBA::loadSettings()
{
	CString	buffer;
	// video
	bool defaultVideoDriver = regQueryDwordValue("defaultVideoDriver", true) ? true : false;
	if (!regQueryBinaryValue("videoDriverGUID", (char *)&videoDriverGUID, sizeof(GUID)))
	{
		defaultVideoDriver = TRUE;
	}
	if (defaultVideoDriver)
		pVideoDriverGUID = NULL;
	else
		pVideoDriverGUID = &videoDriverGUID;

	videoOption = regQueryDwordValue("video", 0);
	if (videoOption < 0 || videoOption > VIDEO_OTHER)
		videoOption = 0;
	switch (videoOption)
	{
	case VIDEO_320x240:
		fsWidth		 = 320;
		fsHeight	 = 240;
		fsColorDepth = 16;
		break;
	case VIDEO_640x480:
		fsWidth		 = 640;
		fsHeight	 = 480;
		fsColorDepth = 16;
		break;
	case VIDEO_800x600:
		fsWidth		 = 800;
		fsHeight	 = 600;
		fsColorDepth = 16;
		break;
	}
	if (videoOption == VIDEO_OTHER)
	{
		if (fsWidth < 0 || fsWidth > 4095 || fsHeight < 0 || fsHeight > 4095)
			videoOption = 0;
		if (fsColorDepth != 16 && fsColorDepth != 24 && fsColorDepth != 32)
			videoOption = 0;
	}

	fsWidth = regQueryDwordValue("fsWidth", 0);
	fsHeight = regQueryDwordValue("fsHeight", 0);
	fsColorDepth	  = regQueryDwordValue("fsColorDepth", 0);
	fsMaxScale		  = regQueryDwordValue("fsMaxScale", 0);
	fullScreenStretch = regQueryDwordValue("stretch", 0) ? true : false;

	renderMethod = (DISPLAY_TYPE)regQueryDwordValue("renderMethod", DIRECT_DRAW);
	if (renderMethod < GDI || renderMethod > OPENGL)
		renderMethod = DIRECT_DRAW;

	ddrawEmulationOnly	= regQueryDwordValue("ddrawEmulationOnly", false) ? true : false;
	ddrawUseVideoMemory = regQueryDwordValue("ddrawUseVideoMemory", false) ? true : false;
	tripleBuffering		= regQueryDwordValue("tripleBuffering", true) ? true : false;
	vsync = regQueryDwordValue("vsync", false) ? true : false;

	d3dFilter = regQueryDwordValue("d3dFilter", 0);
	if (d3dFilter < 0 || d3dFilter > 1)
		d3dFilter = 0;
	glFilter = regQueryDwordValue("glFilter", 0);
	if (glFilter < 0 || glFilter > 1)
		glFilter = 0;
	glType = regQueryDwordValue("glType", 0);
	if (glType < 0 || glType > 1)
		glType = 0;

	// pixel filter & ifb
	filterType = regQueryDwordValue("filter", 0);
	if (filterType < 0 || filterType > 20)
		filterType = 0;
	disableMMX = regQueryDwordValue("disableMMX", 0) ? true : false;
	ifbType	   = regQueryDwordValue("ifbType", 0);
	if (ifbType < 0 || ifbType > 2)
		ifbType = 0;

	// frame skipping
	frameSkip = regQueryDwordValue("frameSkip", /*2*/ 0);
	if (frameSkip < 0 || frameSkip > 9)
		frameSkip = 1;
	gbFrameSkip = regQueryDwordValue("gbFrameSkip", 0);
	if (gbFrameSkip < 0 || gbFrameSkip > 9)
		gbFrameSkip = 0;
///  autoFrameSkip = regQueryDwordValue("autoFrameSkip", FALSE) ? TRUE : FALSE;

	// input
	joypadDefault = regQueryDwordValue("joypadDefault", 0);
	if (joypadDefault < 0 || joypadDefault > 3)
		joypadDefault = 0;
	allowLeftRight		   = regQueryDwordValue("allowLeftRight", false) ? true : false;
	autofireAccountForLag  = regQueryDwordValue("autofireAccountForLag", false) ? true : false;
	nextframeAccountForLag = regQueryDwordValue("nextframeAccountForLag", false) ? true : false;
	theApp.AsscWithSaveState = regQueryDwordValue("AsscWithSaveState", false) ? true : false;

	// speed
	throttle = regQueryDwordValue("throttle", 0);
	if (throttle < 5 || throttle > 1000)
		throttle = 100;

	synchronize = regQueryDwordValue("synchronize", 1) ? true : false;
	accuratePitchThrottle = regQueryDwordValue("accuratePitchThrottle", FALSE) ? TRUE : FALSE;

	// sound
	int resChannels = regQueryDwordValue("soundEnable", 0x30f);
	systemSoundEnableChannels(resChannels);
	systemSoundDisableChannels(~resChannels);
	soundOffFlag = (regQueryDwordValue("soundOff", 0)) ? true : false;
	soundQuality = regQueryDwordValue("soundQuality", 2);
	soundEcho	 = regQueryDwordValue("soundEcho", 0) ? true : false;
	soundLowPass = regQueryDwordValue("soundLowPass", 0) ? true : false;
	soundReverse = regQueryDwordValue("soundReverse", 0) ? true : false;
	soundVolume	 = regQueryDwordValue("soundVolume", 0);
	if (soundVolume < 0 || soundVolume > 5)
		soundVolume = 0;
	muteFrameAdvance = regQueryDwordValue("muteFrameAdvance", 0) ? TRUE : FALSE;
	muteWhenInactive = regQueryDwordValue("muteWhenInactive", 0) ? TRUE : FALSE;

	// emulation
#ifdef USE_GBA_CORE_V7
	memLagEnabled	  = regQueryDwordValue("memLagEnabled", false) ? true : false;
	memLagTempEnabled = memLagEnabled;
#endif
#ifdef USE_GB_CORE_V7
	gbNullInputHackEnabled	   = regQueryDwordValue("gbNullInputHackEnabled", false) ? true : false;
	gbNullInputHackTempEnabled = gbNullInputHackEnabled;
#else
	gbV20GBFrameTimingHack	   = regQueryDwordValue("gbV20GBFrameTimingHack", false) ? true : false;
	gbV20GBFrameTimingHackTemp = gbV20GBFrameTimingHack;
#endif
	useOldSync		  = regQueryDwordValue("useOldSync", 0) ? TRUE : FALSE;
	useOldFrameTiming = regQueryDwordValue("useOldGBTiming", false) ? true : false;

	useBiosFile	  = regQueryDwordValue("useBios", 0) ? true : false;
	skipBiosIntro = regQueryDwordValue("skipBios", 0) ? true : false;
	buffer		  = regQueryStringValue("biosFile", "");
	if (!buffer.IsEmpty())
	{
		biosFileName = buffer;
	}
//	removeIntros = regQueryDwordValue("removeIntros", false) ? true : false;

	autoIPS = regQueryDwordValue("autoIPS", true) ? true : false;

	agbPrintEnable(regQueryDwordValue("agbPrint", 0) ? true : false);
	winRtcEnable = regQueryDwordValue("rtcEnabled", 0) ? true : false;
	rtcEnable(winRtcEnable);

	winSaveType = regQueryDwordValue("saveType", 0);
	if (winSaveType < 0 || winSaveType > 5)
		winSaveType = 0;
	cpuEnhancedDetection = regQueryDwordValue("enhancedDetection", 1) ? true : false;
	winFlashSize		 = regQueryDwordValue("flashSize", 0x10000);
	if (winFlashSize != 0x10000 && winFlashSize != 0x20000)
		winFlashSize = 0x10000;

	cpuDisableSfx = regQueryDwordValue("disableSfx", 0) ? true : false;

	// GBx
	winGbPrinterEnabled = regQueryDwordValue("gbPrinter", false) ? true : false;
	if (winGbPrinterEnabled)
		gbSerialFunction = gbPrinterSend;
	else
		gbSerialFunction = NULL;
	gbEmulatorType = regQueryDwordValue("emulatorType", 0);
	if (gbEmulatorType < 0 || gbEmulatorType > 5)
		gbEmulatorType = 1;
	winGbBorderOn	  = regQueryDwordValue("borderOn", 0);
	gbBorderAutomatic = regQueryDwordValue("borderAutomatic", 0);

	gbColorOption	= regQueryDwordValue("colorOption", 0);
	gbPaletteOption = regQueryDwordValue("gbPaletteOption", 0);
	if (gbPaletteOption < 0)
		gbPaletteOption = 0;
	if (gbPaletteOption > 2)
		gbPaletteOption = 2;
	regQueryBinaryValue("gbPalette", (char *)systemGbPalette, 24 * sizeof(u16));

	// head-up display
	showSpeed = regQueryDwordValue("showSpeed", 1);
	if (showSpeed < 0 || showSpeed > 2)
		showSpeed = 1;
	showSpeedTransparent = regQueryDwordValue("showSpeedTransparent", TRUE) ? TRUE : FALSE;
	outlinedText		 = regQueryDwordValue("outlinedText", TRUE) != 0;
	transparentText		 = regQueryDwordValue("transparentText", FALSE) != 0;
	textColor			 = regQueryDwordValue("textColor", 0);
	textMethod			 = regQueryDwordValue("textMethod", 1);
	frameCounter		 = regQueryDwordValue("frameCounter", false) ? true : false;
	lagCounter			 = regQueryDwordValue("lagCounter", false) ? true : false;
	extraCounter		 = regQueryDwordValue("extraCounter", false) ? true : false;
	inputDisplay		 = regQueryDwordValue("inputDisplay", false) ? true : false;
	nextInputDisplay	 = regQueryDwordValue("nextInputDisplay", false) ? true : false;
	disableStatusMessage = regQueryDwordValue("disableStatus", 0) ? true : false;

	// UI
	windowPositionX = regQueryDwordValue("windowX", -32000);
	windowPositionY = regQueryDwordValue("windowY", -32000);

	autoHideMenu = regQueryDwordValue("autoHideMenu", 0) ? true : false;

	languageOption = regQueryDwordValue("language", 1);
	if (languageOption < 0 || languageOption > 2)
		languageOption = 1;
	buffer = regQueryStringValue("languageName", "");
	if (!buffer.IsEmpty())
	{
		languageName = buffer.Left(3);
	}
	else
		languageName = "";
	winSetLanguageOption(languageOption, true);

	// preferences
	alwaysOnTop = regQueryDwordValue("alwaysOnTop", false) ? true : false;
	pauseWhenInactive	  = regQueryDwordValue("pauseWhenInactive", 1) ? true : false;
	enableBackgroundInput = regQueryDwordValue("enableBackgroundInput", 0) ? true : false;
	threadPriority		  = regQueryDwordValue("priority", 2);
	if (threadPriority < 0 || threadPriority > 3)
		threadPriority = 2;
	updatePriority();

	filenamePreference = regQueryDwordValue("filenamePreference", 0);
	altAviRecordMethod = regQueryDwordValue("altAviRecordMethod", false) ? true : false;
	captureFormat	   = regQueryDwordValue("captureFormat", 0);

	rewindTimer = regQueryDwordValue("rewindTimer", 0);
	rewindSlots = regQueryDwordValue("rewindSlots", 64);
	if (rewindTimer < 0 || rewindTimer > 600)
		rewindTimer = 0;
	if (rewindSlots <= 0)
		rewindTimer = rewindSlots = 0;
	if (rewindSlots > MAX_REWIND_SLOTS)
		rewindSlots = MAX_REWIND_SLOTS;
	if (rewindTimer != 0)
	{
		if (rewindMemory == NULL)
			rewindMemory = (char *)malloc(rewindSlots * REWIND_SIZE);
	}

	if (frameSearchMemory == NULL)
		frameSearchMemory = (char *)malloc(3 * REWIND_SIZE);

	recentFreeze = regQueryDwordValue("recentFreeze", false) ? true : false;
	for (int i = 0, j = 0; i < 10; ++i)
	{
		buffer.Format("recent%d", i);
		const char *s = regQueryStringValue(buffer, NULL);
		if (s == NULL)
			continue;
		recentFiles[j] = s;
		++j;
	}

	autoLoadMostRecent = regQueryDwordValue("autoLoadMostRecent", false) ? true : false;
	loadMakesRecent	   = regQueryDwordValue("loadMakesRecent", false) ? true : false;
	loadMakesCurrent   = regQueryDwordValue("loadMakesCurrent", false) ? true : false;
	saveMakesCurrent   = regQueryDwordValue("saveMakesCurrent", false) ? true : false;
	currentSlot		   = regQueryDwordValue("currentSlot", 0);
	showSlotTime	   = regQueryDwordValue("showSlotTime", 0) ? true : false;

	cheatsEnabled = regQueryDwordValue("cheatsEnabled", true) ? true : false;
	autoSaveLoadCheatList  = regQueryDwordValue("autoSaveCheatList", 0) ? true : false;
	pauseDuringCheatSearch = regQueryDwordValue("pauseDuringCheatSearch2", 0) ? true : false;

	int movieEditMode  = regQueryDwordValue("movieEditMode", 0);
	if (movieEditMode < MovieEditMode::MOVIE_EDIT_MODE_DISCARD || movieEditMode >= MovieEditMode::MOVIE_EDIT_MODE_COUNT)
		movieEditMode = MovieEditMode::MOVIE_EDIT_MODE_DISCARD;
	VBAMovieSetEditMode(MovieEditMode(movieEditMode));

	movieOnEndBehavior = regQueryDwordValue("movieOnEndBehavior", 0);
	movieOnEndPause	   = regQueryDwordValue("movieOnEndPause", 0) ? true : false;

	extern bool autoConvertMovieWhenPlaying;    // from movie.cpp
	autoConvertMovieWhenPlaying = regQueryDwordValue("autoConvertMovieWhenPlaying", 0) ? true : false;

	// RamWatch Settings
	AutoRWLoad		= regQueryDwordValue(AUTORWLOAD, false);
	RWSaveWindowPos = regQueryDwordValue(RWSAVEPOS, false);
	ramw_x = regQueryDwordValue(RAMWX, 0);
	ramw_y = regQueryDwordValue(RAMWY, 0);

	// this is FILO
	for (int i = MAX_RECENT_WATCHES; i > 0; --i)
	{
		buffer.Format("recentWatch%d", i);
		const char *s = regQueryStringValue(buffer, NULL);
		if (s == NULL)
			continue;
		RWAddRecentFile(s);
	}
}

void VBA::saveSettings()
{
	regSetDwordValue("defaultVideoDriver", pVideoDriverGUID == NULL);
	if (pVideoDriverGUID)
	{
		regSetBinaryValue("videoDriverGUID", (char *)&videoDriverGUID,
		                  sizeof(GUID));
	}
	regSetDwordValue("video", videoOption);

	regSetDwordValue("fsWidth", fsWidth);
	regSetDwordValue("fsHeight", fsHeight);
	regSetDwordValue("fsColorDepth", fsColorDepth);
	regSetDwordValue("fsMaxScale", fsMaxScale);

	regSetDwordValue("stretch", fullScreenStretch);

	regSetDwordValue("renderMethod", renderMethod);

	regSetDwordValue("ddrawEmulationOnly", ddrawEmulationOnly);
	regSetDwordValue("ddrawUseVideoMemory", ddrawUseVideoMemory);
	regSetDwordValue("tripleBuffering", tripleBuffering);
	regSetDwordValue("vsync", vsync);

	regSetDwordValue("d3dFilter", d3dFilter);
	regSetDwordValue("glFilter", glFilter);
	regSetDwordValue("glType", glType);

	// pixel filter & ifb
	regSetDwordValue("filter", filterType);
	regSetDwordValue("ifbType", ifbType);
	regSetDwordValue("disableMMX", disableMMX);

	// frame skipping
	regSetDwordValue("frameSkip", frameSkip);
	regSetDwordValue("gbFrameSkip", gbFrameSkip);
///  regSetDwordValue("autoFrameSkip", autoFrameSkip);

	// input
	regSetDwordValue("joypadDefault", joypadDefault);
	regSetDwordValue("allowLeftRight", allowLeftRight);
	regSetDwordValue("autofireAccountforLag", autofireAccountForLag);
	regSetDwordValue("nextframeAccountforLag", nextframeAccountForLag);
	regSetDwordValue("AsscWithSaveState", theApp.AsscWithSaveState);

	// speed
	regSetDwordValue("throttle", throttle);
	regSetDwordValue("synchronize", synchronize);
	regSetDwordValue("accuratePitchThrottle", accuratePitchThrottle);

	// sound
	regSetDwordValue("soundEnable", systemSoundGetEnabledChannels() & 0x030f);
	regSetDwordValue("soundOff", soundOffFlag);
	regSetDwordValue("soundQuality", soundQuality);
	regSetDwordValue("soundEcho", soundEcho);
	regSetDwordValue("soundLowPass", soundLowPass);
	regSetDwordValue("soundReverse", soundReverse);
	regSetDwordValue("soundVolume", soundVolume);
	regSetDwordValue("muteFrameAdvance", muteFrameAdvance);
	regSetDwordValue("muteWhenInactive", muteWhenInactive);

	// emulation
	regSetDwordValue("useBios", useBiosFile);
	regSetDwordValue("skipBios", skipBiosIntro);
	if (!biosFileName.IsEmpty())
		regSetStringValue("biosFile", biosFileName);
//	regSetDwordValue("removeIntros", removeIntros);

	regSetDwordValue("autoIPS", autoIPS);

#ifdef USE_GBA_CORE_V7
	regSetDwordValue("memLagEnabled", memLagEnabled);
#endif
#ifdef USE_GB_CORE_V7
	regSetDwordValue("gbNullInputHackEnabled", gbNullInputHackEnabled);
#else
	regSetDwordValue("gbV20GBFrameTimingHack", gbV20GBFrameTimingHack);
#endif
	regSetDwordValue("useOldGBTiming", useOldFrameTiming);
	regSetDwordValue("useOldSync", useOldSync);

	regSetDwordValue("agbPrint", agbPrintIsEnabled());
	regSetDwordValue("rtcEnabled", winRtcEnable);

	regSetDwordValue("saveType", winSaveType);
	regSetDwordValue("enhancedDetection", cpuEnhancedDetection);
	regSetDwordValue("flashSize", winFlashSize);

	regSetDwordValue("disableSfx", cpuDisableSfx);

	// GBx
	regSetDwordValue("emulatorType", gbEmulatorType);
	regSetDwordValue("gbPrinter", winGbPrinterEnabled);
	regSetDwordValue("borderOn", winGbBorderOn);
	regSetDwordValue("borderAutomatic", gbBorderAutomatic);

	regSetDwordValue("colorOption", gbColorOption);
	regSetDwordValue("gbPaletteOption", gbPaletteOption);
	regSetBinaryValue("gbPalette", (char *)systemGbPalette, 24 * sizeof(u16));

	// head-up display
	regSetDwordValue("showSpeed", showSpeed);
	regSetDwordValue("showSpeedTransparent", showSpeedTransparent);

	regSetDwordValue("outlinedText", outlinedText);
	regSetDwordValue("transparentText", transparentText);
	regSetDwordValue("textColor", textColor);
	regSetDwordValue("textMethod", textMethod);
	regSetDwordValue("frameCounter", frameCounter);
	regSetDwordValue("lagCounter", lagCounter);
	regSetDwordValue("extraCounter", extraCounter);
	regSetDwordValue("inputDisplay", inputDisplay);
	regSetDwordValue("nextInputDisplay", nextInputDisplay);
	regSetDwordValue("disableStatus", disableStatusMessage);

	// UI
	regSetDwordValue("windowX", windowPositionX);
	regSetDwordValue("windowY", windowPositionY);

	regSetDwordValue("autoHideMenu", autoHideMenu);

	regSetDwordValue("language", languageOption);
	regSetStringValue("languageName", languageName);

	// preferences
	regSetDwordValue("alwaysOnTop", alwaysOnTop);
	regSetDwordValue("pauseWhenInactive", pauseWhenInactive);
	regSetDwordValue("enableBackgroundInput", enableBackgroundInput);
	regSetDwordValue("priority", threadPriority);

	regSetDwordValue("filenamePreference", filenamePreference);
	regSetDwordValue("altAviRecordMethod", altAviRecordMethod);
	regSetDwordValue("captureFormat", captureFormat);

	regSetDwordValue("cheatsEnabled", cheatsEnabled);
	regSetDwordValue("autoSaveCheatList", autoSaveLoadCheatList);
	regSetDwordValue("pauseDuringCheatSearch2", pauseDuringCheatSearch);

	regSetDwordValue("rewindTimer", rewindTimer);
	regSetDwordValue("rewindSlots", rewindSlots);

	regSetDwordValue("recentFreeze", recentFreeze);
	CString buffer;
	for (int i = 0; i < 10; i++)
	{
		buffer.Format("recent%d", i);
		regSetStringValue(buffer, recentFiles[i]);
	}

	regSetDwordValue("autoLoadMostRecent", autoLoadMostRecent);
	regSetDwordValue("loadMakesRecent", loadMakesRecent);
	regSetDwordValue("loadMakesCurrent", loadMakesCurrent);
	regSetDwordValue("saveMakesCurrent", saveMakesCurrent);
	regSetDwordValue("currentSlot", currentSlot);
	regSetDwordValue("showSlotTime", showSlotTime);

	regSetDwordValue("movieEditMode", VBAMovieGetEditMode());
	regSetDwordValue("movieOnEndBehavior", movieOnEndBehavior);
	regSetDwordValue("movieOnEndPause", movieOnEndPause);

	extern bool autoConvertMovieWhenPlaying;    // from movie.cpp
	regSetDwordValue("autoConvertMovieWhenPlaying", autoConvertMovieWhenPlaying);
}

