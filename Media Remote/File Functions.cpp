#include <stdio.h>
#include <windows.h>
#include "resource.h"
#include "WinMain.h"


OPENFILENAME ofn;

extern MainControls MainBinds;
extern WAControls WinampBinds;
extern WMPControls WMPBinds;
extern ITControls ITBinds;
extern MKControls MediaKeyBinds;
extern HWND hSelf;
extern bool PollPrograms, WindowKeysDisabled, DisableBoundKeys, EnableWinampSongNotify;
extern unsigned int Tab;


bool ParseFileInfo(FILE *pFile, HWND *Controls, unsigned int nBinds, char *MatchString, unsigned int &FirstBind,
					unsigned int &FirstlParam, bool SendMessages)
{
	char FileBuffer[200] = "", TempBuf[200];
	unsigned int *pData, *lParams;

	fgets(FileBuffer, 199, pFile);
	while(strcmp(FileBuffer, MatchString) != 0)
		if(fgets(FileBuffer, 199, pFile) == NULL) {
			MessageBox(hSelf, "The config file is corrupted. Delete it and restart the program.", "Error", MB_OK | MB_ICONERROR);
			exit(9);
		}

	for(unsigned int x=0; x<nBinds; x++) {
		fgets(FileBuffer, 199, pFile);
		unsigned int z;

		pData = &FirstBind;
		lParams = &FirstlParam;

		pData += x;
		lParams += x;

		for(z=0; z<strlen(FileBuffer)+1; z++)
			if(FileBuffer[z] == '=') {
				z++;
				break;
			}


		strncpy(TempBuf, FileBuffer+(z), 4);
		TempBuf[4] = '\0';
		*pData = atoi(TempBuf);
		strncpy(TempBuf, FileBuffer+(z+5), 20);
		*lParams = atoi(TempBuf);

		if(*lParams == 0 || SendMessages == false)
			continue;

		SendMessage(Controls[nBinds+x], WM_KEYDOWN, *pData, *lParams);

	}

	return true;

}




void ParseFileMiscSettings(FILE *pFile, unsigned int BoolCount, char *MatchString, const HWND &hwnd)
{
	char FileBuffer[200] = "", TempBuf[200];
	unsigned int TempHolder;

	fgets(FileBuffer, 199, pFile);
	while(strcmp(FileBuffer, MatchString) != 0)
		if(fgets(FileBuffer, 199, pFile) == NULL) {
			MessageBox(hSelf, "The config file is corrupted. Delete it and restart the program.", "Error", MB_OK | MB_ICONERROR);
			exit(9);
		}

	for(unsigned int x=0; x<BoolCount; x++) {
		fgets(FileBuffer, 199, pFile);
		unsigned int z;

		for(z=0; z<strlen(FileBuffer)+1; z++)
			if(FileBuffer[z] == '=') {
				z++;
				break;
			}

		strncpy(TempBuf, FileBuffer+(z), 4);
		TempBuf[4] = '\0';

		TempHolder = atoi(TempBuf);

		


		if(x == 0) {
			EnableWinampSongNotify = (TempHolder) ? false : true;
			SendMessage(hwnd, WM_COMMAND, LOWORD(IDM_WINAMPSONGNOTIFY), 0);
		}
		else if(x == 1) {
			DisableBoundKeys = (TempHolder) ? false : true;
			SendMessage(hwnd, WM_COMMAND, LOWORD(IDM_DISABLEBOUNDKEYS), 0);
		}
		else if(x == 2) {
			WindowKeysDisabled = (TempHolder) ? false : true;
			SendMessage(hwnd, WM_COMMAND, LOWORD(IDM_DISABLEWINKEYS), 0);
		}
		else if(x == 3) {
			PollPrograms = (TempHolder) ? false : true;
			SendMessage(hwnd, WM_COMMAND, LOWORD(IDM_POLLALL), 0);
		}
	}


}





void ParseWinampPlaylist(FILE *pFile)
{
	unsigned int z;
	char FileBuffer[1000] = "", TempBuf[1000];
	fgets(FileBuffer, 999, pFile);
	while(strcmp(FileBuffer, "[Winamp Playlist List]\n") != 0)
		if(fgets(FileBuffer, 999, pFile) == NULL) {
			MessageBox(hSelf, "The config file is corrupted. Delete it and restart the program.", "Error", MB_OK | MB_ICONERROR);
			exit(9);
		}
	fgets(FileBuffer, 999, pFile);
	for(z=0; z<strlen(FileBuffer)+1; z++)
		if(FileBuffer[z] == '=') {
			z++;
			break;
		}


	strncpy(TempBuf, FileBuffer+(z), 500);
	WinampBinds.PlaylistCount = atoi(TempBuf);

	WinampBinds.PlaylistPaths = new char[WinampBinds.PlaylistCount+1][1000];
	memset(WinampBinds.PlaylistPaths, 0, 1000*WinampBinds.PlaylistCount+1000);
	WinampBinds.Playlists = new unsigned int[WinampBinds.PlaylistCount+1];
	memset(WinampBinds.Playlists, 0, WinampBinds.PlaylistCount*4);
	WinampBinds.plParams = new unsigned int[WinampBinds.PlaylistCount+1];
	memset(WinampBinds.plParams, 0, WinampBinds.PlaylistCount*4);

	fgets(FileBuffer, 999, pFile);
	for(unsigned int x=0; x<WinampBinds.PlaylistCount; x++) {
		FileBuffer[strlen(FileBuffer)-1] = '\0';
		strcpy_s(WinampBinds.PlaylistPaths[x], FileBuffer);
		if(fgets(FileBuffer, 999, pFile) == NULL)
			break;
	}
}



