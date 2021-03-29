#define _WIN32_WINNT 0x500

#include <windows.h>
#include <math.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <iostream>
#include <stdio.h>
#include <uxtheme.h>
#include "WinampPluginMain.h"
#include "../Media Remote/WinMain.h"
#include "../Media Remote/Timer.h"


LRESULT CALLBACK WinampSongToolTipProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);


HWND hSelf;
HINSTANCE hInst;
HWND hWinampSongToolTip, hWinamp;
WNDPROC WndprocWinampHUDTooltip;
char szWinampPath[MAX_PATH];
LONG_PTR nWinampVolumeMemoryOffset;


int WINAPI DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls((HINSTANCE)hModule);
			hSelf = FindWindow("Media RM", "Media Remote");
			hInst = (HINSTANCE)GetWindowLongPtr(hSelf, GWLP_HINSTANCE);

			hWinampSongToolTip = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
						TOOLTIPS_CLASS, "Hello", WS_POPUP| TTS_ALWAYSTIP,		
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 200,
						hSelf, NULL, hInst, NULL);

			WndprocWinampHUDTooltip = (WNDPROC) SetWindowLongPtr(hWinampSongToolTip, GWLP_WNDPROC, (LONG_PTR)WinampSongToolTipProc);
			SendMessage(hWinampSongToolTip, 1, 0, 0);
			break;
	}

	return TRUE;
}







BOOL ListProcessModules(DWORD dwPID)
{
	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;
	std::string Path;


	hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPID);

	if(hModuleSnap == INVALID_HANDLE_VALUE)
		return FALSE;


	me32.dwSize = sizeof(MODULEENTRY32);

	// Retrieve information about the first module,
	// and exit if unsuccessful
	if(!Module32First(hModuleSnap, &me32))
	{
		CloseHandle(hModuleSnap);
		return FALSE;
	}

	// Now walk the module list of the process,
	// and display information about each module
	do
	{
		Path = me32.szExePath;
		Path = Path.substr(Path.rfind('\\', Path.length()-1)+1, 50);
		if(_stricmp(Path.c_str(), "winamp.exe") == 0) {
			strcpy(szWinampPath, me32.szExePath);
			break;
		}

	} while(Module32Next(hModuleSnap, &me32));

	CloseHandle(hModuleSnap);
	return TRUE;
}






BOOL GetProcessInfo()
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return FALSE;


	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if(!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return FALSE;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if(_stricmp(pe32.szExeFile, "winamp.exe") == 0) {
			ListProcessModules(pe32.th32ProcessID);
			break;
		}
	} while(Process32Next(hProcessSnap, &pe32));


	CloseHandle(hProcessSnap);
	return TRUE;
}




DWORD GetWinampProcessId()
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if(hProcessSnap == INVALID_HANDLE_VALUE)
		return FALSE;


	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if(!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);
		return FALSE;
	}

	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		if(_stricmp(pe32.szExeFile, "winamp.exe") == 0) {
			CloseHandle(hProcessSnap);
			return pe32.th32ProcessID;
		}

	} while(Process32Next(hProcessSnap, &pe32));


	return FALSE;
}






