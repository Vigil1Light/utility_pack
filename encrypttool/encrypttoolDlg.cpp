
// encrypttoolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "encrypttool.h"
#include "encrypttoolDlg.h"
#include "afxdialogex.h"
#include "EncDec.h"
#include <iostream>
#include <fstream>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//#define READFILE_METHOD   //XOR method. If not define, CREATREFILE_METHOD

//#define DEVELOP_MODE
//#define STATIC_HDDSN "KU53Rstq~1*$$121@#$!"  //must be equal to the data of encrypttool.exe
#define STATIC_HDDSN "@PangKal_client_test@"  //must be equal to the data of encrypttool.exe
#define UI_TEXTURES_DONOTENCRYPT

BOOL bShow;
BYTE pXORFix;

//----------------------------------------------------
//---- values for thread  and functions --------------
HWND		hWnd = NULL;
CString		refcstrRootDirectory;
CString		strPassword;	
bool		bConsiderSubdirectories;
CString		strCurFolder;
CString		strDevNumber;
CString		strShow;
int			nSucNum;
int			nErrNum;
BOOL		bEncrypted;
HANDLE		hThread;
unsigned __stdcall Decryptthread( void* pArguments );
unsigned __stdcall Encryptthread( void* pArguments );
//----end-----------------------------------------------

extern bool MyEncryptFile(
	LPTSTR szSource, 
	LPTSTR szDestination, 
	LPTSTR szPassword);

extern bool MyDecryptFile(
	LPTSTR szSource, 
	LPTSTR szDestination, 
	LPTSTR szPassword);
extern bool MyEncryptFileXOR(
	LPTSTR szSource, 
	LPTSTR szDestination);

extern bool MyDecryptFileXOR(
	LPTSTR szSource, 
	LPTSTR szDestination); 

extern bool MyCompressFile(
	LPTSTR szSource, 
	LPTSTR szDestination);

extern bool MyDecompressFile(
	LPTSTR szSource, 
	LPTSTR szDestination);
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


// CencrypttoolDlg dialog


CencrypttoolDlg::CencrypttoolDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CencrypttoolDlg::IDD, pParent)
	, m_strDevNumber(_T(""))
	, m_strShow(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CencrypttoolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DEVICENUMBER, m_strDevNumber);
	DDX_Control(pDX, IDC_DEVICENUMBER, m_ctrlDevNumber);
	DDX_Control(pDX, IDC_ENCDEC, m_ctrlEncDec);
	DDX_Text(pDX, IDC_ShowProcess, m_strShow);
	DDX_Control(pDX, IDC_ShowProcess, m_ctrlShow);
}

#define WM_UPDATE_DATA (WM_USER + 0x101)

BEGIN_MESSAGE_MAP(CencrypttoolDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_ENCDEC, &CencrypttoolDlg::OnBnClickedEncdec)
	ON_BN_CLICKED(IDC_CLEAR, &CencrypttoolDlg::OnBnClickedClear)
	ON_MESSAGE(WM_UPDATE_DATA, &CencrypttoolDlg::OnUpdateData)
	ON_BN_CLICKED(IDC_DECRYPT, &CencrypttoolDlg::OnBnClickedDecrypt)
END_MESSAGE_MAP()


// CencrypttoolDlg message handlers

BOOL CencrypttoolDlg::OnInitDialog()
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
	//Get Current folder
	TCHAR pBuf[MAX_PATH] = {0};
	::GetCurrentDirectory(MAX_PATH, (LPSTR)pBuf);
	strCurFolder = pBuf;
#ifdef DEVELOP_MODE
	strCurFolder = "F:\\test";
