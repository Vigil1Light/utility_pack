// process.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include <fstream>
#include <process.h>
#include <tlhelp32.h>
#include <strsafe.h>
#include <algorithm>
#include <psapi.h> // For access to GetModuleFileNameEx

#pragma comment(lib,"Version.lib")
#pragma comment(lib,"Psapi.lib")

//---         prevent Auto  ----------
BOOL		bStartUpPA = TRUE;
BOOL		bProcessExitPA = FALSE;
HANDLE		hThreadPA = NULL;
unsigned	threadIDPA;
unsigned __stdcall PreventAutothread( void* pArguments );


//--------------------------   Get process number ------------------------
int GetProcessNumber(char* strProcessName)
{
	int nNum = 0;
	HANDLE hSnap = INVALID_HANDLE_VALUE;
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 ProcessStruct;
	ProcessStruct.dwSize = sizeof(PROCESSENTRY32);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return 0;
	if (Process32First(hSnap, &ProcessStruct) == FALSE)
		return 0;

	do
	{
		if (_stricmp((char*)ProcessStruct.szExeFile, strProcessName) == 0)
		{
			int nID = ProcessStruct.th32ProcessID;
			nNum++;
		}
	} while (Process32Next(hSnap, &ProcessStruct));
	CloseHandle(hSnap);
	return nNum;
}

void GetVolumePaths(
	__in PWCHAR VolumeName, __out PWCHAR VolumePath
	)
{
	DWORD  CharCount = MAX_PATH + 1;
	PWCHAR Names     = NULL;
	PWCHAR NameIdx   = NULL;
	BOOL   Success   = FALSE;

	int i = 0;
	for (;;) 
	{
		//
		//  Allocate a buffer to hold the paths.
		Names = (PWCHAR) new BYTE [CharCount * sizeof(TCHAR)];

		if ( !Names ) 
		{
			//
			//  If memory can't be allocated, return.
			return;
		}

		//
		//  Obtain all of the paths
		//  for this volume.
		Success = GetVolumePathNamesForVolumeNameW(
			VolumeName, Names, CharCount, &CharCount
			);

		if ( Success ) 
		{
			break;
		}

		if ( GetLastError() != ERROR_MORE_DATA ) 
		{
			break;
		}

		//
		//  Try again with the
		//  new suggested size.
		delete [] Names;
		Names = NULL;
	}

	if ( Success )
	{
		//
		//  Display the various paths.
		for ( NameIdx = Names; 
			NameIdx[0] != L'\0'; 
			NameIdx += wcslen(NameIdx) + 1 ) 
		{
			wsprintfW(VolumePath, L"%s", NameIdx);
		}
		//sprintf("\n");
	}

	if ( Names != NULL ) 
	{
		delete [] Names;
		Names = NULL;
	}

	return;
}

