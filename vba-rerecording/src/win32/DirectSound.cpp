#include "stdafx.h"
#include <mmreg.h>
#include <dsound.h>

#include "resource.h"
#include "AVIWrite.h"
#include "Sound.h"
#include "WavWriter.h"
#include "VBA.h"

#include "../common/SystemGlobals.h"
#include "../common/nesvideos-piece.h"

extern void directXMessage(const char *);

class DirectSound : public ISound
{
private:
	HINSTANCE			dsoundDLL;
	LPDIRECTSOUND		pDirectSound;
	LPDIRECTSOUNDBUFFER dsbPrimary;
	LPDIRECTSOUNDBUFFER dsbSecondary;
	LPDIRECTSOUNDNOTIFY dsbNotify;
	HANDLE		 dsbEvent;
	WAVEFORMATEX wfx;
	float		 curRate;
public:
	DirectSound();
	virtual ~DirectSound();

	bool init();
	void pause();
	void reset();
	void resume();
	void write();
	void setSpeed(float rate);
	bool isPlaying();
	void clearAudioBuffer();
};

DirectSound::DirectSound()
{
	dsoundDLL	 = NULL;
	pDirectSound = NULL;
	dsbPrimary	 = NULL;
	dsbSecondary = NULL;
	dsbNotify	 = NULL;
	dsbEvent	 = NULL;
}

DirectSound::~DirectSound()
{
	if (theApp.aviRecorder != NULL)
	{
		delete theApp.aviRecorder;
		theApp.aviRecorder	= NULL;
		theApp.aviRecording = false;
	}

	if (theApp.soundRecording)
	{
		if (theApp.soundRecorder != NULL)
		{
			delete theApp.soundRecorder;
			theApp.soundRecorder = NULL;
		}
		theApp.soundRecording = false;
	}

	if (dsbNotify != NULL)
	{
		dsbNotify->Release();
		dsbNotify = NULL;
	}

	if (dsbEvent != NULL)
	{
		CloseHandle(dsbEvent);
		dsbEvent = NULL;
	}

	if (pDirectSound != NULL)
	{
		if (dsbPrimary != NULL)
		{
			dsbPrimary->Release();
			dsbPrimary = NULL;
		}

		if (dsbSecondary != NULL)
		{
			dsbSecondary->Release();
			dsbSecondary = NULL;
		}

		pDirectSound->Release();
		pDirectSound = NULL;
	}

	if (dsoundDLL != NULL)
	{
		FreeLibrary(dsoundDLL);
		dsoundDLL = NULL;
	}
}

