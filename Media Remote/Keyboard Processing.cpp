#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <WinUser.h>
#include <stdio.h>
#include <math.h>
#include <process.h>
#include <shlwapi.h>
#include "Resource.h"
#include "WinMain.h"
#include "Timer.h"
#include "../WinampPlugin/WinampPluginMain.h"
#include "iTunesCOMInterface.h"




extern MainControls MainBinds;
extern WAControls WinampBinds;
extern WMPControls WMPBinds;
extern ITControls ITBinds;
extern MKControls MediaKeyBinds;
extern bool ProgramEnabled, WinampControlsEnabled, WMPControlsEnabled, ITControlsEnabled;
extern HWND hWinamp, hWMP, hSelf, hITunes;
extern HHOOK hHook;
extern unsigned int Tab;
extern WNDPROC WndprocHotKey;
extern HWND hwndCB;
extern bool WindowKeysDisabled, DisableBoundKeys, EnableWinampSongNotify;
extern IiTunes* iITunes;


BOOL CALLBACK CheckButtonExistsProc(HWND hwnd, LPARAM lParam)
{
	if (GetWindowLong(hwnd, GWL_ID) == 1) {
		return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK EnumChildProc(HWND hwnd, LPARAM lParam)
{
	unsigned int nPath;

	if(GetWindowLong(hwnd, GWL_ID) == 1001) {
		nPath = (unsigned int)lParam;
		SendMessage(hwnd, EM_SETSEL, 0, -1);
		SendMessage(hwnd, EM_REPLACESEL, TRUE, (LPARAM)WMPBinds.PlaylistPaths[nPath]);
	}
	else if(GetWindowLong(hwnd, GWL_ID) == 1) {
		SendMessage(hwnd, BM_CLICK, NULL, NULL);
	}

	return TRUE;
}



DWORD WINAPI WMPPlaylistThread(LPVOID lpParameter)
{
	HWND MainWindow = NULL;
	CTimer cTimeoutTimer(1);

	cTimeoutTimer.SetStartTime(); //Start Timer
	while(true) {
		if(MainWindow = FindWindow(NULL, "Open location"))
			break;
		if(MainWindow = FindWindow(NULL, "Open URL"))
			break;

		cTimeoutTimer.SetEndTime(); //Set current stop time
		//check to see if current end time is greater than start time + seconds added
		if(cTimeoutTimer.GetDifference() < cTimeoutTimer.CalculateNumberSeconds())
			//if not rinse and repeat...
			cTimeoutTimer.SetEndTime();
		else {
			MessageBox(hSelf, "Attempt to load playlist failed.\nMake sure that Windows Media Player is able to receive input.", "Playlist load failed", MB_OK | MB_ICONWARNING);
			//if it is return error code.
			return 0;
		}
	}

	Sleep(50); //Give Dialog some time to load

	bool bBtnOkDoesntExists = TRUE;

	cTimeoutTimer.SetStartTime();
	while(bBtnOkDoesntExists == TRUE) { //Wait for the OK button to load before continuing
		bBtnOkDoesntExists = EnumChildWindows(MainWindow, CheckButtonExistsProc, NULL);
		Sleep(1);
		cTimeoutTimer.SetEndTime(); //Set current stop time
		//check to see if current end time is greater than start time + seconds added
		if(cTimeoutTimer.GetDifference() < cTimeoutTimer.CalculateNumberSeconds())
			//if not rinse and repeat...
			cTimeoutTimer.SetEndTime();
		else {
			MessageBox(hSelf, "Attempt to load playlist failed.\nMake sure that Windows Media Player is able to receive input.", "Playlist load failed", MB_OK | MB_ICONWARNING);
			//if it is return error code.
			return 0;
		}
	}


	EnumChildWindows(MainWindow, EnumChildProc, (LPARAM)lpParameter);

	//return success code
	return 1;

}



LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam)
{
	static BYTE KeyStates[256];
	bool KeyProcessed = false;


	if(code < 0)
		return CallNextHookEx(hHook, code, wParam, lParam);


	if(wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
		KeyStates[((KBDLLHOOKSTRUCT*)lParam)->vkCode] = 0; //set keystate to released state
		return CallNextHookEx(hHook, code, wParam, lParam);
	}

	if(hSelf == GetForegroundWindow())
		if(GetWindowLong(GetFocus(), GWL_ID) >= 5001 && GetWindowLong(GetFocus(), GWL_ID) <= 6001 
				&& GetWindowLong(GetFocus(), GWL_ID) != IDC_WAPLCB)
			return CallNextHookEx(hHook, code, wParam, lParam);

	if((((KBDLLHOOKSTRUCT*)lParam)->vkCode == MainBinds.Enabled)) {
		HWND hSelf;
		hSelf = FindWindow("Media RM", "Media Remote");
		SendMessage(hSelf, WM_COMMAND, IDM_ENABLED, NULL);
		if(DisableBoundKeys)
			return 1;
		else
			return CallNextHookEx(hHook, code, wParam, lParam);
	}


	if(ProgramEnabled == false)
		return CallNextHookEx(hHook, code, wParam, lParam);



	if(IsWindow(hITunes) && ITControlsEnabled == true)
	{
		for(int x=0; x<(int)ITBinds.PlaylistCount; x++)
			if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Playlists[x]) {
				if(KeyStates[ITBinds.Playlists[x]] == 1) //if key was pressed previously and never released return
					return 1;

				KeyStates[ITBinds.Playlists[x]] = 1; //set key state to pressed

				//Not Implimented Yet
				
				KeyProcessed = true;
			}

		if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Play) { //These messages need to be sent back to the main handler because of COM deadlock issues
			PostMessage(hSelf, WM_KEYDOWN, IT_PLAY, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Stop) {
			PostMessage(hSelf, WM_KEYDOWN, IT_STOP, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Previous) {
			PostMessage(hSelf, WM_KEYDOWN, IT_PREV, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Next) {
			PostMessage(hSelf, WM_KEYDOWN, IT_NEXT, NULL);
			ShowToolTip("TESTING!!");
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.VolumeUp) {
			PostMessage(hSelf, WM_KEYDOWN, IT_VOLUMEUP, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.VolumeDown) {
			PostMessage(hSelf, WM_KEYDOWN, IT_VOLUMEDOWN, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Mute) {
			PostMessage(hSelf, WM_KEYDOWN, IT_MUTE, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Repeat) {
			PostMessage(hITunes, IT_REPEAT, NULL, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == ITBinds.Shuffle) {
			PostMessage(hITunes, 0X011F, 0XFFFF0000, NULL);
			KeyProcessed = true;
		}
	}

	if(IsWindow(hWMP) && WMPControlsEnabled == true)
	{
		for(int x=0; x<(int)WMPBinds.PlaylistCount; x++)
			if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Playlists[x]) {
				if(KeyStates[WMPBinds.Playlists[x]] == 1) //if key was pressed previously and never released return
					return 1;

				KeyStates[WMPBinds.Playlists[x]] = 1; //set key state to pressed

				int nPath = x;
				HANDLE hThread;
				DWORD ThreadID;

				PostMessage(hWMP, WM_COMMAND, WMP_OPENURL, 0);
				hThread = CreateThread(NULL, NULL, WMPPlaylistThread, (LPVOID)nPath, NULL, &ThreadID);

				CloseHandle(hThread);
				KeyProcessed = true;
			}

		if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Play) {
			PostMessage(hWMP, WM_COMMAND, WMP_PLAY, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Stop) {
			PostMessage(hWMP, WM_COMMAND, WMP_STOP, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Previous) {
			PostMessage(hWMP, WM_COMMAND, WMP_PREV, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Next) {
			PostMessage(hWMP, WM_COMMAND, WMP_NEXT, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.VolumeUp) {
			PostMessage(hWMP, WM_COMMAND, WMP_VOLUMEUP, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.VolumeDown) {
			PostMessage(hWMP, WM_COMMAND, WMP_VOLUMEDOWN, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Mute) {
			PostMessage(hWMP, WM_COMMAND, WMP_MUTE, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Repeat) {
			PostMessage(hWMP, WM_COMMAND, WMP_REPEAT, NULL);
			KeyProcessed = true;
		}
		else if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == WMPBinds.Shuffle) {
			PostMessage(hWMP, WM_COMMAND, WMP_SHUFFLE, NULL);
			KeyProcessed = true;
		}
	}

	
	if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == MediaKeyBinds.Play) {
		SendMessage(hSelf, WM_APPCOMMAND, (WPARAM)hSelf, MAKELONG(0, FAPPCOMMAND_KEY | APPCOMMAND_MEDIA_PLAY_PAUSE));
		KeyProcessed = true;
	}
	else if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == MediaKeyBinds.Previous) {
		SendMessage(hSelf, WM_APPCOMMAND, (WPARAM)hSelf, MAKELONG(0, FAPPCOMMAND_KEY | APPCOMMAND_MEDIA_PREVIOUSTRACK));
		KeyProcessed = true;
	}
	else if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == MediaKeyBinds.Next) {
		SendMessage(hSelf, WM_APPCOMMAND, (WPARAM)hSelf, MAKELONG(0, FAPPCOMMAND_KEY | APPCOMMAND_MEDIA_NEXTTRACK));
		KeyProcessed = true;
	}
	else if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == MediaKeyBinds.VolumeUp) {
		SendMessage(hSelf, WM_APPCOMMAND, (WPARAM)hSelf, MAKELONG(0, FAPPCOMMAND_KEY | APPCOMMAND_VOLUME_UP));
		KeyProcessed = true;
	}
	else if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == MediaKeyBinds.VolumeDown) {
		SendMessage(hSelf, WM_APPCOMMAND, (WPARAM)hSelf, MAKELONG(0, FAPPCOMMAND_KEY | APPCOMMAND_VOLUME_DOWN));
		KeyProcessed = true;
	}
	else if (((KBDLLHOOKSTRUCT*)lParam)->vkCode == MediaKeyBinds.Mute) {
		SendMessage(hSelf, WM_APPCOMMAND, (WPARAM)hSelf, MAKELONG(0, FAPPCOMMAND_KEY | APPCOMMAND_VOLUME_MUTE));
		KeyProcessed = true;
	}
	

	

	if(IsWindow(hWinamp) && WinampControlsEnabled == true) { //if winamp is open and controls are enabled
		WinampKeyboard(WinampBinds, (KBDLLHOOKSTRUCT*)lParam, KeyStates, hWinamp, 
							KeyProcessed, EnableWinampSongNotify);
	}

	if(DisableBoundKeys == true && KeyProcessed == true) //if disable keys option is on don't let windows process keystroke
		return 1;

	if(WindowKeysDisabled) //if user wants windows keys disabled don't let windows process them...
		if(((KBDLLHOOKSTRUCT*)lParam)->vkCode == 91 || ((KBDLLHOOKSTRUCT*)lParam)->vkCode == 92)
			return 1;		



	return CallNextHookEx(hHook, code, wParam, lParam);
}





void ConfirmNoDupInWinamp(unsigned int &Key)
{
	unsigned int *pData = NULL, *plParam;

	pData = &WinampBinds.Play;
	plParam = &WinampBinds.lParams[0];

	for(int p=0; p<TOTALWINAMPBUTTONS; p++) {
		if(&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if(Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}

	pData = &WinampBinds.Playlists[0];
	plParam = &WinampBinds.plParams[0];


	for(int p=0; p<(int)WinampBinds.PlaylistCount; p++) {
		if(&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if(Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}


}


void ConfirmNoDupInWMP(unsigned int &Key)
{
	unsigned int *pData = NULL, *plParam;

	pData = &WMPBinds.Play;
	plParam = &WMPBinds.lParams[0];

	for(int p=0; p<TOTALWMPBUTTONS; p++) {
		if(&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if(Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}


	pData = &WMPBinds.Playlists[0];
	plParam = &WMPBinds.plParams[0];


	for(int p=0; p<(int)WMPBinds.PlaylistCount; p++) {
		if(&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if(Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}
}


void ConfirmNoDupInITunes(unsigned int &Key)
{
	unsigned int *pData = NULL, *plParam;

	pData = &ITBinds.Play;
	plParam = &ITBinds.lParams[0];

	for(int p=0; p<TOTALITUNESBUTTONS; p++) {
		if(&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if(Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}


	pData = &ITBinds.Playlists[0];
	plParam = &ITBinds.plParams[0];


	for(int p=0; p<(int)ITBinds.PlaylistCount; p++) {
		if(&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if(Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}
}


void ConfirmNoDupInMediaKeys(unsigned int& Key)
{
	unsigned int* pData = NULL, * plParam;

	pData = &MediaKeyBinds.Play;
	plParam = &MediaKeyBinds.lParams[0];

	for (int p = 0; p < TOTALWMPBUTTONS; p++) {
		if (&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if (Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}


}

void ConfirmNoDupInMain(unsigned int &Key)
{
	unsigned int *pData = NULL, *plParam;

	pData = &MainBinds.Enabled;
	plParam = &MainBinds.lParams[0];

	for(int p=0; p<TOTALMAINBUTTONS; p++) {
		if(&Key == pData) {
			pData++;
			plParam++;
			continue;
		}
		else if(Key == *pData) {
			(*pData) = 0;
			(*plParam) = 0;
		}
		pData++;
		plParam++;
	}
}



void SetBindSettings(unsigned int BindData, unsigned int lParamData, unsigned int MemoryOffset, const HWND &hwnd)
{
	unsigned int *DataPtr = NULL, *lParamPtr = NULL;
	HWND hwndTemp;


	if(Tab == MAINPROGRAMTAB) {
		DataPtr = &MainBinds.Enabled;
		lParamPtr = &MainBinds.lParams[0];
		
		DataPtr += MemoryOffset;
		lParamPtr += MemoryOffset;

		*DataPtr = BindData;
		*lParamPtr = lParamData;
	
		
		ConfirmNoDupInWinamp(*DataPtr);
		ConfirmNoDupInWMP(*DataPtr);
		ConfirmNoDupInITunes(*DataPtr);
		

		

		hwndTemp = hwnd;
		for(int x=0; x<(GetWindowLong(hwndTemp, GWL_ID)- IDC_MAINENABLE); )
			hwndTemp = GetWindow(hwndTemp, GW_HWNDPREV);


		lParamPtr = &MainBinds.lParams[0];

		for(unsigned int x=0; x<TOTALMAINBUTTONS; x++) {
			char Label[20];
			GetKeyNameText((LONG)*lParamPtr, Label, 20);


			if(strcmp(Label, "Delete") == 0)
				sprintf(Label, "Delete"); //For some reason it doesn't show with 1 D

			SetWindowText(hwndTemp, Label);


			if(*lParamPtr == 0)
				SetWindowText(hwndTemp, "Unbound");

			lParamPtr++;
			hwndTemp = GetWindow(hwndTemp, GW_HWNDNEXT);
		}
		
	}
	if (Tab == MEDIATAB) {
		DataPtr = &MediaKeyBinds.Play;
		lParamPtr = &MediaKeyBinds.lParams[0];

		DataPtr += MemoryOffset;
		lParamPtr += MemoryOffset;

		*DataPtr = BindData;
		*lParamPtr = lParamData;


		ConfirmNoDupInMain(*DataPtr);
		ConfirmNoDupInMediaKeys(*DataPtr);




		hwndTemp = hwnd;
		for (int x = 0; x < (GetWindowLong(hwndTemp, GWL_ID) - IDC_MEDPLAY); )
			hwndTemp = GetWindow(hwndTemp, GW_HWNDPREV);


		lParamPtr = &MediaKeyBinds.lParams[0];

		for (unsigned int x = 0; x < TOTALMEDIABUTTONS; x++) {
			char Label[20];
			GetKeyNameText((LONG)*lParamPtr, Label, 20);


			if (strcmp(Label, "Delete") == 0)
				sprintf(Label, "Delete"); //For some reason it doesn't show with 1 D

			SetWindowText(hwndTemp, Label);


			if (*lParamPtr == 0)
				SetWindowText(hwndTemp, "Unbound");

			lParamPtr++;
			hwndTemp = GetWindow(hwndTemp, GW_HWNDNEXT);
		}
	}
	if(Tab == WINAMPTAB) {
		if(MemoryOffset < TOTALWINAMPBUTTONS) {
			DataPtr = &WinampBinds.Play;
			lParamPtr = &WinampBinds.lParams[0];
		}
		else {
			DataPtr = &WinampBinds.Playlists[0];
			lParamPtr = &WinampBinds.plParams[0];
			MemoryOffset -= TOTALWINAMPBUTTONS;
		}
		DataPtr += MemoryOffset;
		lParamPtr += MemoryOffset;

		*DataPtr = BindData;
		*lParamPtr = lParamData;

		ConfirmNoDupInMain(*DataPtr);
		ConfirmNoDupInWinamp(*DataPtr);

	
		hwndTemp = hwnd;
		for(int x=0; x<(GetWindowLong(hwndTemp, GWL_ID)- IDC_WAPLAY); )
			hwndTemp = GetWindow(hwndTemp, GW_HWNDPREV);


		lParamPtr = &WinampBinds.lParams[0];

		for(unsigned int x=0; x<=TOTALWINAMPBUTTONS; x++) {
			char Label[20];

			if(x != TOTALWINAMPBUTTONS)
				GetKeyNameText((LONG)*lParamPtr, Label, 20);
			else
				GetKeyNameText((LONG)WinampBinds.plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)], Label, 20);

			if(strcmp(Label, "Delete") == 0)
				sprintf(Label, "Delete"); //For some reason it doesn't show with 1 D

			SetWindowText(hwndTemp, Label);


			if((x != TOTALWINAMPBUTTONS && *lParamPtr == 0) || (x == TOTALWINAMPBUTTONS && WinampBinds.plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)] == 0))
				SetWindowText(hwndTemp, "Unbound");

			if(x != TOTALWINAMPBUTTONS-1)
				lParamPtr++;
			hwndTemp = GetWindow(hwndTemp, GW_HWNDNEXT);
		}
	}
	if(Tab == WMPTAB) {
		if(MemoryOffset < TOTALWMPBUTTONS) {
			DataPtr = &WMPBinds.Play;
			lParamPtr = &WMPBinds.lParams[0];
		}
		else {
			DataPtr = &WMPBinds.Playlists[0];
			lParamPtr = &WMPBinds.plParams[0];
			MemoryOffset -= TOTALWMPBUTTONS;
		}


		DataPtr += MemoryOffset;
		lParamPtr += MemoryOffset;

		*DataPtr = BindData;
		*lParamPtr = lParamData;
	
		
		ConfirmNoDupInMain(*DataPtr);
		ConfirmNoDupInWMP(*DataPtr);
		
		

		hwndTemp = hwnd;
		for(int x=0; x<(GetWindowLong(hwndTemp, GWL_ID)- IDC_WMPPLAY); )
			hwndTemp = GetWindow(hwndTemp, GW_HWNDPREV);

		lParamPtr = &WMPBinds.lParams[0];

		for(unsigned int x=0; x<=TOTALWMPBUTTONS; x++) {
			char Label[20];

			if(x != TOTALWMPBUTTONS)
				GetKeyNameText((LONG)*lParamPtr, Label, 20);
			else
				GetKeyNameText((LONG)WMPBinds.plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)], Label, 20);


			if(strcmp(Label, "Delete") == 0)
				sprintf(Label, "Delete"); //For some reason it doesn't show with 1 D

			SetWindowText(hwndTemp, Label);


			if((x != TOTALWMPBUTTONS && *lParamPtr == 0) || (x == TOTALWMPBUTTONS && WMPBinds.plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)] == 0))
				SetWindowText(hwndTemp, "Unbound");


			if(x != TOTALWMPBUTTONS-1)
				lParamPtr++;

			hwndTemp = GetWindow(hwndTemp, GW_HWNDNEXT);
		}
	}

	if(Tab == ITUNESTAB) {
		if(MemoryOffset < TOTALITUNESBUTTONS) {
			DataPtr = &ITBinds.Play;
			lParamPtr = &ITBinds.lParams[0];
		}
		else {
			DataPtr = &ITBinds.Playlists[0];
			lParamPtr = &ITBinds.plParams[0];
			MemoryOffset -= TOTALITUNESBUTTONS;
		}


		DataPtr += MemoryOffset;
		lParamPtr += MemoryOffset;

		*DataPtr = BindData;
		*lParamPtr = lParamData;
	
		
		ConfirmNoDupInMain(*DataPtr);
		ConfirmNoDupInITunes(*DataPtr);
		
		

		hwndTemp = hwnd;
		for(int x=0; x<(GetWindowLong(hwndTemp, GWL_ID)- IDC_ITPLAY); )
			hwndTemp = GetWindow(hwndTemp, GW_HWNDPREV);

		lParamPtr = &ITBinds.lParams[0];

		for(unsigned int x=0; x<=TOTALITUNESBUTTONS; x++) {
			char Label[20];

			if(x != TOTALITUNESBUTTONS)
				GetKeyNameText((LONG)*lParamPtr, Label, 20);
			else
				GetKeyNameText((LONG)ITBinds.plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)], Label, 20);


			if(strcmp(Label, "Delete") == 0)
				sprintf(Label, "Delete"); //For some reason it doesn't show with 1 D

			SetWindowText(hwndTemp, Label);


			if((x != TOTALITUNESBUTTONS && *lParamPtr == 0) || (x == TOTALITUNESBUTTONS && ITBinds.plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)] == 0))
				SetWindowText(hwndTemp, "Unbound");


			if(x != TOTALITUNESBUTTONS -1)
				lParamPtr++;

			hwndTemp = GetWindow(hwndTemp, GW_HWNDNEXT);
		}
	}


}


LRESULT CALLBACK HotKeyProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char Label[20];


	switch (message)
	{
	case WM_LBUTTONDBLCLK:
		switch(GetWindowLong(hwnd, GWL_ID))
		{
		case IDC_WAPLAY:
			SetBindSettings(0, 0, 0, hwnd);
			break;
		case IDC_WASTOP:
			SetBindSettings(0, 0, 1, hwnd);
			break;
		case IDC_WAPAUS:
			SetBindSettings(0, 0, 2, hwnd);
			break;
		case IDC_WAPREV:
			SetBindSettings(0, 0, 3, hwnd);
			break;
		case IDC_WANEXT:
			SetBindSettings(0, 0, 4, hwnd);
			break;
		case IDC_WASHUF:
			SetBindSettings(0, 0, 5, hwnd);
			break;
		case IDC_WAREPT:
			SetBindSettings(0, 0, 6, hwnd);
			break;
		case IDC_WAVUP:
			SetBindSettings(0, 0, 7, hwnd);
			break;
		case IDC_WAVDN:
			SetBindSettings(0, 0, 8, hwnd);
			break;
		case IDC_WAREW:
			SetBindSettings(0, 0, 9, hwnd);
			break;
		case IDC_WAFWD:
			SetBindSettings(0, 0, 10, hwnd);
			break;
		case IDC_WAPLL:
			SetBindSettings(0, 0, TOTALWINAMPBUTTONS+(unsigned int)SendMessage(hwndCB, CB_GETCURSEL, NULL, NULL), hwnd);
			break;
		case IDC_WMPPLAY:
			SetBindSettings(0, 0, 0, hwnd);
			break;
		case IDC_WMPSTOP:
			SetBindSettings(0, 0, 1, hwnd);
			break;
		case IDC_WMPPREV:
			SetBindSettings(0, 0, 2, hwnd);
			break;
		case IDC_WMPNEXT:
			SetBindSettings(0, 0, 3, hwnd);
			break;
		case IDC_WMPVUP:
			SetBindSettings(0, 0, 4, hwnd);
			break;
		case IDC_WMPVDN:
			SetBindSettings(0, 0, 5, hwnd);
			break;
		case IDC_WMPMUTE:
			SetBindSettings(0, 0, 6, hwnd);
			break;
		case IDC_WMPREPT:
			SetBindSettings(0, 0, 7, hwnd);
			break;
		case IDC_WMPSHUF:
			SetBindSettings(0, 0, 8, hwnd);
			break;
		case IDC_WMPPLL:
			SetBindSettings(0, 0, TOTALWMPBUTTONS+(unsigned int)SendMessage(hwndCB, CB_GETCURSEL, NULL, NULL), hwnd);
			break;
		case IDC_ITPLAY:
			SetBindSettings(0, 0, 0, hwnd);
			break;
		case IDC_ITSTOP:
			SetBindSettings(0, 0, 1, hwnd);
			break;
		case IDC_ITPREV:
			SetBindSettings(0, 0, 2, hwnd);
			break;
		case IDC_ITNEXT:
			SetBindSettings(0, 0, 3, hwnd);
			break;
		case IDC_ITVUP:
			SetBindSettings(0, 0, 4, hwnd);
			break;
		case IDC_ITVDN:
			SetBindSettings(0, 0, 5, hwnd);
			break;
		case IDC_ITMUTE:
			SetBindSettings(0, 0, 6, hwnd);
			break;
		case IDC_ITREPT:
			SetBindSettings(0, 0, 7, hwnd);
			break;
		case IDC_ITSHUF:
			SetBindSettings(0, 0, 8, hwnd);
			break;
		case IDC_ITPLL:
			SetBindSettings(0, 0, TOTALITUNESBUTTONS+(unsigned int)SendMessage(hwndCB, CB_GETCURSEL, NULL, NULL), hwnd);
			break;
		case IDC_MEDPLAY:
			SetBindSettings(0, 0, 0, hwnd);
			break;
		case IDC_MEDPREV:
			SetBindSettings(0, 0, 1, hwnd);
			break;
		case IDC_MEDNEXT:
			SetBindSettings(0, 0, 2, hwnd);
			break;
		case IDC_MEDVUP:
			SetBindSettings(0, 0, 3, hwnd);
			break;
		case IDC_MEDVDN:
			SetBindSettings(0, 0, 4, hwnd);
			break;
		case IDC_MEDMUTE:
			SetBindSettings(0, 0, 5, hwnd);
			break;
		case IDC_MAINENABLE:
			SetBindSettings(0, 0, 0, hwnd);
			break;
		}

		SetWindowText(hwnd, "UNBOUND");
		SetFocus(GetParent(hwnd));
		break;

	case WM_CHAR:
	case WM_RBUTTONDOWN:
		return 0;

	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		return 0;

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if(wParam == 16 && GetKeyState(VK_LSHIFT) & 0x80)
			wParam = 160;
		else if(wParam == 16 && GetKeyState(VK_RSHIFT) & 0x80)
			wParam = 161;
		else if(wParam == 17 && GetKeyState(VK_LCONTROL) & 0x80)
			wParam = 162;
		else if(wParam == 17 && GetKeyState(VK_RCONTROL) & 0x80)
			wParam = 163;
		else if(wParam == 18 && GetKeyState(VK_LMENU) & 0x80)
			wParam = 164;
		else if(wParam == 18 && GetKeyState(VK_RMENU) & 0x80)
			wParam = 165;

		GetKeyNameText((LONG)lParam, Label, 20);


		if(strcmp(Label, "Delete") == 0)
			sprintf(Label, "Delete"); //For some reason it doesn't show with 1 D

		SetWindowText(hwnd, Label);

		switch(GetWindowLong(hwnd, GWL_ID))
		{
		case IDC_WAPLAY:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 0, hwnd);
			break;
		case IDC_WASTOP:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 1, hwnd);
			break;
		case IDC_WAPAUS:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 2, hwnd);
			break;
		case IDC_WAPREV:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 3, hwnd);
			break;
		case IDC_WANEXT:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 4, hwnd);
			break;
		case IDC_WASHUF:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 5, hwnd);
			break;
		case IDC_WAREPT:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 6, hwnd);
			break;
		case IDC_WAVUP:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 7, hwnd);
			break;
		case IDC_WAVDN:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 8, hwnd);
			break;
		case IDC_WAREW:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 9, hwnd);
			break;
		case IDC_WAFWD:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 10, hwnd);
			break;
		case IDC_WAPLL:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, TOTALWINAMPBUTTONS+(unsigned int)SendMessage(hwndCB, CB_GETCURSEL, NULL, NULL), hwnd);
			break;
		case IDC_WMPPLAY:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 0, hwnd);
			break;
		case IDC_WMPSTOP:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 1, hwnd);
			break;
		case IDC_WMPPREV:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 2, hwnd);
			break;
		case IDC_WMPNEXT:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 3, hwnd);
			break;
		case IDC_WMPVUP:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 4, hwnd);
			break;
		case IDC_WMPVDN:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 5, hwnd);
			break;
		case IDC_WMPMUTE:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 6, hwnd);
			break;
		case IDC_WMPREPT:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 7, hwnd);
			break;
		case IDC_WMPSHUF:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 8, hwnd);
			break;
		case IDC_WMPPLL:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, TOTALWMPBUTTONS+(unsigned int)SendMessage(hwndCB, CB_GETCURSEL, NULL, NULL), hwnd);
			break;
		case IDC_ITPLAY:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 0, hwnd);
			break;
		case IDC_ITSTOP:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 1, hwnd);
			break;
		case IDC_ITPREV:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 2, hwnd);
			break;
		case IDC_ITNEXT:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 3, hwnd);
			break;
		case IDC_ITVUP:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 4, hwnd);
			break;
		case IDC_ITVDN:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 5, hwnd);
			break;
		case IDC_ITMUTE:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 6, hwnd);
			break;
		case IDC_ITREPT:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 7, hwnd);
			break;
		case IDC_ITSHUF:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 8, hwnd);
			break;
		case IDC_ITPLL:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, TOTALITUNESBUTTONS+(unsigned int)SendMessage(hwndCB, CB_GETCURSEL, NULL, NULL), hwnd);
			break;
		case IDC_MEDPLAY:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 0, hwnd);
			break;
		case IDC_MEDPREV:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 1, hwnd);
			break;
		case IDC_MEDNEXT:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 2, hwnd);
			break;
		case IDC_MEDVUP:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 3, hwnd);
			break;
		case IDC_MEDVDN:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 4, hwnd);
			break;
		case IDC_MEDMUTE:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 5, hwnd);
			break;
		case IDC_MAINENABLE:
			SetBindSettings((unsigned int)wParam, (unsigned int)lParam, 0, hwnd);
			break;
		}

		SetFocus(GetParent(hwnd));
		return 0;
	}

	return CallWindowProc(WndprocHotKey, hwnd, message, wParam, lParam);
}