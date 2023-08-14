// Common.cpp : Defines the entry point for the console 
// application.
//
#include "stdafx.h"

#include <tchar.h>
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <iostream>
#include <sstream>

//------------ float to hex---------------
void ConvertFloatToHex(float f_value, PSTR pStr)
{
	uint8_t *float_data = (uint8_t*)&f_value;
	memcpy(pStr, float_data, 4);
}

//------------ hex to float---------------
float ConvertHexToFloat(PSTR pStr)
{
	uint32_t num;
	float f;
	sscanf(pStr, "%x", &num);  // assuming you checked input
	f = *((float*)&num);
	return f;
}

int Hex4ToAsc(PBYTE pBuf, PBYTE pStr)
{
	DWORD dwVal = (*(pBuf + 3) & 0xff);
	dwVal <<= 8;
	dwVal |= (*(pBuf + 2) & 0xff);
	dwVal <<= 8;
	dwVal |= (*(pBuf + 1) & 0xff);
	dwVal <<= 8;
	dwVal |= (*pBuf & 0xff);
	if( (*(pBuf + 3) & 0xff) != 0xff )
		ultoa(dwVal, (PTCHAR)pStr, 10);
	else
		ltoa(dwVal, (PTCHAR)pStr, 10);
	return strlen((PTCHAR)pStr);
}

void DeletePattern(PBYTE pbBuffer, PDWORD pdwLen, DWORD dwPos, DWORD dwSize)
{
	DWORD dwBackUpSize;
	PBYTE pbTmp;
	dwBackUpSize = *pdwLen - dwPos - dwSize;
	pbTmp = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp, pbBuffer + dwPos + dwSize, dwBackUpSize);
	memcpy(pbBuffer + dwPos, pbTmp, dwBackUpSize);
	free(pbTmp);
	*pdwLen -= dwSize;
}

void DeletePattern(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pbPos, DWORD dwSize)
{
	DWORD dwBackUpSize;
	PBYTE pbTmp;
	dwBackUpSize = *pdwLen - (pbPos - pbBuffer) - dwSize;
	pbTmp = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp, pbPos + dwSize, dwBackUpSize);
	memcpy(pbPos, pbTmp, dwBackUpSize);
	free(pbTmp);
	*pdwLen -= dwSize;
}

void InsertPattern(PBYTE pbBuffer, PDWORD pdwLen, DWORD dwPos, PBYTE pbBlock, DWORD dwSize)
{
	DWORD dwBackUpSize;
	PBYTE pbTmp;
	dwBackUpSize = *pdwLen - dwPos;
	pbTmp = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp, pbBuffer + dwPos, dwBackUpSize);
	memcpy(pbBuffer + dwPos, pbBlock, dwSize);
	memcpy(pbBuffer + dwPos + dwSize, pbTmp, dwBackUpSize);
	free(pbTmp);
	*pdwLen += dwSize;
}

void InsertPattern(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pbPos, PBYTE pbBlock, DWORD dwSize)
{
	DWORD dwBackUpSize;
	PBYTE pbTmp;
	dwBackUpSize = *pdwLen - (pbPos - pbBuffer);
	pbTmp = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp, pbPos, dwBackUpSize);
	memcpy(pbPos, pbBlock, dwSize);
	memcpy(pbPos + dwSize, pbTmp, dwBackUpSize);
	free(pbTmp);
	*pdwLen += dwSize;
}
