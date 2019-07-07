#ifndef VBA_WIN32_WINMISCUTIL_H
#define VBA_WIN32_WINMISCUTIL_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern const char IDS_ROM_DIR[];
extern const char IDS_GBXROM_DIR[];
extern const char IDS_BATTERY_DIR[];
extern const char IDS_SAVE_DIR[];
extern const char IDS_MOVIE_DIR[];
extern const char IDS_CHEAT_DIR[];
extern const char IDS_LUA_DIR[];
extern const char IDS_IPS_DIR[];
extern const char IDS_AVI_DIR[];
extern const char IDS_WAV_DIR[];
extern const char IDS_CAPTURE_DIR[];
extern const char IDS_WATCH_DIR[];

extern bool winFileExists(const CString &filename);
extern bool winIsDriveRoot(const CString &file);
extern CString winGetOriginalFilename(const CString &file);
extern CString winGetDirFromFilename(const CString &file);
extern CString winGetSavestateFilename(const CString &LogicalRomName, int nID);
extern CString winGetSavestateMenuString(const CString &LogicalRomName, int nID);
extern CString winGetDestDir(const CString &TargetDirReg);
extern CString winGetDestFilename(const CString &LogicalRomName, const CString &TargetDirReg, const CString &ext);
extern void winCorrectPath(CString &path);
extern void winCorrectPath(char *path);

int winScreenCapture(int captureNumber = 0);
bool winImportGSACodeFile(CString& fileName);
void winLoadCheatList(const char *name);
void winSaveCheatList(const char *name);
void winLoadCheatListDefault();
void winSaveCheatListDefault();
bool winReadBatteryFile();
bool winWriteBatteryFile();
bool winEraseBatteryFile();
bool winReadSaveGame(const char *name);
bool winWriteSaveGame(const char *name);
bool winEraseSaveGame(const char *name);

#endif // VBA_WIN32_WINMISCUTIL_H
