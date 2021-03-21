#ifndef _WINMAIN
#define _WINMAIN

#define TOTALMAINBUTTONS 1
#define TOTALWMPBUTTONS 9
#define TOTALWINAMPBUTTONS 11
#define TOTALITUNESBUTTONS 9
#define TOTALMEDIABUTTONS 6

#define TOTALTABS 5
#define MAINPROGRAMTAB 0
#define MEDIATAB 1
#define WINAMPTAB 2
#define WMPTAB 3
#define ITUNESTAB 4

#define IDC_WAPLAY 5001
#define IDC_WASTOP 5002
#define IDC_WAPAUS 5003
#define IDC_WAPREV 5004
#define IDC_WANEXT 5005
#define IDC_WASHUF 5006
#define IDC_WAREPT 5007
#define IDC_WAVUP  5008
#define IDC_WAVDN  5009
#define IDC_WAREW  5010
#define IDC_WAFWD  5011
#define IDC_WAPLL  5012
#define IDC_WAPLCB 5013

#define IDC_WMPPLAY 5050
#define IDC_WMPSTOP 5051
#define IDC_WMPPREV 5052
#define IDC_WMPNEXT 5053
#define IDC_WMPVUP  5054
#define IDC_WMPVDN  5055
#define IDC_WMPMUTE 5056
#define IDC_WMPSHUF 5057
#define IDC_WMPREPT 5058
#define IDC_WMPPLL  5059
#define IDC_WMPPLCB 5060

#define IDC_ITPLAY 5100
#define IDC_ITSTOP 5101
#define IDC_ITPREV 5102
#define IDC_ITNEXT 5103
#define IDC_ITVUP  5104
#define IDC_ITVDN  5105
#define IDC_ITMUTE 5106
#define IDC_ITSHUF 5107
#define IDC_ITREPT 5108
#define IDC_ITPLL  5109
#define IDC_ITPLCB 5110

#define IDC_MEDPLAY 5150
#define IDC_MEDPREV 5151
#define IDC_MEDNEXT 5152
#define IDC_MEDVUP  5153
#define IDC_MEDVDN  5154
#define IDC_MEDMUTE 5155

#define IDC_MAINENABLE 6001


#define TRAYID 9001
#define IDM_OPEN 2001
//#define IDM_SAYHELLO 2002
#define IDM_CLOSE 2003


/* WM_COMMAND WM_USER */
#define WMP_PLAY 0x4978
#define WMP_STOP 0x4979
#define WMP_NEXT 0x497B
#define WMP_PREV 0x497A
#define WMP_VOLUMEUP 0x497F
#define WMP_VOLUMEDOWN 0x4980
#define WMP_MUTE 0x4981
#define WMP_REPEAT 0x499B
#define WMP_SHUFFLE 0x499A
#define WMP_OPENURL 0x49B8

#define IT_PLAY 1005
#define IT_STOP 1006
#define IT_NEXT 1007
#define IT_PREV 1008
#define IT_VOLUMEUP 1009
#define IT_VOLUMEDOWN 1010
#define IT_MUTE 1011
#define IT_REPEAT 0x829A
#define IT_SHUFFLE 1013
#define IT_OPENURL 1014


#define WINAMP_PREV			40044
#define WINAMP_PLAY			40045
#define WINAMP_PAUSE		40046
#define WINAMP_STOP			40047
#define WINAMP_NEXT			40048
#define WINAMP_FFWD5S		40060
#define WINAMP_REW5S		40061
#define WINAMP_VOLUMEUP		40058
#define WINAMP_VOLUMEDOWN	40059
#define WINAMP_OPENURL		40155
#define IPC_GET_SHUFFLE		250
#define IPC_SET_SHUFFLE		252
#define IPC_GET_REPEAT		251
#define IPC_SET_REPEAT		253
/* WM_COMMAND WM_USER */

#define WINAMPTOTALPLAYLISTBUFFER 200000




typedef struct 
{
	unsigned int Play;
	unsigned int Stop;
	unsigned int Pause;
	unsigned int Previous;
	unsigned int Next;
	unsigned int Shuffle;
	unsigned int Repeat;
	unsigned int VolumeUp;
	unsigned int VolumeDown;
	unsigned int Rewind;
	unsigned int Forward;
	unsigned int lParams[TOTALWINAMPBUTTONS];
	unsigned int *Playlists;
	unsigned int *plParams;
	char (*PlaylistPaths)[1000];
	unsigned int PlaylistCount;
} WAControls;


typedef struct
{
	unsigned int Play;
	unsigned int Stop;
	unsigned int Previous;
	unsigned int Next;
	unsigned int VolumeUp;
	unsigned int VolumeDown;
	unsigned int Mute;
	unsigned int Repeat;
	unsigned int Shuffle;
	unsigned int lParams[TOTALWMPBUTTONS];
	unsigned int *Playlists;
	unsigned int *plParams;
	char (*PlaylistPaths)[1000];
	unsigned int PlaylistCount;
} WMPControls;

typedef struct
{
	unsigned int Play;
	unsigned int Stop;
	unsigned int Previous;
	unsigned int Next;
	unsigned int VolumeUp;
	unsigned int VolumeDown;
	unsigned int Mute;
	unsigned int Repeat;
	unsigned int Shuffle;
	unsigned int lParams[TOTALITUNESBUTTONS];
	unsigned int *Playlists;
	unsigned int *plParams;
	char (*PlaylistPaths)[1000];
	unsigned int PlaylistCount;
} ITControls;

typedef struct
{
	unsigned int Play;
	unsigned int Previous;
	unsigned int Next;
	unsigned int VolumeUp;
	unsigned int VolumeDown;
	unsigned int Mute;
	unsigned int lParams[TOTALMEDIABUTTONS];
} MKControls;


typedef struct {
	unsigned int Enabled;
	unsigned int lParams[TOTALMAINBUTTONS];
} MainControls;



#endif