void ConvertName(PWCHAR filenameNative, PWCHAR filename)
{
	DWORD  CharCount            = 0;
	WCHAR  DeviceName[MAX_PATH] = L"";
	DWORD  Error                = ERROR_SUCCESS;
	HANDLE FindHandle           = INVALID_HANDLE_VALUE;
	BOOL   Found                = FALSE;
	size_t Index                = 0;
	BOOL   Success              = FALSE;
	WCHAR  VolumeName[MAX_PATH] = L"";

	//
	//  Enumerate all volumes in the system.
	FindHandle = FindFirstVolumeW(VolumeName, ARRAYSIZE(VolumeName));

	if (FindHandle == INVALID_HANDLE_VALUE)
	{
		//Error = GetLastError();
		//sprintf("FindFirstVolumeW failed with error code %d\n", Error);
		return;
	}

	for (;;)
	{
		//
		//  Skip the \\?\ prefix and remove the trailing backslash.
		Index = wcslen(VolumeName) - 1;

		if (VolumeName[0]     != '\\' ||
			VolumeName[1]     != '\\' ||
			VolumeName[2]     != '?'  ||
			VolumeName[3]     != '\\' ||
			VolumeName[Index] != '\\') 
		{
			Error = ERROR_BAD_PATHNAME;
			wsprintfW(L"FindFirstVolumeW/FindNextVolumeW returned a bad path: %s\n", VolumeName);
			break;
		}

		//
		//  QueryDosDeviceW does not allow a trailing backslash,
		//  so temporarily remove it.
		VolumeName[Index] = '\0';

		CharCount = QueryDosDeviceW(&VolumeName[4], DeviceName, ARRAYSIZE(DeviceName)); 

		VolumeName[Index] = '\\';

		if ( CharCount == 0 ) 
		{
			//Error = GetLastError();
			//sprintf("QueryDosDeviceW failed with error code %d\n", Error);
			break;
		}

		//printf("\nFound a device:\n %s", DeviceName);

		PBYTE pOccurrence = std::search((PBYTE)filenameNative, (PBYTE)filenameNative + 2 * wcslen(filenameNative), (PBYTE)DeviceName, (PBYTE)DeviceName + 2 * wcslen(DeviceName));
		if(pOccurrence != (PBYTE)filenameNative + 2 * wcslen(filenameNative))
		{
			WCHAR VolumePath[MAX_PATH] = { 0 };
			GetVolumePaths(VolumeName, VolumePath);
			wsprintfW(filename, L"%s%s", VolumePath, filenameNative + wcslen(DeviceName) + 1);
		}


		//
		//  Move on to the next volume.
		Success = FindNextVolumeW(FindHandle, VolumeName, ARRAYSIZE(VolumeName));

		if ( !Success ) 
		{
			Error = GetLastError();

			if (Error != ERROR_NO_MORE_FILES) 
			{
				//				sprintf("FindNextVolumeW failed with error code %d\n", Error);
				break;
			}

			//
			//  Finished iterating
			//  through all the volumes.
			Error = ERROR_SUCCESS;
			break;
		}
	}

	FindVolumeClose(FindHandle);
	FindHandle = INVALID_HANDLE_VALUE;

	return;
}

BOOL EnableDebugPrivilege(BOOL bEnable)
{
	HANDLE hToken = nullptr;
	LUID luid;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) return FALSE;
	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid)) return FALSE;

	TOKEN_PRIVILEGES tokenPriv;
	tokenPriv.PrivilegeCount = 1;
	tokenPriv.Privileges[0].Luid = luid;
	tokenPriv.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, sizeof(TOKEN_PRIVILEGES), NULL, NULL) || GetLastError() == ERROR_NOT_ALL_ASSIGNED) return FALSE;

	return TRUE;
}

void GetFilePath(DWORD procID, WCHAR *pwfilename)
{
	HANDLE processHandle = NULL;
	WCHAR filenameNative[MAX_PATH] = { 0 };

	processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, procID);
	if (processHandle == NULL) 
	{
		if(!EnableDebugPrivilege(true))
			return;
		processHandle = OpenProcess(PROCESS_VM_READ, FALSE, procID);

		if (processHandle == NULL)
		{
			//Error("OpenProcess");
			return;
		}
	}
	GetProcessImageFileNameW(processHandle, filenameNative, MAX_PATH);
	ConvertName(filenameNative, pwfilename);

	CloseHandle(processHandle);

}

