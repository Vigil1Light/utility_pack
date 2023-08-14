// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>
#include <memory>
#include <ios>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <string>
#include <stdio.h>
#include <time.h>
#include <tlhelp32.h>
#include <setupapi.h>
#include <devguid.h>
#include <stack>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define READFILE_METHOD
#define CREATEFILE_METHOD
//#define MEMORY_LEAK_WATCH  //HeapAlloc, HeapFree in kernel32.dll
//#define MEMORY_LEAK_WATCH_MALLOCFREE //malloc, free in msvcr100.dll



#define STATIC_HDDSN "@PangKal_client_test@"  //must be equal to the data of encrypttool.exe
//#define STATIC_HDDSN "00.)!@(#*$&%^><.TamGioi.Net"  //must be equal to the data of encrypttool.exe
static AFX_EXTENSION_MODULE clientDLL = { NULL, NULL };
BOOL bMemLeakLog = FALSE;
int nProcessOrder;
CString strHDDSN;

//-------------- thread -----------
//---         clean ram  ----------
BOOL		bProcessExit = FALSE;
HANDLE		hThread;
unsigned	threadID;
unsigned __stdcall CleanRAMthread( void* pArguments );

extern int CleanMemory(bool standbylist, bool modifiedlist, bool workingset, bool lowpriostandbylist);

enum METHOD
{
	HEAPALLOC, HEAPFREE, CREATEFILE, CLOSEHANDLE, MALLOC, FREE, DECRYPT_ERROR, DELETE_ERROR
};

void WriteLog(HANDLE handle, size_t size, METHOD meth);
void WriteLog(char* pszname, METHOD meth);

char pCurPath[MAX_PATH] = { 0 };
HANDLE hCurrentFile = NULL;
BYTE pXORFix;
BYTE pXOR;
extern CString GetSN();
BOOL    GetFileNameFromHandle(HANDLE hFile, TCHAR *pszFileName, const unsigned int uiMaxLen);

bool MyDecryptFile(
	LPTSTR pszSourceFile, 
	LPTSTR pszDestinationFile, 
	LPTSTR pszPassword);

extern void DecryptSimple(PBYTE Buf, DWORD dwCount);
extern BOOL WINAPI MyReadFileToBuffer(
	_In_         HANDLE hFile,
	_Out_        LPVOID lpBuffer,
	_In_         DWORD nNumberOfBytesToRead,
	_Out_opt_    LPDWORD lpNumberOfBytesRead,
	_Inout_opt_  LPOVERLAPPED lpOverlapped);

typedef int (WINAPI *pMessageBoxA)(HWND, LPCSTR, LPCSTR, UINT);
typedef BOOL (WINAPI *pCloseHandleA)(HANDLE hObject);
typedef HANDLE (WINAPI *pCreateFileA)(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);
typedef BOOL(WINAPI *pReadFileA)(
	_In_         HANDLE hFile,
	_Out_        LPVOID lpBuffer,
	_In_         DWORD nNumberOfBytesToRead,
	_Out_opt_    LPDWORD lpNumberOfBytesRead,
	_Inout_opt_  LPOVERLAPPED lpOverlapped);
typedef LPVOID (WINAPI *pHeapAllocA)(HANDLE, DWORD, SIZE_T);
typedef BOOL (WINAPI *pHeapFreeA)(HANDLE, DWORD, LPVOID);
typedef LPVOID (*pMallocA)(size_t);
typedef void (*pFreeA)(LPVOID);

int WINAPI MyMessageBoxA(HWND, LPCSTR, LPCSTR, UINT);
BOOL WINAPI MyCloseHandleA(HANDLE);
HANDLE WINAPI MyCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile);
BOOL WINAPI MyReadFileA(
	_In_         HANDLE hFile,
	_Out_        LPVOID lpBuffer,
	_In_         DWORD nNumberOfBytesToRead,
	_Out_opt_    LPDWORD lpNumberOfBytesRead,
	_Inout_opt_  LPOVERLAPPED lpOverlapped);
LPVOID WINAPI MyHeapAllocA(HANDLE, DWORD, SIZE_T);
BOOL WINAPI MyHeapFreeA(HANDLE, DWORD, LPVOID);
LPVOID MyMallocA(size_t);
void MyFreeA(LPVOID);

void BeginRedirectMB(LPVOID);
void BeginRedirectCH(LPVOID);
void BeginRedirectCF(LPVOID);
void BeginRedirectRF(LPVOID);
void BeginRedirectHA(LPVOID);
void BeginRedirectHF(LPVOID);
void BeginRedirectMM(LPVOID);
void BeginRedirectMF(LPVOID);

pMessageBoxA pOrigMBAddress = NULL;
pCloseHandleA pOrigCHAddress = NULL;
pCreateFileA pOrigCFAddress = NULL;
pReadFileA pOrigRFAddress = NULL;
pHeapAllocA pOrigHAAddress = NULL;
pHeapFreeA pOrigHFAddress = NULL;
pMallocA pOrigMMAddress = NULL;
pFreeA pOrigMFAddress = NULL;

