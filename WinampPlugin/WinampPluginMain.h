#include "../Media Remote/WinMain.h"


#ifdef __cplusplus
#define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif

EXPORT void WinampKeyboard(WAControls WinampBinds, KBDLLHOOKSTRUCT *KeyData, BYTE *KeyStates, HWND hWinamp, bool &KeyProcessed,
				   bool EnableWinampSongNotify);

EXPORT void ShowToolTip(char sText[5000]);