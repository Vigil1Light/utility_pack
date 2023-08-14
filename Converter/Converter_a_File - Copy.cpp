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

// Link with the Advapi32.lib file.
#pragma comment (lib, "advapi32")

#define KEYLENGTH  0x00800000
#define ENCRYPT_ALGORITHM CALG_RC4 
#define ENCRYPT_BLOCK_SIZE 8 

void ConvertData(PBYTE pbBuffer, LPDWORD dwSize, KIND_OF_FILE kof);


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
	if(pbBuffer = (BYTE *)malloc((dwBufferLen/dwBlockLen + 1) * dwBlockLen + 1000))
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

void EVA_Delete8Bytes(PBYTE pbBuffer, LPDWORD pdwLen)
{
	BOOL fFound;

	BYTE Pattern[] = { 0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	DWORD dwPatternSize = sizeof(Pattern);


	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pOccurrence + dwPatternSize, dwBackUpSize);
			*pdwLen -= dwPatternSize;
			memcpy(pOccurrence, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence;
			pBufLast -= dwPatternSize;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void EVA_ReplaceX028001withXffffffff(PBYTE pbBuffer, LPDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x02, 0x80, 0x01 };
	DWORD dwPatternSize = sizeof(Pattern);


	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);

		if( fFound )
		{
			BYTE pTmp = *(pOccurrence + dwPatternSize);
			if( pTmp > 0x0f )
			{
				pbBuf = pOccurrence + dwPatternSize;
			}
			else
			{
				DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - 2;
				PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
				memcpy(pbTmp, pOccurrence + 2, dwBackUpSize);
				*pdwLen += 2;
				for(int j = 0; j < 4; j++)
					*(pOccurrence + j) = 0xff;
				memcpy(pOccurrence + 4, pbTmp, dwBackUpSize);
				free(pbTmp);

				pbBuf = pOccurrence + 4;
				pBufLast += 2;
			}

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void EVA_5Bytes(PBYTE pbBuffer, LPDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	DWORD dwPatternSize = sizeof(Pattern);


	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pOccurrence + dwPatternSize, dwBackUpSize);
			*pdwLen -= 5;
			memcpy(pOccurrence + dwPatternSize - 5, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + dwPatternSize - 5;
			pBufLast -= 5;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
};

void EVA_ReplaceFileName1(PBYTE pbBuffer, LPDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = {
		0x08, 0x00, 0x00, 0x00, 0x42, 0x5F, 0x72, 0x69,
		0x64, 0x65, 0x30, 0x31, 0x0C 
	};
	DWORD dwPatternSize = sizeof(Pattern);


	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize + 3;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pOccurrence + dwPatternSize - 1, dwBackUpSize);
			*pdwLen -= 2;
			*pOccurrence = 0x06;
			memcpy(pOccurrence + dwPatternSize - 3, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + dwPatternSize - 1;
			pBufLast -= 2;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);

}

void ConvertEVA(PBYTE pbBuffer, LPDWORD pdwLen)
{

	//EVA_Delete8Bytes(pbBuffer, pdwLen);
	EVA_ReplaceX028001withXffffffff(pbBuffer, pdwLen);
	EVA_5Bytes(pbBuffer, pdwLen);
	EVA_ReplaceFileName1(pbBuffer, pdwLen);

}

void DAT_GetSecondRowString(PBYTE pOccurrence, DWORD dwPatternSize, int* pLen)
{
	PBYTE pbTmp = pOccurrence + dwPatternSize;

	DWORD dwNumber;
	DWORD dwTmp;
	dwTmp = *(pbTmp + 5);
	dwTmp &= 0xff;
	dwTmp <<= 24;
	dwNumber = dwTmp;

	dwTmp = *(pbTmp + 4);
	dwTmp &= 0xff;
	dwTmp <<= 16;
	dwNumber |= dwTmp;

	dwTmp = *(pbTmp + 3);
	dwTmp &= 0xff;
	dwTmp <<= 8;
	dwNumber |= dwTmp;

	dwNumber |= *(pbTmp + 2);

	*pLen = dwNumber;
}

void DAT_GetFirstRowNumber(PBYTE pOccurrence, DWORD dwPatternSize, int* pLen)
{
	*pOccurrence = 0x0d;
	*(pOccurrence + 1) = 0x0a;

	PBYTE pbTmp = pOccurrence + dwPatternSize;
	DWORD dwNumber = *(pbTmp + 1);
	dwNumber &= 0xff;
	dwNumber <<= 8;
	dwNumber |= *pbTmp;
	char pStr[MAX_PATH] = { 0 };
	itoa(dwNumber, pStr, 10);
	char pStrNum[MAX_PATH] = { 0 };
	*pLen = strlen(pStr);
	memcpy(pOccurrence + 2, pStr, *pLen);
	*(pOccurrence + 2 + *pLen) = ',';
	*pLen += 1;
}

void DAT_RemoveFirst3Bytes(PBYTE pbBuf, int nSize, LPDWORD pdwLen)
{
	DWORD dwBackUpSize = *pdwLen - nSize;
	PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp, pbBuf + nSize, dwBackUpSize);
	*pdwLen -= nSize;
	memcpy(pbBuf, pbTmp, dwBackUpSize);
	free(pbTmp);
}


