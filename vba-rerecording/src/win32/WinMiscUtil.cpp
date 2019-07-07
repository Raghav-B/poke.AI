#include "stdafx.h"
#include "WinMiscUtil.h"
#include "WinResUtil.h"
#include "resource.h"
#include "../NLS.h"
#include "VBA.h"
#include "Reg.h"
#include "../common/SystemGlobals.h"
#include "../common/movie.h"
#include <direct.h>
#include <algorithm>

#include "Dialogs/GSACodeSelect.h"
#include "../gba/GBACheats.h"
#include "../gb/gbCheats.h"

// #undef WinDef macro garbage
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

using std::max;
using std::min;

extern int emulating;

extern const char IDS_ROM_DIR[]		= "romDir";
extern const char IDS_GBXROM_DIR[]	= "gbromDir";
extern const char IDS_BATTERY_DIR[]	= "batteryDir";
extern const char IDS_SAVE_DIR[]	= "saveDir";
extern const char IDS_MOVIE_DIR[]	= "moviesDir";
extern const char IDS_CHEAT_DIR[]	= "cheatsDir";
extern const char IDS_LUA_DIR[]		= "luaDir";
extern const char IDS_IPS_DIR[]		= "ipsDir";
extern const char IDS_AVI_DIR[]		= "aviRecordDir";
extern const char IDS_WAV_DIR[]		= "soundRecordDir";
extern const char IDS_CAPTURE_DIR[] = "captureDir";
extern const char IDS_WATCH_DIR[]	= "watchDir";

extern const char IDS_ROM_DEFAULT_DIR[]		= "\\roms";
extern const char IDS_GBXROM_DEFAULT_DIR[]	= "\\gbroms";
extern const char IDS_BATTERY_DEFAULT_DIR[]	= "\\battery";
extern const char IDS_SAVE_DEFAULT_DIR[]	= "\\save";
extern const char IDS_MOVIE_DEFAULT_DIR[]	= "\\movies";
extern const char IDS_CHEAT_DEFAULT_DIR[]	= "\\cheats";
extern const char IDS_LUA_DEFAULT_DIR[]		= "\\lua";
extern const char IDS_IPS_DEFAULT_DIR[]		= "\\ips";
extern const char IDS_AVI_DEFAULT_DIR[]		= "\\avi";
extern const char IDS_WAV_DEFAULT_DIR[]		= "\\wav";
extern const char IDS_CAPTURE_DEFAULT_DIR[] = "\\screen";
extern const char IDS_WATCH_DEFAULT_DIR[]	= "\\watches";

extern const char *IDS_tbl[] = {
	IDS_ROM_DIR,   IDS_GBXROM_DIR, IDS_BATTERY_DIR, IDS_SAVE_DIR, 
	IDS_MOVIE_DIR, IDS_CHEAT_DIR,  IDS_LUA_DIR,     IDS_IPS_DIR, 
	IDS_AVI_DIR,   IDS_WAV_DIR,    IDS_CAPTURE_DIR, IDS_WATCH_DIR
};

extern const char *IDS_def_tbl[] = {
	IDS_ROM_DEFAULT_DIR,   IDS_GBXROM_DEFAULT_DIR, IDS_BATTERY_DEFAULT_DIR, IDS_SAVE_DEFAULT_DIR, 
	IDS_MOVIE_DEFAULT_DIR, IDS_CHEAT_DEFAULT_DIR,  IDS_LUA_DEFAULT_DIR,     IDS_IPS_DEFAULT_DIR, 
	IDS_AVI_DEFAULT_DIR,   IDS_WAV_DEFAULT_DIR,    IDS_CAPTURE_DEFAULT_DIR, IDS_WATCH_DEFAULT_DIR
};

// these could be made VBA members, but  the VBA class is already oversized too much
//

bool winFileExists(const CString &filename)
{
	FILE *f = fopen(filename, "rb");
	if (f)
	{
		fclose(f);
		return true;
	}
	return false;
}

bool winIsDriveRoot(const CString &file)
{
	if (file.GetLength() == 3)
	{
		if (file[1] == ':' && file[2] == '\\')
			return true;
	}
	return false;
}

CString winGetOriginalFilename(const CString &file)
{
	int index = file.Find('|');

	if (index != -1)
		return file.Left(index);
	else
		return file;
}

CString winGetDirFromFilename(const CString &file)
{
	CString temp  = winGetOriginalFilename(file);
	int		index = max(temp.ReverseFind('/'), temp.ReverseFind('\\'));
	if (index != -1)
	{
		temp = temp.Left(index);
		if (temp.GetLength() == 2 && temp[1] == ':')
			temp += "\\";
	}

	return temp;
}

