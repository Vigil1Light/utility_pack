
// testAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "testApp.h"
#include "testAppDlg.h"
#include "afxdialogex.h"
#include <memory>
#include <ios>
#include <fstream>
#include <sys/stat.h>
#include <tlhelp32.h>
#include <stdint.h>
#include <strsafe.h>
#include <algorithm>
#include <psapi.h> // For access to GetModuleFileNameEx

#pragma comment(lib,"Version.lib")
#pragma comment(lib,"Psapi.lib")

using namespace std;

BOOL		bProcessExit = FALSE;
HANDLE		hThread;
unsigned	threadID;
unsigned __stdcall CleanRAMthread( void* pArguments );
extern int CleanMemory(bool standbylist, bool modifiedlist, bool workingset, bool lowpriostandbylist);

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CtestAppDlg dialog




CtestAppDlg::CtestAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CtestAppDlg::IDD, pParent)
	, m_strProcessName(_T("client.exe"))
	, m_nProcessNum(0)
	, m_strFileName(_T("frm_carve_ex.lua"))
	, m_lOffset(0)
	, m_dwlength(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_PROCESSNAME, m_strProcessName);
	DDX_Text(pDX, IDC_NUMBER, m_nProcessNum);
	DDX_Text(pDX, IDC_FILENAME, m_strFileName);
	DDX_Text(pDX, IDC_OFFSET, m_lOffset);
	DDX_Text(pDX, IDC_LENGTH, m_dwlength);
}

BEGIN_MESSAGE_MAP(CtestAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_TEST, &CtestAppDlg::OnBnClickedTest)
	ON_BN_CLICKED(IDC_GETPROCESSNUMBER, &CtestAppDlg::OnBnClickedGetprocessnumber)
	ON_BN_CLICKED(IDC_MAKEFILE, &CtestAppDlg::OnBnClickedMakefile)
	ON_BN_CLICKED(IDC_CLEARMEMORY, &CtestAppDlg::OnBnClickedClearmemory)
	ON_BN_CLICKED(IDC_STOPCLEAN, &CtestAppDlg::OnBnClickedStopclean)
	ON_BN_CLICKED(IDC_BUTTON2, &CtestAppDlg::OnBnClickedButton2)
END_MESSAGE_MAP()


// CtestAppDlg message handlers

BOOL CtestAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CtestAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CtestAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CtestAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL    GetFileNameFromHandle(HANDLE hFile, TCHAR *pszFileName, const unsigned int uiMaxLen);