void ParseWMPPlaylist(FILE *pFile)
{
	unsigned int z;
	char FileBuffer[1000] = "", TempBuf[1000];
	fgets(FileBuffer, 999, pFile);
	while(strcmp(FileBuffer, "[Windows Media Player Playlist List]\n") != 0)
		if(fgets(FileBuffer, 999, pFile) == NULL) {
			MessageBox(hSelf, "The config file is corrupted. Delete it and restart the program.", "Error", MB_OK | MB_ICONERROR);
			exit(9);
		}
	fgets(FileBuffer, 999, pFile);
	for(z=0; z<strlen(FileBuffer)+1; z++)
		if(FileBuffer[z] == '=') {
			z++;
			break;
		}


	strncpy(TempBuf, FileBuffer+(z), 500);
	WMPBinds.PlaylistCount = atoi(TempBuf);

	WMPBinds.PlaylistPaths = new char[WMPBinds.PlaylistCount+1][1000];
	memset(WMPBinds.PlaylistPaths, 0, 1000*WMPBinds.PlaylistCount+1000);
	WMPBinds.Playlists = new unsigned int[WMPBinds.PlaylistCount+1];
	memset(WMPBinds.Playlists, 0, WMPBinds.PlaylistCount*4);
	WMPBinds.plParams = new unsigned int[WMPBinds.PlaylistCount+1];
	memset(WMPBinds.plParams, 0, WMPBinds.PlaylistCount*4);

	fgets(FileBuffer, 999, pFile);
	for(unsigned int x=0; x<WMPBinds.PlaylistCount; x++) {
		FileBuffer[strlen(FileBuffer)-1] = '\0';
		strcpy(WMPBinds.PlaylistPaths[x], FileBuffer);
		if(fgets(FileBuffer, 999, pFile) == NULL)
			break;
	}
}


void ParseITPlaylist(FILE *pFile)
{
	unsigned int z;
	char FileBuffer[1000] = "", TempBuf[1000];
	fgets(FileBuffer, 999, pFile);
	while(strcmp(FileBuffer, "[iTunes Playlist List]\n") != 0)
		if(fgets(FileBuffer, 999, pFile) == NULL) {
			MessageBox(hSelf, "The config file is corrupted. Delete it and restart the program.", "Error", MB_OK | MB_ICONERROR);
			exit(9);
		}
	fgets(FileBuffer, 999, pFile);
	for(z=0; z<strlen(FileBuffer)+1; z++)
		if(FileBuffer[z] == '=') {
			z++;
			break;
		}


	strncpy(TempBuf, FileBuffer+(z), 500);
	ITBinds.PlaylistCount = atoi(TempBuf);

	ITBinds.PlaylistPaths = new char[ITBinds.PlaylistCount+1][1000];
	memset(ITBinds.PlaylistPaths, 0, 1000*ITBinds.PlaylistCount+1000);
	ITBinds.Playlists = new unsigned int[ITBinds.PlaylistCount+1];
	memset(ITBinds.Playlists, 0, ITBinds.PlaylistCount*4);
	ITBinds.plParams = new unsigned int[ITBinds.PlaylistCount+1];
	memset(ITBinds.plParams, 0, ITBinds.PlaylistCount*4);

	fgets(FileBuffer, 999, pFile);
	for(unsigned int x=0; x<ITBinds.PlaylistCount; x++) {
		FileBuffer[strlen(FileBuffer)-1] = '\0';
		strcpy(ITBinds.PlaylistPaths[x], FileBuffer);
		if(fgets(FileBuffer, 999, pFile) == NULL)
			break;
	}
}