CString winGetDestDir(const CString &TargetDirReg)
{
	CString targetDir = regQueryStringValue(TargetDirReg, NULL);
	int pos = targetDir.ReverseFind('\\');
	if (pos > 0 && pos == targetDir.GetLength() - 1)
		targetDir.Delete(pos);

	// it makes no sense to create rom directories
	// see MainWnd::winFileOpenSelect for more info
	if (!TargetDirReg.Compare(IDS_ROM_DIR) || !TargetDirReg.Compare(IDS_GBXROM_DIR))
		return targetDir;

	if (targetDir.IsEmpty())
	{
		targetDir = theApp.exeDir;		// reset the targetDir to the application's path
		if (!TargetDirReg.Compare(IDS_BATTERY_DIR))
		{
			targetDir += IDS_BATTERY_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_SAVE_DIR))
		{
			targetDir += IDS_SAVE_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_MOVIE_DIR))
		{
			targetDir += IDS_MOVIE_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_CHEAT_DIR))
		{
			targetDir += IDS_CHEAT_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_LUA_DIR))
		{
			targetDir += IDS_LUA_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_IPS_DIR))
		{
			targetDir += IDS_IPS_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_AVI_DIR))
		{
			targetDir += IDS_AVI_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_WAV_DIR))
		{
			targetDir += IDS_WAV_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_CAPTURE_DIR))
		{
			targetDir += IDS_CAPTURE_DEFAULT_DIR;
		}
		else if (!TargetDirReg.Compare(IDS_WATCH_DIR))
		{
			targetDir += IDS_WATCH_DEFAULT_DIR;
		}
		regSetStringValue(TargetDirReg, targetDir);	// Add the directory to the INI file
	}

	_mkdir(targetDir);			// make the directory

	return targetDir;
}

CString winGetDestFilename(const CString &LogicalRomName, const CString &TargetDirReg, const CString &ext)
{
	if (LogicalRomName.GetLength() == 0)
		return CString();

	CString targetDir = winGetDestDir(TargetDirReg);
	targetDir += '\\';

	CString buffer = LogicalRomName;

	int index = max(buffer.ReverseFind('/'), max(buffer.ReverseFind('\\'), buffer.ReverseFind('|')));
	if (index != -1)
		buffer = buffer.Right(buffer.GetLength() - index - 1);

	index = buffer.ReverseFind('.');
	if (index != -1)
		buffer = buffer.Left(index);

	CString filename;
	filename.Format("%s%s%s", targetDir, buffer, ext);
	bool fileExists = winFileExists(filename);

	// check for old style of naming, for better backward compatibility
	if (!fileExists || theApp.filenamePreference == 0)
	{
		index = LogicalRomName.Find('|');
		if (index != -1)
		{
			buffer = LogicalRomName.Left(index);
			index  = max(buffer.ReverseFind('/'), buffer.ReverseFind('\\'));

			int dotIndex = buffer.ReverseFind('.');
			if (dotIndex > index)
				buffer = buffer.Left(dotIndex);

			if (index != -1)
				buffer = buffer.Right(buffer.GetLength() - index - 1);

			CString filename2;
			filename2.Format("%s%s%s", targetDir, buffer, ext);
			bool file2Exists = winFileExists(filename2);

			if ((file2Exists && !fileExists) || (theApp.filenamePreference == 0 && (file2Exists || !fileExists)))
				return filename2;
		}
	}

	return filename;
}

CString winGetSavestateFilename(const CString &LogicalRomName, int nID)
{
	CString		ext;
//	size_t		startindex;	// forget about C89/ANSI-C
//	size_t		endindex;
	if (VBAMovieIsActive() && theApp.AsscWithSaveState)
	{
		std::string fs(VBAMovieGetFilename());	// RVO tip
		size_t startindex = fs.find_last_of("/\\");
		if (startindex < fs.length())
			++startindex;	// luckily the found character can't be at the end of fs
		else
			startindex = 0;
		size_t endindex = fs.find_last_of(".");
		if (endindex < fs.length() && endindex > startindex)
			endindex;	//??
		else
			endindex = fs.length();
		fs = fs.substr(startindex, endindex - startindex);
		ext.Format("-%s-%d.sgm", fs.c_str(), nID);
	}
	else
	{
		ext.Format("%d.sgm", nID);
	}
	return winGetDestFilename(LogicalRomName, IDS_SAVE_DIR, ext);
}

CString winGetSavestateMenuString(const CString &LogicalRomName, int nID)
{
	CString str;
	if (theApp.showSlotTime)
	{
		CFileStatus status;
		if (emulating && CFile::GetStatus(winGetSavestateFilename(LogicalRomName, nID), status))
		{
			str.Format("#&%d %s", nID, status.m_mtime.Format("%Y/%m/%d %H:%M:%S"));
		}
		else
		{
			str.Format("#&%d ----/--/-- --:--:--", nID);
		}
	}
	else
	{
		str.Format("Slot #&%d", nID);
	}

	return str;
}

void winCorrectPath(CString &path)
{
	if (winFileExists(path))
	{
		return;
	}

	CString tempStr = theApp.exeDir;
	tempStr += "\\";
	tempStr += path;

	if (winFileExists(tempStr))
	{
		path = tempStr;
		return;
	}

	for (int i = 0; i < _countof(IDS_tbl); ++i)
	{
		tempStr = winGetDestDir(IDS_tbl[i]);
		tempStr += "\\";
		tempStr += path;

		if (winFileExists(tempStr))
		{
			path = tempStr;
			return;
		}
	}
}