void WinampKeyboard(WAControls WinampBinds, KBDLLHOOKSTRUCT *KeyData, BYTE *KeyStates, HWND hWinamp, bool &KeyProcessed,
				   bool EnableWinampSongNotify)
{
	if(::hWinamp != hWinamp) { //if winamp has been closed and reopened again
		nWinampVolumeMemoryOffset = 0; //reset volume offset incase so it can be reestablish
		::hWinamp = hWinamp;	//reassign handle to winamp window
	}

	//::WinampBinds = &WinampBinds;


	for(int x=0; x<(int)WinampBinds.PlaylistCount; x++)
		if(KeyData->vkCode == WinampBinds.Playlists[x]) {
			if(KeyStates[WinampBinds.Playlists[x]] == 1) //if key was pressed previously and never released return
				return;

			KeyStates[WinampBinds.Playlists[x]] = 1; //set key state to pressed

			PROCESS_INFORMATION pi;
			STARTUPINFO si;
			char ProgramString[2000];

			GetProcessInfo();
			sprintf(ProgramString, "%s \"%s\"", szWinampPath, WinampBinds.PlaylistPaths[x]);

			memset(&si, 0, sizeof(STARTUPINFO));
			memset(&pi, 0, sizeof(PROCESS_INFORMATION));
			
			CreateProcess(NULL, ProgramString, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &si, &pi);

			CloseHandle(pi.hThread);
			CloseHandle(pi.hProcess);
			KeyProcessed = true;
		}
	if(KeyData->vkCode == WinampBinds.Play) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_PLAY, 0);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, 7099, 6000);
	}
	else if(KeyData->vkCode == WinampBinds.Stop) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_STOP, 0);
		KeyProcessed = true;
	}
	else if(KeyData->vkCode == WinampBinds.Pause) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_PAUSE, 0);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, 7099, 4000);
	}
	else if(KeyData->vkCode == WinampBinds.Previous) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_PREV, 0);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, 7099, 6000);
	}
	else if(KeyData->vkCode == WinampBinds.Next) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_NEXT, 0);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, 7099, 6000);
	}
	else if(KeyData->vkCode == WinampBinds.Shuffle) {
		SendMessage(hWinamp, WM_USER, !SendMessage(hWinamp, WM_USER, 0, IPC_GET_SHUFFLE), IPC_SET_SHUFFLE);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, MAKEWPARAM(7099, 2), 4000);
	}
	else if(KeyData->vkCode == WinampBinds.Repeat) {
		SendMessage(hWinamp, WM_USER, !SendMessage(hWinamp, WM_USER, 0, IPC_GET_REPEAT), IPC_SET_REPEAT);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, MAKEWPARAM(7099, 3), 4000);
	}
	else if(KeyData->vkCode == WinampBinds.VolumeUp) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_VOLUMEUP, 0);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, MAKEWPARAM(7099, 1), 4000);
	}
	else if(KeyData->vkCode == WinampBinds.VolumeDown) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_VOLUMEDOWN, 0);
		KeyProcessed = true;
		if(EnableWinampSongNotify)
			SendMessage(hWinampSongToolTip, WM_COMMAND, MAKEWPARAM(7099, 1), 4000);
	}
	else if(KeyData->vkCode == WinampBinds.Rewind) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_REW5S, 0);
		KeyProcessed = true;
	}
	else if(KeyData->vkCode == WinampBinds.Forward) {
		SendMessage(hWinamp, WM_COMMAND, WINAMP_FFWD5S, 0);
		KeyProcessed = true;
	}
}



int GetTextWidth(const HWND &hwnd, char *Text, HFONT hf)
{
	HDC hdc;
	SIZE TextSize;

	hdc = GetDC(hwnd);
	if(hf == NULL)
		SelectObject(hdc, GetStockObject(ANSI_VAR_FONT));
	else
		SelectObject(hdc, hf);
	GetTextExtentPoint32(hdc, Text, (int)strlen(Text), &TextSize);
	DeleteDC(hdc);

	return TextSize.cx;
}




char *GetVolumeLevel(DWORD Pid)
{
	static char szVolume[50];
	HANDLE process;
	BYTE *MemoryBuffer, *pMemorySpace;
	double dVolume;

	if(nWinampVolumeMemoryOffset == 0) {
		nWinampVolumeMemoryOffset = 5094303; //Set offset to maximum address of 0x4DBB9F
		MemoryBuffer = new BYTE[900000];
		pMemorySpace = &MemoryBuffer[899999]; //assign memory space pointer to point to last array element of MemoryBuffer
		process = OpenProcess(PROCESS_VM_READ, FALSE, Pid);
		ReadProcessMemory(process, (LPCVOID )0x400000, MemoryBuffer, 900000, NULL); //Store process memory in buffer
		CloseHandle(process);

		while(true) {
			if(memcmp((LPCSTR)pMemorySpace, "out_ds.dll", 10) == 0
				|| memcmp((LPCSTR)pMemorySpace, "out_wave.dll", 12) == 0) { //when these values are found it means that volume level is just 152 bytes ahead
				pMemorySpace += 152;
				nWinampVolumeMemoryOffset += 152;
				break;
			}

			pMemorySpace--; //subtract from memory offset for future use and subtract from memory pointer to go to the previous byte
			nWinampVolumeMemoryOffset--;
		}

		dVolume = floor(((*pMemorySpace/255.0)*100.0) + 0.5); //set volume level double variable

		delete [] MemoryBuffer;
	}
	else {
		pMemorySpace = new BYTE; //1 byte is all thats needed to store volume

		process = OpenProcess(PROCESS_VM_READ, FALSE, Pid);
		ReadProcessMemory(process, (LPCVOID )nWinampVolumeMemoryOffset, pMemorySpace, 1, NULL); //get volume byte by offset variable
		CloseHandle(process);

		dVolume = floor(((*pMemorySpace/255.0)*100.0) + 0.5); //set volume level double variable

		delete pMemorySpace;
	}


	sprintf(szVolume, "Winamp Volume: %.0f%%", dVolume); //Set volume string and return it

	return szVolume;
}


