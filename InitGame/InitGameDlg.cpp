
// InitGameDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InitGame.h"
#include "InitGameDlg.h"
#include "afxdialogex.h"
#include <fstream>
#include <iostream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

DWORD nDeletedNum;
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


// CInitGameDlg dialog




CInitGameDlg::CInitGameDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CInitGameDlg::IDD, pParent)
	, m_strFinished(_T("Ready..."))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CInitGameDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_CLEAN, m_strFinished);
}

BEGIN_MESSAGE_MAP(CInitGameDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CInitGameDlg::OnBnClickedButton1)
END_MESSAGE_MAP()


// CInitGameDlg message handlers

BOOL CInitGameDlg::OnInitDialog()
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

void CInitGameDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CInitGameDlg::OnPaint()
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
HCURSOR CInitGameDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
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
						nDeletedNum++;
						// Delete file
						if(::DeleteFile(strFilePath) == FALSE)
							return ::GetLastError();
						char pLogFileName[MAX_PATH] = { 0 };
						sprintf_s(pLogFileName, "delete.log");
						std::ofstream log(pLogFileName, std::ios_base::app | std::ios_base::out);
						log << strFilePath; log << " deleted"; log << "\n";
					}
				}
			}
		} while(::FindNextFile(hFile, &FileInformation) == TRUE);

		// Close handle
		::FindClose(hFile);
	}
	return 0;
}

void CInitGameDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	nDeletedNum = 0;
	char pCurPath[MAX_PATH] = { 0 };
	::GetCurrentDirectory(MAX_PATH, pCurPath);
	m_strFinished = _T("Started...");
	UpdateData(FALSE);
	DeleteProperFiles(pCurPath);
	char pShow[MAX_PATH] = { 0 };
	sprintf(pShow, "Clean finished.(%d)", nDeletedNum);
	m_strFinished = pShow;
	UpdateData(FALSE);
	//AfxMessageBox(pShow);
}
