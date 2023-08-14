// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>
#include <D3dx9tex.h>
#include <memory>

#pragma comment(lib, "D3dx9.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE ExtractDataDLL = { NULL, NULL };

typedef BOOL (WINAPI *pCloseHandleA)(HANDLE hObject);
typedef HRESULT (WINAPI *pD3DXCreateTextureFromFileInMemoryEx)(
	_In_    LPDIRECT3DDEVICE9  pDevice,
	_In_    LPCVOID            pSrcData,
	_In_    UINT               SrcDataSize,
	_In_    UINT               Width,
	_In_    UINT               Height,
	_In_    UINT               MipLevels,
	_In_    DWORD              Usage,
	_In_    D3DFORMAT          Format,
	_In_    D3DPOOL            Pool,
	_In_    DWORD              Filter,
	_In_    DWORD              MipFilter,
	_In_    D3DCOLOR           ColorKey,
	_Inout_ D3DXIMAGE_INFO     *pSrcInfo,
	_Out_   PALETTEENTRY       *pPalette,
	_Out_   LPDIRECT3DTEXTURE9 *ppTexture
	);
BOOL WINAPI MyCloseHandleA(HANDLE);
HRESULT WINAPI MyD3DXCreateTextureFromFileInMemoryEx(
	_In_    LPDIRECT3DDEVICE9  pDevice,
	_In_    LPCVOID            pSrcData,
	_In_    UINT               SrcDataSize,
	_In_    UINT               Width,
	_In_    UINT               Height,
	_In_    UINT               MipLevels,
	_In_    DWORD              Usage,
	_In_    D3DFORMAT          Format,
	_In_    D3DPOOL            Pool,
	_In_    DWORD              Filter,
	_In_    DWORD              MipFilter,
	_In_    D3DCOLOR           ColorKey,
	_Inout_ D3DXIMAGE_INFO     *pSrcInfo,
	_Out_   PALETTEENTRY       *pPalette,
	_Out_   LPDIRECT3DTEXTURE9 *ppTexture
	);

void BeginRedirectCH(LPVOID);
void BeginRedirectTFM(LPVOID);

pCloseHandleA pOrigCHAddress = NULL;
pD3DXCreateTextureFromFileInMemoryEx pOrigTFMAddress = NULL;

#define SIZE 6 //Number of bytes needed to redirect

BYTE oldBytesCH[SIZE] = {0}; //This will hold the overwritten bytes
BYTE oldBytesTFM[SIZE] = {0}; //This will hold the overwritten bytes

BYTE JMPCH[SIZE] = {0};	//This holds the JMP to our code
BYTE JMPTFM[SIZE] = {0};	//This holds the JMP to our code

DWORD oldProtectCH, myProtectCH = PAGE_EXECUTE_READWRITE; //Protection settings on memory
DWORD oldProtectTFM, myProtectTFM = PAGE_EXECUTE_READWRITE; //Protection settings on memory

char debugBuffer[128]; //Used for DbgView

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("ExtractData.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(ExtractDataDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		//pOrigCHAddress = (pCloseHandleA) //Get CloseHandleA pointer
		//	GetProcAddress(GetModuleHandle("kernel32.dll"), "CloseHandle");
		//if(pOrigCHAddress != NULL)
		//	BeginRedirectCH(MyCloseHandleA);	//Valid? Redirect

		pOrigTFMAddress = (pD3DXCreateTextureFromFileInMemoryEx) //Get D3DXCreateTextureFromFileInMemoryEx pointer
			GetProcAddress(GetModuleHandle("d3dx9_42.dll"), "D3DXCreateTextureFromFileInMemoryEx");
		if(pOrigTFMAddress != NULL)
		{
			MessageBoxA(NULL, "found D3DXCreateTextureFromFileInMemoryEx", "Infor", 0L);
			BeginRedirectTFM(MyD3DXCreateTextureFromFileInMemoryEx);	//Valid? Redirect
		}
		new CDynLinkLibrary(ExtractDataDLL);

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("ExtractData.DLL Terminating!\n");

//		memcpy(pOrigCHAddress, oldBytesCH, SIZE);
		memcpy(pOrigTFMAddress, oldBytesTFM, SIZE);
		// Terminate the library before destructors are called
		AfxTermExtensionModule(ExtractDataDLL);
	}
	return 1;   // ok
}