#endif
	bEncrypted = IsEncrypted();
	if(bEncrypted)
	{
		m_ctrlEncDec.SetWindowText(_T("Decrypt"));
	}
	else
	{
		m_ctrlEncDec.SetWindowText(_T("Encrypt"));
	}

	m_strShow = "Ready...\r\n";
	strShow = m_strShow;
	m_strDevNumber = STATIC_HDDSN;

	UpdateData(FALSE);
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CencrypttoolDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CencrypttoolDlg::OnPaint()
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
HCURSOR CencrypttoolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CencrypttoolDlg::OnBnClickedEncdec()
{
	// TODO: Add your control notification handler code here
	unsigned threadID;
	
	nSucNum = 0;	//Success number
	nErrNum = 0;	//error number
	
	UpdateData(TRUE);

	bShow = IsShowenable();

	hWnd = m_hWnd;
	CString strTemp = STATIC_HDDSN;
	pXORFix = 0xab;
	for(DWORD i = 0; i < strlen(strTemp); i++)
	{
		pXORFix ^= strTemp.GetAt(i);
	}
	strDevNumber = STATIC_HDDSN;  //password

	if(hThread)
	{
		WaitForSingleObject( hThread, INFINITE );	
		CloseHandle( hThread );
		hThread = NULL;
	}
	if(bEncrypted)
	{
		int nRes;
		if((nRes = AfxMessageBox("You already encrypted all the files. Do you really want to encrypt once more?", MB_YESNO)) == 7)
			return;
	}
	bEncrypted = FALSE;
	if(bEncrypted)
	{
		m_strShow.Append("Now Decrypting..\r\n");
		strShow = m_strShow;
		UpdateData(FALSE);

		//------ for edit control to bottom----------//
		m_ctrlShow.SendMessage(EM_SETSEL, 0, -1); //Select all. 
		m_ctrlShow.SendMessage(EM_SETSEL, -1, -1);//Unselect and stay at the end pos
		m_ctrlShow.SendMessage(EM_SCROLLCARET, 0, 0); //Set scrollcaret to the current 

		// Create the decrypt thread.
		hThread = (HANDLE)_beginthreadex( NULL, 0, &Decryptthread, NULL, 0, &threadID );
	}
	else
	{
		m_strShow.Append("Now Encrypting..\r\n");
		strShow = m_strShow;
		UpdateData(FALSE);

		//------ for edit control to bottom----------//
		m_ctrlShow.SendMessage(EM_SETSEL, 0, -1); //Select all. 
		m_ctrlShow.SendMessage(EM_SETSEL, -1, -1);//Unselect and stay at the end pos
		m_ctrlShow.SendMessage(EM_SCROLLCARET, 0, 0); //Set scrollcaret to the current 

		// Create the encrypt thread.
		hThread = (HANDLE)_beginthreadex( NULL, 0, &Encryptthread, NULL, 0, &threadID );
	}
}

//------------------------------------
//--------check encrpted -------------
BOOL CencrypttoolDlg::IsEncrypted()
{
	HANDLE          hFile;                       // Handle to directory
	CString     strFilePath;                 // Filepath
	WIN32_FIND_DATA FileInformation;             // File information
	BOOL bFoundFile = FALSE;

	TCHAR buf[MAX_PATH];
	memset(buf, 0, MAX_PATH);

	strFilePath = strCurFolder + "\\encrypttool.ini";
	hFile = ::FindFirstFile(strFilePath, &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		ifstream infile;
		infile.open(strFilePath,ios::binary|ios::in);
		infile.read((char*)buf, 256);
		infile.close();

		CString strContents = buf;
		int nIndex;
		if(((nIndex = strContents.Find("Encrypted")) != -1) || ((nIndex = strContents.Find("encrypted")) != -1))
		{
			int nLen = strlen("Encrypted");
			int nEnd = strContents.Find("\r\n", nIndex + nLen);
			if(nEnd == -1)
				nEnd = strContents.GetLength();
			CString strTemp = strContents.Mid(nIndex + nLen, nEnd - nIndex - nLen);
			strTemp.Remove(' ');
			if(!strTemp.Compare("=true") || !strTemp.Compare("=TRUE"))
				bFoundFile = TRUE;
		}
	}
	::FindClose(hFile);
	return bFoundFile;
}

//---------------------------------------------------------------------------
//--------check if showing files when  encryptint or decrypting -------------
BOOL CencrypttoolDlg::IsShowenable()
{
	HANDLE          hFile;                       // Handle to directory
	CString     strFilePath;                 // Filepath
	WIN32_FIND_DATA FileInformation;             // File information
	BOOL bShowFile = FALSE;

	TCHAR buf[MAX_PATH];
	memset(buf, 0, MAX_PATH);

	strFilePath = strCurFolder + "\\encrypttool.ini";
	hFile = ::FindFirstFile(strFilePath, &FileInformation);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		ifstream infile;
		infile.open(strFilePath,ios::binary|ios::in);
		infile.read((char*)buf, 256);
		infile.close();

		CString strContents = buf;
		int nIndex;
		if(((nIndex = strContents.Find("Show")) != -1) || ((nIndex = strContents.Find("show")) != -1))
		{
			int nLen = strlen("Show");
			int nEnd = strContents.Find("\r\n", nIndex + nLen);
			if(nEnd == -1)
				nEnd = strContents.GetLength();
			CString strTemp = strContents.Mid(nIndex + nLen, nEnd - nIndex - nLen);
			strTemp.Remove(' ');
			if(!strTemp.Compare("=true") || !strTemp.Compare("=TRUE"))
				bShowFile = TRUE;
		}
	}
	::FindClose(hFile);
	return bShowFile;
}