bool exists_file (const std::string& name) {
	struct stat buffer;   
	return (stat (name.c_str(), &buffer) == 0); 
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

void CtestAppDlg::OnBnClickedTest()
{

	
	//TODO: Add your control notification handler code here
	HANDLE hSourceFile;
	DWORD dwFileSize;
	DWORD dwBytesRead;
	DWORD dwBytesWrite;

	//void* buf1 = malloc(20000);
	//memset(buf1, 0 , 20000);
	//free(buf1);
	//return;

	hSourceFile = CreateFile(
		_T("F:\\test\\local\\local_and_language.ini"), 
		FILE_READ_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(hSourceFile == INVALID_HANDLE_VALUE)
	{
		MessageBox("CreateFile Error");
		return;
	}
	else{
		MessageBox("Successfully Created");
	}
	
	dwFileSize = GetFileSize(hSourceFile, NULL);
	
	TCHAR pszFileName[MAX_PATH] = {0};
	GetFileNameFromHandle(hSourceFile, pszFileName, MAX_PATH);
	
	TCHAR pChar[100] = { 0 };
	void* buf = malloc(dwFileSize);
	memset(buf, 0 , dwFileSize);
	sprintf_s(pChar, "read number  %d", dwFileSize);
	MessageBox(pChar, "Infor", 0);
	BOOL result = ReadFile(hSourceFile, buf, dwFileSize, &dwBytesRead, NULL);

	CloseHandle(hSourceFile);

	sprintf(pChar, "result, read number  %d, %d", result, dwBytesRead);
	MessageBox(pChar, "Infor", 0);
	if (result && dwBytesRead > 0)
	{

		//---------------------------------------------------------------
		// Open the destination file. 
		HANDLE hDestinationFile = INVALID_HANDLE_VALUE; 
		hDestinationFile = CreateFile(
			"F:\\battle.lua_", 
			FILE_WRITE_DATA,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if(INVALID_HANDLE_VALUE == hDestinationFile)
		{
			MessageBox("Error opening destination file!", "Error", 0L);
			return ;
		}
		MessageBox("Create file!", "Success", 0L);
		if(!WriteFile(
			hDestinationFile, 
			buf, 
			dwBytesRead,
			&dwBytesWrite,
			NULL))
		{ 
			MessageBox("Error writeFile.", "Error", 0L);
		}
		else
		{
			sprintf(pChar, "Write number  %d", dwBytesWrite);
			MessageBox(pChar, "Infor", 0);
		}
		if(hDestinationFile)
		{
			CloseHandle(hDestinationFile);
		}
	}
	
	free(buf);
	return ;
}

BOOL GetFileNameFromHandle(HANDLE hFile, TCHAR *pszFileName, const unsigned int uiMaxLen)
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

void ProcessIdToName(DWORD processId, char* pszName)
{
	HANDLE handle = OpenProcess(
		PROCESS_QUERY_LIMITED_INFORMATION,
		FALSE,
		processId /* This is the PID, you can find one from windows task manager */
		);
	if (handle)
	{
		DWORD buffSize = 1024;
		CHAR buffer[1024];
		if (QueryFullProcessImageNameA(handle, 0, buffer, &buffSize))
		{
			memcpy(pszName, buffer, buffSize);
		}
		else
		{
			printf("Error GetModuleBaseNameA : %lu", GetLastError());
		}
		CloseHandle(handle);
	}
	else
	{
		printf("Error OpenProcess : %lu", GetLastError());
	}
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
		char p1[MAX_PATH] = { 0 };
		sprintf_s(p1, "%s", ProcessStruct.szExeFile);
		if(_stricmp("Auto3.exe", p1) == 0)
			int y = 1;

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

void MySleep(DWORD dwTime, PBYTE pMsg)
{
	if(pMsg != NULL)
	{
		char pmsg[MAX_PATH] = { 0 };
		for(size_t i = 0; i < strlen((const char*)pMsg); i++)
			*(pmsg + i) = *(pMsg + i) ^ 0x82;
		MessageBox(NULL, pmsg, "Infor", 0L);
	}
	//Sleep(dwTime);
}

void crashThisGame()
{
	//*((unsigned int*)0) = 0xDEAD;
}

int GetProcessNumber(CString strProcessName)
{
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

	

	if( CheckProcessRunning(data1, 0))//TSAuto 2.0 Final
	{
		MySleep(100000, data1);  //every 100s
		crashThisGame();
	}
	else if( CheckProcessRunning(data2, 0) )//AutoSimple
	{
		MySleep(100000, data2);  //every 100s
		crashThisGame();
	}
	else if( CheckProcessRunning(data4, 0) )//TTDAuto
	{
		MySleep(150000, data4);  //every 150s
		crashThisGame();
	}
	//else if( CheckProcessRunning(data5, 0) )//TSAuto 2.0
	//{
	//	MySleep(150000, data5);  //every 150s
	//	crashThisGame();
	//}
	else if( CheckProcessRunning(data6, 0) )//TSAutoDK-NC
	{
		MySleep(150000, data6);  //every 150s
		crashThisGame();
	}
	else if( CheckProcessRunning(data7, 0) )//TSAutoHH1.1
	{
		MySleep(150000, data7);  //every 150s
		crashThisGame();
	}
	else if( CheckProcessRunning(data8, 0) )//AutoParty
	{
		MySleep(150000, data8);  //every 150s
		crashThisGame();
	}
	else if( CheckProcessRunning(data9, 1) )//"Thục Sơn Tam Giới"
	{
		//MySleep(150000, data9);  //every 150s
	}
	else if( CheckProcessRunning(data3, 0) )//auto2
	{
		MySleep(200000, data3);  //every 200s
		crashThisGame();
	}

	return 1;
}



void CtestAppDlg::OnBnClickedGetprocessnumber()
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_strProcessName.IsEmpty())
	{
		AfxMessageBox("Input Process Name");
		return;
	}
	m_nProcessNum =  GetProcessNumber(m_strProcessName);
	UpdateData(FALSE);
}


void CtestAppDlg::OnBnClickedMakefile()
{
	// TODO: Add your control notification handler code here
	char pszSourceFile[MAX_PATH];
	char pszDestinationFile[MAX_PATH];

	PBYTE pbBuffer = NULL; 
	DWORD dwBlockLen; 
	DWORD dwBufferLen; 
	DWORD dwCount; 
	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hDestinationFile = INVALID_HANDLE_VALUE; 

	UpdateData(TRUE);
	sprintf_s(pszSourceFile, "F:\\diep.lv\\new1\\game\\model.evp");
	sprintf_s(pszDestinationFile, m_strFileName);
	
	dwBlockLen = m_dwlength;
	dwBufferLen = m_dwlength;

	// Open the destination file. 

	// Open the source file. 
	hSourceFile = CreateFile(
		pszSourceFile, 
		FILE_READ_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(INVALID_HANDLE_VALUE == hSourceFile)
		return;

	//---------------------------------------------------------------
	// Open the destination file. 
	hDestinationFile = CreateFile(
		pszDestinationFile, 
		FILE_WRITE_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(INVALID_HANDLE_VALUE == hDestinationFile)
		return;
	
	//---------------------------------------------------------------
	// Allocate memory. 
	if(!(pbBuffer = (BYTE *)malloc(dwBufferLen)))
		return;

	// Try to move hFile file pointer some distance  
	DWORD dwPtr = SetFilePointer( hSourceFile, 
		m_lOffset, 
		NULL, 
		FILE_BEGIN ); 

	if (dwPtr == INVALID_SET_FILE_POINTER) // Test for failure
		return;
//---------------------------------------------------------------
	// Read Data. 
	if(!ReadFile(
		hSourceFile, 
		pbBuffer, 
		dwBlockLen, 
		&dwCount, 
		NULL))
	return;
	
	dwPtr = SetFilePointer( hDestinationFile, 
		0L, 
		NULL, 
		FILE_END ); 

	if(!WriteFile(
		hDestinationFile, 
		pbBuffer, 
		dwCount,
		&dwCount,
		NULL))
	return;
	//---------------------------------------------------------------
	// Close files.
	if(hSourceFile)
	{
		CloseHandle(hSourceFile);
	}

	if(hDestinationFile)
	{
		CloseHandle(hDestinationFile);
	}

	//---------------------------------------------------------------
	// Free memory. 
	if(pbBuffer) 
	{
		free(pbBuffer); 
	}
	return;
}

unsigned __stdcall CleanRAMthread( void* pArguments )
{
	while( !bProcessExit )
	{
		Sleep(10000);  //every 10s
		CleanMemory(FALSE, FALSE, TRUE, FALSE);
	}
	_endthreadex( 0 );
	return 0;
}

void CtestAppDlg::OnBnClickedClearmemory()
{
	// Create thread for clean RAM. 
	hThread = (HANDLE)_beginthreadex( NULL, 0, &CleanRAMthread, NULL, 0, &threadID );
	if(hThread)
	{
		AfxMessageBox("Clean RAM Started every 10s");
	}	
	return;
}



void CtestAppDlg::OnBnClickedStopclean()
{
	// TODO: Add your control notification handler code here
	if(hThread)
	{
		bProcessExit = TRUE;
		WaitForSingleObject( hThread, INFINITE );	
		CloseHandle( hThread );
		hThread = NULL;
	}
	AfxMessageBox("Clean RAM Stopped");
}


void CtestAppDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	std::string str = currentDateTime();
	return;
}