bool SaveSettings(HWND hwnd, FILE *pFile)
{
	char FileBuffer[5000];
	unsigned int x;
	sprintf(FileBuffer, "[Winamp Playlist List]\n");
	fputs(FileBuffer, pFile);
	for(x=0; WinampBinds.PlaylistPaths[x][0] != '\0'; x++);
	sprintf(FileBuffer, "Count = %u\n", x);
	fputs(FileBuffer, pFile);
	for(x=0; WinampBinds.PlaylistPaths[x][0] != '\0'; x++) {
		sprintf(FileBuffer, "%s\n", WinampBinds.PlaylistPaths[x]);
		fputs(FileBuffer, pFile);
	}
	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);


	sprintf(FileBuffer, "[Windows Media Player Playlist List]\n");
	fputs(FileBuffer, pFile);
	for(x=0; WMPBinds.PlaylistPaths[x][0] != '\0'; x++);
	sprintf(FileBuffer, "Count = %u\n", x);
	fputs(FileBuffer, pFile);
	for(x=0; WMPBinds.PlaylistPaths[x][0] != '\0'; x++) {
		sprintf(FileBuffer, "%s\n", WMPBinds.PlaylistPaths[x]);
		fputs(FileBuffer, pFile);
	}
	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);

	sprintf(FileBuffer, "[iTunes Playlist List]\n");
	fputs(FileBuffer, pFile);
	for(x=0; ITBinds.PlaylistPaths[x][0] != '\0'; x++);
	sprintf(FileBuffer, "Count = %u\n", x);
	fputs(FileBuffer, pFile);
	for(x=0; ITBinds.PlaylistPaths[x][0] != '\0'; x++) {
		sprintf(FileBuffer, "%s\n", ITBinds.PlaylistPaths[x]);
		fputs(FileBuffer, pFile);
	}




	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "[Main Program Settings]\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Enabled = %u  %u\n", MainBinds.Enabled, MainBinds.lParams[0]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "[Misc Program Settings]\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Winamp Song Notification = %u\n", (EnableWinampSongNotify) ? 1 : 0);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Disable Bound Keys = %u\n", (DisableBoundKeys) ? 1 : 0);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Disable Window Keys = %u\n", (WindowKeysDisabled) ? 1 : 0);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Attach Players Automatically = %u\n", (PollPrograms) ? 1 : 0);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "[Winamp Settings]\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Play = %u  %u\n", WinampBinds.Play, WinampBinds.lParams[0]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Stop = %u  %u\n", WinampBinds.Stop, WinampBinds.lParams[1]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Pause = %u  %u\n", WinampBinds.Pause, WinampBinds.lParams[2]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Previous = %u  %u\n", WinampBinds.Previous, WinampBinds.lParams[3]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Next = %u  %u\n", WinampBinds.Next, WinampBinds.lParams[4]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Shuffle = %u  %u\n", WinampBinds.Shuffle, WinampBinds.lParams[5]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Repeat = %u  %u\n", WinampBinds.Repeat, WinampBinds.lParams[6]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Up = %u  %u\n", WinampBinds.VolumeUp, WinampBinds.lParams[7]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Down = %u  %u\n", WinampBinds.VolumeDown, WinampBinds.lParams[8]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Rewind = %u  %u\n", WinampBinds.Rewind, WinampBinds.lParams[9]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Fast Forward = %u  %u\n", WinampBinds.Forward, WinampBinds.lParams[10]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "[Winamp Playlist Binds]\n");
	fputs(FileBuffer, pFile);
	for(x=0; x<WinampBinds.PlaylistCount; x++) {
		sprintf(FileBuffer, "Playlist%i = %u  %u\n", x+1, WinampBinds.Playlists[x], WinampBinds.plParams[x]);
		fputs(FileBuffer, pFile);
	}
	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);



	sprintf(FileBuffer, "[Windows Media Player Playlist Binds]\n");
	fputs(FileBuffer, pFile);
	for(x=0; x<WMPBinds.PlaylistCount; x++) {
		sprintf(FileBuffer, "Playlist%i = %u  %u\n", x+1, WMPBinds.Playlists[x], WMPBinds.plParams[x]);
		fputs(FileBuffer, pFile);
	}
	



	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "[Windows Media Player Settings]\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Play/Pause = %u  %u\n", WMPBinds.Play, WMPBinds.lParams[0]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Stop = %u  %u\n", WMPBinds.Stop, WMPBinds.lParams[1]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Previous = %u  %u\n", WMPBinds.Previous, WMPBinds.lParams[2]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Next = %u  %u\n", WMPBinds.Next, WMPBinds.lParams[3]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Up = %u  %u\n", WMPBinds.VolumeUp, WMPBinds.lParams[4]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Down = %u  %u\n", WMPBinds.VolumeDown, WMPBinds.lParams[5]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Mute = %u  %u\n", WMPBinds.Mute, WMPBinds.lParams[6]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Repeat = %u  %u\n", WMPBinds.Repeat, WMPBinds.lParams[7]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Shuffle = %u  %u\n", WMPBinds.Shuffle, WMPBinds.lParams[8]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);

	sprintf(FileBuffer, "[iTunes Playlist Binds]\n");
	fputs(FileBuffer, pFile);
	for(x=0; x<ITBinds.PlaylistCount; x++) {
		sprintf(FileBuffer, "Playlist%i = %u  %u\n", x+1, ITBinds.Playlists[x], ITBinds.plParams[x]);
		fputs(FileBuffer, pFile);
	}
	



	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "[iTunes Settings]\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Play/Pause = %u  %u\n", ITBinds.Play, ITBinds.lParams[0]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Stop = %u  %u\n", ITBinds.Stop, ITBinds.lParams[1]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Previous = %u  %u\n", ITBinds.Previous, ITBinds.lParams[2]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Next = %u  %u\n", ITBinds.Next, ITBinds.lParams[3]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Up = %u  %u\n", ITBinds.VolumeUp, ITBinds.lParams[4]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Down = %u  %u\n", ITBinds.VolumeDown, ITBinds.lParams[5]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Mute = %u  %u\n", ITBinds.Mute, ITBinds.lParams[6]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Repeat = %u  %u\n", ITBinds.Repeat, ITBinds.lParams[7]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Shuffle = %u  %u\n", ITBinds.Shuffle, ITBinds.lParams[8]);
	fputs(FileBuffer, pFile);



	sprintf(FileBuffer, "\n\n\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "[Media Key Settings]\n");
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Play/Pause = %u  %u\n", MediaKeyBinds.Play, MediaKeyBinds.lParams[0]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Previous = %u  %u\n", MediaKeyBinds.Previous, MediaKeyBinds.lParams[1]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Next = %u  %u\n", MediaKeyBinds.Next, MediaKeyBinds.lParams[2]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Up = %u  %u\n", MediaKeyBinds.VolumeUp, MediaKeyBinds.lParams[3]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Volume Down = %u  %u\n", MediaKeyBinds.VolumeDown, MediaKeyBinds.lParams[4]);
	fputs(FileBuffer, pFile);
	sprintf(FileBuffer, "Mute = %u  %u\n", MediaKeyBinds.Mute, MediaKeyBinds.lParams[5]);
	fputs(FileBuffer, pFile);


	return true;
}



