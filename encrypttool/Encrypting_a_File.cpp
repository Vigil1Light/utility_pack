// Encrypting_a_File.cpp : Defines the entry point for the console 
// application.
//
#include "stdafx.h"
#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#include <conio.h>
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

extern void MyHandleError(LPTSTR psz, int nErrorNumber);
extern BYTE pXORFix;

//-------------------------------------------------------------------
// Code for the function MyEncryptFile called by main.
//-------------------------------------------------------------------
// Parameters passed are:
//  pszSource, the name of the input, a plaintext file.
//  pszDestination, the name of the output, an encrypted file to be 
//   created.
//  pszPassword, either NULL if a password is not to be used or the 
//   string that is the password.
bool MyEncryptFile(
	LPTSTR pszSourceFile, 
	LPTSTR pszDestinationFile, 
	LPTSTR pszPassword)
{ 
	//---------------------------------------------------------------
	// Declare and initialize local variables.
	bool fReturn = false;
	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hDestinationFile = INVALID_HANDLE_VALUE; 

	HCRYPTPROV hCryptProv = NULL; 
	HCRYPTKEY hKey = NULL; 
	HCRYPTKEY hXchgKey = NULL; 
	HCRYPTHASH hHash = NULL; 

	PBYTE pbKeyBlob = NULL; 
	DWORD dwKeyBlobLen; 

	PBYTE pbBuffer = NULL; 
	DWORD dwBlockLen; 
	DWORD dwBufferLen; 
	DWORD dwCount; 

	BYTE pXOR = pXORFix;
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

	//---------------------------------------------------------------
	// Get the handle to the default provider. 
	if(CryptAcquireContext(
		&hCryptProv, 
		NULL, 
		MS_ENHANCED_PROV, 
		PROV_RSA_FULL, 
		0))
	{
		_tprintf(
			TEXT("A cryptographic provider has been acquired. \n"));
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
				goto Exit_MyEncryptFile;
			}
		}
		else
		{
			printf("A cryptographic service handle could not be "
				"acquired.\n");
			goto Exit_MyEncryptFile;
		}

		//int n = GetLastError();
		//MyHandleError(
		//	TEXT("Error during CryptAcquireContext!\n"), 
		//	GetLastError());
		//goto Exit_MyEncryptFile;
	}

	//---------------------------------------------------------------
	// Create the session key.
	if(!pszPassword || !pszPassword[0]) 
	{ 
		//-----------------------------------------------------------
		// No password was passed.
		// Encrypt the file with a random session key, and write the 
		// key to a file. 

		//-----------------------------------------------------------
		// Create a random session key. 
		if(CryptGenKey(
			hCryptProv, 
			ENCRYPT_ALGORITHM, 
			KEYLENGTH | CRYPT_EXPORTABLE, 
			&hKey))
		{
			_tprintf(TEXT("A session key has been created. \n"));
		} 
		else
		{
			MyHandleError(
				TEXT("Error during CryptGenKey. \n"), 
				GetLastError()); 
			goto Exit_MyEncryptFile;
		}

		//-----------------------------------------------------------
		// Get the handle to the exchange public key. 
		if(CryptGetUserKey(
			hCryptProv, 
			AT_KEYEXCHANGE, 
			&hXchgKey))
		{
			_tprintf(
				TEXT("The user public key has been retrieved. \n"));
		}
		else
		{ 
			if(NTE_NO_KEY == GetLastError())
			{
				// No exchange key exists. Try to create one.
				if(!CryptGenKey(
					hCryptProv, 
					AT_KEYEXCHANGE, 
					CRYPT_EXPORTABLE, 
					&hXchgKey))
				{
					MyHandleError(
						TEXT("Could not create a user public key.\n"), 
						GetLastError()); 
					goto Exit_MyEncryptFile;
				}
			}
			else
			{
				MyHandleError(
					TEXT("User public key is not available and may ")
					TEXT("not exist.\n"), 
					GetLastError()); 
				goto Exit_MyEncryptFile;
			}
		}

		//-----------------------------------------------------------
		// Determine size of the key BLOB, and allocate memory. 
		if(CryptExportKey(
			hKey, 
			hXchgKey, 
			SIMPLEBLOB, 
			0, 
			NULL, 
			&dwKeyBlobLen))
		{
			_tprintf(
				TEXT("The key BLOB is %d bytes long. \n"), 
				dwKeyBlobLen);
		}
		else
		{  
			MyHandleError(
				TEXT("Error computing BLOB length! \n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}

		if(pbKeyBlob = (BYTE *)malloc(dwKeyBlobLen))
		{ 
			_tprintf(
				TEXT("Memory is allocated for the key BLOB. \n"));
		}
		else
		{ 
			MyHandleError(TEXT("Out of memory. \n"), E_OUTOFMEMORY); 
			goto Exit_MyEncryptFile;
		}

		//-----------------------------------------------------------
		// Encrypt and export the session key into a simple key 
		// BLOB. 
		if(CryptExportKey(
			hKey, 
			hXchgKey, 
			SIMPLEBLOB, 
			0, 
			pbKeyBlob, 
			&dwKeyBlobLen))
		{
			_tprintf(TEXT("The key has been exported. \n"));
		} 
		else
		{
			MyHandleError(
				TEXT("Error during CryptExportKey!\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		} 

		//-----------------------------------------------------------
		// Release the key exchange key handle. 
		if(hXchgKey)
		{
			if(!(CryptDestroyKey(hXchgKey)))
			{
				MyHandleError(
					TEXT("Error during CryptDestroyKey.\n"), 
					GetLastError()); 
				goto Exit_MyEncryptFile;
			}

			hXchgKey = 0;
		}

		//-----------------------------------------------------------
		// Write the size of the key BLOB to the destination file. 
		if(!WriteFile(
			hDestinationFile, 
			&dwKeyBlobLen, 
			sizeof(DWORD),
			&dwCount,
			NULL))
		{ 
			MyHandleError(
				TEXT("Error writing header.\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}
		else
		{
			_tprintf(TEXT("A file header has been written. \n"));
		}

		//-----------------------------------------------------------
		// Write the key BLOB to the destination file. 
		if(!WriteFile(
			hDestinationFile, 
			pbKeyBlob, 
			dwKeyBlobLen,
			&dwCount,
			NULL))
		{ 
			MyHandleError(
				TEXT("Error writing header.\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}
		else
		{
			_tprintf(
				TEXT("The key BLOB has been written to the ")
				TEXT("file. \n"));
		}

		// Free memory.
		free(pbKeyBlob);
	} 
	else 
	{ 

		//-----------------------------------------------------------
		// The file will be encrypted with a session key derived 
		// from a password.
		// The session key will be recreated when the file is 
		// decrypted only if the password used to create the key is 
		// available. 

		//-----------------------------------------------------------
		// Create a hash object. 
		if(CryptCreateHash(
			hCryptProv, 
			CALG_MD5, 
			0, 
			0, 
			&hHash))
		{
			_tprintf(TEXT("A hash object has been created. \n"));
		}
		else
		{ 
			MyHandleError(
				TEXT("Error during CryptCreateHash!\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}  

		//-----------------------------------------------------------
		// Hash the password. 
		if(CryptHashData(
			hHash, 
			(BYTE *)pszPassword, 
			lstrlen(pszPassword), 
			0))
		{
			_tprintf(
				TEXT("The password has been added to the hash. \n"));
		}
		else
		{
			MyHandleError(
				TEXT("Error during CryptHashData. \n"), 
				GetLastError()); 
			goto Exit_MyEncryptFile;
		}

		//-----------------------------------------------------------
		// Derive a session key from the hash object. 
		if(CryptDeriveKey(
			hCryptProv, 
			ENCRYPT_ALGORITHM, 
			hHash, 
			KEYLENGTH, 
			&hKey))
		{
			_tprintf(
				TEXT("An encryption key is derived from the ")
				TEXT("password hash. \n")); 
		}
		else
		{
			MyHandleError(
				TEXT("Error during CryptDeriveKey!\n"), 
				GetLastError()); 
			goto Exit_MyEncryptFile;
		}
	} 

	//---------------------------------------------------------------
	// The session key is now ready. If it is not a key derived from 
	// a  password, the session key encrypted with the private key 
	// has been written to the destination file.

	//---------------------------------------------------------------
	// Determine the number of bytes to encrypt at a time. 
	// This must be a multiple of ENCRYPT_BLOCK_SIZE.
	// ENCRYPT_BLOCK_SIZE is set by a #define statement.
	dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE; 

	//---------------------------------------------------------------
	// Determine the block size. If a block cipher is used, 
	// it must have room for an extra block. 
	if(ENCRYPT_BLOCK_SIZE > 1) 
	{
		dwBufferLen = dwBlockLen + ENCRYPT_BLOCK_SIZE; 
	}
	else 
	{
		dwBufferLen = dwBlockLen; 
	}

	//---------------------------------------------------------------
	// Allocate memory. 
	if(pbBuffer = (BYTE *)malloc(dwBufferLen))
	{
		_tprintf(
			TEXT("Memory has been allocated for the buffer. \n"));
	}
	else
	{ 
		MyHandleError(TEXT("Out of memory. \n"), E_OUTOFMEMORY); 
		goto Exit_MyEncryptFile;
	}
	
	//encrypt 16bytes to 128bytes with rsa
	{
		//-----------------------------------------------------------
		// Read up to dwBlockLen bytes from the source file. 
		BYTE pbIn[16] = { 0 };
		DWORD dwIn;
		if(!ReadFile(
			hSourceFile, 
			pbIn, 
			16, 
			&dwIn, 
			NULL))
		{
			MyHandleError(
				TEXT("Error reading plaintext!\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}
		char *public_key_str = "-----BEGIN RSA PUBLIC KEY-----\nMIGJAoGBAL331YpDOljAJznk4eNt0TfZJREYypIhWTN/gx0g1iUIaLPlFR7ydjaB\npd9V7G3GvvOf3mGijP+9LjKdgQ8p1pgDW7DeXZk2dTAeQ4hdY287/sw6NFKJxMXA\nFGoUdARObVespCZBdHSqo8kFMAjVGge6ZoH6nAjGzvIfijgsj+2jAgMBAAE=\n-----END RSA PUBLIC KEY-----\n";
		RSA *rsa;
		int rsa_len;
		PBYTE p_en;
		BIO* p_bio = BIO_new_mem_buf(public_key_str, -1);
		rsa = PEM_read_bio_RSAPublicKey(p_bio, NULL, NULL, NULL); //PEM_read_bio_RSAPrivateKey
		if ( rsa == NULL ) {
			goto Exit_MyEncryptFile;
		}
		rsa_len = RSA_size(rsa);
		p_en = (PBYTE)calloc(rsa_len + 1, 1);
		if((RSA_public_encrypt(16, (PBYTE)pbIn, (PBYTE)p_en, rsa, RSA_PKCS1_PADDING)) <= 0 ) 
		{
			goto Exit_MyEncryptFile;
		}
		RSA_free(rsa);


		//-----------------------------------------------------------
		// Write the encrypted data to the destination file. 
		if(!WriteFile(
			hDestinationFile, 
			p_en, 
			128,
			&dwCount,
			NULL))
		{ 
			MyHandleError(
				TEXT("Error writing ciphertext.\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}

	}

	//---------------------------------------------------------------
	// In a do loop, encrypt the source file, 
	// and write to the source file. 
	bool fEOF = FALSE;
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
			MyHandleError(
				TEXT("Error reading plaintext!\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}

		if(dwCount < dwBlockLen)
		{
			fEOF = TRUE;
		}

		//-----------------------------------------------------------
		// if dwCount == 0, continue; 
		if(dwCount == 0) continue;

		//-----------------------------------------------------------
		// Encrypt data. 
		if(!CryptEncrypt(
			hKey, 
			NULL, 
			fEOF,
			0, 
			pbBuffer, 
			&dwCount, 
			dwBufferLen))
		{ 
			MyHandleError(
				TEXT("Error during CryptEncrypt. \n"), 
				GetLastError()); 
			goto Exit_MyEncryptFile;
		} 

		for(DWORD i = 0;i < dwCount; i++)
		{
			BYTE pChar = *(pbBuffer + i);
			pChar ^= pXOR; 
			*(pbBuffer + i) = pChar;
			//pXOR = pChar;
		}

		//-----------------------------------------------------------
		// Write the encrypted data to the destination file. 
		if(!WriteFile(
			hDestinationFile, 
			pbBuffer, 
			dwCount,
			&dwCount,
			NULL))
		{ 
			MyHandleError(
				TEXT("Error writing ciphertext.\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}

		//-----------------------------------------------------------
		// End the do loop when the last block of the source file 
		// has been read, encrypted, and written to the destination 
		// file.
	} while(!fEOF);

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


	//-----------------------------------------------------------
	// Release the hash object. 
	if(hHash) 
	{
		if(!(CryptDestroyHash(hHash)))
		{
			MyHandleError(
				TEXT("Error during CryptDestroyHash.\n"), 
				GetLastError()); 
		}

		hHash = NULL;
	}

	//---------------------------------------------------------------
	// Release the session key. 
	if(hKey)
	{
		if(!(CryptDestroyKey(hKey)))
		{
			MyHandleError(
				TEXT("Error during CryptDestroyKey!\n"), 
				GetLastError());
		}
	}

	//---------------------------------------------------------------
	// Release the provider handle. 
	if(hCryptProv)
	{
		if(!(CryptReleaseContext(hCryptProv, 0)))
		{
			MyHandleError(
				TEXT("Error during CryptReleaseContext!\n"), 
				GetLastError());
		}
	}

	return fReturn; 
} // End Encryptfile.

bool MyEncryptFileXOR(
	LPTSTR pszSourceFile, 
	LPTSTR pszDestinationFile)
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

	
	BYTE pXOR = pXORFix;
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

	//---------------------------------------------------------------
	// Determine the number of bytes to encrypt at a time. 
	// This must be a multiple of ENCRYPT_BLOCK_SIZE.
	// ENCRYPT_BLOCK_SIZE is set by a #define statement.
	dwBlockLen = 1000; 
	dwBufferLen = dwBlockLen;
	//---------------------------------------------------------------
	// Allocate memory. 
	if(pbBuffer = (BYTE *)malloc(dwBufferLen))
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
			MyHandleError(
				TEXT("Error reading plaintext!\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}

		if(dwCount < dwBlockLen)
		{
			fEOF = TRUE;
		}

		//-----------------------------------------------------------
		// Encrypt data. 
		for(DWORD i = 0;i < dwCount; i++)
		{
			BYTE pChar = *(pbBuffer + i);
			pChar ^= pXOR; 
			*(pbBuffer + i) = pChar;
			//pXOR = pChar;
		}
		//-----------------------------------------------------------
		// Write the encrypted data to the destination file. 
		if(!WriteFile(
			hDestinationFile, 
			pbBuffer, 
			dwCount,
			NULL,
			NULL))
		{ 
			MyHandleError(
				TEXT("Error writing ciphertext.\n"), 
				GetLastError());
			goto Exit_MyEncryptFile;
		}

		//-----------------------------------------------------------
		// End the do loop when the last block of the source file 
		// has been read, encrypted, and written to the destination 
		// file.
	} while(!fEOF);

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

bool MyCompressFile(
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

	//  Setup the streams.
	//  streamC is for compression
	CkStream streamC;
	//  streamE is for encryption
	CkStream streamE;

	//  The source for the compressor is a file.
	streamC.put_SourceFile(pszSourceFile);
	//  The source for the encryptor is the output of the compressor stream.
	success = streamE.SetSourceStream(streamC);

	//  The sink for the encryptor is the output file.
	streamE.put_SinkFile(pszDestinationFile);

	//  Start the compression and encryption streams in background threads..
	CkTask *taskC = compress.CompressStreamAsync(streamC);
	CkTask *taskE = crypt.EncryptStreamAsync(streamE);
	success = taskC->Run();
	success = taskE->Run();

	//  Let the streams run until finished.
	//  Let's make sure the background task finished.
	//  It should already be the case that the task is finished.
	while (((taskE->get_Finished() != true) || (taskC->get_Finished() != true))) {
		taskE->SleepMs(20);
	}

	//  check for success..
	if (taskE->get_TaskSuccess() != true) {
// 		std::cout << "async encryption failed:" << "\r\n";
// 		std::cout << taskE->resultErrorText() << "\r\n";
		success = false;
	}

	//  check for success..
	if (taskC->get_TaskSuccess() != true) {
// 		std::cout << "async compression failed:" << "\r\n";
// 		std::cout << taskC->resultErrorText() << "\r\n";
		success = false;
	}

	delete taskE;
	delete taskC;
	return success;
}