BOOL GetFileDescription(const WCHAR* filename, WCHAR* DescriptionName)
{
	int dwLen = GetFileVersionInfoSizeW(filename, NULL);
	if(!dwLen)
		return 0;
	auto *sKey = new BYTE[dwLen];
	std::unique_ptr<BYTE[]> skey_automatic_cleanup(sKey);
	if(!GetFileVersionInfoW(filename, NULL, dwLen, sKey))
		return 0;

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	UINT cbTranslate = 0;
	if(!VerQueryValue(sKey, "\\VarFileInfo\\Translation",
		(LPVOID*)&lpTranslate, &cbTranslate))
		return 0;

	for(unsigned int i = 0; i < (cbTranslate / sizeof(LANGANDCODEPAGE)); i++)
	{
		WCHAR subblock[256];
		//use sprintf if sprintf_s is not available
		wsprintfW(subblock, L"\\StringFileInfo\\%04x%04x\\FileDescription",
			lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
		WCHAR *description = NULL;
		UINT dwBytes;
		if(VerQueryValueW(sKey, subblock, (LPVOID*)&description, &dwBytes))
		{
			//MessageBox(0, description, 0, 0);
			memcpy((void *)DescriptionName, (const void*)description, 2 * wcslen(description));
		}
	}
	return TRUE;
}

void GetProcessDescription(DWORD ProcID, WCHAR* DescriptionName)
{
	WCHAR pPath[MAX_PATH] = { 0 };
	GetFilePath(ProcID, pPath);
	if(wcslen(pPath) != 0)
		GetFileDescription(pPath, DescriptionName);
}

BOOL CheckProcessRunning(PBYTE strProcessNameEnc, BOOL bUnicode)
{
	char strProcessNameTM[MAX_PATH] = { 0 };

	HANDLE hSnap = INVALID_HANDLE_VALUE;
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 ProcessStruct;
	ProcessStruct.dwSize = sizeof(PROCESSENTRY32);
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap == INVALID_HANDLE_VALUE)
		return FALSE;
	if (Process32First(hSnap, &ProcessStruct) == FALSE)
		return FALSE;
	do
	{

		WCHAR DescriptionName[MAX_PATH] = { 0 };
		GetProcessDescription(ProcessStruct.th32ProcessID, DescriptionName);

		for(size_t i = 0; i < strlen((const char*)strProcessNameEnc); i++)
			*(strProcessNameTM + i) = *(strProcessNameEnc + i) ^ 0x82;
		WCHAR pwTmp[MAX_PATH] = { 0 };

		if(bUnicode)
		{
			wsprintfW(pwTmp, L"%s", strProcessNameTM);
		}
		else
		{
			wsprintfW(pwTmp, L"%S", strProcessNameTM);
		}

		if (wcscmp(DescriptionName, pwTmp) == 0)
			return TRUE;

		char pDescr[MAX_PATH] = { 0 };
		sprintf(pDescr, "%S", DescriptionName);
		if(_stricmp("auto0", strProcessNameTM) == 0)
		{
			BYTE pPattern[] = {0x54, 0x68, 0x3f, 0x63, 0x20, 0x53, 0x3f, 0x6e, 0x20, 0x54, 0x61, 0x6d, 0x20, 0x47, 0x69, 0x3f, 0x69};
			PBYTE pOccurrence = std::search((PBYTE)pDescr, (PBYTE)pDescr + strlen(pDescr), (PBYTE)pPattern, (PBYTE)pPattern + sizeof(pPattern));
			if(pOccurrence != (PBYTE)pDescr + strlen(pDescr))
				return TRUE;
		}
		else if(_stricmp("auto2", strProcessNameTM) == 0 && strlen(pDescr) == 0)
		{
			char pTmp[MAX_PATH] = { 0 };
			sprintf_s(pTmp, "%s", ProcessStruct.szExeFile);
			char *s = pTmp;
			while (*s) {
				*s = toupper((unsigned char) *s);
				s++;
			}
			char pPattern[MAX_PATH] = { 0 };
			sprintf_s(pPattern, "AUTO");
			PBYTE pOccurrence = std::search((PBYTE)pTmp, (PBYTE)pTmp + strlen(pTmp), (PBYTE)pPattern, (PBYTE)pPattern + 4);
			if(pOccurrence != (PBYTE)pTmp + strlen(pTmp))
				return TRUE;
		}
	} while (Process32Next(hSnap, &ProcessStruct));
	CloseHandle(hSnap);
	return FALSE;
}

void MyMesaagebox(PBYTE pMsg)
{
	if(pMsg != NULL)
	{
		char pmsg[MAX_PATH] = { 0 };
		for(size_t i = 0; i < strlen((const char*)pMsg); i++)
			*(pmsg + i) = *(pMsg + i) ^ 0x82;
		MessageBox(NULL, pmsg, "Infor", 0L);
	}
}

