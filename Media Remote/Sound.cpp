#include <Dsound.h>
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>

IDirectSound8 *MainSound = NULL;
IDirectSoundBuffer *MR_Disabled, *MR_Enabled;//, *Lost, *Rguess, *Wguess;


extern HWND hSelf;


//const int ResourceID, const HINSTANCE &hLib, 
				  //IDirectSoundBuffer *&TestSound, HWND ghwnd, char *Path)//char *Type)


void PlaySoundDll(IDirectSoundBuffer *&TestSound, HWND ghwnd, char *Path)
{
	if(MR_Enabled)
		MR_Enabled->Stop();
	if(MR_Disabled)
		MR_Disabled->Stop();
	/*if(Lost)
		Lost->Stop();
	if(Rguess)
		Rguess->Stop();
	if(Wguess)
		Wguess->Stop();*/


	if(TestSound) {
		TestSound->SetCurrentPosition(0);
		TestSound->Play(NULL, 0, NULL);
		return;
	}

	//Open sound file from resource here
	/*HRSRC hResource = FindResource(hLib, MAKEINTRESOURCE(ResourceID), "WAVE");
	if(FindResource(hLib, MAKEINTRESOURCE(ResourceID), "WAVE") == NULL)
		exit(8);
	int iFileLength = SizeofResource(hLib, hResource);
	const void *pResourceData = LockResource((HRSRC)LoadResource(hLib, hResource));
	//HGLOBAL m_hBuffer  = GlobalAlloc(GMEM_MOVEABLE, imageSize);
	char *pBuffer;// = (char*)GlobalLock(m_hBuffer);
	pBuffer = (char*)malloc(iFileLength);
	CopyMemory(pBuffer, pResourceData, iFileLength);*/
	
	//Open sound file from hard drive here
	int iFileLength;
	char *pBuffer;
	HANDLE hFile;
	if(!PathFileExists(Path))
		return;
	hFile = CreateFile(Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	iFileLength = GetFileSize(hFile, NULL);
	DWORD dwBytesRead;
	pBuffer = (char*)malloc(iFileLength);
	ReadFile(hFile, pBuffer, iFileLength, &dwBytesRead, NULL);
	CloseHandle(hFile);

	//CopyMemory(pBuffer, F->, F->_bufsiz);
	//while(F != EOF)
	//	pBuffer[ getc(F);


	DirectSoundCreate8(NULL, &MainSound, NULL);
	MainSound->SetCooperativeLevel(ghwnd, DSSCL_PRIORITY);


	DSBUFFERDESC dsbd; // directsound buffer description
	WAVEFORMATEX pcmwf; // holds the format description
	// set up the format data structure
	memset(&pcmwf, 0, sizeof(WAVEFORMATEX));
	pcmwf.wFormatTag = WAVE_FORMAT_PCM; // always need this
	pcmwf.nChannels = 1;//2; // Stereo, so channels = 2
	pcmwf.nSamplesPerSec = 16000;//44100; // sample rate 44khz
	pcmwf.wBitsPerSample = 16; // 16 bits per sample
	pcmwf.nBlockAlign = pcmwf.wBitsPerSample/8 * pcmwf.nChannels; // see below
	// set to the total data per
	// block, in our case 1 channel times 1 byte per sample
	// so 1 byte total, if it was stereo then it would be
	// 2 and if stereo and 16 bit then it would be 4
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
	pcmwf.cbSize = 0; // always 0
	// set up the directsound buffer description
	memset(&dsbd,0,sizeof(DSBUFFERDESC));
	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = DSBCAPS_STATIC | DSBCAPS_LOCSOFTWARE | DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = iFileLength-42; 
	dsbd.lpwfxFormat = &pcmwf; // the WAVEFORMATEX struct

	//Create the buffer
	MainSound->CreateSoundBuffer(&dsbd, &TestSound, NULL);

	char *audio_ptr_1 = NULL, *audio_ptr_2 = NULL; //Used to retrieve buffer memory
	DWORD audio_length_1, audio_length_2; //Length of each buffer section
	//lock the buffer
	TestSound->Lock(0,0xFFFFFFFF, (void **)&audio_ptr_1, &audio_length_1, (void **)&audio_ptr_2, &audio_length_2,
	DSBLOCK_ENTIREBUFFER);    //Lock so you can inject the .wav file into it

	pBuffer+=42;  //Bypass the header of the .wav file
	memcpy(audio_ptr_1, pBuffer, audio_length_1);  //Copy .wav into DS memory
	memcpy(audio_ptr_2, (pBuffer+audio_length_1), audio_length_2);

	TestSound->Unlock(audio_ptr_1, audio_length_1, audio_ptr_2, audio_length_2);
	TestSound->SetVolume(1000);
	//TestSound->SetCurrentPosition(0);
	//TestSound->Play(NULL, 0, NULL);

	//MainSound->Release();
	//TestSound->Release();
	pBuffer-=42;  //Return to original allocation starting point
	free(pBuffer);
	//	GlobalUnlock(m_hBuffer);
	//GlobalFree(m_hBuffer);
}





void InitSounds(char *InstallPath, HWND hwnd)
{
	char FilePath[300];

	sprintf(FilePath, "%s\\Sounds\\Enabled.wav", InstallPath);
	PlaySoundDll(MR_Enabled, hwnd, FilePath);
	sprintf(FilePath, "%s\\Sounds\\Disabled.wav", InstallPath);
	PlaySoundDll(MR_Disabled, hwnd, FilePath);
}