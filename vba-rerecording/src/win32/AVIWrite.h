#ifndef VBA_WIN32_AVIWRITE_H
#define VBA_WIN32_AVIWRITE_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vfw.h>

class AVIWrite {
 public:
  AVIWrite();
  virtual ~AVIWrite();

  bool Open(const char *filename);
  virtual bool AddFrame(const u8 *bmp);
  void SetVideoFormat(BITMAPINFOHEADER *);
  bool IsSoundAdded();
  void SetSoundFormat(WAVEFORMATEX *);
  bool AddSound(const u8 *sound, u32 len);
  int videoFrames();
  void Pause(bool pause);
  bool IsPaused();

 private:
  WAVEFORMATEX m_soundFormat;
  BITMAPINFOHEADER m_bitmap;
  AVISTREAMINFO m_header;
  AVISTREAMINFO m_soundHeader;
  PAVIFILE m_file;
  PAVISTREAM m_stream;
  PAVISTREAM m_streamCompressed;
  PAVISTREAM m_streamSound;
  AVICOMPRESSOPTIONS m_options;
  AVICOMPRESSOPTIONS *m_arrayOptions[1];
  LONG m_videoFrames;
  LONG m_samplesSound;
  LONG m_videoFramesTotal;
  LONG m_samplesSoundTotal;
  LONG m_totalBytes;
  bool m_failed;
  int m_segmentNumber;
  bool m_usePrevOptions;
  bool m_pauseRecording;
  char m_aviFileName[MAX_PATH];
  char m_aviBaseName[MAX_PATH];
  char m_aviExtension[MAX_PATH];
  void CleanUp();
  bool NextSegment();
};

#endif // VBA_WIN32_AVIWRITE_H