int EncryptAllFiles(CString refcstrRootDirectory,
	CString		strPassword,	
	bool        bConsiderSubdirectories = TRUE)
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
						int iRC = EncryptAllFiles(strFilePath, strPassword, bConsiderSubdirectories);
						if(iRC)
							return iRC;
					}
				}
				else
				{
					// Set file attributes

					if(::SetFileAttributes(strFilePath,
						FILE_ATTRIBUTE_NORMAL) == FALSE)
					{
						strShow.Append(strFilePath);
						strShow.Append("---->");
						strShow.Append("This file name is unicode-written and so can't encrypt. You must delete this file to play successfully.\r\n");
						continue;
					}
					//return ::GetLastError();
#ifdef UI_TEXTURES_DONOTENCRYPT
					if(strstr(strFilePath, "ui\\textures\\") != NULL || strstr(strFilePath, "ui\\textures/") != NULL )
						continue;
#endif
					//---------------------------------------------------------------
					// Call EncryptFile to do the actual encryption.int EncryptFile(CString strFile)
					DWORD dwRetVal = 0;
					UINT uRetVal   = 0;
					TCHAR szTempFileName[MAX_PATH];  
					TCHAR lpTempPathBuffer[MAX_PATH];

					//  Gets the temp path env string (no guarantee it's a valid path).
					dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
						lpTempPathBuffer); // buffer for path 
					if (dwRetVal > MAX_PATH || (dwRetVal == 0))
					{
						//MessageBox(NULL, "GetTempPath failed", "Infor", 0L);
						return NULL;
					}

					//  Generates a temporary file name. 
					uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
						TEXT("clienttmp"),     // temp file name prefix 
						0,                // create unique name 
						szTempFileName);  // buffer for name 
					if (uRetVal == 0)
					{
						//MessageBox(NULL, "GetTempFileName failed", "Infor", 0L);
						return NULL;
					}

					CString strDestFilePath = szTempFileName;
#ifdef READFILE_METHOD
					BOOL bRes = MyEncryptFileXOR((LPTSTR)strFilePath.GetBuffer(), (LPTSTR)strDestFilePath.GetBuffer());
#else
					//BOOL bRes = MyEncryptFile((LPTSTR)strFilePath.GetBuffer(), (LPTSTR)strDestFilePath.GetBuffer(), strPassword.GetBuffer());

					BOOL bRes = MyCompressFile((LPTSTR)strFilePath.GetBuffer(), (LPTSTR)strDestFilePath.GetBuffer());
#endif
					if(bRes)
					{
						if(bShow)
						{
							strShow.Append(strFilePath);
							strShow.Append("--->Encrypt successful.\r\n");
						}
						nSucNum++;

						// Delete file
						if(::DeleteFile(strFilePath) == FALSE)
							return ::GetLastError();
						//Rename file
						if(::MoveFile(strDestFilePath,strFilePath) == FALSE)
							return ::GetLastError();
					}
					else
					{
						strShow.Append(strFilePath);
						strShow.Append("--->Error Encrypt.\r\n");
						nErrNum++;

						// Delete file
						if(::DeleteFile(strDestFilePath) == FALSE)
							return ::GetLastError();
					}
					PostMessage(hWnd, WM_UPDATE_DATA, 0U, 0L);
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}
	return 0;
}