void BeginRedirectCH(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigCHAddress: %x", pOrigCHAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPCH, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigCHAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigCHAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectCH);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesCH, pOrigCHAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesCH[0], oldBytesCH[1],
		oldBytesCH[2], oldBytesCH[3], oldBytesCH[4], oldBytesCH[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPCH[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPCH[0], JMPCH[1],
		JMPCH[2], JMPCH[3], JMPCH[4], JMPCH[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigCHAddress, JMPCH, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigCHAddress, SIZE, oldProtectCH, NULL); //Change setts back
}

void BeginRedirectTFM(LPVOID newFunction)
{
	sprintf_s(debugBuffer, 128, "pOrigTFMAddress: %x", pOrigTFMAddress);
	OutputDebugString(debugBuffer);
	BYTE tempJMP[SIZE] = {0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3}; //JMP <NOP> RET for now
	memcpy(JMPTFM, tempJMP, SIZE); //Copy into global for convenience later
	DWORD JMPSize = ((DWORD)newFunction - (DWORD)pOrigTFMAddress - 5); //Get address difference
	VirtualProtect((LPVOID)pOrigTFMAddress, SIZE, PAGE_EXECUTE_READWRITE, &oldProtectTFM);
	//Change memory settings to make sure we can write the JMP in
	memcpy(oldBytesTFM, pOrigTFMAddress, SIZE); //Copy old bytes before writing JMP
	sprintf_s(debugBuffer, 128, "Old bytes: %x%x%x%x%x", oldBytesTFM[0], oldBytesTFM[1],
		oldBytesTFM[2], oldBytesTFM[3], oldBytesTFM[4], oldBytesTFM[5]);
	OutputDebugString(debugBuffer);
	memcpy(&JMPTFM[1], &JMPSize, 4); //Write the address to JMP to
	sprintf_s(debugBuffer, 128, "JMP: %x%x%x%x%x", JMPTFM[0], JMPTFM[1],
		JMPTFM[2], JMPTFM[3], JMPTFM[4], JMPTFM[5]);
	OutputDebugString(debugBuffer);
	memcpy(pOrigTFMAddress, JMPTFM, SIZE); //Write it in process memory
	VirtualProtect((LPVOID)pOrigTFMAddress, SIZE, oldProtectTFM, NULL); //Change setts back
}

BOOL    GetFileNameFromHandle(HANDLE hFile, TCHAR *pszFileName, const unsigned int uiMaxLen)
{
	pszFileName[0]=0;

	std::unique_ptr<BYTE[]> ptrcFni(new BYTE[_MAX_PATH * sizeof(TCHAR) + sizeof(FILE_NAME_INFO)]);
	FILE_NAME_INFO *pFni = reinterpret_cast<FILE_NAME_INFO *>(ptrcFni.get());

	BOOL b = GetFileInformationByHandleEx(hFile, 
		FileNameInfo,
		pFni,
		sizeof(FILE_NAME_INFO) + (_MAX_PATH * sizeof(TCHAR)) );
	if ( b )
	{
#ifdef  _UNICODE
		wcsncpy_s(pszFileName, 
			min(uiMaxLen, (pFni->FileNameLength / sizeof(pFni->FileName[0])) + 1 ), 
			pFni->FileName, 
			_TRUNCATE);
#else
		strncpy_s(pszFileName, 
			min(uiMaxLen, (pFni->FileNameLength / sizeof(pFni->FileName[0])) + 1), 
			CW2A(pFni->FileName), 
			_TRUNCATE);
#endif
	}
	return b;
}

BOOL  WINAPI MyCloseHandleA(HANDLE hobject)
{
	BOOL bRet;
	//--------------         CloseHandle          ------------------
	VirtualProtect((LPVOID)pOrigCHAddress, SIZE, myProtectCH, NULL); //ReadWrite again
	memcpy(pOrigCHAddress, oldBytesCH, SIZE); //Unhook API

	char pszFileName[MAX_PATH] = {0};
	GetFileNameFromHandle(hobject, pszFileName, MAX_PATH);
	bRet = CloseHandle(hobject);
	int nLen = strlen(pszFileName);
	if(nLen > 0 && pszFileName[nLen-1] == '_' && pszFileName[nLen-2] == '_')
	{
		if(::DeleteFile(pszFileName))
			;
			//WriteLog(pszFileName, CLOSEHANDLE);
		else
			;//WriteLog(pszFileName, DELETE_ERROR);
	}
	//--------------         CloseHandle          ------------------
	memcpy(pOrigCHAddress, JMPCH, SIZE); //Rehook API
	VirtualProtect((LPVOID)pOrigCHAddress, SIZE, oldProtectCH, NULL); //Normal setts

	return bRet; //Return what should be returned
}

HRESULT  WINAPI MyD3DXCreateTextureFromFileInMemoryEx(
	_In_    LPDIRECT3DDEVICE9  pDevice,
	_In_    LPCVOID            pSrcData,
	_In_    UINT               SrcDataSize,
	_In_    UINT               Width,
	_In_    UINT               Height,
	_In_    UINT               MipLevels,
	_In_    DWORD              Usage,
	_In_    D3DFORMAT          Format,
	_In_    D3DPOOL            Pool,
	_In_    DWORD              Filter,
	_In_    DWORD              MipFilter,
	_In_    D3DCOLOR           ColorKey,
	_Inout_ D3DXIMAGE_INFO     *pSrcInfo,
	_Out_   PALETTEENTRY       *pPalette,
	_Out_   LPDIRECT3DTEXTURE9 *ppTexture
	)
{
	HRESULT hRet = 0;
	//--------------         D3DXCreateTextureFromFileInMemoryEx          ------------------
	VirtualProtect((LPVOID)pOrigTFMAddress, SIZE, myProtectTFM, NULL); //ReadWrite again
	memcpy(pOrigTFMAddress, oldBytesTFM, SIZE); //Unhook API

	hRet = D3DXCreateTextureFromFileInMemoryEx(pDevice,
		pSrcData,
		SrcDataSize,
		Width,
		Height,
		MipLevels,
		Usage,
		Format,
		Pool,
		Filter,
		MipFilter,
		ColorKey,
		pSrcInfo,
		pPalette,
		ppTexture
		);

	//--------------         D3DXCreateTextureFromFileInMemoryEx          ------------------
	memcpy(pOrigTFMAddress, JMPTFM, SIZE); //Rehook API
	VirtualProtect((LPVOID)pOrigTFMAddress, SIZE, oldProtectTFM, NULL); //Normal setts

	return hRet; //Return what should be returned
}


extern "C" BOOL WINAPI foo()
{
	return TRUE;
}