void DAT_MakeLR1(PBYTE pbBuffer, LPDWORD pdwLen, BYTE b1, BYTE b2)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x70, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x70, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	Pattern[46] = b1;
	Pattern[47] = b2;

	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			if(pOccurrence + dwPatternSize <  pBufLast)
			{
				int nLen2 = 0;
				DAT_GetSecondRowString(pOccurrence, dwPatternSize, &nLen2);

				DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize - 6;
				PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
				memcpy(pbTmp, pOccurrence + dwPatternSize + 6, dwBackUpSize);
				int nLen1 = 0;
				DAT_GetFirstRowNumber(pOccurrence, dwPatternSize, &nLen1);
				*pdwLen -= dwPatternSize;
				*pdwLen -= 6;
				*pdwLen += 2;
				*pdwLen += nLen1;
				memcpy(pOccurrence + 2 + nLen1, pbTmp, dwBackUpSize);
				free(pbTmp);

				pbBuf = pOccurrence + 2 + nLen1;
				pBufLast -= dwPatternSize;
				pBufLast -= 6;
				pBufLast += 2;
				pBufLast += nLen1;


				//---------continue
				dwBackUpSize -= 2;
				pbTmp = (PBYTE)malloc(dwBackUpSize);
				memcpy(pbTmp, pbBuf + nLen2 + 2, dwBackUpSize);

				*pdwLen -= 2;
				memcpy(pbBuf + nLen2, pbTmp, dwBackUpSize);

				free(pbTmp);

				pbBuf += nLen2;
				pBufLast -= 2;

				if( pbBuf > pBufLast)
					break;
			}
			else
			{
				*pOccurrence = 0x0d;
				*(pOccurrence + 1) = 0x0a;
				*pdwLen -= dwPatternSize;
				*pdwLen += 2;
				break;
			}

		}
	} while (fFound);
}

void DAT_MakeLR2(PBYTE pbBuffer, LPDWORD pdwLen, BYTE b1, BYTE b2)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x70, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	Pattern[22] = b1;
	Pattern[23] = b2;

	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			if(pOccurrence + dwPatternSize <  pBufLast)
			{
				int nLen2 = 0;
				DAT_GetSecondRowString(pOccurrence, dwPatternSize, &nLen2);

				DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize - 6;
				PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
				memcpy(pbTmp, pOccurrence + dwPatternSize + 6, dwBackUpSize);
				int nLen1 = 0;
				DAT_GetFirstRowNumber(pOccurrence, dwPatternSize, &nLen1);
				*pdwLen -= dwPatternSize;
				*pdwLen -= 6;
				*pdwLen += 2;
				*pdwLen += nLen1;
				memcpy(pOccurrence + 2 + nLen1, pbTmp, dwBackUpSize);
				free(pbTmp);

				pbBuf = pOccurrence + 2 + nLen1;
				pBufLast -= dwPatternSize;
				pBufLast -= 6;
				pBufLast += 2;
				pBufLast += nLen1;


				//---------continue
				dwBackUpSize -= 2;
				pbTmp = (PBYTE)malloc(dwBackUpSize);
				memcpy(pbTmp, pbBuf + nLen2 + 2, dwBackUpSize);

				*pdwLen -= 2;
				*pdwLen += 1;
				*(pbBuf + nLen2) = ',';
				memcpy(pbBuf + nLen2 + 1, pbTmp, dwBackUpSize);
				free(pbTmp);

				pbBuf += nLen2;
				pbBuf += nLen1;
				pBufLast -= 2;
				pBufLast += 1;

				if( pbBuf > pBufLast)
					break;
			}
			else
			{
				*pOccurrence = 0x0d;
				*(pOccurrence + 1) = 0x0a;
				*pdwLen -= dwPatternSize;
				*pdwLen += 2;
				break;
			}

		}
	} while (fFound);
}

