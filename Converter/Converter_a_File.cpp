// Encrypting_a_File.cpp : Defines the entry point for the console 
// application.
//
#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#include <conio.h>
#include <algorithm>
#include <math.h>
#include <iostream>
#include <sys/stat.h>

enum DAT_FORMAT
{
	DAT_1, DAT_2, DAT_3, DAT_4, DAT_5, DAT_6, DAT_7
};

extern DAT_FORMAT nDatFormat;

extern void DAT_GetSecondRowString(PBYTE pOccurrence, DWORD dwPatternSize, int* pLen);
extern void DAT_GetFirstRowNumber(PBYTE pOccurrence, DWORD dwPatternSize, int* pLen);
extern void DAT_RemoveFirstString(PBYTE pbBuf, PDWORD pdwLen);
extern DAT_FORMAT DAT_GetFormat(PBYTE pbBuffer, PDWORD pdwLen);
extern void DAT_MakeLR1(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2);
extern void DAT_MakeLR2(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2);
extern void DAT_Replace8(PBYTE pbBuffer, PDWORD pdwLen, BYTE b);
extern void DAT_Replace4(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2, BYTE b3, BYTE b4);
extern void DAT_Delete4(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2, BYTE b3, BYTE b4);
extern void DAT_Replace2(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2);
extern void DAT_Delete(PBYTE pbBuffer, PDWORD pdwLen, PBYTE Pattern);
extern void DAT_Convert1(PBYTE pbBuffer, PDWORD pdwLen);
extern void DAT_Convert2(PBYTE pbBuffer, PDWORD pdwLen);

enum EVA_FORMAT
{
	EVA_1, EVA_2, EVA_3, EVA_4, EVA_5, EVA_6, EVA_7
};

extern EVA_FORMAT nEvaFormat;

