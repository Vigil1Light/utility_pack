
// GetHDDSNDlg.cpp : implementation file
//

#include "stdafx.h"
#include <Windows.h>
#include <winioctl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

CString GetSN()
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
	strSerialNumber.Remove(' ');
	return strSerialNumber;
}

