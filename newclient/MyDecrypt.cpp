// Decrypting_a_File.cpp : Defines the entry point for the console 
// application.
//
#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <winsock2.h>
#include <windows.h>
#include <wincrypt.h>
#include <conio.h>
#include <malloc.h>
#include <openssl\rsa.h>
#include <openssl\pem.h>

#include <CkCompression.h>
#include <CkCrypt2.h>
#include <CkStream.h>
#include <CkTask.h>
#include <CkBinData.h>
#include <CkGlobal.h>

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

void MyHandleError(LPTSTR psz, int nErrorNumber)
{
	_ftprintf(stderr, TEXT("An error occurred in the program. \n"));
	_ftprintf(stderr, TEXT("%s\n"), psz);
	_ftprintf(stderr, TEXT("Error number %x.\n"), nErrorNumber);
}

enum METHOD
{
	HEAPALLOC, HEAPFREE, CREATEFILE, CLOSEHANDLE, MALLOC, FREE, DECRYPT_ERROR, DELETE_ERROR
};
extern void WriteLog(char* pszname, METHOD meth);

extern BYTE pXOR;
extern char* strHDDSN;

//-------------------------------------------------------------------
// Code for the function MyDecryptFile called by main.
//-------------------------------------------------------------------
// Parameters passed are:
//  pszSource, the name of the input file, an encrypted file.
//  pszDestination, the name of the output, a plaintext file to be 
//   created.
//  pszPassword, either NULL if a password is not to be used or the 
//   string that is the password.
bool MyDecryptFile(
	LPTSTR pszSourceFile, 
	LPTSTR pszDestinationFile, 
	LPTSTR pszPassword)
{ 
	//---------------------------------------------------------------
	// Declare and initialize local variables.
	bool fReturn = false;
	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hDestinationFile = INVALID_HANDLE_VALUE; 
	HCRYPTKEY hKey = NULL; 
	HCRYPTHASH hHash = NULL; 

	HCRYPTPROV hCryptProv = NULL; 

	DWORD dwCount;
	PBYTE pbBuffer = NULL; 
	DWORD dwBlockLen; 
	DWORD dwBufferLen; 

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
		//_tprintf(
		//	TEXT("The source encrypted file, %s, is open. \n"), 
		//	pszSourceFile);
	}
	else
	{ 
		//MyHandleError(
		//	TEXT("Error opening source plaintext file!\n"), 
		//	GetLastError());
		goto Exit_MyDecryptFile;
	} 

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
	if(INVALID_HANDLE_VALUE != hDestinationFile)
	{
		//_tprintf(
		//	TEXT("The destination file, %s, is open. \n"), 
		//	pszDestinationFile);
	}
	else
	{
		//MyHandleError(
		//	TEXT("Error opening destination file!\n"), 
		//	GetLastError()); 
		goto Exit_MyDecryptFile;
	}

	//---------------------------------------------------------------
	// Get the handle to the default provider. 
	if(CryptAcquireContext(
		&hCryptProv, 
		NULL, 
		MS_ENHANCED_PROV, 
		PROV_RSA_FULL, 
		0))
	{
		//_tprintf(
		//	TEXT("A cryptographic provider has been acquired. \n"));
	}
	else
	{
		if (GetLastError() == NTE_BAD_KEYSET)
		{
			if(CryptAcquireContext(
				&hCryptProv, 
				NULL, 
				MS_ENHANCED_PROV, 
				PROV_RSA_FULL, 
				CRYPT_NEWKEYSET)) 
			{
				printf("A new key container has been created.\n");
			}
			else
			{
				printf("Could not create a new key container.\n");
				goto Exit_MyDecryptFile;
			}
		}
		else
		{
			printf("A cryptographic service handle could not be "
				"acquired.\n");
			goto Exit_MyDecryptFile;
		}
		//MyHandleError(
		//	TEXT("Error during CryptAcquireContext!\n"), 
		//	GetLastError());
		//goto Exit_MyDecryptFile;
	}

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
			hSourceFile, 
			&dwKeyBlobLen, 
			sizeof(DWORD), 
			&dwCount, 
			NULL))
		{
			//MyHandleError(
			//	TEXT("Error reading key BLOB length!\n"), 
			//	GetLastError());
			goto Exit_MyDecryptFile;
		}

		// Allocate a buffer for the key BLOB.
		if(!(pbKeyBlob = (PBYTE)malloc(dwKeyBlobLen)))
		{
			//MyHandleError(
			//	TEXT("Memory allocation error.\n"), 
			//	E_OUTOFMEMORY); 
		}

		//-----------------------------------------------------------
		// Read the key BLOB from the source file. 
		if(!ReadFile(
			hSourceFile, 
			pbKeyBlob, 
			dwKeyBlobLen, 
			&dwCount, 
			NULL))
		{
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
		//MyHandleError(TEXT("Out of memory!\n"), E_OUTOFMEMORY); 
		goto Exit_MyDecryptFile;
	}

	//decrypt 128bytes to 16bytes with rsa
	{
		//-----------------------------------------------------------
		// Read up to dwBlockLen bytes from the source file. 
		BYTE pbIn[128] = { 0 };
		DWORD dwIn;
		if(!ReadFile(
			hSourceFile, 
			pbIn, 
			128, 
			&dwIn, 
			NULL))
		{
			MyHandleError(
				TEXT("Error reading plaintext!\n"), 
				GetLastError());
			goto Exit_MyDecryptFile;
		}
		char *private_key_str = "-----BEGIN RSA PRIVATE KEY-----\nMIICXQIBAAKBgQC999WKQzpYwCc55OHjbdE32SURGMqSIVkzf4MdINYlCGiz5RUe\n8nY2gaXfVextxr7zn95hooz/vS4ynYEPKdaYA1uw3l2ZNnUwHkOIXWNvO/7MOjRS\nicTFwBRqFHQETm1XrKQmQXR0qqPJBTAI1RoHumaB+pwIxs7yH4o4LI/towIDAQAB\nAoGBAI1ALF2EI2w+ZGxdzcBntXtLUI5n2qfReBwcogcUlWYv3Hp2yb+bFV7uA8IO\nh6AQeYd4xcffL+wwZJtqFb6Ko25XAei8Os3xjb9k5fCcyrmyY+5oeXdQHlcbd/f8\niy8/rOEHZTr4iBXe/8ADlQZlRUkYCblPZ4i4BgzBUB6HzhxhAkEA8wJRx/FjOo6F\noO1aTewbvFIv4Dckqq5j/pBu9fkv1AhMxSfdGnsYcuIn15Y1/RlnpxrmJNWgryvd\n+6LJGDgjWQJBAMgfoINe80YiPCdMoboMd/u1uf1BhwujbiJPSrS40lc3jfyPmHA4\n8hppo8QuELI4rXRE/im4c+zmyphxEyULpVsCQQDnD96JGin65MeE1AsYqpdYwmEJ\ndgVkUXt88wK+2ZizqMyubpAa/M6rdgTiRc7CASUAzF/myEYIKdLh0NAbOk3JAkAE\nxEQVfPh8bipSoU+k19EvzKdOcfYef9kKtirIXTKdYzRdlKoD2kdh+6wr6xD4vcLb\n5xzKr5sLRIAE24SiOEHLAkB1TBlvvvIltttSc9lOpq3UhmtHQJaS32lD2Lk2/zNx\nW6Jbsk+sCQXM0ww4GTCpHMISfokEPtqOPikPcVFs98Oj\n-----END RSA PRIVATE KEY-----\n";
		RSA *rsa;
		BIO *p_bio = BIO_new_mem_buf(private_key_str, -1);
		rsa = PEM_read_bio_RSAPrivateKey(p_bio, NULL, NULL, NULL); //PEM_read_bio_RSAPrivateKey
		if ( rsa == NULL ) {
			goto Exit_MyDecryptFile;
		}
		BYTE res[16] = {0};
		if((RSA_private_decrypt(128, (const unsigned char*)pbIn, res, rsa, RSA_PKCS1_PADDING)) <= 0) {
			goto Exit_MyDecryptFile;
		}
		RSA_free(rsa);


		//-----------------------------------------------------------
		// Write the encrypted data to the destination file. 
		if(!WriteFile(
			hDestinationFile, 
			res, 
			16,
			&dwIn,
			NULL))
		{ 
			MyHandleError(
				TEXT("Error writing ciphertext.\n"), 
				GetLastError());
			goto Exit_MyDecryptFile;
		}

	}

	//---------------------------------------------------------------
	// Decrypt the source file, and write to the destination file. 
	bool fEOF = false;
	do
	{
		//-----------------------------------------------------------
		// Read up to dwBlockLen bytes from the source file. 
		if(!ReadFile(
			hSourceFile, 
			pbBuffer, 
			dwBlockLen, 
			&dwCount, 
			NULL))
		{
			//MyHandleError(
			//	TEXT("Error reading from source file!\n"), 
			//	GetLastError());
			goto Exit_MyDecryptFile;
		}

		for(DWORD i = 0;i < dwCount; i++)
		{
			BYTE pChar = *(pbBuffer + i);
			BYTE pXOR1 = pChar;
			pChar ^= pXOR; 
			*(pbBuffer + i) = pChar;
			//pXOR = pXOR1;
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
			//MyHandleError(
			//	TEXT("Error during CryptDecrypt!\n"), 
			//	GetLastError()); 
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// Write the decrypted data to the destination file. 
		if(!WriteFile(
			hDestinationFile, 
			pbBuffer, 
			dwCount,
			&dwCount,
			NULL))
		{ 
			//MyHandleError(
			//	TEXT("Error writing ciphertext.\n"), 
			//	GetLastError());
			goto Exit_MyDecryptFile;
		}

		//-----------------------------------------------------------
		// End the do loop when the last block of the source file 
		// has been read, encrypted, and written to the destination 
		// file.
	}while(!fEOF);

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
	if(hSourceFile)
	{
		CloseHandle(hSourceFile);
	}

	if(hDestinationFile)
	{
		CloseHandle(hDestinationFile);
	}

	//-----------------------------------------------------------
	// Release the hash object. 
	if(hHash) 
	{
		if(!(CryptDestroyHash(hHash)))
		{
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
			//MyHandleError(
			//	TEXT("Error during CryptReleaseContext!\n"), 
			//	GetLastError());
		}
	} 

	return fReturn;
}

