// MyReadFileToBuffer.cpp : Defines the entry point for the console 
// application.
//
#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#include <conio.h>
#include <malloc.h>

// Link with the Advapi32.lib file.
#pragma comment (lib, "advapi32")

#define KEYLENGTH  0x00800000
#define ENCRYPT_ALGORITHM CALG_RC4 
#define ENCRYPT_BLOCK_SIZE 8 

//-------------------------------------------------------------------
//  This example uses the function MyHandleError, a simple error
//  handling function, to print an error message to the  
//  standard error (stderr) file and exit the program. 
//  For most applications, replace this function with one 
//  that does more extensive error reporting.

extern void MyHandleError(LPTSTR psz, int nErrorNumber);
//-------------------------------------------------------------------
// Code for the function MyReadFile called by main.
//-------------------------------------------------------------------
// Parameters passed are:
//  pszSource, the name of the input file, an encrypted file.
//  pszDestination, the name of the output, a plaintext file to be 
//   created.
//  pszPassword, either NULL if a password is not to be used or the 
//   string that is the password.
BOOL WINAPI MyReadFileToBuffer(
	_In_         HANDLE hFile,
	_Out_        LPVOID lpBuffer,
	_In_         DWORD nNumberOfBytesToRead,
	_Out_opt_    LPDWORD lpNumberOfBytesRead,
	_Inout_opt_  LPOVERLAPPED lpOverlapped)
{ 
	//---------------------------------------------------------------
	// Declare and initialize local variables.
	bool fReturn = false;
	HCRYPTKEY hKey = NULL; 
	HCRYPTHASH hHash = NULL; 

	HCRYPTPROV hCryptProv = NULL; 

	DWORD dwCount;
	PBYTE pbBuffer = NULL; 
	DWORD dwBlockLen; 
	DWORD dwBufferLen; 


	TCHAR pChar[MAX_PATH] = { 0 };
	void* buf = malloc(nNumberOfBytesToRead);
	memset(buf, 0, nNumberOfBytesToRead);

	//---------------------------------------------------------------
	// Get the handle to the default provider. 
	if(CryptAcquireContext(
		&hCryptProv, 
		NULL, 
		MS_ENHANCED_PROV, 
		PROV_RSA_FULL, 
		0))
	{
		//MessageBox(NULL, "A cryptographic provider has been acquired.", "Success", 0L);
		//_tprintf(
		//	TEXT("A cryptographic provider has been acquired. \n"));
	}
	else
	{
		//MessageBox(NULL, "Error during CryptAcquireContext!", "Error", 0L);
		//MyHandleError(
		//	TEXT("Error during CryptAcquireContext!\n"), 
		//	GetLastError());
		goto Exit_MyDecryptFile;
	}
	//---------------------------------------------------------------
	// Get Password.
	TCHAR pszPassword[MAX_PATH] = { 0 };
	//wsprintf(pszPassword, L"%s", strHDDSN.GetBuffer());

	//---------------------------------------------------------------
	// Create the session key.
	if(!pszPassword || !pszPassword[0]) 
	{ 
		//-----------------------------------------------------------
		// Decrypt the file with the saved session key. 

		DWORD dwKeyBlobLen;
		PBYTE pbKeyBlob = NULL;

		// Read the key BLOB length from the source file. 
		if(!ReadFile(
			hFile, 
			&dwKeyBlobLen, 
			sizeof(DWORD), 
			&dwCount, 
			NULL))
		{
			//MessageBox(NULL, "Error reading key BLOB length!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error reading key BLOB length!\n"), 
			//	GetLastError());
			goto Exit_MyDecryptFile;
		}

		// Allocate a buffer for the key BLOB.
		if(!(pbKeyBlob = (PBYTE)malloc(dwKeyBlobLen)))
		{
			//MessageBox(NULL, "Memory allocation error.", "Error", 0L);
			//MyHandleError(
			//	TEXT("Memory allocation error.\n"), 
			//	E_OUTOFMEMORY); 
		}

		//-----------------------------------------------------------
		// Read the key BLOB from the source file. 
		if(!ReadFile(
			hFile, 
			pbKeyBlob, 
			dwKeyBlobLen, 
			&dwCount, 
			NULL))
		{
			//MessageBox(NULL, "Error reading key BLOB length!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error reading key BLOB length!\n"), 
			//	GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Import the key BLOB into the CSP. 
		if(!CryptImportKey(
			hCryptProv, 
			pbKeyBlob, 
			dwKeyBlobLen, 
			0, 
			0, 
			&hKey))
		{
			//MessageBox(NULL, "Error during CryptImportKey!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptImportKey!/n"), 
			//	GetLastError()); 
			goto Exit_MyDecryptFile;
		}

		if(pbKeyBlob)
		{
			free(pbKeyBlob);
		}
	}
	else
	{
		//-----------------------------------------------------------
		// Decrypt the file with a session key derived from a 
		// password. 

		//-----------------------------------------------------------
		// Create a hash object. 
		if(!CryptCreateHash(
			hCryptProv, 
			CALG_MD5, 
			0, 
			0, 
			&hHash))
		{
			//MessageBox(NULL, "Error during CryptCreateHash!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptCreateHash!\n"), 
			//	GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Hash in the password data. 
		if(!CryptHashData(
			hHash, 
			(BYTE *)pszPassword, 
			lstrlen(pszPassword), 
			0)) 
		{
			//MessageBox(NULL, "Error during CryptHashData!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptHashData!\n"), 
			//	GetLastError()); 
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Derive a session key from the hash object. 
		if(!CryptDeriveKey(
			hCryptProv, 
			ENCRYPT_ALGORITHM, 
			hHash, 
			KEYLENGTH, 
			&hKey))
		{ 
			//MessageBox(NULL, "Error during CryptDeriveKey!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptDeriveKey!\n"), 
			//	GetLastError()) ; 
			goto Exit_MyDecryptFile;
		}
	}

	//---------------------------------------------------------------
	// The decryption key is now available, either having been 
	// imported from a BLOB read in from the source file or having 
	// been created by using the password. This point in the program 
	// is not reached if the decryption key is not available.

	//---------------------------------------------------------------
	// Determine the number of bytes to decrypt at a time. 
	// This must be a multiple of ENCRYPT_BLOCK_SIZE. 

	dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE; 
	dwBufferLen = dwBlockLen; 

	//---------------------------------------------------------------
	// Allocate memory for the file read buffer. 
	if(!(pbBuffer = (PBYTE)malloc(dwBufferLen)))
	{
		//MessageBox(NULL, "Out of memory!", "Error", 0L);
		//MyHandleError(TEXT("Out of memory!\n"), E_OUTOFMEMORY); 
		goto Exit_MyDecryptFile;
	}

	//---------------------------------------------------------------
	// Decrypt the source file, and write to the destination file. 
	bool fEOF = false;
	DWORD dwAmount = 0;
	do
	{
		//-----------------------------------------------------------
		// Read up to dwBlockLen bytes from the source file. 
		if(!ReadFile(
			hFile, 
			pbBuffer, 
			dwBlockLen, 
			&dwCount, 
			NULL))
		{
			//MessageBox(NULL, "Error reading from source file!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error reading from source file!\n"), 
			//	GetLastError());
			goto Exit_MyDecryptFile;
		}

		if(dwCount < dwBlockLen)
		{
			fEOF = TRUE;
		}

		//-----------------------------------------------------------
		// if dwCount == 0, continue; 
		if(dwCount == 0) continue;

		//-----------------------------------------------------------
		// Decrypt the block of data. 
		if(!CryptDecrypt(
			hKey, 
			0, 
			fEOF, 
			0, 
			pbBuffer, 
			&dwCount))
		{
			//MessageBox(NULL, "Error during CryptDecrypt!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptDecrypt!\n"), 
			//	GetLastError()); 
			goto Exit_MyDecryptFile;
		}


		//-----------------------------------------------------------
		// copy the decrypted data to buf. 

		memcpy((BYTE*)buf + dwAmount, pbBuffer, dwCount);
		dwAmount += dwCount;


		////-----------------------------------------------------------
		//// Write the decrypted data to the destination file. 
		//if(!WriteFile(
		//	hDestinationFile, 
		//	pbBuffer, 
		//	dwCount,
		//	&dwCount,
		//	NULL))
		//{ 
		//	MessageBox(NULL, L"Error writing ciphertext.", L"Error", 0L);
		//	//MyHandleError(
		//	//	TEXT("Error writing ciphertext.\n"), 
		//	//	GetLastError());
		//	goto Exit_MyDecryptFile;
		//}



		//-----------------------------------------------------------
		// End the do loop when the last block of the source file 
		// has been read, encrypted, and written to the destination 
		// file.
	}while(!fEOF);

	memcpy(lpBuffer, buf, dwAmount);
	*lpNumberOfBytesRead = dwAmount;
	free(buf);

	fReturn = true;

Exit_MyDecryptFile:

	//---------------------------------------------------------------
	// Free the file read buffer.
	if(pbBuffer)
	{
		free(pbBuffer);
	}

	//---------------------------------------------------------------
	// Close files.
	if(hFile)
	{
		CloseHandle(hFile);
	}

	//-----------------------------------------------------------
	// Release the hash object. 
	if(hHash) 
	{
		if(!(CryptDestroyHash(hHash)))
		{
			//MessageBox(NULL, "Error during CryptDestroyHash.", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptDestroyHash.\n"), 
			//	GetLastError()); 
		}

		hHash = NULL;
	}

	//---------------------------------------------------------------
	// Release the session key. 
	if(hKey)
	{
		if(!(CryptDestroyKey(hKey)))
		{
			//MessageBox(NULL, "Error during CryptDestroyKey!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptDestroyKey!\n"), 
			//	GetLastError());
		}
	} 

	//---------------------------------------------------------------
	// Release the provider handle. 
	if(hCryptProv)
	{
		if(!(CryptReleaseContext(hCryptProv, 0)))
		{
			//MessageBox(NULL, "Error during CryptReleaseContext!", "Error", 0L);
			//MyHandleError(
			//	TEXT("Error during CryptReleaseContext!\n"), 
			//	GetLastError());
		}
	} 

	return fReturn;
}