void ConfigFileInitialize(HWND hwnd)
{
	static TCHAR szFilter[] = TEXT("Media Remote Config Files (*.cfg)\0*.cfg\0")  \
							  TEXT("All Files (*.*)\0*.*\0\0");
     
	ofn.lStructSize       = sizeof(OPENFILENAME);
	ofn.hwndOwner         = hwnd;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL;          // Set in Open and Close functions
	ofn.nMaxFile          = MAX_PATH;
	ofn.lpstrFileTitle    = NULL;          // Set in Open and Close functions
	ofn.nMaxFileTitle     = MAX_PATH;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = NULL;
	ofn.Flags             = 0;             // Set in Open and Close functions
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = TEXT("txt");
	ofn.lCustData         = 0L;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;
}

void WinampAddFileInitialize(HWND hwnd)
{
	static TCHAR szFilter[] = TEXT("Playlist/Mp3 Files (*.m3u, *.mp3)\0*.m3u;*.mp3\0")  \
							  TEXT("All Files (*.*)\0*.*\0\0");
     
	ofn.lStructSize       = sizeof(OPENFILENAME);
	ofn.hwndOwner         = hwnd;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.lpstrFile         = NULL;          // Set in Open and Close functions
	ofn.nMaxFile          = WINAMPTOTALPLAYLISTBUFFER;
	ofn.lpstrFileTitle    = NULL;          // Set in Open and Close functions
	ofn.nMaxFileTitle     = NULL;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = NULL;
	ofn.Flags             = 0;             // Set in Open and Close functions
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = TEXT("txt");
	ofn.lCustData         = 0L;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;
}


BOOL FileOpenDlgWinamp(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner         = hwnd;
	ofn.lpstrTitle		  = "Add Files To Playlist";
	ofn.lpstrFile         = pstrFileName;
	//ofn.lpstrFileTitle    = pstrTitleName;
	ofn.Flags             = OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

	return GetOpenFileName(&ofn);
}

BOOL FileOpenDlgConfig(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner         = hwnd;
	ofn.lpstrTitle		  = "Load Config File";
	ofn.lpstrFile         = pstrFileName;
	ofn.lpstrFileTitle    = pstrTitleName;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_EXPLORER;

	return GetOpenFileName(&ofn);
}


BOOL FileSaveDlgConfig(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName)
{
	ofn.hwndOwner         = hwnd;
	ofn.lpstrTitle		  = "Save Config File As";
	ofn.lpstrFile         = pstrFileName;
	ofn.lpstrFileTitle    = pstrTitleName;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT;

	return GetSaveFileName(&ofn);
}