int DecryptAllFiles(CString refcstrRootDirectory,
	CString		strPassword,	
	bool        bConsiderSubdirectories = TRUE)
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
						int iRC = DecryptAllFiles(strFilePath, strPassword, bConsiderSubdirectories);
						if(iRC)
							return iRC;
					}
				}
				else
				{
					// Set file attributes
					if(::SetFileAttributes(strFilePath,
						FILE_ATTRIBUTE_NORMAL) == FALSE)
					{
						strShow.Append(strFilePath);
						strShow.Append("---->");
						strShow.Append("This file name is unicode-written and so didn't be encrypted. So don't need decrypt.\r\n");
						continue;
					}
//					return ::GetLastError();
#ifdef UI_TEXTURES_DONOTENCRYPT
					if(strstr(strFilePath, "ui\\textures\\") != NULL || strstr(strFilePath, "ui\\textures/") != NULL )
						continue;
#endif

					//---------------------------------------------------------------
					// Call DecryptFile to do the actual decryption.
					DWORD dwRetVal = 0;
					UINT uRetVal   = 0;
					TCHAR szTempFileName[MAX_PATH];  
					TCHAR lpTempPathBuffer[MAX_PATH];

					//  Gets the temp path env string (no guarantee it's a valid path).
					dwRetVal = GetTempPath(MAX_PATH,          // length of the buffer
						lpTempPathBuffer); // buffer for path 
					if (dwRetVal > MAX_PATH || (dwRetVal == 0))
					{
						//MessageBox(NULL, "GetTempPath failed", "Infor", 0L);
						return NULL;
					}

					//  Generates a temporary file name. 
					uRetVal = GetTempFileName(lpTempPathBuffer, // directory for tmp files
						TEXT("clienttmp"),     // temp file name prefix 
						0,                // create unique name 
						szTempFileName);  // buffer for name 
					if (uRetVal == 0)
					{
						//MessageBox(NULL, "GetTempFileName failed", "Infor", 0L);
						return NULL;
					}

					CString strDestFilePath = szTempFileName;
//					CString strDestFilePath = strFilePath + "_";
#ifdef READFILE_METHOD
					BOOL bRes = MyDecryptFileXOR((LPTSTR)strFilePath.GetBuffer(), (LPTSTR)strDestFilePath.GetBuffer());
#else
					//BOOL bRes = MyDecryptFile((LPTSTR)strFilePath.GetBuffer(), (LPTSTR)strDestFilePath.GetBuffer(), strPassword.GetBuffer());

					BOOL bRes = MyDecompressFile((LPTSTR)strFilePath.GetBuffer(), (LPTSTR)strDestFilePath.GetBuffer());
#endif
					if(bRes)
					{
						if(bShow)
						{
							strShow.Append(strFilePath);
							strShow.Append("--->Decrypt successful.\r\n");
						}
						nSucNum++;

						// Delete file
						if(::DeleteFile(strFilePath) == FALSE)
							return ::GetLastError();
						//Rename file
						if(::MoveFile(strDestFilePath,strFilePath) == FALSE)
							return ::GetLastError();
					}
					else
					{
						strShow.Append(strFilePath);
						strShow.Append("--->Error Decrypt.\r\n");
						nErrNum++;

						// Delete file
						if(::DeleteFile(strDestFilePath) == FALSE)
							return ::GetLastError();

					}
					PostMessage(hWnd, WM_UPDATE_DATA, 0U, 0L);
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}
	return 0;
}

void CencrypttoolDlg::WriteEncStaus(CString str)
{
	CString     strFilePath;                 // Filepath
	strFilePath = strCurFolder + "\\encrypttool.ini";
	ofstream outfile;
	outfile.open(strFilePath,ios::binary|ios::out);
	outfile.write((const char*)str.GetBuffer(), str.GetLength());
	outfile.close();
}

void CencrypttoolDlg::OnBnClickedClear()
{
	// TODO: Add your control notification handler code here
	m_strShow.Empty();
	strShow.Empty();
	m_strShow.Append("Ready...\r\n");
	strShow = m_strShow;
	UpdateData(FALSE);
}

LRESULT CencrypttoolDlg::OnUpdateData(WPARAM wParam, LPARAM)
{
	if(!m_strShow.Compare(strShow))
		return 0;
	m_strShow = strShow;
	if(bEncrypted)
	{
		WriteEncStaus("Encrypted = TRUE\r\n");
		//m_ctrlEncDec.SetWindowText(_T("Decrypt"));
	}
	else
	{
		WriteEncStaus("Encrypted = FALSE\r\n");
		//m_ctrlEncDec.SetWindowText(_T("Encrypt"));
	}
	UpdateData(FALSE);
	
	//------ for edit control to bottom----------//
	m_ctrlShow.SendMessage(EM_SETSEL, 0, -1); //Select all. 
	m_ctrlShow.SendMessage(EM_SETSEL, -1, -1);//Unselect and stay at the end pos
	m_ctrlShow.SendMessage(EM_SCROLLCARET, 0, 0); //Set scrollcaret to the current 
	return 0;
}

