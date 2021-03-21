#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"") 

#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <string>
#include <commctrl.h>
#include <DSound.h>
#include "WinMain.h"
#include "Resource.h"
#include "iTunesCOMInterface.h"



/* Function Prototypes */
LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK HotKeyProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK KeyboardProc(int, WPARAM, LPARAM);
extern LRESULT CALLBACK WinampSongToolTipProc(HWND, UINT, WPARAM, LPARAM);
extern BOOL CALLBACK DlgWinampPlaylistProc(HWND, UINT, WPARAM, LPARAM);
extern bool SaveSettings(HWND, FILE *);
extern bool ParseFileInfo(FILE *, HWND *, unsigned int, char *, unsigned int&, unsigned int&, bool);
extern void ParseFileMiscSettings(FILE *, unsigned int, char *, const HWND &);
extern bool SetWindowButtons(HWND *, unsigned int, unsigned int &, unsigned int &);
extern void SetupWindowMain(HWND, HWND *);
extern void SetupWindowMediaKeys(HWND, HWND *);
extern void SetupWindowWinamp(HWND, HWND *);
extern void SetupWindowWMP(HWND, HWND *);
extern void SetupWindowITunes(HWND, HWND *);
extern void DestroyWindowControls(HWND *);
extern void ConfigFileInitialize(HWND);
extern BOOL FileOpenDlgConfig(HWND, PTSTR, PTSTR);
extern BOOL FileSaveDlgConfig(HWND, PTSTR, PTSTR);
extern void InitSounds(char *, HWND);
extern void PlaySoundDll(IDirectSoundBuffer *&, HWND, char *);
extern void ParseWinampPlaylist(FILE *);
extern void ParseWMPPlaylist(FILE *pFile);
extern void ParseITPlaylist(FILE *pFile);
extern void RefreshComboBox();
extern bool Authenticate();



/* Global Variables */
HINSTANCE hInst;
HWND hWinamp, hWMP, hITunes;
IiTunes* iITunes;
HWND hSelf;
HHOOK hHook;
WNDPROC WndprocHotKey, WndprocCBToolTip1, WndprocCBToolTip2, WndprocWinampHUDTooltip;
WAControls WinampBinds;
WMPControls WMPBinds;
ITControls ITBinds;
MKControls MediaKeyBinds;
MainControls MainBinds;
bool ProgramEnabled = true, WinampControlsEnabled = true, WMPControlsEnabled = true, ITControlsEnabled = true, PollPrograms = false,
		WindowKeysDisabled = false, DisableBoundKeys = false, EnableWinampSongNotify = false;
unsigned int Tab = 0;



extern HWND hwndCB, hwndST;
extern IDirectSoundBuffer *MR_Disabled, *MR_Enabled, *Lost, *Rguess, *Wguess;


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT ("Media RM");
	HWND         hwnd;
	MSG          msg;
	WNDCLASS     wndclass;
	
	
	if(hSelf = FindWindow("Media RM", "Media Remote")) {
		if(IsIconic(hSelf))
			ShowWindow(hSelf, SW_SHOWNORMAL);
		SetForegroundWindow(hSelf);
		exit(9);
	}
	hWinamp = FindWindow("Winamp v1.x", NULL);
	hWMP = FindWindow("WMPlayerApp", NULL);


	wndclass.style         = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = NULL;//(HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName  = NULL;//(HMENU)LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1));
	wndclass.lpszClassName = szAppName;
	 
	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), 
			szAppName, MB_ICONERROR);
		return 0;
	}
    
	hInst = hInstance;
	hwnd = CreateWindow(szAppName,													// window class name
						TEXT("Media Remote"),										// window caption
						WS_OVERLAPPEDWINDOW ^ (WS_MAXIMIZEBOX | WS_THICKFRAME),     // window style
						CW_USEDEFAULT,												// initial x position
						CW_USEDEFAULT,												// initial y position
						330,														// initial x size
						500,														// initial y size
						NULL,														// parent window handle
						LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1)),            // window menu handle
						hInstance,													// program instance handle
						NULL);														// creation parameters
     
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	hSelf = hwnd;
	
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(IsDialogMessage(hwnd, &msg) == 0) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
	}
	return (int)msg.wParam;
}




LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC         hdc, hdc2;
	PAINTSTRUCT ps;
	RECT        rect;
	FILE *pFile;
	HMENU hMenu, hPopMenu;
	HBITMAP hPic;
	TCITEM tie;
	HKEY hKey;
	DWORD PathSize;
	static char DefConfigPath[MAX_PATH]={'\0'}, ConfigPath[MAX_PATH]={'\0'}, ConfigName[MAX_PATH]={'\0'};
	static HWND Controls[30], hwndTab;
	static NOTIFYICONDATA TrayData;
	static bool TrueQuit = false;
	static char InstallPath[MAX_PATH];
	static HMODULE hWinampDll;
	static BOOL (WINAPI *InstallMsgHook)(BOOL, HWND);
	static long ITunesVolume = 0;


     
	switch (message)
	{
	case WM_CREATE:
		hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInst, NULL);

		hWinampDll = LoadLibrary("WinampPlugin.dll");


		if(hHook == NULL)
			SendMessage(hwnd, WM_DESTROY, NULL, NULL);


		GetClientRect(hwnd, &rect);
		hwndTab = CreateWindow(WC_TABCONTROL, "",
						WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_TABSTOP,// | BS_OWNERDRAW,
						0, 0, rect.right, 23,
						hwnd, NULL, hInst, NULL);
		
		tie.mask = TCIF_TEXT | TCIF_IMAGE;
		tie.iImage = -1;
		tie.pszText = "Main";
		TabCtrl_InsertItem(hwndTab, MAINPROGRAMTAB, &tie);
		tie.pszText = "Media Keys";
		TabCtrl_InsertItem(hwndTab, MEDIATAB, &tie);
		tie.pszText = "Winamp";
		TabCtrl_InsertItem(hwndTab, WINAMPTAB, &tie);
		tie.pszText = "Media Player";
		TabCtrl_InsertItem(hwndTab, WMPTAB, &tie);
		tie.pszText = "iTunes";
		TabCtrl_InsertItem(hwndTab, ITUNESTAB, &tie);
		SendMessage(hwndTab, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT ), TRUE);
		TabCtrl_SetMinTabWidth(hwndTab, 57);
		TabCtrl_SetPadding(hwndTab, 24, 4);



		SetupWindowMain(hwnd, Controls);

		RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\JSoft\\Media Remote\\", NULL, KEY_QUERY_VALUE, &hKey);

		
		
		RegQueryValueEx(hKey, "InstallPath", NULL, NULL, NULL, &PathSize);
		if(RegQueryValueEx(hKey, "InstallPath", NULL, NULL, (LPBYTE)InstallPath, &PathSize) != ERROR_SUCCESS
			){//|| Authenticate() == false) {
			MessageBox(hSelf, "Sorry, you don't have authorization to use this program.", "Too Bad", MB_OK | MB_ICONINFORMATION);
			exit(9);
		}

		RegCloseKey(hKey);

		sprintf(DefConfigPath, "%s\\Config.cfg", InstallPath);

		if(PathFileExists(DefConfigPath) && (pFile = fopen(DefConfigPath, TEXT("r")))) {
			ParseWinampPlaylist(pFile);
			ParseWMPPlaylist(pFile);
			ParseITPlaylist(pFile);
			ParseFileInfo(pFile, Controls, TOTALMAINBUTTONS, "[Main Program Settings]\n", MainBinds.Enabled, MainBinds.lParams[0], true);
			ParseFileMiscSettings(pFile, 4, "[Misc Program Settings]\n", hwnd);
			ParseFileInfo(pFile, Controls, TOTALWINAMPBUTTONS, "[Winamp Settings]\n", WinampBinds.Play, WinampBinds.lParams[0], false);
			ParseFileInfo(pFile, Controls, WinampBinds.PlaylistCount, "[Winamp Playlist Binds]\n", WinampBinds.Playlists[0], WinampBinds.plParams[0], false);
			ParseFileInfo(pFile, Controls, WMPBinds.PlaylistCount, "[Windows Media Player Playlist Binds]\n", WMPBinds.Playlists[0], WMPBinds.plParams[0], false);
			ParseFileInfo(pFile, Controls, TOTALWMPBUTTONS, "[Windows Media Player Settings]\n", WMPBinds.Play, WMPBinds.lParams[0], false);	
			ParseFileInfo(pFile, Controls, ITBinds.PlaylistCount, "[iTunes Playlist Binds]\n", ITBinds.Playlists[0], ITBinds.plParams[0], false);
			ParseFileInfo(pFile, Controls, TOTALITUNESBUTTONS, "[iTunes Settings]\n", ITBinds.Play, ITBinds.lParams[0], false);
			ParseFileInfo(pFile, Controls, TOTALMEDIABUTTONS, "[Media Key Settings]\n", MediaKeyBinds.Play, MediaKeyBinds.lParams[0], false);
			fclose(pFile);
		}
		else { //initialize all data structures so they are ready for use in the program
			WinampBinds.PlaylistCount = 0;
			WinampBinds.PlaylistPaths = new char[WinampBinds.PlaylistCount+1][1000];
			memset(WinampBinds.PlaylistPaths, 0, 1000*WinampBinds.PlaylistCount+1000);
			WinampBinds.Playlists = new unsigned int[WinampBinds.PlaylistCount+1];
			memset(WinampBinds.Playlists, 0, WinampBinds.PlaylistCount*4);
			WinampBinds.plParams = new unsigned int[WinampBinds.PlaylistCount+1];
			memset(WinampBinds.plParams, 0, WinampBinds.PlaylistCount*4);

			WMPBinds.PlaylistCount = 0;
			WMPBinds.PlaylistPaths = new char[WMPBinds.PlaylistCount+1][1000];
			memset(WMPBinds.PlaylistPaths, 0, 1000*WMPBinds.PlaylistCount+1000);
			WMPBinds.Playlists = new unsigned int[WMPBinds.PlaylistCount+1];
			memset(WMPBinds.Playlists, 0, WMPBinds.PlaylistCount*4);
			WMPBinds.plParams = new unsigned int[WMPBinds.PlaylistCount+1];
			memset(WMPBinds.plParams, 0, WMPBinds.PlaylistCount*4);

			ITBinds.PlaylistCount = 0;
			ITBinds.PlaylistPaths = new char[ITBinds.PlaylistCount+1][1000];
			memset(ITBinds.PlaylistPaths, 0, 1000*ITBinds.PlaylistCount+1000);
			ITBinds.Playlists = new unsigned int[ITBinds.PlaylistCount+1];
			memset(ITBinds.Playlists, 0, ITBinds.PlaylistCount*4);
			ITBinds.plParams = new unsigned int[ITBinds.PlaylistCount+1];
			memset(ITBinds.plParams, 0, ITBinds.PlaylistCount*4);
		}


		CoInitialize(NULL);
		CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iITunes);

		SetTimer(hwnd, 20, 5000, NULL);
		InitSounds((char*)InstallPath, hwnd);

		return 0;


	case WM_KEYDOWN:
		if (wParam == IT_PLAY)
			iITunes->PlayPause();
		else if (wParam == IT_STOP)
			iITunes->Stop();
		else if (wParam == IT_PREV)
			iITunes->PreviousTrack();
		else if (wParam == IT_NEXT)
			iITunes->NextTrack();
		else if (wParam == IT_MUTE) {
			static long ITunesVolumeBeforeMuted = 0;
			iITunes->get_SoundVolume(&ITunesVolume);
			if (ITunesVolume > 0) {
				iITunes->put_SoundVolume(0);
				ITunesVolumeBeforeMuted = ITunesVolume;
			}
			else
				iITunes->put_SoundVolume(ITunesVolumeBeforeMuted);
		}
		//else if (wParam == IT_REPEAT) Not Implimented Yet
		//else if (wParam == IT_SHUFFLE) Not Implimented Yet

		else if(wParam == IT_VOLUMEUP) { //volume up
			iITunes->get_SoundVolume(&ITunesVolume);
			ITunesVolume = ITunesVolume + 2;
			if(ITunesVolume < 100)
				iITunes->put_SoundVolume(ITunesVolume);

		}
		else if(wParam == IT_VOLUMEDOWN) {
			iITunes->get_SoundVolume(&ITunesVolume);
			if(ITunesVolume > 0)
				iITunes->put_SoundVolume(ITunesVolume-2);
		}
		break;

	case WM_TIMER:
		if(!IsWindow(hWinamp))
			hWinamp = FindWindow("Winamp v1.x", NULL);
		if(!IsWindow(hWMP))
			hWMP = FindWindow("WMPlayerApp", NULL);
		if(!IsWindow(hITunes)) {
			hITunes = FindWindow("iTunes", NULL);
			if(hITunes) {
				CoInitialize(NULL);
				CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iITunes);
			}
		}
		return 0;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case TCN_SELCHANGE:
			Tab = TabCtrl_GetCurSel(hwndTab);

			if(Tab == MAINPROGRAMTAB) {
				DestroyWindowControls(Controls);
				SetupWindowMain(hwnd, Controls);
				SetWindowButtons(Controls, TOTALMAINBUTTONS, MainBinds.Enabled, MainBinds.lParams[0]);
			}
			if (Tab == MEDIATAB) {
				DestroyWindowControls(Controls);
				SetupWindowMediaKeys(hwnd, Controls);
				SetWindowButtons(Controls, TOTALMEDIABUTTONS, MediaKeyBinds.Play, MediaKeyBinds.lParams[0]);
			}
			else if(Tab == WINAMPTAB) {
				DestroyWindowControls(Controls);
				SetupWindowWinamp(hwnd, Controls);
				SetWindowButtons(Controls, TOTALWINAMPBUTTONS, WinampBinds.Play, WinampBinds.lParams[0]);
			} 
			else if(Tab == WMPTAB) {
				DestroyWindowControls(Controls);
				SetupWindowWMP(hwnd, Controls);
				SetWindowButtons(Controls, TOTALWMPBUTTONS, WMPBinds.Play, WMPBinds.lParams[0]);
			}
			else if(Tab == ITUNESTAB) {
				DestroyWindowControls(Controls);
				SetupWindowITunes(hwnd, Controls);
				SetWindowButtons(Controls, TOTALITUNESBUTTONS, ITBinds.Play, ITBinds.lParams[0]);
			}
		}
		break;




	case WM_PAINT:
		hdc = BeginPaint (hwnd, &ps);
		GetClientRect(hwnd, &rect);
		SelectObject(hdc, (HBRUSH)GetStockObject(BLACK_BRUSH));
		hdc2 = CreateCompatibleDC(hdc);
		Rectangle(hdc, 0, 0, rect.right, rect.bottom);
		hPic = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_WINAMPPIC));
		SelectObject(hdc2, hPic);
		BitBlt(hdc, 60, 25, 200, 40, hdc2, 0, 0, SRCCOPY);
		
        DeleteObject(hPic); 
		DeleteDC(hdc2);
		EndPaint (hwnd, &ps);
		return 0;

	case WM_CTLCOLOREDIT:
		if(GetWindowLong((HWND)lParam, GWL_ID) == IDC_WAPLCB 
				|| GetWindowLong((HWND)lParam, GWL_ID) == IDC_WMPPLCB
				|| GetWindowLong((HWND)lParam, GWL_ID) == IDC_ITPLCB)
			break;
		HPEN Pen;
		Pen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
		SelectObject((HDC) wParam, Pen);
		GetClientRect((HWND) lParam, &rect);
		SetBkMode((HDC) wParam, TRANSPARENT);
		SelectObject((HDC) wParam, (HBRUSH)GetStockObject(BLACK_BRUSH));
		Rectangle((HDC) wParam, 0, 0, rect.right, rect.bottom);
		SetTextColor((HDC) wParam, RGB(0, 255, 0));
		DeleteObject(Pen);

		return (WPARAM)GetStockObject(BLACK_BRUSH);

	case WM_CTLCOLORSTATIC:
		SetBkMode((HDC) wParam, TRANSPARENT);
		SetTextColor((HDC) wParam, RGB(0, 255, 0));
		return (WPARAM)GetStockObject(BLACK_BRUSH);


	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_WAPLCB:
			if(HIWORD(wParam) == CBN_SELCHANGE) {
				char Label[1000];
				GetKeyNameText((LONG)WinampBinds.plParams[SendMessage(hwndCB, CB_GETCURSEL, NULL, NULL)], Label, 20);
				if(strcmp(Label, "") == 0)
					SetWindowText(hwndST, "Unbound");
				else
					SetWindowText(hwndST, Label);
				
			}
			break;

		case IDM_WINAMPPLAYLIST:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hwnd, DlgWinampPlaylistProc, (LPARAM)&WinampBinds);
			break;

		case IDM_WMPPLAYLIST:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hwnd, DlgWinampPlaylistProc, (LPARAM)&WMPBinds);
			break;

		case IDM_ITPLAYLIST:
			DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_FORMVIEW), hwnd, DlgWinampPlaylistProc, (LPARAM)&ITBinds);
			break;


		case IDM_WINAMPSONGNOTIFY:
			hMenu = GetMenu(hwnd);
			if(EnableWinampSongNotify == true) {
				EnableWinampSongNotify = false;
				CheckMenuItem(hMenu, IDM_WINAMPSONGNOTIFY, MF_UNCHECKED);
			}
			else {
				EnableWinampSongNotify = true;
				CheckMenuItem(hMenu, IDM_WINAMPSONGNOTIFY, MF_CHECKED);
			}
			return 0;


		case IDM_DISABLEWINKEYS:
			hMenu = GetMenu(hwnd);
			if(WindowKeysDisabled == true) {
				WindowKeysDisabled = false;
				CheckMenuItem(hMenu, IDM_DISABLEWINKEYS, MF_UNCHECKED);
			}
			else {
				WindowKeysDisabled = true;
				CheckMenuItem(hMenu, IDM_DISABLEWINKEYS, MF_CHECKED);
			}
			return 0;

		case IDM_DISABLEBOUNDKEYS:
			hMenu = GetMenu(hwnd);
			if(DisableBoundKeys == true) {
				DisableBoundKeys = false;
				CheckMenuItem(hMenu, IDM_DISABLEBOUNDKEYS, MF_UNCHECKED);
			}
			else {
				DisableBoundKeys = true;
				CheckMenuItem(hMenu, IDM_DISABLEBOUNDKEYS, MF_CHECKED);
			}
			return 0;

		case IDM_SAVEDEF:
			sprintf(DefConfigPath, "%s\\Config.cfg", InstallPath);
			pFile = fopen(TEXT(DefConfigPath), TEXT("w"));
			SaveSettings(hwnd, pFile);
			fclose(pFile);
			return 0;

		case IDM_SAVE:
			pFile = fopen(TEXT(ConfigPath), TEXT("w"));
			SaveSettings(hwnd, pFile);
			fclose(pFile);
			return 0;

		case IDM_SAVEAS:
			ConfigFileInitialize(hwnd);
			if(FileSaveDlgConfig(hwnd, ConfigPath, ConfigName) == 0)
				return 1;

			pFile = fopen(TEXT(ConfigPath), TEXT("w"));
			SaveSettings(hwnd, pFile);
			fclose(pFile);
			hMenu = GetMenu(hwnd);
			EnableMenuItem(hMenu, IDM_SAVE, MF_ENABLED);
			return 0;

		case IDM_OPENFILE:
			ConfigFileInitialize(hwnd);
			if(FileOpenDlgConfig(hwnd, ConfigPath, ConfigName) == 0)
				return 1;

			if(PathFileExists(ConfigPath) && (pFile = fopen(ConfigPath, TEXT("r")))) {
				bool Show[TOTALTABS] = {false};
				Show[Tab] = true;
				ParseWinampPlaylist(pFile);
				ParseWMPPlaylist(pFile);
				ParseITPlaylist(pFile);
				ParseFileInfo(pFile, Controls, TOTALMAINBUTTONS, "[Main Program Settings]\n", MainBinds.Enabled, MainBinds.lParams[0], Show[MAINPROGRAMTAB]);
				ParseFileMiscSettings(pFile, 4, "[Misc Program Settings]\n", hwnd);
				ParseFileInfo(pFile, Controls, TOTALWINAMPBUTTONS, "[Winamp Settings]\n", WinampBinds.Play, WinampBinds.lParams[0], Show[WINAMPTAB]);
				ParseFileInfo(pFile, Controls, WinampBinds.PlaylistCount, "[Winamp Playlist Binds]\n", WinampBinds.Playlists[0], WinampBinds.plParams[0], false);
				ParseFileInfo(pFile, Controls, WMPBinds.PlaylistCount, "[Windows Media Player Playlist Binds]\n", WMPBinds.Playlists[0], WMPBinds.plParams[0], false);
				ParseFileInfo(pFile, Controls, TOTALWMPBUTTONS, "[Windows Media Player Settings]\n", WMPBinds.Play, WMPBinds.lParams[0], Show[WMPTAB]);
				ParseFileInfo(pFile, Controls, ITBinds.PlaylistCount, "[iTunes Playlist Binds]\n", ITBinds.Playlists[0], ITBinds.plParams[0], false);
				ParseFileInfo(pFile, Controls, TOTALITUNESBUTTONS, "[iTunes Settings]\n", ITBinds.Play, ITBinds.lParams[0], Show[ITUNESTAB]);
				ParseFileInfo(pFile, Controls, TOTALMEDIABUTTONS, "[Media Key Settings]\n", MediaKeyBinds.Play, MediaKeyBinds.lParams[0], Show[MEDIATAB]);
				fclose(pFile);
			}

			RefreshComboBox();
			return 0;

		case IDM_AWINAMP:
			hWinamp = FindWindow("Winamp v1.x", NULL);
			if(hWinamp) {
				MessageBox(hwnd, "Attached to Winamp succesfully.", "Information", MB_OK | MB_ICONINFORMATION);
			}
			else
				MessageBox(hwnd, "Attachment unsuccessful. \nBe sure that Winamp is running before you attempt to attach.", "Error", MB_OK | MB_ICONERROR);
			return 0;

		case IDM_AWMP:
			hWMP = FindWindow("WMPlayerApp", NULL);
			if(hWMP) {
				MessageBox(hwnd, "Attached to Windows Media Player succesfully.", "Information", MB_OK | MB_ICONINFORMATION);
			}
			else
				MessageBox(hwnd, "Attachment unsuccessful. \nBe sure that Windows Media Player is running before you attempt to attach.", "Error", MB_OK | MB_ICONERROR);
			return 0;

		case IDM_AITUNES:
			hITunes = FindWindow("iTunes", NULL);
			if(hITunes) {
				CoInitialize(NULL);
				CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID *)&iITunes);
				MessageBox(hwnd, "Attached to iTunes succesfully.", "Information", MB_OK | MB_ICONINFORMATION);
			}
			else
				MessageBox(hwnd, "Attachment unsuccessful. \nBe sure that iTunes is running before you attempt to attach.", "Error", MB_OK | MB_ICONERROR);
			return 0;


		case IDM_ENABLED:
			hMenu = GetMenu(hwnd);
			if(ProgramEnabled == true) {
				sprintf(ConfigPath, "%s\\Sounds\\Disabled.wav", InstallPath);
				PlaySoundDll(MR_Disabled, hwnd, ConfigPath);
				ProgramEnabled = false;
				CheckMenuItem(hMenu, IDM_ENABLED, MF_UNCHECKED);
			}
			else {
				sprintf(ConfigPath, "%s\\Sounds\\Enabled.wav", InstallPath);
				PlaySoundDll(MR_Enabled, hwnd, ConfigPath);
				ProgramEnabled = true;
				CheckMenuItem(hMenu, IDM_ENABLED, MF_CHECKED);
			}
			return 0;

		case IDM_EWINAMP:
			hMenu = GetMenu(hwnd);
			if(WinampControlsEnabled == true) {
				WinampControlsEnabled = false;
				CheckMenuItem(hMenu, IDM_EWINAMP, MF_UNCHECKED);
			}
			else {
				WinampControlsEnabled = true;
				CheckMenuItem(hMenu, IDM_EWINAMP, MF_CHECKED);
			}
			return 0;

		case IDM_EWMP:
			hMenu = GetMenu(hwnd);
			if(WMPControlsEnabled == true) {
				WMPControlsEnabled = false;
				CheckMenuItem(hMenu, IDM_EWMP, MF_UNCHECKED);
			}
			else {
				WMPControlsEnabled = true;
				CheckMenuItem(hMenu, IDM_EWMP, MF_CHECKED);
			}
			return 0;

		case IDM_EIT:
			hMenu = GetMenu(hwnd);
			if(ITControlsEnabled == true) {
				ITControlsEnabled = false;
				CheckMenuItem(hMenu, IDM_EIT, MF_UNCHECKED);
			}
			else {
				ITControlsEnabled = true;
				CheckMenuItem(hMenu, IDM_EIT, MF_CHECKED);
			}
			return 0;

		case IDM_POLLALL:
			hMenu = GetMenu(hwnd);
			if(PollPrograms == true) {
				PollPrograms = false;
				CheckMenuItem(hMenu, IDM_POLLALL, MF_UNCHECKED);
				KillTimer(hwnd, 20);
			}
			else {
				PollPrograms = true;
				CheckMenuItem(hMenu, IDM_POLLALL, MF_CHECKED);
				SetTimer(hwnd, 20, 5000, NULL);
			}
			return 0;

		case IDM_HELP:
			MessageBox(hwnd, "Good help is yet to be implemented but here is a summary:\n\n1. You attach this program to Winamp while Winamp is running.\n2. You set your buttons up how you want them then save them. You do this by clicking in the textboxes and hitting the key that you want binded to that function. You can also unbind the function by double clicking in the edit box.\n3. Close the main window and the program will run in the background in your system tray.\n4. To exit the program right click on it in the system tray and click close.", "Help", MB_OK | MB_ICONINFORMATION);
			return 0;

		case IDM_ABOUT:
			MessageBox(hwnd, "I, Joel, wrote this program so I could more easily access Winamp without actually alt-tabbing out of everything just to get to it all the time.  Over time I expanded it to work with Windows Media Player, and ITunes.  I also added functionality to mimic keyboard media keys so you can bind them to whatever keys you want.", "About", MB_OK | MB_ICONINFORMATION);
			return 0;

		case IDM_EXIT:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			return 0;

		case IDM_OPEN:
			Shell_NotifyIcon(NIM_DELETE, &TrayData);
			ShowWindow(hwnd, SW_SHOW);
			return 0;

		case IDM_CLOSE:
			TrueQuit = true;
			Shell_NotifyIcon(NIM_DELETE, &TrayData);
			DestroyWindow(hwnd);
			SendMessage(hwnd, WM_DESTROY, NULL, NULL);
			return 0;
		}

		break;


	case WM_DESTROY:
		FreeLibrary(hWinampDll);
		delete [] WinampBinds.Playlists;
		delete [] WinampBinds.plParams;
		delete [] WinampBinds.PlaylistPaths;
		delete [] WMPBinds.Playlists;
		delete [] WMPBinds.plParams;
		delete [] WMPBinds.PlaylistPaths;
		delete [] ITBinds.Playlists;
		delete [] ITBinds.plParams;
		delete [] ITBinds.PlaylistPaths;
		PostQuitMessage (0);
		break;

          
	case WM_CLOSE:
		if(TrueQuit == true) {
			break;
		}

		ShowWindow(hwnd, SW_HIDE);
		TrayData.cbSize = sizeof(NOTIFYICONDATA);
		TrayData.hWnd = hwnd; 
		TrayData.uID = TRAYID; 
		TrayData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		TrayData.uCallbackMessage = TRAYID;
		TrayData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
		sprintf(TrayData.szTip, "Media Remote");

		
		Shell_NotifyIcon(NIM_ADD, &TrayData);
		return 0;

	case TRAYID:
		if (lParam == WM_RBUTTONDOWN)
		{ 
			POINT lpClickPoint;

			GetCursorPos(&lpClickPoint);

			hPopMenu = CreatePopupMenu();
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_OPEN,"&Open");
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_ENABLED,"E&nabled");
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_SEPARATOR,NULL,NULL);
			InsertMenu(hPopMenu,0xFFFFFFFF,MF_BYPOSITION|MF_STRING,IDM_CLOSE,"&Close");

			if(ProgramEnabled == true)
				CheckMenuItem(hPopMenu, IDM_ENABLED, MF_CHECKED);
			else
				CheckMenuItem(hPopMenu, IDM_ENABLED, MF_UNCHECKED);

			SetForegroundWindow(hwnd);
			TrackPopupMenu(hPopMenu,TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_BOTTOMALIGN,lpClickPoint.x, lpClickPoint.y,0,hwnd,NULL);
			SendMessage(hwnd,WM_NULL,0,0);
					
			return TRUE; 
		} 
		else if(lParam == WM_LBUTTONDBLCLK)
			SendMessage(hwnd, WM_COMMAND, IDM_OPEN, NULL);

		break;
	}
	return DefWindowProc (hwnd, message, wParam, lParam);
}
