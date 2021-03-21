#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include "WinMain.h"
#include "PlaylistDlg.h"


HWND hwndCB, hwndST;


extern HINSTANCE hInst;
extern WNDPROC WndprocHotKey, WndprocCBToolTip1;
extern unsigned int Tab;
extern WAControls WinampBinds;
extern WMPControls WMPBinds;
extern ITControls ITBinds;


extern LRESULT CALLBACK HotKeyProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT CALLBACK CBToolTipProc(HWND, UINT, WPARAM, LPARAM);




void SetupWindowMain(HWND hwnd, HWND *Controls)
{
	/* Static Control Boxes */
	Controls[0] = CreateWindow("static", "Enable/Disable:",
					WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
					60, 80, 150, 20, hwnd, NULL, hInst, NULL);
	


	/* Edit Control Boxes */
	Controls[1] = CreateWindow("Edit", "Unbound",
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE,
					170, 80, 100, 22, hwnd, (HMENU)IDC_MAINENABLE, hInst, NULL);





	WndprocHotKey = (WNDPROC) SetWindowLongPtr(Controls[1], GWL_WNDPROC, (LONG_PTR) HotKeyProc);


}


HWND CreateStaticHotKeyWindowHelper(HWND hwnd, char* Text, int yAxis)
{
	return CreateWindow("static", Text,
		WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
		60, yAxis, 77, 20, hwnd, NULL, hInst, NULL);
}

HWND CreateEditHotKeyWindowHelper(HWND hwnd, int yAxis, unsigned int Menu)
{
	return CreateWindow("Edit", "Unbound",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE,
		140, yAxis, 100, 22, hwnd, (HMENU)Menu, hInst, NULL);
}


void SetupWindowMediaKeys(HWND hwnd, HWND* Controls)
{
	/* Static Control Boxes */
	char ButtonText[TOTALMEDIABUTTONS][20] = { "Play/Pause:", "Prev Song:", "Next Song:", "Vol Up:", "Vol Down:", "Mute:" };

	for (int x = 0, yAxis = 80; x < TOTALMEDIABUTTONS; x++) {
		Controls[x] = CreateStaticHotKeyWindowHelper(hwnd, ButtonText[x], yAxis);
		yAxis += 25;
	}

	/* Edit Control Boxes */
	unsigned int MenuItems[TOTALMEDIABUTTONS] = { IDC_MEDPLAY, IDC_MEDPREV, IDC_MEDNEXT, IDC_MEDVUP, IDC_MEDVDN, IDC_MEDMUTE };
	for (int x = 0, yAxis = 80; x < TOTALMEDIABUTTONS; x++) {
		Controls[x + TOTALMEDIABUTTONS] = CreateEditHotKeyWindowHelper(hwnd, yAxis, MenuItems[x]);
		yAxis += 25;
	}


	WndprocHotKey = (WNDPROC)SetWindowLongPtr(Controls[TOTALMEDIABUTTONS], GWL_WNDPROC, (LONG_PTR)HotKeyProc);
	for (int i = TOTALMEDIABUTTONS + 1; i <= TOTALMEDIABUTTONS * 2; i++)
		SetWindowLongPtr(Controls[i], GWL_WNDPROC, (LONG_PTR)HotKeyProc);

}

