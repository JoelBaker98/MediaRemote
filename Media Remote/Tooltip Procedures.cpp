#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <commctrl.h>
#include "WinMain.h"




extern HINSTANCE hInst;
extern WNDPROC WndprocCBToolTip1, WndprocCBToolTip2, WndprocWinampHUDTooltip;
extern HWND hSelf, hWinamp;



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









void MakeToolTip(const HWND &hwnd, HWND &hwndTT, const HWND &hwndLB, const RECT &CBRect, 
			  POINT &TTPoint, POINT &OldTTPoint, TOOLINFO &ti, int &TextLength)
{
	int index, topindex;
	POINT CursorPnt, LBPoint;
	RECT LBRect;



	topindex = (int)SendMessage(hwnd, CB_GETTOPINDEX, 0, 0);
	index = (int)SendMessage(hwnd, CB_GETCURSEL, 0, 0);
	SendMessage(hwnd, CB_GETLBTEXT, index, (LPARAM) ti.lpszText);


	if((index-topindex) >= 10 || (index-topindex) < 0) {
		SendMessage(hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
		return;
	}

	GetCursorPos(&CursorPnt);
	ScreenToClient(hwnd, &CursorPnt);

	if(CursorPnt.y < CBRect.bottom+1 && CursorPnt.y > CBRect.top) {
		memset(&OldTTPoint, 0, sizeof(POINT));
		SendMessage(hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
		return;
	}


	GetClientRect(hwndLB, &LBRect);
	LBPoint.y = LBRect.top;
	ClientToScreen(hwndLB, &LBPoint);

	TTPoint.x = TTPoint.y = 0;
	ClientToScreen(hwnd, &TTPoint);
	TTPoint.y = LBPoint.y + (13*(index - topindex))-4;

		
	if((TTPoint.x != OldTTPoint.x) || (TTPoint.y != OldTTPoint.y))
		OldTTPoint = TTPoint;
	else
		return;


	TextLength = GetTextWidth(hwnd, ti.lpszText, NULL);

	if(TextLength < LBRect.right) {
		SendMessage(hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
		return;
	}


	DestroyWindow(hwndTT);

	hwndTT = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
				TOOLTIPS_CLASS, NULL, WS_POPUP| TTS_ALWAYSTIP,		
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				hwnd, NULL, hInst, NULL);

	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
	SendMessage(hwndTT, TTM_TRACKPOSITION, 0, MAKELONG (TTPoint.x, TTPoint.y+2));
	SendMessage(hwndTT, TTM_TRACKACTIVATE, (WPARAM)true, (LPARAM) &ti);
}











/* This is used so when the tooltip on top of the combobox is clicked it will open the dropdown menu and hide the tooltip. */
LRESULT CALLBACK CBToolTipProc1(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TOOLINFO *ToolInfoPointer;



	switch(message)
	{
	case WM_CREATE:
		ToolInfoPointer = (TOOLINFO*)wParam;
		return 0;


	case WM_SETFOCUS:
		SendMessage(hwnd, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) ToolInfoPointer);
	
		SetFocus(ToolInfoPointer->hwnd);
		SendMessage(ToolInfoPointer->hwnd, CB_SHOWDROPDOWN, TRUE, 0);
		return 0;

	}


	return CallWindowProc(WndprocCBToolTip2, hwnd, message, wParam, lParam);
}


/* To change this function to the combo boxes you wish to use it just use the IDs of 
your controls and integrate them with this function with the if statements and thats
really all that has to be done. You can expand or shrink the color array and the 
hwndT_T array to what fits your program. You can also change the VScrollWidth constant
if your scroll bar is different than what this was used for.  */
LRESULT CALLBACK CBToolTipProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	const int VScrollWidth = 19;

    static char strTT[5000];
	static POINT TTPoint, OldTTPoint;
	static HWND hwndLB, Oldhwnd;
	static HWND *hwndTT = NULL, hwndTTStorage, hwndT_T[3] = {NULL};
	static RECT CBRect, LBRect;
	static TOOLINFO ti;
	static int TextLength;
	static bool TimerSet;
	static COLORREF color[3] = { RGB(255, 255, 255), RGB(255, 255, 255), RGB(255, 255, 255) };
	POINT CursorPnt, LBPoint, CBPoint;



	switch(message)
	{
	case WM_SETFOCUS:
		if(GetWindowLong(hwnd, GWL_ID) == IDC_WAPLCB
				|| GetWindowLong(hwnd, GWL_ID) == IDC_WMPPLCB
				|| GetWindowLong(hwnd, GWL_ID) == IDC_ITPLCB)
			color[0] = RGB(178, 180, 191);

		break;

	case WM_KILLFOCUS:
		if(GetWindowLong(hwnd, GWL_ID) == IDC_WAPLCB
				|| GetWindowLong(hwnd, GWL_ID) == IDC_WMPPLCB
				|| GetWindowLong(hwnd, GWL_ID) == IDC_ITPLCB)
			color[0] = RGB(255, 255, 255);

		break;

	case WM_TIMER:
		if(hwndTT == NULL) //Capture synthesized event and return if the window does not exist yet
			return 0;
		GetCursorPos(&CursorPnt);
		GetClientRect (hwnd, &CBRect);

		if(SendMessage(hwnd, CB_GETDROPPEDSTATE, 0, 0) == FALSE) {
			ScreenToClient(hwnd, &CursorPnt);
			if(CursorPnt.x >= CBRect.left && CursorPnt.x <= CBRect.right-VScrollWidth &&
			   CursorPnt.y >= CBRect.top && CursorPnt.y <= CBRect.bottom)
				return 0;
			else {
				TimerSet = false;
				KillTimer(hwnd, GetWindowLong(hwnd, GWL_ID));
				SendMessage(*hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
				return 0;
			}

		}
		
		LBPoint.x = LBRect.left;
		LBPoint.y = LBRect.top;
		ClientToScreen(hwndLB, &LBPoint);

		CBPoint.x = CBRect.left;
		CBPoint.y = CBRect.top;
		ClientToScreen(hwnd, &CBPoint);
		

		if(CursorPnt.x > LBPoint.x && CursorPnt.x < LBPoint.x+CBRect.right-VScrollWidth && 
		   CursorPnt.y > CBPoint.y && CursorPnt.y < CBPoint.y+LBRect.bottom+23 && 
		   LBPoint.y > CBPoint.y) {
			hwndTT = &hwndTTStorage;
			MakeToolTip(hwnd, *hwndTT, hwndLB, CBRect, TTPoint, OldTTPoint, ti, TextLength);
		}
		else if(CursorPnt.x > LBPoint.x && CursorPnt.x < LBPoint.x+CBRect.right-VScrollWidth && 
				CursorPnt.y < CBPoint.y && CursorPnt.y > CBPoint.y-LBRect.bottom &&
				LBPoint.y < CBPoint.y) {
			hwndTT = &hwndTTStorage;
			MakeToolTip(hwnd, *hwndTT, hwndLB, CBRect, TTPoint, OldTTPoint, ti, TextLength);
		} else {
			memset(&OldTTPoint, 0, sizeof(POINT));
			SendMessage(*hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
		}
		return 0;

	case WM_MOUSEMOVE:

		if(GetWindowLong(hwnd, GWL_ID) == IDC_WAPLCB
			|| GetWindowLong(hwnd, GWL_ID) == IDC_WMPPLCB
			|| GetWindowLong(hwnd, GWL_ID) == IDC_ITPLCB)
			hwndTT = &hwndT_T[0];


		if(hwnd != Oldhwnd || TimerSet == false) {
			KillTimer(Oldhwnd, GetWindowLong(Oldhwnd, GWL_ID));
			Oldhwnd = hwnd;
			SetTimer(hwnd, GetWindowLong(hwnd, GWL_ID), 100, NULL);
			GetClientRect (hwnd, &CBRect);
			TimerSet = true;
		}

		SendMessage(hwnd, WM_TIMER, 0, 0);


		if(*hwndTT == NULL) {

			*hwndTT = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
						TOOLTIPS_CLASS, NULL, WS_POPUP| TTS_ALWAYSTIP,
						CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
						hwnd, NULL, hInst, NULL);


 
			ti.cbSize = sizeof(TOOLINFO);
			ti.uFlags = TTF_TRACK | TTF_TRANSPARENT | TTF_ABSOLUTE;
			ti.hwnd = hwnd;
			ti.hinst = hInst;
			ti.uId = 0;
			ti.lpszText = strTT;


			WndprocCBToolTip2 = (WNDPROC)SetWindowLongPtr(*hwndTT, GWL_WNDPROC, (LONG_PTR) CBToolTipProc1);
			SendMessage(*hwndTT, WM_CREATE, (WPARAM)&ti, 0);
	

			SendMessage(*hwndTT, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &ti);
		}
		
		else if(SendMessage(hwnd, CB_GETDROPPEDSTATE, 0, 0) == FALSE &&
				LOWORD(lParam) < CBRect.right-VScrollWidth) {

			SendMessage(hwnd, CB_GETLBTEXT, SendMessage(hwnd, CB_GETCURSEL, 0, 0), (LPARAM) ti.lpszText);
	
			if((TextLength = GetTextWidth(hwnd, ti.lpszText, NULL)) < CBRect.right - VScrollWidth) {
				SendMessage(*hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
				break;
			}

			memset(&TTPoint, 0, sizeof(POINT));
			ClientToScreen(hwnd, &TTPoint);

			ti.hwnd = hwnd;

			SendMessage(*hwndTT, TTM_UPDATETIPTEXT, 0, (LPARAM) &ti);
			SendMessage(*hwndTT, TTM_TRACKPOSITION, 0, MAKELONG(TTPoint.x+1, TTPoint.y+2));
			SendMessage (*hwndTT, TTM_TRACKACTIVATE, (WPARAM)true, (LPARAM) &ti);
		}

		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case TTN_SHOW:
			SendMessage(*hwndTT, WM_SETFONT, (WPARAM)GetStockObject(ANSI_VAR_FONT), FALSE);
			if(GetWindowLong(Oldhwnd, GWL_ID) == IDC_WAPLCB
					|| GetWindowLong(hwnd, GWL_ID) == IDC_WMPPLCB
					|| GetWindowLong(hwnd, GWL_ID) == IDC_ITPLCB)
				SendMessage(*hwndTT, TTM_SETTIPBKCOLOR, color[0], 0);
			break;
		}

		break;

	case WM_CTLCOLORLISTBOX:
		hwndLB = (HWND) lParam;
		GetClientRect((HWND) lParam, &LBRect);
		SendMessage(hwnd, WM_TIMER, 0, 0);
		break;


	case WM_COMMAND:
		TimerSet = false;
		KillTimer(hwnd, GetWindowLong(hwnd, GWL_ID));
		SendMessage (*hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
		break;



	case WM_LBUTTONDOWN:
		SendMessage(*hwndTT, TTM_TRACKACTIVATE, (WPARAM)false, (LPARAM) &ti);
		break;

	case WM_DESTROY:
		DestroyWindow(hwndT_T[0]), DestroyWindow(hwndT_T[1]), DestroyWindow(hwndT_T[2]), 
		DestroyWindow(Oldhwnd), DestroyWindow(hwndTTStorage), DestroyWindow(hwndLB);
		hwndT_T[0] = hwndT_T[1] = hwndT_T[2] = hwndTTStorage = hwndLB = Oldhwnd = NULL;
		hwndTT = NULL;
		memset(&TTPoint, 0, sizeof(POINT));
		memset(&OldTTPoint, 0, sizeof(POINT));
		memset(&ti, 0, sizeof(TOOLINFO));
		TextLength = 0;
		strTT[0] = '\0';
		TimerSet = false;
		
		return 0;
	}

	return CallWindowProc(WndprocCBToolTip1, hwnd, message, wParam, lParam);
}