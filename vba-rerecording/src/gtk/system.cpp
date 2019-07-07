// VisualBoyAdvance - Nintendo Gameboy/GameboyAdvance (TM) emulator.
// Copyright (C) 1999-2003 Forgotten
// Copyright (C) 2004 Forgotten and the VBA development team

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or(at your option)
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <SDL.h>
#include <SDL_thread.h>

#include "../common/System.h"
#include "../common/Util.h"
#include "../gba/GBAGlobals.h"
#include "../gb/gbGlobals.h"
#include "../gba/GBASound.h"

#include "window.h"
#include "intl.h"

// Required vars, used by the emulator core
//
int  systemRedShift;
int  systemGreenShift;
int  systemBlueShift;
int  systemColorDepth;
int  systemDebug;
int  systemVerbose;
int  systemSaveUpdateCounter;
int  systemFrameSkip;
u32  systemColorMap32[0x10000];
u16  systemColorMap16[0x10000];
u16  systemGbPalette[24];
bool systemSoundOn;

char filename[2048];
char biosFileName[2048];
//char captureDir[2048];
char saveDir[2048];
char batteryDir[2048];

int sensorX = 2047;
int sensorY = 2047;
bool sensorOn = false;

int  emulating;
bool debugger;
int  RGB_LOW_BITS_MASK;

int cartridgeType = 3;
int sizeOption = 0;
int captureFormat = 0;
int throttle = 0;
bool paused = false;
bool removeIntros = false;
int sdlFlashSize = 0;
int sdlRtcEnable = 0;

int sdlDefaultJoypad = 0;

SDL_Joystick **sdlDevices = NULL;

u16 motion[4] = {
  SDLK_KP4, SDLK_KP6, SDLK_KP8, SDLK_KP2
};

struct EmulatedSystem emulator = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  false,
  0
};

// Extra vars, only used for the GUI
//
int systemRenderedFrames;
int systemFPS;

// Sound stuff
//
const  int         iSoundSamples  = 2048;
const  int         iSoundTotalLen = iSoundSamples * 4;
static u8          auiSoundBuffer[iSoundTotalLen];
static int         iSoundLen;
static SDL_cond *  pstSoundCond;
static SDL_mutex * pstSoundMutex;

inline VBA::Window * GUI()
{
  return VBA::Window::poGetInstance();
}

void systemMessage(int _iId, const char * _csFormat, ...)
{
  va_list args;
  va_start(args, _csFormat);

  GUI()->vPopupErrorV(_(_csFormat), args);

  va_end(args);
}

void systemDrawScreen()
{
  GUI()->vDrawScreen();
  systemRenderedFrames++;
}

bool systemReadJoypads()
{
  return true;
}

u32 systemReadJoypad(int,bool)
{
  return GUI()->uiReadJoypad();
}

void systemShowSpeed(int _iSpeed)
{
  systemFPS = systemRenderedFrames;
  systemRenderedFrames = 0;

  GUI()->vShowSpeed(_iSpeed);
}

void system10Frames(int _iRate)
{
  GUI()->vComputeFrameskip(_iRate);
}

void systemFrame(int)
{
}

void systemSetTitle(const char * _csTitle)
{
  GUI()->set_title(_csTitle);
}

int systemScreenCapture(int _iNum)
{
  GUI()->vCaptureScreen(_iNum);
}

void systemSoundWriteToBuffer()
{
  if (SDL_GetAudioStatus() != SDL_AUDIO_PLAYING)
  {
    SDL_PauseAudio(0);
  }

  bool bWait = true;
  while (bWait && ! speedup && GUI()->iGetThrottle() == 0)
  {
    SDL_mutexP(pstSoundMutex);
    if (iSoundLen < iSoundTotalLen)
    {
      bWait = false;
    }
    SDL_mutexV(pstSoundMutex);
  }

  int iLen = soundBufferLen;
  int iCopied = 0;
  if (iSoundLen + iLen >= iSoundTotalLen)
  {
    iCopied = iSoundTotalLen - iSoundLen;
    memcpy(&auiSoundBuffer[iSoundLen], soundFinalWave, iCopied);

    iSoundLen = iSoundTotalLen;
    SDL_CondSignal(pstSoundCond);

    bWait = true;
    if (! speedup && GUI()->iGetThrottle() == 0)
    {
      while(bWait)
      {
        SDL_mutexP(pstSoundMutex);
        if (iSoundLen < iSoundTotalLen)
        {
          bWait = false;
        }
        SDL_mutexV(pstSoundMutex);
      }

      memcpy(auiSoundBuffer, ((u8 *)soundFinalWave) + iCopied,
             soundBufferLen - iCopied);

      iSoundLen = soundBufferLen - iCopied;
    }
    else
    {
      memcpy(auiSoundBuffer, ((u8 *)soundFinalWave) + iCopied,
             soundBufferLen);
    }
  }
  else
  {
    memcpy(&auiSoundBuffer[iSoundLen], soundFinalWave, soundBufferLen);
    iSoundLen += soundBufferLen;
  }
}