void SetupWindowWinamp(HWND hwnd, HWND *Controls)
{
	/* Static Control Boxes */
	char ButtonText[TOTALWINAMPBUTTONS][20] = { "Play:", "Stop:", "Pause:", "Prev Song:", "Next Song:", "Shuffle:", "Repeat:", "Vol Up:", "Vol Down:", "Rewind:", "Forward:" };

	for (int x=0, yAxis=80; x<TOTALWINAMPBUTTONS; x++) {
		Controls[x] = CreateStaticHotKeyWindowHelper(hwnd, ButtonText[x], yAxis);
		yAxis += 25;
	}

	/* Edit Control Boxes */
	unsigned int MenuItems[TOTALWINAMPBUTTONS] = { IDC_WAPLAY, IDC_WASTOP, IDC_WAPAUS, IDC_WAPREV, IDC_WANEXT, IDC_WASHUF, IDC_WAREPT, IDC_WAVUP, IDC_WAVDN, IDC_WAREW, IDC_WAFWD };
	for (int x=0, yAxis=80; x<TOTALWINAMPBUTTONS; x++) {
		Controls[x+ TOTALWINAMPBUTTONS] = CreateEditHotKeyWindowHelper(hwnd, yAxis, MenuItems[x]);
		yAxis += 25;
	}

	/* Playlist Control Boxes */
	Controls[TOTALWINAMPBUTTONS*2] = CreateWindow("Edit", "Unbound",
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE,
					100, 405, 100, 22, hwnd, (HMENU)IDC_WAPLL, hInst, NULL);
	Controls[TOTALWINAMPBUTTONS*2+1] = CreateWindow(WC_COMBOBOX, "",
					WS_CHILD | WS_VISIBLE |  CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_VSCROLL,
					40, 380, 230, 157, hwnd, (HMENU)IDC_WAPLCB, hInst, NULL);
	Controls[TOTALWINAMPBUTTONS*2+2] = CreateWindow("static", "Playlists:",
					WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
					120, 360, 70, 20, hwnd, NULL, hInst, NULL);


	hwndST = Controls[TOTALWINAMPBUTTONS*2];
	hwndCB = Controls[TOTALWINAMPBUTTONS*2+1];
	

	for(unsigned int p=0; p<WinampBinds.PlaylistCount; p++) {
		if(WinampBinds.PlaylistPaths[p][0] != '\0')
			SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)WinampBinds.PlaylistPaths[p]);
		else
			break;
	}

	if(WinampBinds.PlaylistCount == 0) {
		ShowWindow(hwndST, SW_HIDE);
		SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)"Playlists");
	}

	SendMessage(hwndCB, CB_SETCURSEL, 0, 0);


	WndprocHotKey = (WNDPROC) SetWindowLongPtr(Controls[TOTALWINAMPBUTTONS], GWL_WNDPROC, (LONG_PTR) HotKeyProc);
	for(int i= TOTALWINAMPBUTTONS+1; i <= TOTALWINAMPBUTTONS*2; i++)
		SetWindowLongPtr(Controls[i], GWL_WNDPROC, (LONG_PTR)HotKeyProc);

	WndprocCBToolTip1 = (WNDPROC)SetWindowLongPtr(hwndCB, GWL_WNDPROC, (LONG_PTR)CBToolTipProc);
	SendMessage(hwndCB, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT ), FALSE);
}