unsigned __stdcall Encryptthread( void* pArguments )
{
	EncryptAllFiles(strCurFolder + "\\local", strDevNumber);
	EncryptAllFiles(strCurFolder + "\\maps", strDevNumber);
	EncryptAllFiles(strCurFolder + "\\script", strDevNumber);
	EncryptAllFiles(strCurFolder + "\\ui", strDevNumber);
	EncryptAllFiles(strCurFolder + "\\model\\actor", strDevNumber);
	EncryptAllFiles(strCurFolder + "\\mode2\\actor", strDevNumber);
	EncryptAllFiles(strCurFolder + "\\scene\\actor", strDevNumber);
	
	strShow.Append("Encrypt Finished. \r\n");
	TCHAR str[MAX_PATH] = {0};
	sprintf_s(str, "Success  %d, Error  %d\r\n", nSucNum, nErrNum);
	strShow.Append(str);
	strShow.Append('\0');

	strShow.Append("Ready..\r\n");
	bEncrypted = TRUE;
	PostMessage(hWnd, WM_UPDATE_DATA, 0U, 0L);
	_endthreadex( 0 );
	return 0;
}

unsigned __stdcall Decryptthread( void* pArguments )
{
	DecryptAllFiles(strCurFolder + "\\local", strDevNumber);
	DecryptAllFiles(strCurFolder + "\\maps", strDevNumber);
	DecryptAllFiles(strCurFolder + "\\script", strDevNumber);
	DecryptAllFiles(strCurFolder + "\\ui", strDevNumber);
	DecryptAllFiles(strCurFolder + "\\model\\actor", strDevNumber);
	DecryptAllFiles(strCurFolder + "\\mode2\\actor", strDevNumber);
	DecryptAllFiles(strCurFolder + "\\scene\\actor", strDevNumber);

	strShow.Append("Decrypt Finished. \r\n");
	TCHAR str[MAX_PATH] = {0};
	sprintf_s(str, "Success  %d, Error  %d\r\n", nSucNum, nErrNum);
	strShow.Append(str);
	strShow.Append('\0');
	
	strShow.Append("Ready..\r\n");
	bEncrypted = FALSE;
	PostMessage(hWnd, WM_UPDATE_DATA, 0U, 0L);
	_endthreadex( 0 );
	return 0;
}


void CencrypttoolDlg::OnBnClickedDecrypt()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	unsigned threadID;

	nSucNum = 0;	//Success number
	nErrNum = 0;	//error number

	UpdateData(TRUE);

	bShow = IsShowenable();

	hWnd = m_hWnd;
	CString strTemp = STATIC_HDDSN;
	pXORFix = 0xab;
	for(DWORD i = 0; i < strlen(strTemp); i++)
	{
		pXORFix ^= strTemp.GetAt(i);
	}
	strDevNumber = STATIC_HDDSN;  //password

	if(hThread)
	{
		WaitForSingleObject( hThread, INFINITE );	
		CloseHandle( hThread );
		hThread = NULL;
	}
	if(!bEncrypted)
	{
		int nRes;
		if((nRes = AfxMessageBox("You already decrypted all the files. Do you really want to decrypt once more?", MB_YESNO)) == 7)
			return;
	}

	bEncrypted = TRUE;
	if(bEncrypted)
	{
		m_strShow.Append("Now Decrypting..\r\n");
		strShow = m_strShow;
		UpdateData(FALSE);

		//------ for edit control to bottom----------//
		m_ctrlShow.SendMessage(EM_SETSEL, 0, -1); //Select all. 
		m_ctrlShow.SendMessage(EM_SETSEL, -1, -1);//Unselect and stay at the end pos
		m_ctrlShow.SendMessage(EM_SCROLLCARET, 0, 0); //Set scrollcaret to the current 

		// Create the decrypt thread.
		hThread = (HANDLE)_beginthreadex( NULL, 0, &Decryptthread, NULL, 0, &threadID );
	}
	else
	{
		m_strShow.Append("Now Encrypting..\r\n");
		strShow = m_strShow;
		UpdateData(FALSE);

		//------ for edit control to bottom----------//
		m_ctrlShow.SendMessage(EM_SETSEL, 0, -1); //Select all. 
		m_ctrlShow.SendMessage(EM_SETSEL, -1, -1);//Unselect and stay at the end pos
		m_ctrlShow.SendMessage(EM_SCROLLCARET, 0, 0); //Set scrollcaret to the current 

		// Create the encrypt thread.
		hThread = (HANDLE)_beginthreadex( NULL, 0, &Encryptthread, NULL, 0, &threadID );
	}
}