void crashThisGame()
{
	*((unsigned int*)0) = 0xDEAD;
}

BYTE data1[] = {
	0xD6, 0xD1, 0xC3, 0xF7, 0xF6, 0xED, 0xA2, 0xB0, 0xAC, 0xB2, 0xA2, 0xC4, 0xEB, 0xEC, 0xE3, 0xEE, 0x00
};
BYTE data2[] = {
	0xC3, 0xF7, 0xF6, 0xED, 0xD1, 0xEB, 0xEF, 0xF2, 0xEE, 0xE7, 0x00
};
BYTE data3[] = {
	0xE3, 0xF7, 0xF6, 0xED, 0xB0, 0x00
};
BYTE data4[] = {
	0xD6, 0xD6, 0xC6, 0xC3, 0xF7, 0xF6, 0xED, 0x00
};
BYTE data5[] = {
	0xD6, 0xD1, 0xC3, 0xF7, 0xF6, 0xED, 0xA2, 0xB0, 0xAC, 0xB2, 0x00
};
BYTE data6[] = {
	0xD6, 0xD1, 0xC3, 0xF7, 0xF6, 0xED, 0xC6, 0xC9, 0xAF, 0xCC, 0xC1, 0x00
};
BYTE data7[] = {
	0xD6, 0xD1, 0xC3, 0xF7, 0xF6, 0xED, 0xCA, 0xCA, 0xA2, 0xB3, 0xAC, 0xB3, 0x00
};
BYTE data8[] = {
	0xC3, 0xF7, 0xF6, 0xED, 0xD2, 0xE3, 0xF0, 0xF6, 0xFB, 0x00
};
BYTE data9[] = {
	0xD6, 0x82, 0xEA, 0x82, 0x67, 0x9C, 0xE1, 0x82, 0xA2, 0x82, 0xD1, 0x82, 0x23, 0x83, 0xEC, 0x82, 0xA2, 0x82, 0xD6, 0x82, 0xE3, 0x82, 0xEF, 0x82, 0xA2, 0x82, 0xC5, 0x82, 0xEB, 0x82, 0x59, 0x9C, 0xEB, 0x82, 0x00
};


unsigned __stdcall PreventAutothread( void* pArguments )
{
	Sleep(10000);
	MyMesaagebox(data1);
	crashThisGame();
	if(bStartUpPA)
	{
		Sleep(180000); //every 180s
		//Sleep(1000); //every 180s
		bStartUpPA = FALSE;
	}
	while( !bProcessExitPA)
	{
		if( CheckProcessRunning(data1, 0))//TSAuto 2.0 Final
		{
			MyMesaagebox(data1);
			crashThisGame();
		}
		else if( CheckProcessRunning(data2, 0) )//AutoSimple
		{
			MyMesaagebox(data2);
			crashThisGame();
		}
		else if( CheckProcessRunning(data4, 0) )//TTDAuto
		{
			MyMesaagebox(data4);
			crashThisGame();
		}
		//else if( CheckProcessRunning(data5, 0) )//TSAuto 2.0
		//{
		//	MyMesaagebox(data5);
		//	crashThisGame();
		//}
		else if( CheckProcessRunning(data6, 0) )//TSAutoDK-NC
		{
			MyMesaagebox(data6);
			crashThisGame();
		}
		else if( CheckProcessRunning(data7, 0) )//TSAutoHH1.1
		{
			MyMesaagebox(data7);
			crashThisGame();
		}
		else if( CheckProcessRunning(data8, 0) )//AutoParty
		{
			MyMesaagebox(data8);
			crashThisGame();
		}
		else if( CheckProcessRunning(data9, 1) )//"Thục Sơn Tam Giới"
		{
		}
		else if( CheckProcessRunning(data3, 0) )//auto2
		{
			MyMesaagebox(data3);
			crashThisGame();
		}
		Sleep(120000); //every 120s
	}
	_endthreadex( 0 );
	return 0;
}