void SetupWindowWMP(HWND hwnd, HWND *Controls)
{
	/* Static Control Boxes */
	char ButtonText[TOTALWMPBUTTONS][20] = { "Play/Pause:", "Stop:", "Prev Song:", "Next Song:", "Vol Up:", "Vol Down:", "Mute:", "Repeat:", "Shuffle:" };

	for (int x = 0, yAxis = 80; x < TOTALWMPBUTTONS; x++) {
		Controls[x] = CreateStaticHotKeyWindowHelper(hwnd, ButtonText[x], yAxis);
		yAxis += 25;
	}

	/* Edit Control Boxes */
	unsigned int MenuItems[TOTALWMPBUTTONS] = { IDC_WMPPLAY, IDC_WMPSTOP, IDC_WMPPREV, IDC_WMPNEXT, IDC_WMPVUP, IDC_WMPVDN, IDC_WMPMUTE, IDC_WMPREPT, IDC_WMPSHUF };
	for (int x = 0, yAxis = 80; x < TOTALWMPBUTTONS; x++) {
		Controls[x + TOTALWMPBUTTONS] = CreateEditHotKeyWindowHelper(hwnd, yAxis, MenuItems[x]);
		yAxis += 25;
	}


	/* Playlist Control Boxes */
	Controls[TOTALWMPBUTTONS*2] = CreateWindow("Edit", "Unbound",
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE,
					100, 365, 100, 22, hwnd, (HMENU)IDC_WMPPLL, hInst, NULL);
	Controls[TOTALWMPBUTTONS*2+1] = CreateWindow(WC_COMBOBOX, "",
					WS_CHILD | WS_VISIBLE |  CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_VSCROLL,
					40, 340, 230, 157, hwnd, (HMENU)IDC_WMPPLCB, hInst, NULL);
	Controls[TOTALWMPBUTTONS*2+2] = CreateWindow("static", "Playlists:",
					WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
					120, 320, 70, 20, hwnd, NULL, hInst, NULL);


	hwndST = Controls[TOTALWMPBUTTONS*2];
	hwndCB = Controls[TOTALWMPBUTTONS*2+1];
	


	for(unsigned int p=0; p<WMPBinds.PlaylistCount; p++) {
		if(WMPBinds.PlaylistPaths[p][0] != '\0')
			SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)WMPBinds.PlaylistPaths[p]);
		else
			break;
	}

	if(WMPBinds.PlaylistCount == 0) {
		ShowWindow(hwndST, SW_HIDE);
		SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)"Playlists");
	}

	SendMessage(hwndCB, CB_SETCURSEL, 0, 0);


	WndprocHotKey = (WNDPROC) SetWindowLongPtr(Controls[TOTALWMPBUTTONS], GWL_WNDPROC, (LONG_PTR) HotKeyProc);
	for(int i= TOTALWMPBUTTONS+1; i <= TOTALWMPBUTTONS*2; i++)
		SetWindowLongPtr (Controls[i], GWL_WNDPROC, (LONG_PTR)HotKeyProc);

	WndprocCBToolTip1 = (WNDPROC)SetWindowLongPtr(hwndCB, GWL_WNDPROC, (LONG_PTR)CBToolTipProc);
	SendMessage(hwndCB, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT ), FALSE);
}


void SetupWindowITunes(HWND hwnd, HWND *Controls)
{
	/* Static Control Boxes */
	char ButtonText[TOTALITUNESBUTTONS][20] = { "Play/Pause:", "Stop:", "Prev Song:", "Next Song:", "Vol Up:", "Vol Down:", "Mute:", "Repeat:", "Shuffle:" };

	for (int x = 0, yAxis = 80; x < TOTALITUNESBUTTONS; x++) {
		Controls[x] = CreateStaticHotKeyWindowHelper(hwnd, ButtonText[x], yAxis);
		yAxis += 25;
	}

	/* Edit Control Boxes */
	unsigned int MenuItems[TOTALITUNESBUTTONS] = { IDC_ITPLAY, IDC_ITSTOP, IDC_ITPREV, IDC_ITNEXT, IDC_ITVUP, IDC_ITVDN, IDC_ITMUTE, IDC_ITREPT, IDC_ITSHUF };
	for (int x = 0, yAxis = 80; x < TOTALITUNESBUTTONS; x++) {
		Controls[x + TOTALITUNESBUTTONS] = CreateEditHotKeyWindowHelper(hwnd, yAxis, MenuItems[x]);
		yAxis += 25;
	}

	/* Playlist Control Boxes */
	Controls[TOTALITUNESBUTTONS*2] = CreateWindow("Edit", "Unbound",
					WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER | ES_UPPERCASE,
					100, 365, 100, 22, hwnd, (HMENU)IDC_ITPLL, hInst, NULL);
	Controls[TOTALITUNESBUTTONS*2+1] = CreateWindow(WC_COMBOBOX, "",
					WS_CHILD | WS_VISIBLE |  CBS_DROPDOWNLIST | CBS_NOINTEGRALHEIGHT | WS_VSCROLL,
					40, 340, 230, 157, hwnd, (HMENU)IDC_ITPLCB, hInst, NULL);
	Controls[TOTALITUNESBUTTONS*2+2] = CreateWindow("static", "Playlists:",
					WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
					120, 320, 70, 20, hwnd, NULL, hInst, NULL);


	hwndST = Controls[TOTALITUNESBUTTONS*2];
	hwndCB = Controls[TOTALITUNESBUTTONS*2+1];
	


	for(unsigned int p=0; p<ITBinds.PlaylistCount; p++) {
		if(ITBinds.PlaylistPaths[p][0] != '\0')
			SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)ITBinds.PlaylistPaths[p]);
		else
			break;
	}

	if(ITBinds.PlaylistCount == 0) {
		ShowWindow(hwndST, SW_HIDE);
		SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)"Playlists");
	}

	SendMessage(hwndCB, CB_SETCURSEL, 0, 0);


	WndprocHotKey = (WNDPROC) SetWindowLongPtr(Controls[TOTALITUNESBUTTONS], GWL_WNDPROC, (LONG_PTR) HotKeyProc);
	for(int i= TOTALITUNESBUTTONS+1; i <= TOTALITUNESBUTTONS*2; i++)
		SetWindowLongPtr (Controls[i], GWL_WNDPROC, (LONG_PTR)HotKeyProc);

	WndprocCBToolTip1 = (WNDPROC)SetWindowLongPtr(hwndCB, GWL_WNDPROC, (LONG_PTR)CBToolTipProc);
	SendMessage(hwndCB, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT ), FALSE);
}


