
// ConverterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Converter.h"
#include "ConverterDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern bool MyConvertFile(
	LPTSTR pszSourceFile, 
	LPTSTR pszDestinationFile,
	KIND_OF_FILE kof);

BOOL bShow;
BOOL bEVA;
BOOL bEVS;
BOOL bDAT;
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
unsigned __stdcall Converthread( void* pArguments );
//----end-----------------------------------------------


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

#define WM_UPDATE_DATA (WM_USER + 0x101)

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CConverterDlg dialog




CConverterDlg::CConverterDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CConverterDlg::IDD, pParent)
	, m_strResult(_T(""))
	, m_bEva(FALSE)
	, m_bEvs(FALSE)
	, m_bDat(TRUE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CConverterDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SHOWRESULT, m_strResult);
	DDX_Control(pDX, IDC_SHOWRESULT, m_ctrlShow);
	DDX_Check(pDX, IDC_EVA, m_bEva);
	DDX_Check(pDX, IDC_EVS, m_bEvs);
	DDX_Check(pDX, IDC_DAT, m_bDat);
}

BEGIN_MESSAGE_MAP(CConverterDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONVERT, &CConverterDlg::OnBnClickedConvert)
	ON_MESSAGE(WM_UPDATE_DATA, &CConverterDlg::OnUpdateData)
	ON_BN_CLICKED(IDC_CLEAR, &CConverterDlg::OnBnClickedClear)
END_MESSAGE_MAP()


// CConverterDlg message handlers

BOOL CConverterDlg::OnInitDialog()
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
	m_strResult = "Ready...\r\n";
	strShow = m_strResult;

	UpdateData(FALSE);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CConverterDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CConverterDlg::OnPaint()
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
HCURSOR CConverterDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}




void CConverterDlg::OnBnClickedConvert()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	unsigned threadID;

	nSucNum = 0;	//Success number
	nErrNum = 0;	//error number

	UpdateData(TRUE);

	bShow = TRUE;

	bEVA = m_bEva;
	bEVS = m_bEvs;
	bDAT = m_bDat;
	
	if(!(bEVA || bEVS || bDAT))
	{
		AfxMessageBox("Check kinds of file please");
		return;
	}


	hWnd = m_hWnd;

	if(hThread)
	{
		WaitForSingleObject( hThread, INFINITE );	
		CloseHandle( hThread );
		hThread = NULL;
	}
	m_strResult.Append("Now Converting..\r\n");
	strShow = m_strResult;
	UpdateData(FALSE);

	//------ for edit control to bottom----------//
	m_ctrlShow.SendMessage(EM_SETSEL, 0, -1); //Select all. 
	m_ctrlShow.SendMessage(EM_SETSEL, -1, -1);//Unselect and stay at the end pos
	m_ctrlShow.SendMessage(EM_SCROLLCARET, 0, 0); //Set scrollcaret to the current 

	// Create the decrypt thread.
	hThread = (HANDLE)_beginthreadex( NULL, 0, &Converthread, NULL, 0, &threadID );
}

LRESULT CConverterDlg::OnUpdateData(WPARAM wParam, LPARAM)
{
	if(!m_strResult.Compare(strShow))
		return 0;
	m_strResult = strShow;
	UpdateData(FALSE);

	//------ for edit control to bottom----------//
	m_ctrlShow.SendMessage(EM_SETSEL, 0, -1); //Select all. 
	m_ctrlShow.SendMessage(EM_SETSEL, -1, -1);//Unselect and stay at the end pos
	m_ctrlShow.SendMessage(EM_SCROLLCARET, 0, 0); //Set scrollcaret to the current 
	return 0;
}

int ConvertAllFiles(CString refcstrRootDirectory,
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
						int iRC = ConvertAllFiles(strFilePath, strPassword, bConsiderSubdirectories);
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
						strShow.Append("This file name is unicode-written and so didn't be converted.\r\n");
						continue;
					}
					// if not proper file, continue
					CString strTmp = strFilePath.Right(3);

					KIND_OF_FILE kof;
					if( (bEVA && strTmp.Compare("eva") == 0))
						kof = KIND_EVA;
					else if( (bEVS && strTmp.Compare("evs") == 0))
						kof = KIND_EVS;
					else if( (bDAT && strTmp.Compare("dat") == 0))
						kof = KIND_DAT;
					else 
						continue;


					//					return ::GetLastError();
					//---------------------------------------------------------------
					// Call DecryptFile to do the actual decryption.
				/*
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
						MessageBox(NULL, "GetTempFileName failed", "Infor", 0L);
						return NULL;
					}
					CString strDestFilePath = szTempFileName;
					*/
					CString strDestFilePath;
					if(kof == KIND_DAT)
					{
						strDestFilePath = strCurFolder + "\\Result";
						CreateDirectory(strDestFilePath, NULL);
						strDestFilePath += "\\";
						strDestFilePath += FileInformation.cFileName;
						strDestFilePath.Replace(".dat", ".csv");
					}
					else
					{
						strDestFilePath = strCurFolder + "\\Result";
						CreateDirectory(strDestFilePath, NULL);
						strDestFilePath += "\\";
						strDestFilePath += FileInformation.cFileName;
					}

					BOOL bRes = MyConvertFile((LPTSTR)strFilePath.GetBuffer(), (LPTSTR)strDestFilePath.GetBuffer(), kof);
					if(bRes)
					{
						if(bShow)
						{
							strShow.Append(strFilePath);
							strShow.Append("--->Convert successful.\r\n");
						}
						nSucNum++;

						//// Delete file
						//if(::DeleteFile(strFilePath) == FALSE)
						//	return ::GetLastError();
						////Rename file
						//if(::MoveFile(strDestFilePath,strFilePath) == FALSE)
						//	return ::GetLastError();
					}
					else
					{
						strShow.Append(strFilePath);
						strShow.Append("--->Error Convert.\r\n");
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


unsigned __stdcall Converthread( void* pArguments )
{
	ConvertAllFiles(strCurFolder + "\\local", strDevNumber);
	ConvertAllFiles(strCurFolder + "\\maps", strDevNumber);
	ConvertAllFiles(strCurFolder + "\\script", strDevNumber);
	ConvertAllFiles(strCurFolder + "\\ui", strDevNumber);
	ConvertAllFiles(strCurFolder + "\\model\\actor", strDevNumber);
	ConvertAllFiles(strCurFolder + "\\scene\\actor", strDevNumber);

	strShow.Append("Convert Finished. \r\n");
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



void CConverterDlg::OnBnClickedClear()
{
	// TODO: Add your control notification handler code here
	m_strResult.Empty();
	m_strResult = "Ready...\r\n";
	UpdateData(FALSE);
}