void winCorrectPath(char *path)
{
	CString pathCStr(path);
	winCorrectPath(pathCStr);
	strcpy(path, pathCStr);
}

// some file I/O

int winScreenCapture(int captureNumber)
{
	CString ext;
	CString captureName;

	do
	{
		if (theApp.captureFormat == 0)
			ext.Format("_%03d.png", captureNumber);
		else
			ext.Format("_%03d.bmp", captureNumber);

		captureName = winGetDestFilename(theApp.gameFilename, IDS_CAPTURE_DIR, ext);
		++captureNumber;
	} while (winFileExists(captureName) && captureNumber > 0);

	if (captureNumber < 0)
	{
		systemMessage(0, "Too many existing files (not less than %d)! Screen capture failed!", captureNumber - 1);
		return 0;
	}

	if (theApp.captureFormat == 0)
		theApp.emulator.emuWritePNG(captureName);
	else
		theApp.emulator.emuWriteBMP(captureName);

	systemScreenMessage(winResLoadString(IDS_SCREEN_CAPTURE));

	return captureNumber;
}

bool winImportGSACodeFile(CString &fileName)
{
	FILE *f = fopen(fileName, "rb");

	if (f == NULL)
	{
		systemMessage(MSG_CANNOT_OPEN_FILE, "Cannot open file %s", fileName);
		return false;
	}

	if (systemCartridgeType == 1)
	{
		fclose(f);
		return gbCheatReadGSCodeFile(fileName);
	}

	u32 len;
	fread(&len, 1, 4, f);
	if (len != 14)
	{
		fclose(f);
		systemMessage(MSG_UNSUPPORTED_CODE_FILE, "Unsupported code file %s",
		              fileName);
		return false;
	}
	char buffer[16];
	fread(buffer, 1, 14, f);
	buffer[14] = 0;
	if (memcmp(buffer, "SharkPortCODES", 14))
	{
		fclose(f);
		systemMessage(MSG_UNSUPPORTED_CODE_FILE, "Unsupported code file %s",
		              fileName);
		return false;
	}
	fseek(f, 0x1e, SEEK_SET);
	fread(&len, 1, 4, f);
	int game = 0;
	if (len > 1)
	{
		GSACodeSelect dlg(f);
		game = dlg.DoModal();
	}
	fclose(f);

	bool v3 = false;

	int index = fileName.ReverseFind('.');

	if (index != -1)
	{
		if (fileName.Right(3).CompareNoCase("XPC") == 0)
			v3 = true;
	}

	if (game != -1)
	{
		return cheatsImportGSACodeFile(fileName, game, v3);
	}

	return true;
}

void winLoadCheatList(const char *name)
{
	bool res = false;

	if (systemCartridgeType == IMAGE_GBA)
		res = cheatsLoadCheatList(name);
	else
		res = gbCheatsLoadCheatList(name);

	if (res)
		systemScreenMessage(winResLoadString(IDS_LOADED_CHEATS));
}

void winSaveCheatList(const char *name)
{
	if (systemCartridgeType == IMAGE_GBA)
		cheatsSaveCheatList(name);
	else
		gbCheatsSaveCheatList(name);
}

void winLoadCheatListDefault()
{
	CString cheatName = winGetDestFilename(theApp.gameFilename, IDS_CHEAT_DIR, ".clt");

	winLoadCheatList(cheatName);
}

void winSaveCheatListDefault()
{
	CString cheatName = winGetDestFilename(theApp.gameFilename, IDS_CHEAT_DIR, ".clt");

	winSaveCheatList(cheatName);
}

bool winReadBatteryFile()
{
	CString batteryName = winGetDestFilename(theApp.gameFilename, IDS_BATTERY_DIR, ".sav");

	bool res = false;

	if (theApp.emulator.emuReadBattery)
		res = theApp.emulator.emuReadBattery(batteryName);

	if (res)
		systemScreenMessage(winResLoadString(IDS_LOADED_BATTERY));

	return res;
}

bool winWriteBatteryFile()
{
	CString batteryName = winGetDestFilename(theApp.gameFilename, IDS_BATTERY_DIR, ".sav");

	if (theApp.emulator.emuWriteBattery)
		return theApp.emulator.emuWriteBattery(batteryName);

	return false;
}

bool winEraseBatteryFile()
{
	CString batteryName = winGetDestFilename(theApp.gameFilename, IDS_BATTERY_DIR, ".sav");
	return !remove(batteryName);
}

bool winReadSaveGame(const char *name)
{
	if (theApp.emulator.emuReadState)
		return theApp.emulator.emuReadState(name);
	return false;
}

bool winWriteSaveGame(const char *name)
{
	if (theApp.emulator.emuWriteState)
		return theApp.emulator.emuWriteState(name);
	return false;
}

bool winEraseSaveGame(const char *name)
{
	return !remove(name);
}