void DestroyWindowControls(HWND *Controls)
{
	for(int c=0; Controls[c]; c++)
		DestroyWindow(Controls[c]);
}


bool SetWindowButtons(HWND *Controls, unsigned int nBinds, unsigned int &FirstBind,
					unsigned int &FirstlParam)
{
	unsigned int *pData, *lParams;

	pData = &FirstBind;
	lParams = &FirstlParam;

	SendMessage(Controls[nBinds], WM_KEYDOWN, *pData, *lParams);


	return true;
}



void RefreshComboBox()
{
	char Label[20];
	SPlaylistInterface *PlaylistInterface;
	SPlaylistInterface WMPPLInterface = {WMPBinds.Playlists, WMPBinds.plParams, WMPBinds.PlaylistPaths, WMPBinds.PlaylistCount};
	SPlaylistInterface WinampPLInterface = {WinampBinds.Playlists, WinampBinds.plParams, WinampBinds.PlaylistPaths, WinampBinds.PlaylistCount};
	SPlaylistInterface ITPLInterface = {ITBinds.Playlists, ITBinds.plParams, ITBinds.PlaylistPaths, ITBinds.PlaylistCount};

	
	if(Tab == WINAMPTAB)
		PlaylistInterface = &WinampPLInterface;
	else if(Tab == WMPTAB)
		PlaylistInterface = &WMPPLInterface;
	else if(Tab == ITUNESTAB)
		PlaylistInterface = &ITPLInterface;
	else
		return;



	SendMessage(hwndCB, CB_RESETCONTENT, 0, 0);
	for(unsigned int p=0; p<PlaylistInterface->PlaylistCount; p++) {
		if(PlaylistInterface->PlaylistPaths[p][0] != '\0')
			SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)PlaylistInterface->PlaylistPaths[p]);
		else
			break;
	}
	if(PlaylistInterface->PlaylistCount == 0) {
		ShowWindow(hwndST, SW_HIDE);
		SendMessage(hwndCB, CB_ADDSTRING, NULL, (LPARAM)"Playlists");
	}
	else {
		ShowWindow(hwndST, SW_SHOW);

	}

	SendMessage(hwndCB, CB_SETCURSEL, 0, 0);
	if(PlaylistInterface->plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)] == 0)
		SetWindowText(hwndST, "Unbound");
	else {
		GetKeyNameText((LONG)PlaylistInterface->plParams[SendMessage(hwndCB, CB_GETCURSEL, 0, 0)], Label, 20);

		if(strcmp(Label, "Delete") == 0)
			sprintf(Label, "Delete"); //For some reason it doesn't show with 1 D

		SetWindowText(hwndST, Label);
	}
}








