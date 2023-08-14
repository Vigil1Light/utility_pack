
// GetHDDSNDlg.cpp : implementation file
//

#include "stdafx.h"
#include <Windows.h>

#include "GetHDDSN.h"
#include "GetHDDSNDlg.h"
#include <winioctl.h>

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


// CGetHDDSNDlg dialog




CGetHDDSNDlg::CGetHDDSNDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGetHDDSNDlg::IDD, pParent)
	, m_strHDDSN(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGetHDDSNDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_HDDSN, m_strHDDSN);
}

BEGIN_MESSAGE_MAP(CGetHDDSNDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_GET, &CGetHDDSNDlg::OnBnClickedGet)
END_MESSAGE_MAP()


// CGetHDDSNDlg message handlers

BOOL CGetHDDSNDlg::OnInitDialog()
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

void CGetHDDSNDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGetHDDSNDlg::OnPaint()
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
HCURSOR CGetHDDSNDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD GetPhysicalDriveSerialNumber(UINT nDriveNumber, CString& strSerialNumber);


void CGetHDDSNDlg::OnBnClickedGet()
{
	// TODO: Add your control notification handler code here
	UINT nDriveNumber = 0;
	CString strSerialNumber;
	DWORD dwResult = GetPhysicalDriveSerialNumber(nDriveNumber, strSerialNumber);
	CString strReport;
	if (NO_ERROR == dwResult)
	{
		strReport.Format(_T("Drive #%u serial number: '%s'"), nDriveNumber, strSerialNumber);
	}
	else
	{
		strReport.Format(_T("GetPhysicalDriveSerialNumber failed. Error: %u"), dwResult);
	}
	//::MessageBox(NULL, strReport, _T("Test"), MB_OK);
	m_strHDDSN = strSerialNumber;
	m_strHDDSN.Remove(' ');
	UpdateData(FALSE);
}



DWORD GetPhysicalDriveSerialNumber(UINT nDriveNumber, CString& strSerialNumber)
{
	DWORD dwResult = NO_ERROR;
	strSerialNumber.Empty();

	// Format physical drive path (may be '\\.\PhysicalDrive0', '\\.\PhysicalDrive1' and so on).
	CString strDrivePath;
	strDrivePath.Format(_T("\\\\.\\PhysicalDrive%u"), nDriveNumber);

	// call CreateFile to get a handle to physical drive
	HANDLE hDevice = ::CreateFile(strDrivePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);

	if(INVALID_HANDLE_VALUE == hDevice)
		return ::GetLastError();

	// set the input STORAGE_PROPERTY_QUERY data structure
	STORAGE_PROPERTY_QUERY storagePropertyQuery;
	ZeroMemory(&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY));
	storagePropertyQuery.PropertyId = StorageDeviceProperty;
	storagePropertyQuery.QueryType = PropertyStandardQuery;

	// get the necessary output buffer size
	STORAGE_DESCRIPTOR_HEADER storageDescriptorHeader = { 0 };
	DWORD dwBytesReturned = 0;
	if(!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		&storageDescriptorHeader, sizeof(STORAGE_DESCRIPTOR_HEADER),
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		::CloseHandle(hDevice);
		return dwResult;
	}

	// allocate the necessary memory for the output buffer
	const DWORD dwOutBufferSize = storageDescriptorHeader.Size;
	BYTE* pOutBuffer = new BYTE[dwOutBufferSize];
	ZeroMemory(pOutBuffer, dwOutBufferSize);

	// get the storage device descriptor
	if (!::DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
		&storagePropertyQuery, sizeof(STORAGE_PROPERTY_QUERY),
		pOutBuffer, dwOutBufferSize,
		&dwBytesReturned, NULL))
	{
		dwResult = ::GetLastError();
		delete[]pOutBuffer;
		::CloseHandle(hDevice);
		return dwResult;
	}

	// Now, the output buffer points to a STORAGE_DEVICE_DESCRIPTOR structure
	// followed by additional info like vendor ID, product ID, serial number, and so on.
	STORAGE_DEVICE_DESCRIPTOR* pDeviceDescriptor = (STORAGE_DEVICE_DESCRIPTOR*)pOutBuffer;
	const DWORD dwSerialNumberOffset = pDeviceDescriptor->SerialNumberOffset;
	if (dwSerialNumberOffset != 0)
	{
		// finally, get the serial number
		strSerialNumber = CString(pOutBuffer + dwSerialNumberOffset);
	}

	// perform cleanup and return
	delete[]pOutBuffer;
	::CloseHandle(hDevice);
	return dwResult;
}