extern void EVA_ReplaceX028001withXffffffff(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_Process5Bytes(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ReplaceFileName1(PBYTE pbBuffer, PDWORD pdwLen);
extern EVA_FORMAT EVA_GetFormat(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessX02ffffff(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessX02ffffff(PBYTE pbBuffer, PDWORD pdwLen, BYTE bIn, BYTE bOut);
extern void EVA_ProcessX01ffffff(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessX01ffffff(PBYTE pbBuffer, PDWORD pdwLen, BYTE bIn, BYTE bOut);
extern void EVA_PreProcess(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessAfterRoot(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_DecryptOne(PBYTE pbBuf, PDWORD pdwSize);
extern void EVA_DecryptAll(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessFromBB08E6ToXBB08E6(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessFromPos25ToX0100000000000001(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessFromPos25ToLastX02FFFFFFFF(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_FoundX0138AndInsertX00000000(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_DeleteFromX02FFFFFFFFToX0101(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_Process0x01X40x00X3(PBYTE pbBuffer, PDWORD pdwLen);
extern void EVA_ProcessX000B00(PBYTE pbBuffer, PDWORD pdwLen);

// Link with the Advapi32.lib file.
#pragma comment (lib, "advapi32")

#define KEYLENGTH  0x00800000
#define ENCRYPT_ALGORITHM CALG_RC4 
#define ENCRYPT_BLOCK_SIZE 8 

void ConvertData(PBYTE pbBuffer, PDWORD dwSize, KIND_OF_FILE kof);


//-------------------------------------------------------------------
//  This example uses the function MyHandleError, a simple error
//  handling function, to print an error message to the  
//  standard error (stderr) file and exit the program. 
//  For most applications, replace this function with one 
//  that does more extensive error reporting.

void MyHandleError(LPTSTR psz, int nErrorNumber)
{
	_ftprintf(stderr, TEXT("An error occurred in the program. \n"));
	_ftprintf(stderr, TEXT("%s\n"), psz);
	_ftprintf(stderr, TEXT("Error number %x.\n"), nErrorNumber);
}

//  If exist file, delete. 
inline bool exists_file(const std::string& name) 
{
	struct stat buffer;   
	return (stat (name.c_str(), &buffer) == 0); 
}

//-------------------------------------------------------------------
// Code for the function MyConvertFile called by main.
//-------------------------------------------------------------------
// Parameters passed are:
//  pszSource, the name of the input, a plaintext file.
//  pszDestination, the name of the output, an encrypted file to be 
//   created.
//  pszPassword, either NULL if a password is not to be used or the 
//   string that is the password.
bool MyConvertFile(
	LPTSTR pszSourceFile, 
	LPTSTR pszDestinationFile,
	KIND_OF_FILE kof)
{ 

	//---------------------------------------------------------------
	// Declare and initialize local variables.
	bool fReturn = false;
	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hDestinationFile = INVALID_HANDLE_VALUE; 

	PBYTE pbBuffer = NULL; 
	DWORD dwBlockLen; 
	DWORD dwBufferLen;
	DWORD dwCount; 

	//---------------------------------------------------------------
	// Open the source file. 
	hSourceFile = CreateFile(
		pszSourceFile, 
		FILE_READ_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if(INVALID_HANDLE_VALUE != hSourceFile)
	{
		_tprintf(
			TEXT("The source plaintext file, %s, is open. \n"), 
			pszSourceFile);
	}
	else
	{ 
		MyHandleError(
			TEXT("Error opening source plaintext file!\n"), 
			GetLastError());
		goto Exit_MyEncryptFile;
	} 

	//---------------------------------------------------------------
	// Open the destination file. 
	if(exists_file(pszDestinationFile))
	{
		DeleteFile(pszDestinationFile);
	}

	hDestinationFile = CreateFile(
		pszDestinationFile, 
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
			pszDestinationFile);
	}
	else
	{
		MyHandleError(
			TEXT("Error opening destination file!\n"), 
			GetLastError()); 
		goto Exit_MyEncryptFile;
	}

	LARGE_INTEGER size;
	if (!GetFileSizeEx(hSourceFile, &size))
	{
		MyHandleError(
			TEXT("Error getting file size!\n"), 
			GetLastError()); 
		goto Exit_MyEncryptFile;
	}

	//---------------------------------------------------------------
	// Determine the number of bytes to encrypt at a time. 
	// This must be a multiple of ENCRYPT_BLOCK_SIZE.
	// ENCRYPT_BLOCK_SIZE is set by a #define statement.
	dwBlockLen = 1000; 
	dwBufferLen = size.QuadPart;
	//---------------------------------------------------------------
	// Allocate memory. 
	if(pbBuffer = (BYTE *)malloc((dwBufferLen/dwBlockLen + 1) * dwBlockLen + 20000))
	{
		_tprintf(
			TEXT("Memory has been allocated for the buffer. \n"));
	}
	else
	{ 
		MyHandleError(TEXT("Out of memory. \n"), E_OUTOFMEMORY); 
		goto Exit_MyEncryptFile;
	}

	//---------------------------------------------------------------
	// In a do loop, encrypt the source file, 
	// and write to the source file. 
	bool fEOF = FALSE;
	DWORD dwLen = 0;
	PBYTE pVarTmp = pbBuffer;
	do 
	{ 
		//-----------------------------------------------------------
		// Read up to dwBlockLen bytes from the source file. 
		if(!ReadFile(
			hSourceFile, 
			pVarTmp, 
			dwBlockLen, 
			&dwCount, 
			NULL))
		{
			MyHandleError(
				TEXT("Error reading plaintext!\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}

		if(dwCount < dwBlockLen)
		{
			fEOF = TRUE;
		}

		pVarTmp += dwCount;
		dwLen += dwCount;
		//-----------------------------------------------------------
		// End the do loop when the last block of the source file 
		// has been read, encrypted, and written to the destination 
		// file.
	} while(!fEOF);
	//-----------------------------------------------------------
	// convert.
	ConvertData(pbBuffer, &dwLen, kof);
	
	
	//-----------------------------------------------------------
	// Write the converted data to the destination file. 
	if(!WriteFile(
		hDestinationFile, 
		pbBuffer, 
		dwLen,
		NULL,
		NULL))
	{ 
		MyHandleError(
			TEXT("Error writing ciphertext.\n"), 
			GetLastError());
		goto Exit_MyEncryptFile;
	}

	fReturn = true;

Exit_MyEncryptFile:
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
	return fReturn; 
}

void ConvertEVA(PBYTE pbBuffer, PDWORD pdwLen)
{

	nEvaFormat = EVA_GetFormat(pbBuffer, pdwLen); 
	switch(nEvaFormat)
	{
	case EVA_1://0x64
		EVA_ReplaceX028001withXffffffff(pbBuffer, pdwLen);
		EVA_Process5Bytes(pbBuffer, pdwLen);
		EVA_ReplaceFileName1(pbBuffer, pdwLen);
		break;
	case EVA_2://0xc8 0x02ffffff
	case EVA_5://after root, 0x01, 0x01
		EVA_PreProcess(pbBuffer, pdwLen);
		EVA_ProcessAfterRoot(pbBuffer, pdwLen);
		EVA_ProcessX02ffffff(pbBuffer, pdwLen, 0x20, 0x02);
		EVA_ProcessX02ffffff(pbBuffer, pdwLen, 0x21, 0x02);
		EVA_ProcessX02ffffff(pbBuffer, pdwLen, 0x22, 0x01);
		EVA_ProcessX02ffffff(pbBuffer, pdwLen, 0x25, 0x06);
		EVA_DecryptAll(pbBuffer, pdwLen);
		EVA_ProcessFromPos25ToLastX02FFFFFFFF(pbBuffer, pdwLen);
		EVA_DeleteFromX02FFFFFFFFToX0101(pbBuffer, pdwLen);
		EVA_ProcessX02ffffff(pbBuffer, pdwLen);
		EVA_Process5Bytes(pbBuffer, pdwLen);
		EVA_ProcessX000B00(pbBuffer, pdwLen);
		EVA_Process0x01X40x00X3(pbBuffer, pdwLen);
		break;
	case EVA_3://0xc8 0x01ffffff
		EVA_PreProcess(pbBuffer, pdwLen);
		EVA_ProcessAfterRoot(pbBuffer, pdwLen);
		EVA_ProcessX01ffffff(pbBuffer, pdwLen, 0x2b, 0x08);
		EVA_ProcessX01ffffff(pbBuffer, pdwLen, 0x29, 0x0a);
		EVA_ProcessX01ffffff(pbBuffer, pdwLen, 0x22, 0x01);
		EVA_ProcessX01ffffff(pbBuffer, pdwLen, 0x20, 0x03);
		EVA_ProcessFromPos25ToX0100000000000001(pbBuffer, pdwLen);
		EVA_ProcessX01ffffff(pbBuffer, pdwLen);
		EVA_Process5Bytes(pbBuffer, pdwLen);
		EVA_FoundX0138AndInsertX00000000(pbBuffer, pdwLen);
		EVA_ProcessX000B00(pbBuffer, pdwLen);
		break;
	case EVA_4://0xc8 0xbb08e6
		EVA_PreProcess(pbBuffer, pdwLen);
		EVA_ProcessAfterRoot(pbBuffer, pdwLen);
		EVA_ProcessFromBB08E6ToXBB08E6(pbBuffer, pdwLen);
		break;
	default:
		break;
	}
}

void ConvertDAT(PBYTE pbBuffer, PDWORD pdwLen)
{
	nDatFormat = DAT_GetFormat(pbBuffer, pdwLen); 
	//nDatFormat = DAT_2;
	switch(nDatFormat)
	{
	case DAT_1:
		DAT_Convert1(pbBuffer, pdwLen);
		break;
	case DAT_2:
		DAT_Convert2(pbBuffer, pdwLen);
		break;
	case DAT_3:
	default:
		break;
	}
}

void ConvertData(PBYTE pbBuffer, PDWORD pdwLen, KIND_OF_FILE kof)
{

	switch(kof)
	{
	case KIND_EVA:
		ConvertEVA(pbBuffer, pdwLen);
		break;
	case KIND_EVS:
		break;
	case KIND_DAT:
		ConvertDAT(pbBuffer, pdwLen);
		break;
	}
}