#define SIZE 6 //Number of bytes needed to redirect
BYTE oldBytesMB[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesCF[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesCH[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesRF[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesHA[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesHF[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesMM[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesMF[SIZE] = {0}; //This will hold the overwritten bytes
BYTE JMPMB[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPCF[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPCH[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPRF[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPHA[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPHF[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPMM[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPMF[SIZE] = {0};	//This holds the JMP to our code
DWORD oldProtectMB, myProtectMB = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectCF, myProtectCF = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectCH, myProtectCH = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectRF, myProtectRF = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectHA, myProtectHA = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectHF, myProtectHF = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectMM, myProtectMM = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectMF, myProtectMF = PAGE_EXECUTE_READWRITE; //Protection settings on memory
char debugBuffer[128]; //Used for DbgView

//---------------------------------------------------------------------------
//--------check if writable log file Memory Leak -------------
BOOL IsWritableForMemoryLeak()
{
	HANDLE          hFile;                       // Handle to directory
	CString     strFilePath;                 // Filepath
	WIN32_FIND_DATA FileInformation;             // File information
	BOOL bWritableLogForLeak = FALSE;

	TCHAR buf[2000];
	memset(buf, 0, 2000);

	strFilePath = pCurPath;
	strFilePath += "\\client_engine.ini";
	hFile = ::FindFirstFile(strFilePath, &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		std::ifstream infile;
		infile.open(strFilePath,std::ios::binary|std::ios::in);
		infile.read((char*)buf, 2000);
		infile.close();

		CString strContents = buf;
		int nIndex;
		if(((nIndex = strContents.Find("MemoryLeak")) != -1) || ((nIndex = strContents.Find("memoryleak")) != -1))
		{
			int nLen = strlen("MemoryLeak");
			int nEnd = strContents.Find("\r\n", nIndex + nLen);
			if(nEnd == -1)
				nEnd = strContents.GetLength();
			CString strTemp = strContents.Mid(nIndex + nLen, nEnd - nIndex - nLen);
			strTemp.Remove(' ');
			if(!strTemp.Compare("=true") || !strTemp.Compare("=TRUE"))
				bWritableLogForLeak = TRUE;
		}
	}
	::FindClose(hFile);
	return bWritableLogForLeak;
}

//---------------------------------------------------------------------------
//--------------------------   Get process number ------------------------
int GetProcessNumber(CString strProcessName)
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
		if (stricmp(strupr((char*)ProcessStruct.szExeFile), strProcessName) == 0)
		{
			int nID = ProcessStruct.th32ProcessID;
			nNum++;
		}
	} while (Process32Next(hSnap, &ProcessStruct));
	CloseHandle(hSnap);
	return nNum;
}

int DeleteProperFiles(CString refcstrRootDirectory,	bool bConsiderSubdirectories = TRUE)
{
	// subdirectories have been found
	HANDLE          hFile;                       // Handle to directory
	CString     strFilePath;                 // Filepath
	CString     strPattern;                  // Pattern
	WIN32_FIND_DATA FileInformation;             // File information


	strPattern = refcstrRootDirectory + "\\*.*";
	hFile = ::FindFirstFile(strPattern, &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		do
		{
			if(FileInformation.cFileName[0] != L'.')
			{
				strFilePath.Empty();
				strFilePath = refcstrRootDirectory + "\\" + FileInformation.cFileName;

				if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if(bConsiderSubdirectories)
					{
						// consider subdirectory
						int iRC = DeleteProperFiles(strFilePath, bConsiderSubdirectories);
						if(iRC)
							return iRC;
					}
				}
				else
				{
					// Set file attributes
					if(::SetFileAttributes(strFilePath,
						FILE_ATTRIBUTE_NORMAL) == FALSE)
						continue;

					//---------------------------------------------------------------
					if(strFilePath.Right(2).GetAt(1) == '_' && strFilePath.Right(2).GetAt(0) == '_')
					{
						// Delete file
						if(::DeleteFile(strFilePath) == FALSE)
							return ::GetLastError();
						//char pLogFileName[MAX_PATH] = { 0 };
						//sprintf_s(pLogFileName, "delete.log");
						//std::ofstream log(pLogFileName, std::ios_base::app | std::ios_base::out);
						//log << strFilePath; log << " deleted"; log << "\n";
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}
	return 0;
}

// IsInsideVPC's exception filter
DWORD __forceinline IsInsideVPC_exceptionFilter(LPEXCEPTION_POINTERS ep)
{
	PCONTEXT ctx = ep->ContextRecord;

	ctx->Ebx = -1; // Not running VPC
	ctx->Eip += 4; // skip past the "call VPC" opcodes
	return EXCEPTION_CONTINUE_EXECUTION;
	// we can safely resume execution since we skipped faulty instruction
}

// High level language friendly version of IsInsideVPC()
bool IsInsideVPC()
{
	bool rc = false;

	__try
	{
		_asm push ebx
		_asm mov  ebx, 0 // It will stay ZERO if VPC is running
		_asm mov  eax, 1 // VPC function number

		// call VPC 
		_asm __emit 0Fh
		_asm __emit 3Fh
		_asm __emit 07h
		_asm __emit 0Bh

		_asm test ebx, ebx
		_asm setz [rc]
		_asm pop ebx
	}
	// The except block shouldn't get triggered if VPC is running!!
	__except(IsInsideVPC_exceptionFilter(GetExceptionInformation()))
	{
	}

	return rc;
}

bool IsInsideVMWare()
{
	bool rc = true;

	__try
	{
		__asm
		{
			push   edx
			push   ecx
			push   ebx

			mov    eax, 'VMXh'
			mov    ebx, 0 // any value but not the MAGIC VALUE
			mov    ecx, 10 // get VMWare version
			mov    edx, 'VX' // port number

			in     eax, dx // read port
			// on return EAX returns the VERSION
			cmp    ebx, 'VMXh' // is it a reply from VMWare?
			setz   [rc] // set return value

			pop    ebx
			pop    ecx
			pop    edx
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		rc = false;
	}

	return rc;
}

BOOL InInSandbox()
{
	HKEY	hOpen;
	char	*szBuff;
	DWORD	iBuffSize;
	HANDLE	hMod;
	BOOL	bResult = FALSE;
	LONG	nRes;
	szBuff = (char*)calloc(512, sizeof(char));

	hMod = GetModuleHandle("SbieDll.dll");	//Sandboxie
	if(hMod != 0)
	{
		//MessageBoxA(NULL, "SbieDll.dll", "Infor", 0L);
		bResult = TRUE;
		return bResult;
	}

	//hMod = GetModuleHandle("dbghelp.dll");	//Thread Expert
	//if(hMod != 0)
	//{
	//	MessageBoxA(NULL, "dbghelp.dll", "Infor", 0L);
	//	bResult = TRUE;
	//	return bResult;
	//}

	nRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion", 0L, KEY_QUERY_VALUE, &hOpen);
	if(nRes == ERROR_SUCCESS)
	{
		iBuffSize = sizeof(szBuff);
		nRes = RegQueryValueEx(hOpen, "ProductId", NULL, NULL, (unsigned char*)szBuff, &iBuffSize);
		if(nRes == ERROR_SUCCESS)
		{
			if(strcmp(szBuff, "55274-640-2673064-23950") == 0) //Joe Box
			{
				//MessageBoxA(NULL, "Joe Box", "Infor", 0L);
				bResult = TRUE;
			}
			else if(strcmp(szBuff, "76487-644-3177037-23510") == 0) //CW Sandbox
			{
				//MessageBoxA(NULL, "CW Sandbox", "Infor", 0L);
				bResult = TRUE;
			}
			else if(strcmp(szBuff, "76487-337-8429955-22614") == 0) //Anubis
			{
				//MessageBoxA(NULL, "Anubis", "Infor", 0L);
				bResult = TRUE;
			}
			else
			{
				bResult = FALSE;
			}
		}
		RegCloseKey(hOpen);
	}
	return bResult;
}

BOOL VMDetect()
{
	char vbox[] = "VBOX";
	char qemu[] = "QEMU";
	char vmw[] = "VMWARE";
	char unkn[] = "VIRTUAL HD";
	char mvd[] = "Microsoft Virtual Disk";
	GUID h =  GUID_DEVCLASS_DISKDRIVE;
	DWORD rg, rsz;
	char *buf = (char*)malloc(4096);
	SP_DEVINFO_DATA devinfo;
	devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
	HDEVINFO devs = SetupDiGetClassDevs(&h, NULL, NULL, DIGCF_PRESENT);
	SetupDiEnumDeviceInfo(devs, 0, &devinfo);
	SetupDiGetDeviceRegistryPropertyA(devs, &devinfo, SPDRP_FRIENDLYNAME, &rg, (BYTE*)buf, 4096, &rsz);
	if(strstr(buf, vbox) != NULL ||
		strstr(buf, qemu) != NULL ||
		strstr(buf, vmw) != NULL || 
		strstr(buf, unkn) != NULL ||
		strstr(buf, mvd) != NULL)
	{
		MessageBoxA(NULL, buf, "Infor", 0L);
		free(buf);
		return TRUE;
	}
	else
	{
		free(buf);
		return FALSE;
	}
}

unsigned __stdcall CleanRAMthread( void* pArguments )
{
	while( !bProcessExit )
	{
		Sleep(10000);  //every 10s
		CleanMemory(false, false, true, false);
		//MessageBox(NULL, "Memory Clean OK", "Memory Clean", 0L);
	}
	_endthreadex( 0 );
	return 0;
}

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("client.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(clientDLL, hInstance))
			return 0;

		//Get Current Directory
		::GetCurrentDirectory(MAX_PATH, pCurPath);
		//Get HDD serial number
		//strHDDSN = GetSN();

		strHDDSN = STATIC_HDDSN;

		pXORFix = 0xab;
		for(DWORD i = 0; i < strHDDSN.GetLength(); i++)
		{
			pXORFix ^= strHDDSN.GetAt(i);
		}

		//check writable log for memory leak
		bMemLeakLog = IsWritableForMemoryLeak();
		
		//Get process number of client.exe
		nProcessOrder = GetProcessNumber("client.exe");

		//if(nProcessOrder == 1)
		//	DeleteProperFiles(pCurPath);

		//if(IsVMRunning)
		//	exit(0);
		//if(IsInsideVPC())
		//	exit(0);
		//if(IsInsideVMWare())
		//	exit(0);
		//if(InInSandbox())
		//	exit(0);
		if(VMDetect())
			exit(0);
				
		// Create thread for clean RAM. 
		hThread = (HANDLE)_beginthreadex( NULL, 0, &CleanRAMthread, NULL, 0, &threadID );

		//pOrigMBAddress = (pMessageBoxA) //Get MessageBoxA pointer
		//	GetProcAddress(GetModuleHandle("user32.dll"), "MessageBoxA");
		//if(pOrigMBAddress != NULL)
		//	BeginRedirectMB(MyMessageBoxA);	//Valid? Redirect
#ifdef CREATEFILE_METHOD
		pOrigCFAddress = (pCreateFileA) //Get CreateFileA pointer
			GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileA");
		if(pOrigCFAddress != NULL)
			BeginRedirectCF(MyCreateFileA);	//Valid? Redirect
		//pOrigCHAddress = (pCloseHandleA) //Get CloseHandleA pointer
		//	GetProcAddress(GetModuleHandle("kernel32.dll"), "CloseHandle");
		//if(pOrigCHAddress != NULL)
		//	BeginRedirectCH(MyCloseHandleA);	//Valid? Redirect
#endif

#ifdef READFILE_METHOD
		pOrigRFAddress = (pReadFileA) //Get ReadFile pointer
			GetProcAddress(GetModuleHandle("kernel32.dll"), "ReadFile");
		if(pOrigRFAddress != NULL)
			BeginRedirectRF(MyReadFileA);	//Valid? Redirect
#endif

		if(bMemLeakLog)
		{
#ifdef MEMORY_LEAK_WATCH
			pOrigHAAddress = (pHeapAllocA) //Get HeapAllocA pointer
				GetProcAddress(GetModuleHandle("kernel32.dll"), "HeapAlloc");
			if(pOrigHAAddress != NULL){
				MessageBox(NULL, "MyHeapAllocA", "Infor", 0L);
				BeginRedirectHA(MyHeapAllocA);	//Valid? Redirect
			}
			pOrigHFAddress = (pHeapFreeA) //Get HeapFreeA pointer
				GetProcAddress(GetModuleHandle("kernel32.dll"), "HeapFree");
			if(pOrigHFAddress != NULL){
				MessageBox(NULL, "MyHeapFreeA", "Infor", 0L);
				BeginRedirectHF(MyHeapFreeA);	//Valid? Redirect
			}
#endif

#ifdef MEMORY_LEAK_WATCH_MALLOCFREE
			pOrigMMAddress = (pMallocA) //Get MallocA pointer
				GetProcAddress(GetModuleHandle("msvcr100.dll"), "malloc");
			if(pOrigMMAddress != NULL)
				BeginRedirectMM(MyMallocA);	//Valid? Redirect
			pOrigMFAddress = (pFreeA) //Get FreeA pointer
				GetProcAddress(GetModuleHandle("msvcr100.dll"), "free");
			if(pOrigMFAddress != NULL)
				BeginRedirectMF(MyFreeA);	//Valid? Redirect
#endif

		}
		new CDynLinkLibrary(clientDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("client.DLL Terminating!\n");

		//memcpy(pOrigMBAddress, oldBytesMB, SIZE);
#ifdef CREATEFILE_METHOD
		memcpy(pOrigCFAddress, oldBytesCF, SIZE);
//		memcpy(pOrigCHAddress, oldBytesCH, SIZE);
#endif

#ifdef READFILE_METHOD
		memcpy(pOrigRFAddress, oldBytesRF, SIZE);
#endif
		if(bMemLeakLog)
		{
#ifdef MEMORY_LEAK_WATCH
			memcpy(pOrigHAAddress, oldBytesHA, SIZE);
			memcpy(pOrigHFAddress, oldBytesHF, SIZE);
#endif
#ifdef MEMORY_LEAK_WATCH_MALLOCFREE
			memcpy(pOrigMMAddress, oldBytesMM, SIZE);
			memcpy(pOrigMFAddress, oldBytesMF, SIZE);
#endif
		}

		if(hThread)
		{
			bProcessExit = TRUE;
			WaitForSingleObject( hThread, INFINITE );	
			CloseHandle( hThread );
			hThread = NULL;
		}
		// Terminate the library before destructors are called
		AfxTermExtensionModule(clientDLL);
	}
	return 1;   // ok
}

BOOL    GetFileNameFromHandle(HANDLE hFile, TCHAR *pszFileName, const unsigned int uiMaxLen)
{
	pszFileName[0]=0;

	std::unique_ptr<BYTE[]> ptrcFni(new BYTE[_MAX_PATH * sizeof(TCHAR) + sizeof(FILE_NAME_INFO)]);
	FILE_NAME_INFO *pFni = reinterpret_cast<FILE_NAME_INFO *>(ptrcFni.get());

	BOOL b = GetFileInformationByHandleEx(hFile, 
		FileNameInfo,
		pFni,
		sizeof(FILE_NAME_INFO) + (_MAX_PATH * sizeof(TCHAR)) );
	if ( b )
	{
#ifdef  _UNICODE
		wcsncpy_s(pszFileName, 
			min(uiMaxLen, (pFni->FileNameLength / sizeof(pFni->FileName[0])) + 1 ), 
			pFni->FileName, 
			_TRUNCATE);
#else
		strncpy_s(pszFileName, 
			min(uiMaxLen, (pFni->FileNameLength / sizeof(pFni->FileName[0])) + 1), 
			CW2A(pFni->FileName), 
			_TRUNCATE);
#endif
	}
	return b;
}

void BeginRedirectMB(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigMBAddress: %x", pOrigMBAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPMB, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigMBAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigMBAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectMB);
			//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesMB, pOrigMBAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesMB[0], oldBytesMB[1],
		oldBytesMB[2], oldBytesMB[3], oldBytesMB[4], oldBytesMB[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPMB[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPMB[0], JMPMB[1],
		JMPMB[2], JMPMB[3], JMPMB[4], JMPMB[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigMBAddress, JMPMB, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigMBAddress, SIZE, oldProtectMB, NULL); //Change setts back
}

void BeginRedirectCF(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigCFAddress: %x", pOrigCFAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPCF, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigCFAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigCFAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectCF);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesCF, pOrigCFAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesCF[0], oldBytesCF[1],
		oldBytesCF[2], oldBytesCF[3], oldBytesCF[4], oldBytesCF[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPCF[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPCF[0], JMPCF[1],
		JMPCF[2], JMPCF[3], JMPCF[4], JMPCF[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigCFAddress, JMPCF, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigCFAddress, SIZE, oldProtectCF, NULL); //Change setts back
}

void BeginRedirectCH(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigCHAddress: %x", pOrigCHAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPCH, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigCHAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigCHAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectCH);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesCH, pOrigCHAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesCH[0], oldBytesCH[1],
		oldBytesCH[2], oldBytesCH[3], oldBytesCH[4], oldBytesCH[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPCH[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPCH[0], JMPCH[1],
		JMPCH[2], JMPCH[3], JMPCH[4], JMPCH[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigCHAddress, JMPCH, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigCHAddress, SIZE, oldProtectCH, NULL); //Change setts back
}

void BeginRedirectRF(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigRFAddress: %x", pOrigRFAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPRF, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigRFAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigRFAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectRF);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesRF, pOrigRFAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesRF[0], oldBytesRF[1],
		oldBytesRF[2], oldBytesRF[3], oldBytesRF[4], oldBytesRF[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPRF[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPRF[0], JMPRF[1],
		JMPRF[2], JMPRF[3], JMPRF[4], JMPRF[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigRFAddress, JMPRF, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigRFAddress, SIZE, oldProtectRF, NULL); //Change setts back
}

void BeginRedirectHA(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigHAAddress: %x", pOrigHAAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPHA, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigHAAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigHAAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectHA);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesHA, pOrigHAAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesHA[0], oldBytesHA[1],
		oldBytesHA[2], oldBytesHA[3], oldBytesHA[4], oldBytesHA[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPHA[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPHA[0], JMPHA[1],
		JMPHA[2], JMPHA[3], JMPHA[4], JMPHA[5]);
	OutputDebugString(debugBuffer);
	MessageBox(NULL, "MyHeapAllocA-13", "Infor", 0L);
	memcpy(pOrigHAAddress, JMPHA, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigHAAddress, SIZE, oldProtectHA, NULL); //Change setts back
}

void BeginRedirectHF(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigHFAddress: %x", pOrigHFAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPHF, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigHFAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigHFAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectHF);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesHF, pOrigHFAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesHF[0], oldBytesHF[1],
		oldBytesHF[2], oldBytesHF[3], oldBytesHF[4], oldBytesHF[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPHF[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPHF[0], JMPHF[1],
		JMPHF[2], JMPHF[3], JMPHF[4], JMPHF[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigHFAddress, JMPHF, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigHFAddress, SIZE, oldProtectHF, NULL); //Change setts back
}

void BeginRedirectMM(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigMMAddress: %x", pOrigMMAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPMM, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigMMAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigMMAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectMM);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesMM, pOrigMMAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesMM[0], oldBytesMM[1],
		oldBytesMM[2], oldBytesMM[3], oldBytesMM[4], oldBytesMM[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPMM[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPMM[0], JMPMM[1],
		JMPMM[2], JMPMM[3], JMPMM[4], JMPMM[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigMMAddress, JMPMM, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigMMAddress, SIZE, oldProtectMM, NULL); //Change setts back
}

void BeginRedirectMF(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigMFAddress: %x", pOrigMFAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPMF, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigMFAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigMFAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectMF);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesMF, pOrigMFAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesMF[0], oldBytesMF[1],
		oldBytesMF[2], oldBytesMF[3], oldBytesMF[4], oldBytesMF[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPMF[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPMF[0], JMPMF[1],
		JMPMF[2], JMPMF[3], JMPMF[4], JMPMF[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigMFAddress, JMPMF, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigMFAddress, SIZE, oldProtectMF, NULL); //Change setts back
}

int  WINAPI MyMessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uiType)
{
	VirtualProtect((LPVOID)pOrigMBAddress, SIZE, myProtectMB, NULL); //ReadWrite again
	memcpy(pOrigMBAddress, oldBytesMB, SIZE); //Unhook API
	MessageBoxA(NULL, "This should pop up", "Hooked MBW", MB_ICONEXCLAMATION);
	int retValue = MessageBoxA(hWnd, lpText, lpCaption, uiType); //Get ret value
	memcpy(pOrigMBAddress, JMPMB, SIZE); //Rehook API
	VirtualProtect((LPVOID)pOrigMBAddress, SIZE, oldProtectMB, NULL); //Normal setts
	return retValue; //Return what should be returned
}

void MyUnHook()
{
	if(bMemLeakLog)
	{
#ifdef MEMORY_LEAK_WATCH
		//--------------         HeapAlloc          ------------------
		VirtualProtect((LPVOID)pOrigHAAddress, SIZE, myProtectHA, NULL); //ReadWrite again
		memcpy(pOrigHAAddress, oldBytesHA, SIZE); //Unhook API

		//--------------         HeapFree          ------------------
		VirtualProtect((LPVOID)pOrigHFAddress, SIZE, myProtectHF, NULL); //ReadWrite again
		memcpy(pOrigHFAddress, oldBytesHF, SIZE); //Unhook API
#endif
#ifdef MEMORY_LEAK_WATCH_MALLOCFREE
		//--------------         malloc          ------------------
		VirtualProtect((LPVOID)pOrigMMAddress, SIZE, myProtectMM, NULL); //ReadWrite again
		memcpy(pOrigMMAddress, oldBytesMM, SIZE); //Unhook API
		//--------------         free          ------------------
		VirtualProtect((LPVOID)pOrigMFAddress, SIZE, myProtectMF, NULL); //ReadWrite again
		memcpy(pOrigMFAddress, oldBytesMF, SIZE); //Unhook API
#endif
	}
#ifdef CREATEFILE_METHOD
	//--------------         CreateFile          ------------------
	VirtualProtect((LPVOID)pOrigCFAddress, SIZE, myProtectCF, NULL); //ReadWrite again
	memcpy(pOrigCFAddress, oldBytesCF, SIZE); //Unhook API

	//--------------         CloseHandle          ------------------
	//VirtualProtect((LPVOID)pOrigCHAddress, SIZE, myProtectCH, NULL); //ReadWrite again
	//memcpy(pOrigCHAddress, oldBytesCH, SIZE); //Unhook API
#endif
}

void MyReHook()
{
#ifdef CREATEFILE_METHOD
	//--------------         CloseHandle          ------------------
	//memcpy(pOrigCHAddress, JMPCH, SIZE); //Rehook API
	//VirtualProtect((LPVOID)pOrigCHAddress, SIZE, oldProtectCH, NULL); //Normal setts
	//--------------         CreateFile          ------------------
	memcpy(pOrigCFAddress, JMPCF, SIZE); //Rehook API
	VirtualProtect((LPVOID)pOrigCFAddress, SIZE, oldProtectCF, NULL); //Normal setts
#endif
	if(bMemLeakLog)
	{
#ifdef MEMORY_LEAK_WATCH_MALLOCFREE
		//--------------         free          ------------------
		memcpy(pOrigMFAddress, JMPMF, SIZE); //Rehook API
		VirtualProtect((LPVOID)pOrigMFAddress, SIZE, oldProtectMF, NULL); //Normal setts
		//--------------         malloc          ------------------
		memcpy(pOrigMMAddress, JMPMM, SIZE); //Rehook API
		VirtualProtect((LPVOID)pOrigMMAddress, SIZE, oldProtectMM, NULL); //Normal setts
#endif
#ifdef MEMORY_LEAK_WATCH
		//--------------         HeapFree          ------------------
		memcpy(pOrigHFAddress, JMPHF, SIZE); //Rehook API
		VirtualProtect((LPVOID)pOrigHFAddress, SIZE, oldProtectHF, NULL); //Normal setts
		//--------------         HeapAlloc          ------------------
		memcpy(pOrigHAAddress, JMPHA, SIZE); //Rehook API
		VirtualProtect((LPVOID)pOrigHAAddress, SIZE, oldProtectHA, NULL); //Normal setts
#endif
	}
}

BOOL  WINAPI MyCloseHandleA(HANDLE hobject)
{
	BOOL bRet;
	MyUnHook();
	char pszFileName[MAX_PATH] = {0};
	GetFileNameFromHandle(hobject, pszFileName, MAX_PATH);
	bRet = CloseHandle(hobject);
	int nLen = strlen(pszFileName);
	if(nLen > 0 && pszFileName[nLen-1] == '_' && pszFileName[nLen-2] == '_')
	{
		if(::DeleteFile(pszFileName))
			WriteLog(pszFileName, CLOSEHANDLE);
		else
			WriteLog(pszFileName, DELETE_ERROR);
	}

	MyReHook();
	return bRet; //Return what should be returned
}

inline bool exists_file(const std::string& name) {
	struct stat buffer;   
	return (stat (name.c_str(), &buffer) == 0); 
}

HANDLE WINAPI MyCreateFileA(
	LPCSTR lpFileName,
	DWORD dwDesiredAccess,
	DWORD dwShareMode,
	LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes,
	HANDLE hTemplateFile)
{
	HANDLE handle = NULL;
	MyUnHook();
	if( strstr(lpFileName, "local\\") != NULL || strstr(lpFileName, "local/") != NULL ||
		strstr(lpFileName, "maps\\") != NULL ||  strstr(lpFileName, "maps/") != NULL ||
		strstr(lpFileName, "script\\") != NULL || strstr(lpFileName, "script/") != NULL ||
		strstr(lpFileName, "ui\\") != NULL || strstr(lpFileName, "ui/") != NULL ||
		strstr(lpFileName, "model\\actor\\") != NULL || strstr(lpFileName, "model\\actor/") != NULL ||
		strstr(lpFileName, "scene\\actor\\") != NULL || strstr(lpFileName, "scene\\actor/") != NULL) 
	{
		//char lpDestFileName[MAX_PATH] = { 0 };
		//sprintf_s(lpDestFileName, "%s%d__", lpFileName, nProcessOrder);
		////while(exists_file(lpDestFileName));

		DWORD dwRetVal = 0;
		UINT uRetVal   = 0;
		TCHAR szTempFileName[MAX_PATH];  
		TCHAR lpTempPathBuffer[MAX_PATH];
		
		//  Gets the temp path env string (no guarantee it's a valid path).
		dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
			lpTempPathBuffer); // buffer for path 
		if (dwRetVal > MAX_PATH || (dwRetVal == 0))
		{
			MessageBox(NULL, "GetTempPath failed", "Infor", 0L);
			return NULL;
		}

		//  Generates a temporary file name. 
		uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
			TEXT("clienttmp"),     // temp file name prefix 
			0,                // create unique name 
			szTempFileName);  // buffer for name 
		if (uRetVal == 0)
		{
			MessageBox(NULL, "GetTempFileName failed", "Infor", 0L);
			return NULL;
		}

		pXOR = pXORFix;
		if(MyDecryptFile((LPTSTR)lpFileName, szTempFileName, strHDDSN.GetBuffer()))
		{
			handle = CreateFile(szTempFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes | FILE_FLAG_DELETE_ON_CLOSE, hTemplateFile);
			WriteLog((PSTR)lpFileName, CREATEFILE);
		}
		else
		{
			WriteLog((PSTR)lpFileName, DECRYPT_ERROR);
//			::DeleteFile(szTempFileName);
		}
	}
	else
	{
		handle = CreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	MyReHook();	
	return handle; //Return what should be returned
}

BOOL WINAPI MyReadFileA(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesToRead, LPOVERLAPPED lpOverlapped)
{
	VirtualProtect((LPVOID)pOrigRFAddress, SIZE, myProtectRF, NULL); //ReadWrite again
	memcpy(pOrigRFAddress, oldBytesRF, SIZE); //Unhook API

	BOOL bRet;

	DWORD dwNumberOfBytesToRead = 0;
	TCHAR pChar[MAX_PATH] = { 0 };
	void* buf = malloc(nNumberOfBytesToRead);
	memset(buf, 0, nNumberOfBytesToRead);

	char lpFileName[MAX_PATH] = { 0 };
	GetFileNameFromHandle(hFile, lpFileName, MAX_PATH);

	if( strstr(lpFileName, "local\\") != NULL || strstr(lpFileName, "local/") != NULL ||
		strstr(lpFileName, "maps\\") != NULL ||  strstr(lpFileName, "maps/") != NULL ||
		strstr(lpFileName, "script\\") != NULL || strstr(lpFileName, "script/") != NULL ||
		strstr(lpFileName, "ui\\") != NULL || strstr(lpFileName, "ui/") != NULL ||
		strstr(lpFileName, "model\\actor\\") != NULL || strstr(lpFileName, "model\\actor/") != NULL ||
		strstr(lpFileName, "scene\\actor\\") != NULL || strstr(lpFileName, "scene\\actor/") != NULL ) 
	{
		bRet = ReadFile(hFile, buf, nNumberOfBytesToRead, &dwNumberOfBytesToRead, lpOverlapped);
		if(hFile != hCurrentFile )
		{
			pXOR = pXORFix;
			hCurrentFile = hFile;
		}
		DecryptSimple((PBYTE)buf, dwNumberOfBytesToRead);
	}
	else
	{
		bRet = ReadFile(hFile, buf, nNumberOfBytesToRead, &dwNumberOfBytesToRead, lpOverlapped);
	}
	memcpy(pOrigRFAddress, JMPRF, SIZE); //Rehook API
	VirtualProtect((LPVOID)pOrigRFAddress, SIZE, oldProtectRF, NULL); //Normal setts

	memcpy(lpBuffer, buf, dwNumberOfBytesToRead);
	*lpNumberOfBytesToRead = dwNumberOfBytesToRead;
	free(buf);
	
	return bRet;
}

LPVOID WINAPI MyHeapAllocA(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
	MessageBoxA(NULL, "HeapAlloc Hooked", "Hooked", 0L);
	LPVOID lpvV;
	MyUnHook();
	lpvV = HeapAlloc(hHeap, dwFlags, dwBytes);
	WriteLog(hHeap, dwBytes, HEAPALLOC);
	MyReHook();
	return lpvV; //Return what should be returned
}

BOOL WINAPI MyHeapFreeA(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
	MessageBoxA(NULL, "HeapFree Hooked", "Hooked", 0L);
	BOOL bRes;
	MyUnHook();
	WriteLog(hHeap, 0, HEAPFREE);
	bRes = HeapFree(hHeap, dwFlags, lpMem);
	MyReHook();
	return bRes; //Return what should be returned
}

LPVOID MyMallocA(size_t size)
{
	LPVOID lpV;
	MyUnHook();
	lpV = malloc(size);
	WriteLog(lpV, size, MALLOC);
	MyReHook();
	return lpV; //Return what should be returned
}

void MyFreeA(LPVOID lpV)
{
	MyUnHook();
	WriteLog(lpV, 0, FREE);
	free(lpV);
	MyReHook();
	return; //Return what should be returned
}

//-------------------------------------------
//---------below no significant--------------
//-------------------------------------------

BOOL bInWorking = FALSE;

BOOL WINAPI fn_ReadFile(
	_In_         HANDLE hFile,
	_Out_        LPVOID lpBuffer,
	_In_         DWORD nNumberOfBytesToRead,
	_Out_opt_    LPDWORD lpNumberOfBytesRead,
	_Inout_opt_  LPOVERLAPPED lpOverlapped)
{
	DWORD readedCount = 0;
	DWORD toReadCount = nNumberOfBytesToRead;
	BOOL result = ReadFile(hFile, lpBuffer, toReadCount, &readedCount, NULL);
	if (result && readedCount > 0)
	{
		//---------------------------------------------------------------
		// Open the destination file. 
		HANDLE hDestinationFile = INVALID_HANDLE_VALUE; 

		hDestinationFile = CreateFile(
			"C:\\temporary.tmp", 
			FILE_WRITE_DATA,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if(INVALID_HANDLE_VALUE != hDestinationFile)
		{
			_tprintf(
				TEXT("The destination file, %s, is open. \n"), 
				L"F:\\temporary.tmp");
		}
		else
		{
			_tprintf(
				TEXT("Error opening destination file!\n"), 
				GetLastError()); 
			return 0;
		}
		if(!WriteFile(
			hDestinationFile, 
			lpBuffer, 
			toReadCount,
			&readedCount,
			NULL))
		{ 
			_tprintf(
				TEXT("Error writing ciphertext.\n"), 
				GetLastError());
		}
		if(hDestinationFile)
		{
			CloseHandle(hDestinationFile);
		}
	}
	return 0;
}

BOOL WINAPI ReadFileProc(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{

	BOOL ret = FALSE;
	if (!bInWorking)
	{
		bInWorking = TRUE;
		ret = fn_ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
		bInWorking = FALSE;
	}
	else
	{
		ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	}
	return ret;
}

void MakeJMP(BYTE *pAddress, DWORD dwJumpTo, DWORD dwLen)
{
	DWORD dwOldProtect, dwBkup, dwRelAddr;

	// give the paged memory read/write permissions

	VirtualProtect(pAddress, dwLen, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	// calculate the distance between our address and our target location
	// and subtract the 5bytes, which is the size of the jmp
	// (0xE9 0xAA 0xBB 0xCC 0xDD) = 5 bytes

	dwRelAddr = (DWORD) (dwJumpTo - (DWORD) pAddress) - 5;

	// overwrite the byte at pAddress with the jmp opcode (0xE9)

	*pAddress = 0xE9;

	// overwrite the next 4 bytes (which is the size of a DWORD)
	// with the dwRelAddr

	*((DWORD *)(pAddress + 0x1)) = dwRelAddr;

	// overwrite the remaining bytes with the NOP opcode (0x90)
	// NOP opcode = No OPeration

	for(DWORD x = 0x5; x < dwLen; x++) *(pAddress + x) = 0x90;

	// restore the paged memory permissions saved in dwOldProtect

	VirtualProtect(pAddress, dwLen, dwOldProtect, &dwBkup);

	return;
}

extern "C" BOOL WINAPI tricky()
{

	MakeJMP((LPBYTE)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "ReadFile"), (DWORD)ReadFileProc, 5);
	return TRUE;
}

// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	return buf;
}

void WriteLog(HANDLE handle, size_t size,  METHOD meth)
{
	if(!bMemLeakLog)
		return;
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	char tmp[MAX_PATH] = { 0 };
	switch(meth)
	{
	case HEAPALLOC: 
		sprintf_s(tmp, "[%s]  HeapAlloc  %08x (%d)", buf, handle, size);   
		break;
	case HEAPFREE:
		sprintf_s(tmp, "[%s]  HeapFree  %08x", buf, handle, size);
		break;
	case MALLOC: 
		sprintf_s(tmp, "[%s]  malloc  %08x (%d)", buf, handle, size);   
		break;
	case FREE:
		sprintf_s(tmp, "[%s]  free  %08x", buf, handle, size);
		break;
	}
	char pLogFileName[MAX_PATH] = { 0 };
	sprintf_s(pLogFileName, "%s\\memoryleak.log", pCurPath);
	std::ofstream log(pLogFileName, std::ios_base::app | std::ios_base::out);
	log << tmp; log << "\n";
//	MessageBoxA(NULL, "malloc write log", "Hooked", 0L);

}

void WriteLog(char* pszname, METHOD meth)
{
	if(!bMemLeakLog)
		return;
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

	char tmp[MAX_PATH] = { 0 };
	switch(meth)
	{
	case CREATEFILE:
		sprintf_s(tmp, "[%s]  CreateFile  %s", buf, pszname);
		break;
	case CLOSEHANDLE:
		sprintf_s(tmp, "[%s]  CloseHandle  %s", buf, pszname);
		break;
	case DECRYPT_ERROR:
		sprintf_s(tmp, "[%s]  Decrypt Error  %s", buf, pszname);
		break;
	case DELETE_ERROR:
		sprintf_s(tmp, "[%s]  Delete Error  %s", buf, pszname);
		break;
	}

	char pLogFileName[MAX_PATH] = { 0 };
	sprintf_s(pLogFileName, "%s\\memoryleak.log", pCurPath);
	std::ofstream log(pLogFileName, std::ios_base::app | std::ios_base::out);
	log << tmp; log << "\n";
}