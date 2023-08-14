/*
   Memory Cleaner

   Copyright (C) 2021 Danske

   This file is part of Memory Cleaner

   Memory Cleaner is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Memory Cleaner is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Memory Cleaner.  If not, see <https://www.gnu.org/licenses/>.
*/

// Based on: https://gist.github.com/bitshifter/c87aa396446bbebeab29

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "stdafx.h"
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <setupapi.h>
#include <devguid.h>
#include <process.h>
#include <tlhelp32.h>

//---         clean ram  ----------
BOOL		bProcessExitCR = FALSE;
HANDLE		hThreadCR = NULL;
unsigned	threadIDCR;
unsigned __stdcall CleanRAMthread( void* pArguments );


typedef enum _SYSTEM_MEMORY_LIST_COMMAND
{
	MemoryCaptureAccessedBits,
	MemoryCaptureAndResetAccessedBits,
	MemoryEmptyWorkingSets,
	MemoryFlushModifiedList,
	MemoryPurgeStandbyList,
	MemoryPurgeLowPriorityStandbyList,
	MemoryCommandMax

} SYSTEM_MEMORY_LIST_COMMAND;

LUID luid;
BOOL bResult;
HANDLE hToken;
DWORD dwBufferLength;
typedef DWORD NTSTATUS;
TOKEN_PRIVILEGES tpNewState;
TOKEN_PRIVILEGES tpPreviousState;
HMODULE ntdll = LoadLibrary("ntdll.dll");

int CleanMemory(bool standbylist, bool modifiedlist, bool workingset, bool lowpriostandbylist) {
	dwBufferLength = 16;
	HANDLE hProcess = GetCurrentProcess();

	bResult = LookupPrivilegeValueA(0, "SeProfileSingleProcessPrivilege", &luid);

	OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken);

	if (bResult) {
		tpNewState.PrivilegeCount = 1;
		tpNewState.Privileges[0].Luid = luid;
		tpNewState.Privileges[0].Attributes = 0;
		bResult = AdjustTokenPrivileges(hToken, 0, &tpNewState, (LPBYTE) & (tpNewState.Privileges[1]) - (LPBYTE)&tpNewState, &tpPreviousState, &dwBufferLength);

		if (bResult) {
			tpPreviousState.PrivilegeCount = 1;
			tpPreviousState.Privileges[0].Luid = luid;
			tpPreviousState.Privileges[0].Attributes = 1 != 0 ? 2 : 0;
			bResult = AdjustTokenPrivileges(hToken, 0, &tpPreviousState, dwBufferLength, 0, 0);
		}
	}
	CloseHandle(hToken);

	NTSTATUS(WINAPI * NtSetSystemInformation)(INT, PVOID, ULONG) = (NTSTATUS(WINAPI*)(INT, PVOID, ULONG))GetProcAddress(ntdll, "NtSetSystemInformation");
	NTSTATUS(WINAPI * NtQuerySystemInformation)(INT, PVOID, ULONG, PULONG) = (NTSTATUS(WINAPI*)(INT, PVOID, ULONG, PULONG))GetProcAddress(ntdll, "NtQuerySystemInformation");

	if (standbylist == true) {
		SYSTEM_MEMORY_LIST_COMMAND command = MemoryPurgeStandbyList;
		NTSTATUS status = NtSetSystemInformation(80, &command, sizeof(SYSTEM_MEMORY_LIST_COMMAND));
	}

	if (modifiedlist == true) {
		SYSTEM_MEMORY_LIST_COMMAND command = MemoryFlushModifiedList;
		NTSTATUS status = NtSetSystemInformation(80, &command, sizeof(SYSTEM_MEMORY_LIST_COMMAND));
	}

	if (workingset == true) {
		SYSTEM_MEMORY_LIST_COMMAND command = MemoryEmptyWorkingSets;
		NTSTATUS status = NtSetSystemInformation(80, &command, sizeof(SYSTEM_MEMORY_LIST_COMMAND));
		//if(status == 0)
		//	MessageBox(NULL,"Success", "NtSetSystemInformation", 0L);
		//else
		//{
		//	char p[MAX_PATH] = { 0 };
		//	sprintf_s(p, "%x", status);
		//	MessageBox(NULL,p, "NtSetSystemInformation", 0L);
		//}
	}

	if (lowpriostandbylist == true) {
		SYSTEM_MEMORY_LIST_COMMAND command = MemoryPurgeLowPriorityStandbyList;
		NTSTATUS status = NtSetSystemInformation(80, &command, sizeof(SYSTEM_MEMORY_LIST_COMMAND));
	}

	return 0;
}

//-------------------------------------------------------------------------------------------------------
//--------------------------   Check if the PID of this process is the smallest ------------------------==
BOOL IsThisProcessSmallestPID(char* strProcessName)
{
	BOOL bRet = TRUE;
	DWORD dwCurPID = GetCurrentProcessId();
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
		if (stricmp(strupr((char*)ProcessStruct.szExeFile), strProcessName) == 0)
		{
			int nID = ProcessStruct.th32ProcessID;
			if(dwCurPID > nID)
				bRet = FALSE;
			nNum++;
		}
	} while (Process32Next(hSnap, &ProcessStruct));
	CloseHandle(hSnap);
	return bRet;

}
unsigned __stdcall CleanRAMthread( void* pArguments )
{
	while( !bProcessExitCR)
	{
		DWORD nCount = 0;
		while( !bProcessExitCR && nCount < 1200)
		{
			Sleep(3000);  //every 3s
			nCount++;
		}
		if(IsThisProcessSmallestPID("client.exe"))
			CleanMemory(FALSE, FALSE, TRUE, FALSE);
	}
	_endthreadex( 0 );
	return 0;
}