void DecryptSimple(PBYTE pbBuffer, DWORD dwCount)
{
	for(DWORD i = 0;i < dwCount; i++)
	{
		BYTE pChar = *(pbBuffer + i);
		BYTE pXOR1 = pChar;
		pChar ^= pXOR; 
		*(pbBuffer + i) = pChar;
		//pXOR = pXOR1;
	}
}

bool MyDecompressFile(
	LPTSTR pszSourceFile, 
	LPTSTR pszDestinationFile)
{
	CkGlobal glob;
	bool state = glob.UnlockBundle("mLjrzW.CBX0923_lKH4ONJrnR9s");
	if (!state)
	{
		return false;
	}

	bool success;

	//  Initialize the compression object.
	CkCompression compress;
	compress.put_Algorithm("deflate");

	//  Initialize the encryption object.
	CkCrypt2 crypt;
	crypt.put_CryptAlgorithm("chacha20");
	crypt.put_KeyLength(256);
	crypt.put_EncodingMode("hex");
	const char *ivHex = "000000000000000000000002";
	crypt.SetEncodedIV(ivHex,"hex");
	crypt.put_InitialCount(42);
	const char *keyHex = "1c9240a5eb55d38af333888604f6b5f0473917c1402b80099dca5cbc207075c0";
	crypt.SetEncodedKey(keyHex,"hex");

	//  Load the file that was produced and decrypt/decompress to verify.
	CkBinData binData;
	success = binData.LoadFile(pszSourceFile);
	success = crypt.DecryptBd(binData);
	success = compress.DecompressBd(binData);

	//  binData should contain the original data..
	success = binData.WriteFile(pszDestinationFile);

	/*	std::cout << "Finished." << "\r\n";*/

	return success;
}