void DAT_Replace8(PBYTE pbBuffer, LPDWORD pdwLen, BYTE b)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00 };
	Pattern[0] = b;
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pOccurrence + dwPatternSize, dwBackUpSize);
			*pOccurrence = 0x2c;
			*(pOccurrence + 1) = 0x2c;
			*pdwLen -= dwPatternSize;
			*pdwLen += 2;
			memcpy(pOccurrence + 2, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + 2;
			pBufLast -= dwPatternSize;
			pBufLast += 2;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void DAT_Replace4(PBYTE pbBuffer, LPDWORD pdwLen, BYTE b1, BYTE b2, BYTE b3, BYTE b4)
{
	BOOL fFound;
	BYTE Pattern[] = { 0xff, 0xff, 0xff, 0xff };
	Pattern[0] = b1;
	Pattern[1] = b2;
	Pattern[2] = b3;
	Pattern[3] = b4;
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pOccurrence + dwPatternSize, dwBackUpSize);
			*pOccurrence = 0x2c;
			*pdwLen -= dwPatternSize;
			*pdwLen += 1;
			memcpy(pOccurrence + 1, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + 1;
			pBufLast -= dwPatternSize;
			pBufLast += 1;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void DAT_Replace2(PBYTE pbBuffer, LPDWORD pdwLen, BYTE b1, BYTE b2)
{
	BOOL fFound;
	BYTE Pattern[] = { 0xff, 0xff };
	Pattern[0] = b1;
	Pattern[1] = b2;
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pOccurrence + dwPatternSize, dwBackUpSize);
			*pOccurrence = 0x2c;
			*pdwLen -= dwPatternSize;
			*pdwLen += 1;
			memcpy(pOccurrence + 1, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + 1;
			pBufLast -= dwPatternSize;
			pBufLast += 1;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}



void DAT_Delete(PBYTE pbBuffer, LPDWORD pdwLen, PBYTE Pattern)
{
	BOOL fFound;
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwBackUpSize = *pdwLen - (pOccurrence - pbBuffer) - dwPatternSize;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pOccurrence + dwPatternSize, dwBackUpSize);
			*pdwLen -= dwPatternSize;
			memcpy(pOccurrence, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence;
			pBufLast -= dwPatternSize;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void ConvertDAT1(PBYTE pbBuffer, LPDWORD pdwLen)
{
	BOOL bEOB = FALSE;
	PBYTE pbWorkBuffer = (PBYTE)malloc(*pdwLen);
	int nByte = 2;
	
	
	do
	{
				
	}while(bEOB);
	free(pbWorkBuffer);
}

void ConvertDAT(PBYTE pbBuffer, LPDWORD pdwLen)
{
	//----------------Remove first 0x00, 0x00, 0x12
	DAT_RemoveFirst3Bytes(pbBuffer, 3, pdwLen);
	
	//----------------------------------- Remove 0x0d0x0a and insert Number ---------------------
	DAT_MakeLR1(pbBuffer, pdwLen, 0x00, 0x00);
	DAT_MakeLR1(pbBuffer, pdwLen, 0x01, 0x00);
	DAT_MakeLR1(pbBuffer, pdwLen, 0x01, 0x01);
	DAT_MakeLR2(pbBuffer, pdwLen, 0x01, 0x00);
	DAT_MakeLR2(pbBuffer, pdwLen, 0x00, 0x00);

	DAT_Replace8(pbBuffer, pdwLen, 0x00);
	DAT_Replace8(pbBuffer, pdwLen, 0x01);
	DAT_Replace8(pbBuffer, pdwLen, 0x02);

	DAT_Replace4(pbBuffer, pdwLen, 0xec, 0x51, 0xb8, 0x3f);

	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xac, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x2c);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3e);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3d);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3e);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3d);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x2c, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x0c, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x0c, 0x3f);

	DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0xd9, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x99, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x99, 0x3e);
	DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x59, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x19, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x19, 0x3f);

	DAT_Replace4(pbBuffer, pdwLen, 0x99, 0x3f, 0x2c, 0x2c);

	DAT_Replace4(pbBuffer, pdwLen, 0x85, 0xeb, 0x31, 0x40);

	DAT_Replace4(pbBuffer, pdwLen, 0x7b, 0x14, 0x2e, 0x3f);

	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0xf6, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0xe6, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0xa6, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x66, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x66, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x26, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x26, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x06, 0x40);

	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0xf3, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0xb3, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x93, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x73, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x73, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x53, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x33, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x33, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x13, 0x40);
	
	DAT_Replace4(pbBuffer, pdwLen, 0x29, 0x5c, 0x8f, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x29, 0x5c, 0x8f, 0x3e);

	DAT_Replace4(pbBuffer, pdwLen, 0x14, 0xae, 0x87, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x0a, 0xd7, 0x23, 0x3c);

	DAT_Replace4(pbBuffer, pdwLen, 0xf4, 0x01, 0x00, 0x00);

	DAT_Replace4(pbBuffer, pdwLen, 0xe8, 0x03, 0x00, 0x00);

	DAT_Replace4(pbBuffer, pdwLen, 0x1c, 0x01, 0x00, 0x00);

	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xf0, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xe0, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xe0, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc8, 0x42);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc8, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc0, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc0, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xb0, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x90, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x90, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x3e);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x70, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x60, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x48, 0x42);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x3f);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x34, 0x42);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x20, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x20, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x16, 0x43);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x10, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x01, 0x01);


	DAT_Replace4(pbBuffer, pdwLen, 0xd4, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xc8, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xc6, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xbe, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xb4, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xaa, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xa3, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xa2, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0xa0, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x9d, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x9c, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x96, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x91, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x8c, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x8a, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x83, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x82, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x7f, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x7d, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x5e, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x3c, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x3e, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x2f, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x2d, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x2a, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x28, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x26, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x25, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x23, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x22, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x1a, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x1e, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x19, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x18, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x17, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x16, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x15, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x14, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x13, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x12, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x11, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x10, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x0f, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x0e, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x0d, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x0c, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x0a, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x09, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x08, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x07, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x06, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x05, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x04, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x03, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x02, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x01, 0x00, 0x00, 0x00);



	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x41);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x40);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x3f);

	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x00);

	DAT_Replace4(pbBuffer, pdwLen, 0x78, 0x00, 0x00, 0x00);//z
	DAT_Replace4(pbBuffer, pdwLen, 0x77, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x71, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x70, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x6c, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x6e, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x6a, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x72, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x69, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x68, 0x00, 0x00, 0x00);//a

	DAT_Replace4(pbBuffer, pdwLen, 0x5a, 0x00, 0x00, 0x00);//Z
	DAT_Replace4(pbBuffer, pdwLen, 0x59, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x58, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x57, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x56, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x55, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x54, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x50, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x4b, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x48, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x46, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x42, 0x00, 0x00, 0x00);//A
	
	DAT_Replace4(pbBuffer, pdwLen, 0x38, 0x00, 0x00, 0x00);//9
	DAT_Replace4(pbBuffer, pdwLen, 0x37, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x32, 0x00, 0x00, 0x00);//~
	DAT_Replace4(pbBuffer, pdwLen, 0x31, 0x00, 0x00, 0x00);//
	DAT_Replace4(pbBuffer, pdwLen, 0x30, 0x00, 0x00, 0x00);//0

	DAT_Replace4(pbBuffer, pdwLen, 0x64, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x0b);
	DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x01);

	DAT_Replace4(pbBuffer, pdwLen, 0x0b, 0x00, 0x00, 0x00);
	DAT_Replace4(pbBuffer, pdwLen, 0x41, 0x00, 0x00, 0x00);
	//DAT_Replace4(pbBuffer, pdwLen, 0x2c, 0x00, 0x00, 0x00);
}

void ConvertData(PBYTE pbBuffer, LPDWORD pdwLen, KIND_OF_FILE kof)
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