static void vSoundCallback(void * _pvUserData, u8 * _puiStream, int _iLen)
{
  if (! emulating)
  {
    return;
  }

  SDL_mutexP(pstSoundMutex);
  if (! speedup && GUI()->iGetThrottle() == 0)
  {
    while (iSoundLen < iSoundTotalLen && emulating)
    {
      SDL_CondWait(pstSoundCond, pstSoundMutex);
    }
  }
  if (emulating)
  {
    memcpy(_puiStream, auiSoundBuffer, _iLen);
  }
  iSoundLen = 0;
  SDL_mutexV(pstSoundMutex);
}

bool systemSoundInit()
{
  SDL_AudioSpec stAudio;

  switch (soundQuality)
  {
  case 1:
    stAudio.freq = 44100;
    soundBufferLen = 1470 * 2;
    break;
  case 2:
    stAudio.freq = 22050;
    soundBufferLen = 736 * 2;
    break;
  case 4:
    stAudio.freq = 11025;
    soundBufferLen = 368 * 2;
    break;
  }

  stAudio.format   = AUDIO_S16SYS;
  stAudio.channels = 2;
  stAudio.samples  = iSoundSamples;
  stAudio.callback = vSoundCallback;
  stAudio.userdata = NULL;

  if (SDL_OpenAudio(&stAudio, NULL) < 0)
  {
    fprintf(stderr, "Failed to open audio: %s\n", SDL_GetError());
    return false;
  }

  pstSoundCond  = SDL_CreateCond();
  pstSoundMutex = SDL_CreateMutex();

  soundBufferTotalLen = soundBufferLen * 10;
  iSoundLen = 0;
  systemSoundOn = true;

  return true;
}

void systemSoundShutdown()
{
  SDL_mutexP(pstSoundMutex);
  int iSave = emulating;
  emulating = 0;
  SDL_CondSignal(pstSoundCond);
  SDL_mutexV(pstSoundMutex);

  SDL_DestroyCond(pstSoundCond);
  pstSoundCond = NULL;

  SDL_DestroyMutex(pstSoundMutex);
  pstSoundMutex = NULL;

  SDL_CloseAudio();

  emulating = iSave;
  systemSoundOn = false;
}

void systemSoundPause()
{
  SDL_PauseAudio(1);
}

void systemSoundResume()
{
  SDL_PauseAudio(0);
}

void systemSoundReset()
{
}

u32 systemGetClock()
{
  return SDL_GetTicks();
}

void systemUpdateMotionSensor()
{
}

int systemGetSensorX()
{
  return 0;
}

int systemGetSensorY()
{
  return 0;
}

void systemGbPrint(u8 * _puiData,
                   int  _iPages,
                   int  _iFeed,
                   int  _iPalette,
                   int  _iContrast)
{
}

void systemScreenMessage(const char * _csMsg, int slot, int duration, const char *colorList)
{
}

bool systemCanChangeSoundQuality()
{
  return true;
}

bool systemPauseOnFrame()
{
  return false;
}

void systemGbBorderOn()
{
}

void debuggerMain()
{
}

void debuggerSignal(int, int)
{
}

void debuggerOutput(char *, u32)
{
}

char *sdlGetFilename(char *name)
{
  static char filebuffer[2048];

  int len = strlen(name);
  
  char *p = name + len - 1;
  
  while(true) {
    if(*p == '/' ||
       *p == '\\') {
      p++;
      break;
    }
    len--;
    p--;
    if(len == 0)
      break;
  }
  
  if(len == 0)
    strcpy(filebuffer, name);
  else
    strcpy(filebuffer, p);
  return filebuffer;
}

bool sdlCheckJoyKey(int key)
{
  int dev = (key >> 12) - 1;
  int what = key & 0xfff;

  if(what >= 128) {
    // joystick button
    int button = what - 128;

    if(button >= SDL_JoystickNumButtons(sdlDevices[dev]))
      return false;
  } else if (what < 0x20) {
    // joystick axis    
    what >>= 1;
    if(what >= SDL_JoystickNumAxes(sdlDevices[dev]))
      return false;
  } else if (what < 0x30) {
    // joystick hat
    what = (what & 15);
    what >>= 2;
    if(what >= SDL_JoystickNumHats(sdlDevices[dev]))
      return false;
  }

  // no problem found
  return true;
}

u16 checksumBIOS()
{
	bool hasBIOS = false;
	u8 * tempBIOS;
	if(useBios)
	{
		tempBIOS = (u8 *)malloc(0x4000);
		int size = 0x4000;
		if(utilLoad(biosFileName,
					utilIsGBABios,
					tempBIOS,
					size)) {
		if(size == 0x4000)
			hasBIOS = true;
		}
	}

	u16 biosCheck = 0;
	if(hasBIOS) {
		for(int i = 0; i < 0x4000; i += 4)
			biosCheck += *((u32 *)&tempBIOS[i]);
		free(tempBIOS);
	}

	return biosCheck;
}

void (*dbgMain)() = debuggerMain;
void (*dbgSignal)(int, int) = debuggerSignal;
void (*dbgOutput)(char *, u32) = debuggerOutput;
