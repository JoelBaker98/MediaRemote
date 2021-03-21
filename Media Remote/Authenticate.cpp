#include <windows.h>
#include <stdio.h>
#include <Iptypes.h>
#include <Iphlpapi.h>


char *MACAddresses[3] = {"00-1A-4D-50-41-F5", "00-16-76-D4-4A-05", "00-0C-F1-AB-06-BB"};



char *GetMACAddress(BYTE *MACData)
{
	static char MACAddressBuf[100];

	sprintf(MACAddressBuf, "%02x-%02x-%02x-%02x-%02x-%02x", MACData[0], MACData[1], MACData[2], MACData[3], MACData[4], MACData[5]);
	strupr(MACAddressBuf);

	return MACAddressBuf;
}



bool Authenticate()
{
	IP_ADAPTER_INFO AdapterInfo[20];
	DWORD dwBufLen = sizeof(AdapterInfo);

	DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);

	if(dwStatus != ERROR_SUCCESS)
		return false;

	IP_ADAPTER_INFO *pAdapterInfo = AdapterInfo;

	do {
		for(int p=0; p<3; p++)
			if(strcmp(GetMACAddress(pAdapterInfo->Address), MACAddresses[p]) == 0)
				return true; //authenticated

		pAdapterInfo = pAdapterInfo->Next;
	} while(pAdapterInfo);

	return false;
}