bool DirectSound::init()
{
	HRESULT hr;

	dsoundDLL = LoadLibrary("DSOUND.DLL");
	HRESULT (WINAPI *DSoundCreate)(LPCGUID, LPDIRECTSOUND *, IUnknown *);
	if (dsoundDLL != NULL)
	{
		DSoundCreate = (HRESULT (WINAPI *)(LPCGUID, LPDIRECTSOUND *, IUnknown *))
		               GetProcAddress(dsoundDLL, "DirectSoundCreate");

		if (DSoundCreate == NULL)
		{
			directXMessage("DirectSoundCreate");
			return false;
		}
	}
	else
	{
		directXMessage("DSOUND.DLL");
		return false;
	}

	if (FAILED(hr = DSoundCreate(NULL, &pDirectSound, NULL)))
	{
		//    errorMessage(myLoadString(IDS_ERROR_SOUND_CREATE), hr);
		systemMessage(IDS_CANNOT_CREATE_DIRECTSOUND,
		              "Cannot create DirectSound %08x", hr);
		pDirectSound = NULL;
		dsbSecondary = NULL;
		return false;
	}

	if (FAILED(hr = pDirectSound->SetCooperativeLevel((HWND)*theApp.m_pMainWnd, DSSCL_EXCLUSIVE)))
	{
		//    errorMessage(myLoadString(IDS_ERROR_SOUND_LEVEL), hr);
		systemMessage(IDS_CANNOT_SETCOOPERATIVELEVEL,
		              "Cannot SetCooperativeLevel %08x", hr);
		return false;
	}

	DSBUFFERDESC dsbdesc;
	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize	= sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

	if (FAILED(hr = pDirectSound->CreateSoundBuffer(&dsbdesc, &dsbPrimary, NULL)))
	{
		//    errorMessage(myLoadString(IDS_ERROR_SOUND_BUFFER),hr);
		systemMessage(IDS_CANNOT_CREATESOUNDBUFFER,
		              "Cannot CreateSoundBuffer %08x", hr);
		return false;
	}

	// Set primary buffer format

	memset(&wfx, 0, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels  = 2;
	switch (soundQuality)
	{
	case 2:
		wfx.nSamplesPerSec	= 22050;
		soundBufferLen		= 736 * 2;
		soundBufferTotalLen = 7360 * 2;
		break;
	case 4:
		wfx.nSamplesPerSec	= 11025;
		soundBufferLen		= 368 * 2;
		soundBufferTotalLen = 3680 * 2;
		break;
	default:
		soundQuality		= 1;
		wfx.nSamplesPerSec	= 44100;
		soundBufferLen		= 1470 * 2;
		soundBufferTotalLen = 14700 * 2;
	}
	wfx.wBitsPerSample	= 16;
	wfx.nBlockAlign		= (wfx.wBitsPerSample / 8) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

	if (FAILED(hr = dsbPrimary->SetFormat(&wfx)))
	{
		//    errorMessage(myLoadString(IDS_ERROR_SOUND_PRIMARY),hr);
		systemMessage(IDS_CANNOT_SETFORMAT_PRIMARY,
		              "Cannot SetFormat for primary %08x", hr);
		return false;
	}

	ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize		  = sizeof(DSBUFFERDESC);
	dsbdesc.dwFlags		  = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS;
	dsbdesc.dwBufferBytes = soundBufferTotalLen;
	dsbdesc.lpwfxFormat	  = &wfx;

	if (FAILED(hr = pDirectSound->CreateSoundBuffer(&dsbdesc, &dsbSecondary, NULL)))
	{
		bool ok = false;
		while (dsbdesc.dwFlags != DSBCAPS_GETCURRENTPOSITION2)
		{
			if (dsbdesc.dwFlags & DSBCAPS_CTRLFREQUENCY)
				dsbdesc.dwFlags ^= DSBCAPS_CTRLFREQUENCY;
			else if (dsbdesc.dwFlags & DSBCAPS_GLOBALFOCUS)
				dsbdesc.dwFlags ^= DSBCAPS_GLOBALFOCUS;
			else if (dsbdesc.dwFlags & DSBCAPS_CTRLPOSITIONNOTIFY)
				dsbdesc.dwFlags ^= DSBCAPS_CTRLPOSITIONNOTIFY;
			if (SUCCEEDED(hr = pDirectSound->CreateSoundBuffer(&dsbdesc, &dsbSecondary, NULL)))
			{
				ok = true;
				break;
			}
		}
		if (!ok)
		{
			systemMessage(IDS_CANNOT_CREATESOUNDBUFFER_SEC, "Cannot CreateSoundBuffer secondary %08x", hr);
			return false;
		}

		dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
	}

	dsbSecondary->SetCurrentPosition(0);

	if (!theApp.useOldSync)
	{
		hr = dsbSecondary->QueryInterface(IID_IDirectSoundNotify,
		                                  (void * *)&dsbNotify);
		if (!FAILED(hr))
		{
			dsbEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

			DSBPOSITIONNOTIFY notify[10];

			for (int i = 0; i < 10; i++)
			{
				notify[i].dwOffset	   = i * soundBufferLen;
				notify[i].hEventNotify = dsbEvent;
			}
			if (FAILED(dsbNotify->SetNotificationPositions(10, notify)))
			{
				dsbNotify->Release();
				dsbNotify = NULL;
				CloseHandle(dsbEvent);
				dsbEvent = NULL;
			}
		}
	}

	hr = dsbPrimary->Play(0, 0, DSBPLAY_LOOPING);

	if (FAILED(hr))
	{
		//    errorMessage(myLoadString(IDS_ERROR_SOUND_PLAYPRIM), hr);
		systemMessage(IDS_CANNOT_PLAY_PRIMARY, "Cannot Play primary %08x", hr);
		return false;
	}

	systemSoundOn = true;

	return true;
}

void DirectSound::setSpeed(float rate)
{
	if (dsbSecondary == NULL || wfx.nSamplesPerSec <= 0)
		return;

	if (rate != curRate)
	{
		curRate = rate;

		if (rate > 4.0f)
			rate = 4.0f;
		if (rate < 0.06f)
			rate = 0.06f;

		dsbSecondary->SetFrequency((DWORD)((float)wfx.nSamplesPerSec * rate));
	}
}

void DirectSound::pause()
{
	if (dsbSecondary != NULL)
	{
		DWORD status = 0;
		dsbSecondary->GetStatus(&status);

		if (status & DSBSTATUS_PLAYING)
		{
			//systemScreenMessage("sound stopped (pause)!", 3);
			dsbSecondary->Stop();
		}
	}
}

bool DirectSound::isPlaying()
{
	if (dsbSecondary != NULL)
	{
		DWORD status = 0;
		dsbSecondary->GetStatus(&status);

		if (status & DSBSTATUS_PLAYING)
		{
			return true;
		}
	}
	return false;
}

void DirectSound::reset()
{
	if (dsbSecondary)
	{
		//systemScreenMessage("sound stopped (reset)!", 3);
		dsbSecondary->Stop();
		dsbSecondary->SetCurrentPosition(0);
	}
}

void DirectSound::resume()
{
	if (dsbSecondary != NULL)
	{
		dsbSecondary->Play(0, 0, DSBPLAY_LOOPING);
	}
}

u32 linearFrameCount	  = 0;
u32 linearSoundByteCount  = 0;
u32 linearSoundFrameCount = 0;

void DirectSound::write()
{
	u32	   len = soundBufferLen;
	LPVOID lpvPtr1;
	DWORD  dwBytes1;
	LPVOID lpvPtr2;
	DWORD  dwBytes2;

	double frameRate = systemGetFrameRate();
	do
	{
		if (pDirectSound != NULL)
		{
			if (theApp.soundRecording)
			{
				if (dsbSecondary)
				{
					if (theApp.soundRecorder == NULL)
					{
						theApp.soundRecorder = new WavWriter;
						WAVEFORMATEX format;
						dsbSecondary->GetFormat(&format, sizeof(format), NULL);
						if (theApp.soundRecorder->Open(theApp.soundRecordName))
							theApp.soundRecorder->SetFormat(&format);
					}
				}

				if (theApp.soundRecorder)
				{
					theApp.soundRecorder->AddSound((u8 *)soundFinalWave, len);
				}
			}

			if (theApp.nvAudioLog)
			{
				NESVideoLoggingAudio((u8 *)soundFinalWave, wfx.nSamplesPerSec, wfx.wBitsPerSample, wfx.nChannels, len /
				                     (wfx.nChannels * (wfx.wBitsPerSample / 8)));
			}

			// alternate avi record routine has been added in VBA.cpp
			if (!theApp.altAviRecordMethod && theApp.aviRecording)
			{
				if (theApp.aviRecorder && !theApp.aviRecorder->IsPaused())
				{
					if (dsbSecondary)
					{
						if (!theApp.aviRecorder->IsSoundAdded())
						{
							WAVEFORMATEX format;
							dsbSecondary->GetFormat(&format, sizeof(format), NULL);
							theApp.aviRecorder->SetSoundFormat(&format);
						}
					}

					theApp.aviRecorder->AddSound((u8 *)soundFinalWave, len);
				}
			}
		}

		linearSoundByteCount += len;
		linearSoundFrameCount = u32(frameRate * double(linearSoundByteCount) / double(wfx.nAvgBytesPerSec));
	}
	while (linearSoundFrameCount <= linearFrameCount);

	// arbitrarily wrap counters at systemGetFrameRateDividend() frames to avoid mismatching wrap-around freeze
	u32 wrap = systemGetFrameRateDividend();
	if (linearSoundFrameCount >= wrap && linearFrameCount >= wrap)
	{
		linearFrameCount	  -= wrap;
		linearSoundByteCount  -= wfx.nAvgBytesPerSec * systemGetFrameRateDivisor();
		linearSoundFrameCount -= wrap;
	}

	if (!pDirectSound)
		return;

	HRESULT hr;

	bool fastForward = speedup;
#if (defined(WIN32) && !defined(SDL))
	fastForward |= theApp.frameSearchSkipping;
#endif

	// slows down emulator to match up with the sound speed
	if (!fastForward && synchronize && !(theApp.throttle > 100 && theApp.accuratePitchThrottle)
		&& theApp.throttle >= 6 && theApp.throttle <= 400)
	{
		DWORD status = 0;
		hr = dsbSecondary->GetStatus(&status);
		if (status & DSBSTATUS_PLAYING)
		{
			if (!soundPaused)
			{
				DWORD play;
				while (true)
				{
					dsbSecondary->GetCurrentPosition(&play, NULL);

					if (soundNextPosition + soundBufferLen < soundBufferTotalLen)
					{
						if (play < soundNextPosition
						    || play > soundNextPosition + soundBufferLen)
							break;
					}
					else
					{
						if (play < soundNextPosition
						    && play > (soundNextPosition + soundBufferLen) % soundBufferTotalLen)
							break;
					}

					if (dsbEvent)
					{
						WaitForSingleObject(dsbEvent, 50);
					}
				}
			}
		}
		else
		{
			soundPaused = 1;
		}
	}

	// Obtain memory address of write block. This will be in two parts
	// if the block wraps around.
	hr = dsbSecondary->Lock(soundNextPosition, soundBufferLen,
	                        &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2,
	                        0);

	if (FAILED(hr))
	{
		char str [256];
		sprintf(str, "Locking secondary failed with %d", hr);
		systemScreenMessage(str);
	}

	// If DSERR_BUFFERLOST is returned, restore and retry lock.
	if (DSERR_BUFFERLOST == hr)
	{
		dsbSecondary->Restore();
		hr = dsbSecondary->Lock(soundNextPosition, soundBufferLen,
		                        &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2,
		                        0);
	}

	if (SUCCEEDED(hr))
	{
		// note: with the old frame timing, this can be entered after theApp.winPauseNextFrame  was set to false...
		if (theApp.muteFrameAdvance && theApp.winPauseNextFrame || theApp.winMuteForNow)
		{
			// Write 0 to pointers.
			if (NULL != lpvPtr1)
				ZeroMemory(lpvPtr1, dwBytes1);
			if (NULL != lpvPtr2)
				ZeroMemory(lpvPtr2, dwBytes2);
		}
		else
		{
			// Write to pointers.
			if (NULL != lpvPtr1)
				CopyMemory(lpvPtr1, soundFinalWave, dwBytes1);
			if (NULL != lpvPtr2)
				CopyMemory(lpvPtr2, soundFinalWave + dwBytes1, dwBytes2);
		}

		// Release the data back to DirectSound.
		hr = dsbSecondary->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	}

	soundNextPosition += soundBufferLen;
	soundNextPosition %= soundBufferTotalLen;
}

void DirectSound::clearAudioBuffer()
{
	LPVOID	lpvPtr1;
	DWORD	dwBytes1;
	LPVOID	lpvPtr2;
	DWORD	dwBytes2;
	HRESULT hr = dsbSecondary->Lock(0, soundBufferTotalLen, &lpvPtr1, &dwBytes1, &lpvPtr2, &dwBytes2, 0);
	if (!FAILED(hr))
	{
		if (lpvPtr1)
			memset(lpvPtr1, 0, dwBytes1);
		if (lpvPtr2)
			memset(lpvPtr2, 0, dwBytes2);
		hr = dsbSecondary->Unlock(lpvPtr1, dwBytes1, lpvPtr2, dwBytes2);
	}
}

ISound *newDirectSound()
{
	return new DirectSound();
}