char sToolTipText[5000];
void ShowToolTip(char *sText)
{
	strcpy(sToolTipText, sText);
	SendMessage(hWinampSongToolTip, WM_COMMAND, MAKEWPARAM(7099, 4), 4000);
}




LRESULT CALLBACK WinampSongToolTipProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	static TOOLINFO ti;
	static char strTT[5000];
	POINT TTPoint;
	static LOGFONT lf;
	HFONT hf;



	switch(message)
	{
	case WM_CREATE:
		lf.lfHeight         = 7;
		lf.lfWidth          = 7;
		lf.lfEscapement     = 0;
		lf.lfOrientation    = 0;
		lf.lfWeight         = 0;
		lf.lfItalic         = 0;
		lf.lfUnderline      = 0;
		lf.lfStrikeOut      = 0;
		lf.lfCharSet        = DEFAULT_CHARSET;
		lf.lfOutPrecision   = 0;
		lf.lfClipPrecision  = 0;
		lf.lfQuality        = 0;
		lf.lfPitchAndFamily = 0;

		SetWindowTheme(hwnd, L"", L"");
		ti.cbSize = sizeof(TOOLINFO);
		ti.uFlags = TTF_TRACK | TTF_TRANSPARENT | TTF_ABSOLUTE;
		ti.hwnd = hSelf;
		ti.hinst = hInst;
		ti.uId = 10001;
		ti.lpszText = strTT;

		SendMessage(hwnd, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);

		return 0;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case 7099:
			hf = CreateFontIndirect(&lf);

			if(HIWORD(wParam) == 1)
				strcpy(strTT, GetVolumeLevel(GetWinampProcessId()));
			else if(HIWORD(wParam) == 2)
				sprintf(strTT, "Winamp Shuffle %s", (SendMessage(hWinamp, WM_USER, 0, IPC_GET_SHUFFLE)) ? "Enabled" : "Disabled");
			else if(HIWORD(wParam) == 3)
				sprintf(strTT, "Winamp Repeat %s", (SendMessage(hWinamp, WM_USER, 0, IPC_GET_REPEAT)) ? "Enabled" : "Disabled");
			else if(HIWORD(wParam) == 4)
				strcpy(strTT, sToolTipText);
			else
				GetWindowText(hWinamp, strTT, 5000);

			GetSystemMetrics(SM_CXSCREEN);
			TTPoint.x = GetSystemMetrics(SM_CXSCREEN)/2-GetTextWidth(hwnd, strTT, hf)/2;
			TTPoint.y = 0;

			SendMessage(hwnd, TTM_UPDATETIPTEXT, 0, (LPARAM) &ti);
			SendMessage(hwnd, TTM_SETTIPBKCOLOR, RGB(0, 0, 0), 0);
			SendMessage(hwnd, TTM_SETTIPTEXTCOLOR, RGB(0, 255, 0), 0);
			SendMessage(hwnd, WM_SETFONT, (WPARAM)&hf, 0);


			SendMessage(hwnd, TTM_TRACKPOSITION, 0, MAKELONG(TTPoint.x, TTPoint.y));
			SendMessage (hwnd, TTM_TRACKACTIVATE, (WPARAM)true, (LPARAM) &ti);
			DeleteObject(hf);

			SetTimer(hwnd, 3, (UINT)lParam, NULL);
			break;
		}
		break;

	case WM_SETFOCUS:
		SetFocus((HWND)wParam);
		SendMessage (hwnd, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
		break;

	case WM_TIMER:
		switch(wParam)
		{
		case 3:
			SendMessage (hwnd, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
			KillTimer(hwnd, 3);
			break;
	
		}
		break;
	}

	return CallWindowProc(WndprocWinampHUDTooltip, hwnd, message, wParam, lParam);
}