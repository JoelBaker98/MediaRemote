#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string>
#include "Winmain.h"
#include "PlaylistDlg.h"
#include "resource.h"




extern void WinampAddFileInitialize(HWND);
extern BOOL FileOpenDlgWinamp(HWND, PTSTR, PTSTR);
extern void RefreshComboBox();

extern ITControls ITBinds;
extern HINSTANCE hInst;
extern WAControls WinampBinds;
extern unsigned int Tab;

extern WMPControls WMPBinds;


unsigned int GetLBTextWidth(HWND hwnd, char *sItem)
{
	HDC hdc;
	SIZE sz;

	hdc = GetDC(hwnd);
	SelectObject(hdc, GetStockObject(ANSI_VAR_FONT));
	GetTextExtentPoint32(hdc, sItem, (int)strlen(sItem), &sz);
	ReleaseDC(hwnd, hdc);

	return sz.cx+5;

}


DLGPROC CALLBACK DlgWinampPlaylistProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	unsigned int LBWidth = 0, SelCount;
	char Buffer[5000];
	char FilePathAndNames[WINAMPTOTALPLAYLISTBUFFER]={'\0'}, FileName[MAX_PATH]={'\0'};
	int *Selections;
	unsigned int *flParams, *fBinds;
	char *pCurrentFile;
	std::string Filepath;
	static HWND hwndLB;
	static SPlaylistInterface *PlaylistInterface;
	static SPlaylistInterface WMPPLInterface = {WMPBinds.Playlists, WMPBinds.plParams, WMPBinds.PlaylistPaths, WMPBinds.PlaylistCount};
	static SPlaylistInterface WinampPLInterface = {WinampBinds.Playlists, WinampBinds.plParams, WinampBinds.PlaylistPaths, WinampBinds.PlaylistCount};
	static SPlaylistInterface ITPLInterface = {ITBinds.Playlists, ITBinds.plParams, ITBinds.PlaylistPaths, ITBinds.PlaylistCount};

	switch(message)
	{
	case WM_INITDIALOG:
		if(&WinampBinds == (WAControls*)lParam) {
			SetWindowText(hwndDlg, "Winamp Playlists");
			PlaylistInterface = &WinampPLInterface;
		}
		else if(&WMPBinds == (WMPControls*)lParam) {
			SetWindowText(hwndDlg, "Windows Media Player Playlists");
			PlaylistInterface = &WMPPLInterface;
		}
		else if(&ITBinds == (ITControls*)lParam) {
			SetWindowText(hwndDlg, "iTunes Playlists");
			PlaylistInterface = &ITPLInterface;
		}



		hwndLB =  CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTBOX, "",
					WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | LBS_EXTENDEDSEL | WS_TABSTOP,// | WS_BORDER,
					20, 20, 320, 180, hwndDlg, NULL, hInst, NULL);

		for(int p=0; PlaylistInterface->PlaylistPaths[p][0] != '\0'; p++)
			SendMessage(hwndLB, LB_ADDSTRING, NULL, (LPARAM)PlaylistInterface->PlaylistPaths[p]);

		SendMessage(hwndLB, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), FALSE);


		for(int x=0, y=(int)SendMessage(hwndLB, LB_GETCOUNT, 0, 0); x<y; x++) {
			SendMessage(hwndLB, LB_GETTEXT, x, (LPARAM)Buffer);
			LBWidth = max(GetLBTextWidth(hwndLB, Buffer), LBWidth);
		}


		SendMessage(hwndLB, LB_SETHORIZONTALEXTENT, LBWidth, NULL);

		return (DLGPROC)TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_ADD:
			WinampAddFileInitialize(hwndDlg);
			if(FileOpenDlgWinamp(hwndDlg, FilePathAndNames, FileName) == 0)
				return (DLGPROC)TRUE;


			
			if(strcmp(PathFindExtension(FilePathAndNames), "") != 0) { //if path leads to actual file or else just dir
				size_t LastChar;
				Filepath = FilePathAndNames;
				LastChar = Filepath.rfind('\\', strlen(FilePathAndNames))+1;
				Filepath[LastChar-1] = '\0';
				memcpy(FilePathAndNames, &FilePathAndNames[LastChar], WINAMPTOTALPLAYLISTBUFFER-LastChar);
			}
			else {
				Filepath = FilePathAndNames;
				memcpy(FilePathAndNames, &FilePathAndNames[strlen(FilePathAndNames)+1], WINAMPTOTALPLAYLISTBUFFER-(strlen(FilePathAndNames)+1));
			}

			pCurrentFile = FilePathAndNames; //assign pointer to current array so it can more easily be scanned

			while(pCurrentFile[0] != '\0') { //go until the last file has been added.. last file ends on double null

				sprintf(FileName, "%s\\%s", Filepath.c_str(), pCurrentFile); //add current file to end of filepath and store in FileName

				for(int x=0; x<SendMessage(hwndLB, LB_GETCOUNT, 0, 0); x++) {
					SendMessage(hwndLB, LB_GETTEXT, x, (LPARAM)Buffer);
					if(strcmp(FileName, Buffer) == 0) {
						strcpy(FileName, ""); //if file exists null it out and go to next..
						break;
					}
				}

				if(PathFileExists(FileName)) {
					SendMessage(hwndLB, LB_ADDSTRING, NULL, (LPARAM)FileName);
					
					for(int x=0, y=(int)SendMessage(hwndLB, LB_GETCOUNT, 0, 0); x<y; x++) {
						SendMessage(hwndLB, LB_GETTEXT, x, (LPARAM)Buffer);
						LBWidth = max(GetLBTextWidth(hwndLB, Buffer), LBWidth);
					}

					SendMessage(hwndLB, LB_SETHORIZONTALEXTENT, LBWidth, NULL);
				}

				pCurrentFile += strlen(pCurrentFile)+1; //increment pointer to next filename
			}
			return (DLGPROC)TRUE;

		case IDC_REMOVE:
			SelCount = (int)SendMessage(hwndLB, LB_GETSELCOUNT, 0, 0);
			Selections = (int*)malloc(SelCount*4);
			SendMessage(hwndLB, LB_GETSELITEMS, SelCount, (LPARAM)Selections);

			for(int p=SelCount-1; p>=0; p--)
				SendMessage(hwndLB, LB_DELETESTRING, Selections[p], 0);

			free(Selections);

			for(int x=0, y=(int)SendMessage(hwndLB, LB_GETCOUNT, 0, 0); x<y; x++) {
				SendMessage(hwndLB, LB_GETTEXT, x, (LPARAM)Buffer);
				LBWidth = max(GetLBTextWidth(hwndLB, Buffer), LBWidth);
			}

			SendMessage(hwndLB, LB_SETHORIZONTALEXTENT, LBWidth, NULL);
			return (DLGPROC)TRUE;

		case IDOK:
			int LBCount;
			LBCount = (int)SendMessage(hwndLB, LB_GETCOUNT, 0, 0);
			fBinds = new unsigned int[LBCount+1];
			flParams = new unsigned int[LBCount+1];

			ZeroMemory(fBinds, LBCount*4+4);
			ZeroMemory(flParams, LBCount*4+4);


			for(int x=0, y=(int)SendMessage(hwndLB, LB_GETCOUNT, 0, 0); x<y; x++) {
				SendMessage(hwndLB, LB_GETTEXT, x, (LPARAM)Buffer);
				for(unsigned int p=0; p<PlaylistInterface->PlaylistCount; p++)
					if(strcmp(Buffer, PlaylistInterface->PlaylistPaths[p]) == 0) {
						fBinds[x] = PlaylistInterface->Playlists[p];
						flParams[x] = PlaylistInterface->plParams[p];
						break;
					}	
			}


			PlaylistInterface->PlaylistCount = (int)SendMessage(hwndLB, LB_GETCOUNT, 0, 0);

			delete [] PlaylistInterface->PlaylistPaths;
			delete [] PlaylistInterface->Playlists;
			delete [] PlaylistInterface->plParams;

			PlaylistInterface->PlaylistPaths = new char[PlaylistInterface->PlaylistCount+1][1000];
			PlaylistInterface->Playlists = new unsigned int[PlaylistInterface->PlaylistCount+1];
			PlaylistInterface->plParams = new unsigned int[PlaylistInterface->PlaylistCount+1];
	
			ZeroMemory(PlaylistInterface->Playlists, PlaylistInterface->PlaylistCount*4);
			ZeroMemory(PlaylistInterface->plParams, PlaylistInterface->PlaylistCount*4);
			ZeroMemory(PlaylistInterface->PlaylistPaths, 1000*PlaylistInterface->PlaylistCount+1000);

			memcpy(PlaylistInterface->Playlists, fBinds, PlaylistInterface->PlaylistCount*4);
			memcpy(PlaylistInterface->plParams, flParams, PlaylistInterface->PlaylistCount*4);


			for(int x=0, y=(int)SendMessage(hwndLB, LB_GETCOUNT, 0, 0); x<y; x++) {
				SendMessage(hwndLB, LB_GETTEXT, x, (LPARAM)Buffer);
				strcpy(PlaylistInterface->PlaylistPaths[x], Buffer);
			}

			RefreshComboBox();

			delete [] fBinds;
			delete [] flParams;

			EndDialog(hwndDlg, 1);
			return (DLGPROC)TRUE;

		case IDCANCEL:
			EndDialog(hwndDlg, 0);
			return (DLGPROC)TRUE;
		}
	}

	return FALSE;
}