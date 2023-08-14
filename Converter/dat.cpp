// Dat.cpp : Defines the entry point for the console 
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
#include <sstream>
#include "common.h"

enum DAT_FORMAT
{
	DAT_1, DAT_2, DAT_3, DAT_4, DAT_5, DAT_6, DAT_7
};

DAT_FORMAT nDatFormat;

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

void DAT_RemoveFirstString(PBYTE pbBuf, PDWORD pdwLen)
{
	int nSize = *(pbBuf + 2);
	DWORD dwBackUpSize = *pdwLen - 3;
	PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp, pbBuf + 3, dwBackUpSize);
	*pdwLen -= 3;
	memcpy(pbBuf, pbTmp, dwBackUpSize);
	free(pbTmp);

	dwBackUpSize = *pdwLen - nSize - 8;
	pbTmp = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp, pbBuf + nSize + 8, dwBackUpSize);
	*(pbBuf + nSize) = ',';

	DWORD dwNumber;
	DWORD dwTmp;
	dwTmp = *(pbBuf + nSize + 7);
	dwTmp &= 0xff;
	dwTmp <<= 24;
	dwNumber = dwTmp;

	dwTmp = *(pbBuf + nSize + 6);
	dwTmp &= 0xff;
	dwTmp <<= 16;
	dwNumber |= dwTmp;

	dwTmp = *(pbBuf + nSize + 5);
	dwTmp &= 0xff;
	dwTmp <<= 8;
	dwNumber |= dwTmp;

	dwNumber |= *(pbBuf + nSize + 4);
	char str[MAX_PATH] = { 0 };
	itoa(dwNumber, str, 10);
	int nLen = strlen(str);
	memcpy(pbBuf + nSize + 1, str, nLen);
	memcpy(pbBuf + nSize + 1 + nLen, pbTmp, dwBackUpSize);
	free(pbTmp);
	*pdwLen -= 8;
	*pdwLen += nLen;
	*pdwLen += 1;
}

void DAT_MakeLR1(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2)
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

void DAT_MakeLR2(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2)
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

void DAT_Replace8(PBYTE pbBuffer, PDWORD pdwLen, BYTE b)
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

void DAT_Replace4(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2, BYTE b3, BYTE b4)
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

void DAT_Delete4(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2, BYTE b3, BYTE b4)
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

void DAT_Replace2(PBYTE pbBuffer, PDWORD pdwLen, BYTE b1, BYTE b2)
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



void DAT_Delete(PBYTE pbBuffer, PDWORD pdwLen, PBYTE Pattern)
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

DAT_FORMAT DAT_GetFormat(PBYTE pbBuffer, PDWORD pdwLen)
{
	TCHAR pbBuf[MAX_PATH] = { 0 }; 
	int nLen = *(pbBuffer + 2);
	int nSubLen = nLen - strlen("CGameResTable");
	memcpy(pbBuf, pbBuffer + 3 + strlen("CGameResTable"), nSubLen);
	CString str = CString(pbBuf);
	if(str.Compare("Model") == 0)
		return DAT_1;
	else if(str.Compare("Effect") == 0)
		return DAT_2;

	return DAT_3;
}

//For model
void DAT_Convert1(PBYTE pbBuffer, PDWORD pdwLen)
{
	//----------------Process first string
	DAT_RemoveFirstString(pbBuffer, pdwLen);

	//----------------------------------- Remove 0x0d0x0a and insert Number ---------------------
	DAT_MakeLR1(pbBuffer, pdwLen, 0x00, 0x00);
	DAT_MakeLR1(pbBuffer, pdwLen, 0x01, 0x00);
	DAT_MakeLR1(pbBuffer, pdwLen, 0x01, 0x01);
	DAT_MakeLR2(pbBuffer, pdwLen, 0x01, 0x00);
	DAT_MakeLR2(pbBuffer, pdwLen, 0x00, 0x00);

	DAT_Replace8(pbBuffer, pdwLen, 0x00);
	DAT_Replace8(pbBuffer, pdwLen, 0x01);
	DAT_Replace8(pbBuffer, pdwLen, 0x02);

	//DAT_Replace4(pbBuffer, pdwLen, 0xec, 0x51, 0xb8, 0x3f);

	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xac, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x2c);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3e);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3d);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3e);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3d);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x2c, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x0c, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x0c, 0x3f);

	//DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0xd9, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x99, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x99, 0x3e);
	//DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x59, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x19, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x9a, 0x99, 0x19, 0x3f);

	//DAT_Replace4(pbBuffer, pdwLen, 0x99, 0x3f, 0x2c, 0x2c);

	//DAT_Replace4(pbBuffer, pdwLen, 0x85, 0xeb, 0x31, 0x40);

	//DAT_Replace4(pbBuffer, pdwLen, 0x7b, 0x14, 0x2e, 0x3f);

	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0xf6, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0xe6, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0xa6, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x66, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x66, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x26, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x26, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x66, 0x66, 0x06, 0x40);

	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0xf3, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0xb3, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x93, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x73, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x73, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x53, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x33, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x33, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x33, 0x33, 0x13, 0x40);
	//
	//DAT_Replace4(pbBuffer, pdwLen, 0x29, 0x5c, 0x8f, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x29, 0x5c, 0x8f, 0x3e);

	//DAT_Replace4(pbBuffer, pdwLen, 0x14, 0xae, 0x87, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x0a, 0xd7, 0x23, 0x3c);

	//DAT_Replace4(pbBuffer, pdwLen, 0xf4, 0x01, 0x00, 0x00);

	//DAT_Replace4(pbBuffer, pdwLen, 0xe8, 0x03, 0x00, 0x00);

	//DAT_Replace4(pbBuffer, pdwLen, 0x1c, 0x01, 0x00, 0x00);

	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xf0, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xe0, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xe0, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc8, 0x42);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc8, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc0, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xc0, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xb0, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x90, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x90, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x3e);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x70, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x60, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x48, 0x42);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x3f);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x34, 0x42);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x20, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x20, 0x40);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x16, 0x43);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x10, 0x41);
	//DAT_Replace4(pbBuffer, pdwLen, 0x00, 0x00, 0x01, 0x01);

	DAT_Delete4(pbBuffer, pdwLen, 0xec, 0x51, 0xb8, 0x3f);

	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xac, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x2c);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3e);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0xcc, 0x3d);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x8c, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3e);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x4c, 0x3d);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x2c, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x0c, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0xcd, 0xcc, 0x0c, 0x3f);

	DAT_Delete4(pbBuffer, pdwLen, 0x9a, 0x99, 0xd9, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x9a, 0x99, 0x99, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x9a, 0x99, 0x99, 0x3e);
	DAT_Delete4(pbBuffer, pdwLen, 0x9a, 0x99, 0x59, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x9a, 0x99, 0x19, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x9a, 0x99, 0x19, 0x3f);

	DAT_Delete4(pbBuffer, pdwLen, 0x99, 0x3f, 0x2c, 0x2c);

	DAT_Delete4(pbBuffer, pdwLen, 0x85, 0xeb, 0x31, 0x40);

	DAT_Delete4(pbBuffer, pdwLen, 0x7b, 0x14, 0x2e, 0x3f);

	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0xf6, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0xe6, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0xa6, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0x66, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0x66, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0x26, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0x26, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x66, 0x66, 0x06, 0x40);

	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0xf3, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0xb3, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0x93, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0x73, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0x73, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0x53, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0x33, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0x33, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x33, 0x33, 0x13, 0x40);

	DAT_Delete4(pbBuffer, pdwLen, 0x29, 0x5c, 0x8f, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x29, 0x5c, 0x8f, 0x3e);

	DAT_Delete4(pbBuffer, pdwLen, 0x14, 0xae, 0x87, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x0a, 0xd7, 0x23, 0x3c);

	DAT_Delete4(pbBuffer, pdwLen, 0xf4, 0x01, 0x00, 0x00);

	DAT_Delete4(pbBuffer, pdwLen, 0xe8, 0x03, 0x00, 0x00);

	DAT_Delete4(pbBuffer, pdwLen, 0x1c, 0x01, 0x00, 0x00);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xf0, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xe0, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xe0, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xc8, 0x42);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xc8, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xc0, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xc0, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xb0, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0xa0, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x90, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x90, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x80, 0x3e);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x70, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x60, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x48, 0x42);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x40, 0x3f);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x34, 0x42);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x20, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x20, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x16, 0x43);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x10, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x01, 0x01);

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

	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x41);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x40);
	DAT_Delete4(pbBuffer, pdwLen, 0x00, 0x00, 0x00, 0x3f);

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
}

void DAT_FindNumber(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd, BOOL bBracket )
{
	BOOL fFound;
	BYTE Pattern[] = { 0x17, 0x03 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast;
	if(ppEnd == NULL)
		pBufferLast = pbBuffer + *pdwLen;
	else
		pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf;
	if(pStart == NULL)
		pbBuf = pbBuffer;
	else
		pbBuf = pStart;

	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound)
		{
			BYTE pTmp[MAX_PATH] = { 0 };
			int nInc = Hex4ToAsc(pOccurrence + dwPatternSize, pTmp);
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize + 4);
			pBufLast -= (dwPatternSize + 4);
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, nInc);
			pbBuf = pOccurrence + nInc;
			pBufLast += nInc;

			if(bBracket)
			{
				if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
					pBufLast -= 3;
					pbBuf -= 3;
					*pTmp = ',';
					InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
					pBufLast += 1;
					pbBuf += 1;
				}
				else //if last
				{
					*pTmp = ')';
					InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
					nInc++;
					pbBuf += 1;
					pBufLast += 1;
				}
			}
			else
				pbBuf += dwPatternSize;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
	if(ppEnd != NULL)
		*ppEnd = pBufLast;
}

void DAT_FindBracketForLong(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x17, 0x03 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = '(';
			int nInc = Hex4ToAsc(pOccurrence + dwPatternSize, pTmp + 1);
			nInc++;

			DeletePattern(pbBuffer, pdwLen, pOccurrence, 8);
			pBufLast -= 8;
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, nInc);
			pBufLast += nInc;
			pbBuf += nInc;

			if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
				pBufLast -= 3;
				pbBuf -= 3;
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				pBufLast += 1;
				pbBuf += 1;
			}
			else //if last
			{
				*pTmp = ')';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				nInc++;
				pbBuf += 1;
				pBufLast += 1;
			}
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

//      EffParam1*2,1
void DAT_FindInBracketPowerComma(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x08, 0x1a, 0x19, 0x07 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			nIncStr[idStr] = *pOccurrence & 0x7f;
			memcpy(pstrStr[idStr], pOccurrence + 1, nIncStr[idStr]);
			DeletePattern(pbBuffer, pdwLen, pOccurrence, 1 + nIncStr[idStr]);
			pBufLast -= (1 + nIncStr[idStr]);
			idStr += 1;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 2 && idFloat == 0 && idStr == 1)
			{
				BYTE pTmp1[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			pbBuf = pbTmp;
		}

		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
	*ppEnd = pBufLast;
}


void DAT_FindBracketForFloat(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x17, 0x05 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = '(';
			char pTmp1[MAX_PATH] = { 0 };
			DWORD dwTmp1 = *((PDWORD)(pOccurrence + sizeof(Pattern)));
			float f = *((float*)&dwTmp1);
			std::ostringstream ss;
			ss << f ;
			std::string s(ss.str());
			int nInc = s.length();
			memcpy(pTmp + 1, s.c_str(), nInc);
			nInc++;

			DeletePattern(pbBuffer, pdwLen, pOccurrence, 8);
			pBufLast -= 8;
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, nInc);

			pbBuf = pOccurrence + nInc;
			pBufLast += nInc;

			if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
				pBufLast -= 3;
				pbBuf -= 3;
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				pBufLast += 1;
				pbBuf += 1;
			}
			else //if last
			{
				*pTmp = ')';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				nInc++;
				pbBuf += 1;
				pBufLast += 1;
			}
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindFloat(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x17, 0x05 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			BYTE pTmp[MAX_PATH] = { 0 };
			char pTmp1[MAX_PATH] = { 0 };
			DWORD dwTmp1 = *((PDWORD)(pOccurrence + dwPatternSize));
			float f = *((float*)&dwTmp1);
			std::ostringstream ss;
			ss << f ;
			std::string s(ss.str());
			int nInc = s.length();
			memcpy(pTmp, s.c_str(), nInc);

			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize + 4);
			pBufLast -= (dwPatternSize + 4);
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, nInc);
			pbBuf = pOccurrence + nInc;
			pBufLast += nInc;

			if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
				pBufLast -= 3;
				pbBuf -= 3;
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				pBufLast += 1;
				pbBuf += 1;
			}
			else //if last
			{
				*pTmp = ')';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				nInc++;
				pbBuf += 1;
				pBufLast += 1;
			}

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}


void DAT_FindQuoteForString(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x17, 0x06 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast;
	if(ppEnd != NULL)
		pBufferLast = *ppEnd;
	else
		pBufferLast = pbBuffer + *pdwLen;

	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf;
	if(pStart != NULL)
		pbBuf = pStart;
	else
		pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			int nSubStrLen = (*(pOccurrence + dwPatternSize) & 0x7f);
			BYTE pTmp[MAX_PATH] = { 0 };
			sprintf((PCHAR)pTmp, "(\"\"");
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize + 1);
			pBufLast -= (dwPatternSize + 1);
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, 3);
			pBufLast += 3;
			pbBuf = pOccurrence +  nSubStrLen + 3;
			sprintf((PCHAR)pTmp, "\"\"");
			InsertPattern(pbBuffer, pdwLen, pOccurrence + 3 + nSubStrLen, pTmp, 2);
			pBufLast += 2;
			pbBuf += 2;
			
			int nInc = nSubStrLen + 5;
			if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
				pBufLast -= 3;
				pbBuf -= 3;
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				pBufLast += 1;
				pbBuf += 1;
			}
			else //if last
			{
				*pTmp = ')';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				nInc++;
				pbBuf += 1;
				pBufLast += 1;
			}
			
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	if(ppEnd != NULL)
		*ppEnd = pBufLast;
}

void DAT_FindString(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x19, 0x07 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwPos = pOccurrence - pbBuffer;
			int nSubStrLen = (*(pOccurrence + dwPatternSize) & 0x7f);
			BYTE pTmp[MAX_PATH] = { 0 };
			DeletePattern(pbBuffer, pdwLen, dwPos, dwPatternSize + 1);
			pBufLast -= (dwPatternSize + 1);
			pbBuf = pOccurrence +  nSubStrLen;

			int nInc = nSubStrLen;
			if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
				pBufLast -= 3;
				pbBuf -= 3;
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				pBufLast += 1;
				pbBuf += 1;
			}
			else //if last
			{
				*pTmp = ')';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				nInc++;
				pbBuf += 1;
				pBufLast += 1;
			}
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindBracketForString1(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x19, 0x07 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DWORD dwPos = pOccurrence - pbBuffer;
			int nSubStrLen = (*(pOccurrence + dwPatternSize) & 0x7f);
			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = '(';
			DeletePattern(pbBuffer, pdwLen, dwPos, dwPatternSize + 1);
			pBufLast -= (dwPatternSize + 1);
			InsertPattern(pbBuffer, pdwLen, dwPos, pTmp, 1);
			pBufLast += 1;
			pbBuf = pOccurrence +  nSubStrLen + 1;

			int nInc = nSubStrLen + 1;
			if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
				pBufLast -= 3;
				pbBuf -= 3;
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				pBufLast += 1;
				pbBuf += 1;
			}
			else //if last
			{
				*pTmp = ')';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				nInc++;
				pbBuf += 1;
				pBufLast += 1;
			}
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindBracketForString2(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x18, 0x07 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			int nSubStrLen = (*(pOccurrence + dwPatternSize) & 0x7f);
			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = '(';
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize + 1);
			pBufLast -= (dwPatternSize + 1);
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, 1);
			pBufLast += 1;
			pbBuf = pOccurrence +  nSubStrLen + 1;

			int nInc = nSubStrLen + 1;
			if( *(pOccurrence + nInc) == 0x1a && *(pOccurrence + nInc + 1) == 0x00 && *(pOccurrence + nInc + 2) == 0x00) //if comma
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 3);
				pBufLast -= 3;
				pbBuf -= 3;
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
				pBufLast += 1;
				pbBuf += 1;
			}
			else //if last
			{
				if( *(pOccurrence + nInc) == 0x00 ) // if exist 00
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, 1);
					pBufLast -= 1;
					pbBuf -= 1;
					nInc -= 1;
					
					//find semicolon and insert ')'
					*pTmp = ')';
					BYTE Pattern1[] = { 0x00, 0x00, 0x00 };
					PBYTE pOccurrence1 = std::search(pOccurrence + nInc, pBufLast, Pattern1, Pattern1 + sizeof(Pattern1));
					if(pOccurrence1 != pBufLast)
						InsertPattern(pbBuffer, pdwLen, pOccurrence1, pTmp, 1);
					else
						InsertPattern(pbBuffer, pdwLen, pOccurrence1 - 1, pTmp, 1);

					pBufLast += 1;
					pbBuf += 1;
					nInc += 1;
				}
				else //normal
				{
					*pTmp = ')';
					InsertPattern(pbBuffer, pdwLen, pOccurrence + nInc, pTmp, 1);
					nInc++;
					pbBuf += 1;
					pBufLast += 1;

				}
			}
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindBracket1Divide(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x09, 0x00, 0x19, 0x07 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			BYTE pTmp[MAX_PATH] = { 0 };
			int nSubStrLen = (*(pOccurrence + dwPatternSize) & 0x7f);
			*pTmp = '(';
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize + 1);
			pBufLast -= (dwPatternSize + 1);
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, 1);
			pBufLast += 1;
			pbBuf = pOccurrence + nSubStrLen + 1;

			*pTmp = '/';
			InsertPattern(pbBuffer, pdwLen, pOccurrence + nSubStrLen + 1, pTmp, 1);
			pBufLast += 1;
			pbBuf += 1;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

//    (-(MaxHp*0.05))
void DAT_FindBrakcketNegative1PowerInBracket(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x03, 0x00, 0x08, 0x00 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 0 && idFloat == 1 && idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

//  (3+(SelfPetLevel(5)-10)+(SelfPetLevel(5)/5.0)*(MasterLevel/5.0)*6)
//	(9+3*(SelfPetLevel(5)-10)+SelfPetLevel(5)*MasterLevel*0.72)
void DAT_FindBracket2Plus2Power(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x0b, 0x00, 0x0b, 0x08 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[2] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[2], pOccurrence + 3, nIncStr[2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[2]);
					pBufLast -= (3 + nIncStr[2]);
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}
			if(idNum == 5 && idFloat == 2 && idStr == 2)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[4], nIncNum[4]);
				pBufLast += nIncNum[4];
				pbTmp += nIncNum[4];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[2], nIncStr[2]);
				pBufLast += nIncStr[2];
				pbTmp += nIncStr[2];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[1], nIncFloat[1]);
				pBufLast += nIncFloat[1];
				pbTmp += nIncFloat[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			else if(idNum == 5 && idFloat == 1 && idStr == 2)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[2], nIncStr[2]);
				pBufLast += nIncStr[2];
				pbTmp += nIncStr[2];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			if( pbTmp >= pBufLast)
				break;

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindBracket2Plus2Power1Divide(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x09, 0x00, 0x0b };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[2] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[2], pOccurrence + 3, nIncStr[2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[2]);
					pBufLast -= (3 + nIncStr[2]);
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}
			if(idNum == 6 && idFloat == 2 && idStr == 2)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[4], nIncNum[4]);
				pBufLast += nIncNum[4];
				pbTmp += nIncNum[4];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[5], nIncNum[5]);
				pBufLast += nIncNum[5];
				pbTmp += nIncNum[5];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[2], nIncStr[2]);
				pBufLast += nIncStr[2];
				pbTmp += nIncStr[2];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[1], nIncFloat[1]);
				pBufLast += nIncFloat[1];
				pbTmp += nIncFloat[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			else if(idNum == 6 && idFloat == 1 && idStr == 2)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[4], nIncNum[4]);
				pBufLast += nIncNum[4];
				pbTmp += nIncNum[4];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[5], nIncNum[5]);
				pBufLast += nIncNum[5];
				pbTmp += nIncNum[5];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[2], nIncStr[2]);
				pBufLast += nIncStr[2];
				pbTmp += nIncStr[2];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;


				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			if( pbTmp >= pBufLast)
				break;

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}


//  30+SelfPetLevel(3)*15+SelfPetLevel(3)*MasterBaseMp*0.01
void DAT_FindInBracket2Plus3Power(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x0b, 0x00, 0x0b, 0x08 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[2] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[2], pOccurrence + 3, nIncStr[2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[2]);
					pBufLast -= (3 + nIncStr[2]);
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 4 && idFloat == 1 && idStr == 2)
			{
				BYTE pTmp1[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[2], nIncStr[2]);
				pBufLast += nIncStr[2];
				pbTmp += nIncStr[2];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

//	2+(SelfPetLevel(9)*1.5+SelfPetLevel(9)*MasterBaseMp*0.001)/2
//  2+(SelfPetLevel(2)+SelfPetLevel(2)*MasterBaseHp*0.001)/2

void DAT_FindInBracket2Plus3Power1Divide(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x0b, 0x00 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[2] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[2], pOccurrence + 3, nIncStr[2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[2]);
					pBufLast -= (3 + nIncStr[2]);
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 4 && idFloat == 2 && idStr == 2)
			{
				BYTE pTmp1[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[2], nIncStr[2]);
				pBufLast += nIncStr[2];
				pbTmp += nIncStr[2];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[1], nIncFloat[1]);
				pBufLast += nIncFloat[1];
				pbTmp += nIncFloat[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			else if(idNum == 4 && idFloat == 1 && idStr == 2)
			{
				BYTE pTmp1[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[2], nIncStr[2]);
				pBufLast += nIncStr[2];
				pbTmp += nIncStr[2];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindBracket1Power1Divide(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x09, 0x00, 0x08 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 1 && idFloat == 1 && idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];


				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindBracket1Divide1Power(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x08, 0x00, 0x09 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[20][MAX_PATH] = { 0 };
			BYTE pstrStr[20][MAX_PATH] = { 0 };
			BYTE pstrFloat[20][MAX_PATH] = { 0 };
			int nIncNum[20];
			int nIncStr[20];
			int nIncFloat[20];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 0 && idFloat == 2 && idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[1], nIncFloat[1]);
				pBufLast += nIncFloat[1];
				pbTmp += nIncFloat[1];


				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindIn1Divide1Power(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x08, 0x00, 0x09 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[20][MAX_PATH] = { 0 };
			BYTE pstrStr[20][MAX_PATH] = { 0 };
			BYTE pstrFloat[20][MAX_PATH] = { 0 };
			int nIncNum[20];
			int nIncStr[20];
			int nIncFloat[20];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 1 && idFloat == 1 && idStr == 1)
			{
				BYTE pTmp1[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];


				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];
			}
			else if(idNum == 0 && idFloat == 2 && idStr == 1)
			{
				BYTE pTmp1[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[1], nIncFloat[1]);
				pBufLast += nIncFloat[1];
				pbTmp += nIncFloat[1];


				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];
			}

			BYTE pTmp1[1];
			*pTmp1 = ')';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
			pBufLast += 1;
			
			pbTmp += 1;
			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindBracketMinus1Divide(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x09, 0x00, 0x03 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idNum == 1 && idFloat == 0 && idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}


void DAT_FindBracket1Plus1Minus(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x0b, 0x00 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}
			if(idNum == 3 && idFloat == 0 && idStr == 1)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			else if(idNum == 4 && idFloat == 0 && idStr == 1)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[3], nIncNum[3]);
				pBufLast += nIncNum[3];
				pbTmp += nIncNum[3];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[2], nIncNum[2]);
				pBufLast += nIncNum[2];
				pbTmp += nIncNum[2];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			else if(idNum == 1 && idFloat == 1 && idStr == 1)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

			}


			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}


	//((Level*Level*13)/5,0)
	//((Level*Level*13)/10,0)
	//((Level*Level*(Level/10)*(Level/10))/10,0)
	//((Level*Level*(Level/10)*(Level/10))/10/2,0)
	//((Level*Level*13+1000)/15,1)
	//((Level*Level*13+1000)/15/2,1)
	//((Level*Level*Level)/30,1)
	//((Level*Level*Level)/30/2,1)
void DAT_FindBracket2Power1Divide(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x09, 0x1a };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;

			typedef struct st_var_str
			{
				BYTE strStr[MAX_PATH];
				int nlenStr;
				BOOL bNumExist;
				BYTE strNum[MAX_PATH];
				int nlenNum;
			} ST_VAR_STR, *PST_VAR_STR;

			typedef struct st_var_num
			{
				BYTE strNum[MAX_PATH];
				int nlenNum;
				BOOL bNumExist;
			} ST_VAR_NUM, *PST_VAR_NUM;

			ST_VAR_STR stStr[20];
			ST_VAR_NUM stNum[20];
			ST_VAR_NUM stNum08[20];
			ST_VAR_NUM stNum09[20];
			ST_VAR_NUM stNum0b[20];
			int idStr = 0;
			int idLastStr = 0;
			int idNum = 0;
			int idNum08 = 0;
			int idNum09 = 0;
			int idNum0b = 0;

			for(int i = 0 ;i < 20; i++)
			{
				memset(stStr[i].strStr, 0, MAX_PATH);
				stStr[i].nlenStr = 0;
				stStr[i].bNumExist = FALSE;
				memset(stStr[i].strNum, 0, MAX_PATH);
				stStr[i].nlenNum = 0;

				memset(stNum[i].strNum, 0, MAX_PATH);
				stNum[i].nlenNum = 0;
				stNum[i].bNumExist = FALSE;

				memset(stNum08[i].strNum, 0, MAX_PATH);
				stNum08[i].nlenNum = 0;
				stNum08[i].bNumExist = FALSE;

				memset(stNum09[i].strNum, 0, MAX_PATH);
				stNum09[i].nlenNum = 0;
				stNum09[i].bNumExist = FALSE;

				memset(stNum0b[i].strNum, 0, MAX_PATH);
				stNum0b[i].nlenNum = 0;
				stNum0b[i].bNumExist = FALSE;
			}
			
			while( 1 )
			{
				if( *pbTmp == 0x0b && *(pbTmp + 1) == 0x17 && *(pbTmp + 2)== 0x03 )
				{
					stNum0b[idNum0b].bNumExist = TRUE;
					stNum0b[idNum0b].nlenNum= Hex4ToAsc(pOccurrence + 3, stNum0b[idNum0b].strNum);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + 4);
					pBufLast -= (3 + 4);
					idNum0b++;
				}
				if( *pbTmp == 0x09 && *(pbTmp + 1) == 0x17 && *(pbTmp + 2)== 0x03 )
				{
					stNum09[idNum09].bNumExist = TRUE;
					stNum09[idNum09].nlenNum= Hex4ToAsc(pOccurrence + 3, stNum09[idNum09].strNum);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + 4);
					pBufLast -= (3 + 4);
					idNum09++;
				}
				if( *pbTmp == 0x08 && *(pbTmp + 1) == 0x17 && *(pbTmp + 2)== 0x03 )
				{
					stNum08[idNum08].bNumExist = TRUE;
					stNum08[idNum08].nlenNum= Hex4ToAsc(pOccurrence + 3, stNum08[idNum08].strNum);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + 4);
					pBufLast -= (3 + 4);
					idNum08++;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					stNum[idNum].nlenNum= Hex4ToAsc(pOccurrence + 2, stNum[idNum].strNum);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					stStr[idStr].nlenStr = *(pOccurrence + 2) & 0x7f;
					memcpy(stStr[idStr].strStr, pOccurrence + 3, stStr[idStr].nlenStr);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + stStr[idStr].nlenStr);
					pBufLast -= (3 + stStr[idStr].nlenStr);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						stStr[idStr].bNumExist = TRUE;
						stStr[idStr].nlenNum = Hex4ToAsc(pOccurrence + 2, stStr[idStr].strNum);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
					}
					idStr += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

			}

			if( idNum0b == 0 && idNum08 == 2 )
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				for( int i = 1 ;i < idStr; i++)
				{
					BOOL bI = FALSE;
					if(i == 0)
						bI = TRUE;

					if(stStr[i].bNumExist)
					{
						*pTmp1 = '(';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;
					}

					InsertPattern(pbBuffer, pdwLen, pbTmp, stStr[i].strStr, stStr[i].nlenStr);
					pBufLast += stStr[i].nlenStr;
					pbTmp += stStr[i].nlenStr;

					if(stStr[i].bNumExist)
					{
						*pTmp1 = '/';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;

						InsertPattern(pbBuffer, pdwLen, pbTmp, stStr[i].strNum, stStr[i].nlenNum);
						pBufLast += stStr[i].nlenNum;
						pbTmp += stStr[i].nlenNum;
					}

					if(stStr[i].bNumExist)
					{
						*pTmp1 = ')';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;
					}

					if( i == idStr - 1)
						i = -1;
					if(bI)
						break;

					*pTmp1 = '*';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;
				}

				if(stNum08[1].bNumExist)
				{
					*pTmp1 = '*';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;

					InsertPattern(pbBuffer, pdwLen, pbTmp, stNum08[1].strNum, stNum08[1].nlenNum);
					pBufLast += stNum08[1].nlenNum;
					pbTmp += stNum08[1].nlenNum;
				}

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				if(stNum08[0].bNumExist)
				{
					*pTmp1 = '/';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;

					InsertPattern(pbBuffer, pdwLen, pbTmp, stNum08[0].strNum, stNum08[0].nlenNum);
					pBufLast += stNum08[0].nlenNum;
					pbTmp += stNum08[0].nlenNum;
				}

				for( int i = 0; i < idNum09; i++)
				{
					if(stNum09[i].bNumExist)
					{
						*pTmp1 = '/';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;

						InsertPattern(pbBuffer, pdwLen, pbTmp, stNum09[i].strNum, stNum09[i].nlenNum);
						pBufLast += stNum09[i].nlenNum;
						pbTmp += stNum09[i].nlenNum;
					}
				}

				*pTmp1 = ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, stNum[0].strNum, stNum[0].nlenNum);
				pBufLast += stNum[0].nlenNum;
				pbTmp += stNum[0].nlenNum;

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			else if( idNum0b == 0 && idNum08 == 1 )
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				for( int i = 1 ;i < idStr; i++)
				{
					BOOL bI = FALSE;
					if(i == 0)
						bI = TRUE;

					if(stStr[i].bNumExist)
					{
						*pTmp1 = '(';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;
					}

					InsertPattern(pbBuffer, pdwLen, pbTmp, stStr[i].strStr, stStr[i].nlenStr);
					pBufLast += stStr[i].nlenStr;
					pbTmp += stStr[i].nlenStr;

					if(stStr[i].bNumExist)
					{
						*pTmp1 = '/';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;

						InsertPattern(pbBuffer, pdwLen, pbTmp, stStr[i].strNum, stStr[i].nlenNum);
						pBufLast += stStr[i].nlenNum;
						pbTmp += stStr[i].nlenNum;
					}

					if(stStr[i].bNumExist)
					{
						*pTmp1 = ')';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;
					}

					if( i == idStr - 1)
						i = -1;
					if(bI)
						break;

					*pTmp1 = '*';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;
				}

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				for( int i = 0; i < idNum08; i++)
				{
					if(stNum08[i].bNumExist)
					{
						*pTmp1 = '/';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;

						InsertPattern(pbBuffer, pdwLen, pbTmp, stNum08[i].strNum, stNum08[i].nlenNum);
						pBufLast += stNum08[i].nlenNum;
						pbTmp += stNum08[i].nlenNum;
					}
				}

				for( int i = 0; i < idNum09; i++)
				{
					if(stNum09[i].bNumExist)
					{
						*pTmp1 = '/';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;

						InsertPattern(pbBuffer, pdwLen, pbTmp, stNum09[i].strNum, stNum09[i].nlenNum);
						pBufLast += stNum09[i].nlenNum;
						pbTmp += stNum09[i].nlenNum;
					}
				}

				*pTmp1 = ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, stNum[0].strNum, stNum[0].nlenNum);
				pBufLast += stNum[0].nlenNum;
				pbTmp += stNum[0].nlenNum;

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			else if( idNum0b == 1)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				for( int i = 1 ;i < idStr; i++)
				{
					BOOL bI = FALSE;
					if(i == 0)
						bI = TRUE;

					if(stStr[i].bNumExist)
					{
						*pTmp1 = '(';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;
					}

					InsertPattern(pbBuffer, pdwLen, pbTmp, stStr[i].strStr, stStr[i].nlenStr);
					pBufLast += stStr[i].nlenStr;
					pbTmp += stStr[i].nlenStr;

					if(stStr[i].bNumExist)
					{
						*pTmp1 = '/';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;

						InsertPattern(pbBuffer, pdwLen, pbTmp, stStr[i].strNum, stStr[i].nlenNum);
						pBufLast += stStr[i].nlenNum;
						pbTmp += stStr[i].nlenNum;
					}

					if(stStr[i].bNumExist)
					{
						*pTmp1 = ')';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;
					}

					if( i == idStr - 1)
						i = -1;
					if(bI)
						break;

					*pTmp1 = '*';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;
				}

				if(stNum08[1].bNumExist)
				{
					*pTmp1 = '*';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;

					InsertPattern(pbBuffer, pdwLen, pbTmp, stNum08[1].strNum, stNum08[1].nlenNum);
					pBufLast += stNum08[1].nlenNum;
					pbTmp += stNum08[1].nlenNum;
				}

				if(stNum08[0].bNumExist)
				{
					*pTmp1 = '+';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;

					InsertPattern(pbBuffer, pdwLen, pbTmp, stNum08[0].strNum, stNum08[0].nlenNum);
					pBufLast += stNum08[0].nlenNum;
					pbTmp += stNum08[0].nlenNum;
				}

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				if(stNum0b[0].bNumExist)
				{
					*pTmp1 = '/';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
					pBufLast += 1;
					pbTmp += 1;

					InsertPattern(pbBuffer, pdwLen, pbTmp, stNum0b[0].strNum, stNum0b[0].nlenNum);
					pBufLast += stNum0b[0].nlenNum;
					pbTmp += stNum0b[0].nlenNum;
				}

				for( int i = 0; i < idNum09; i++)
				{
					if(stNum09[i].bNumExist)
					{
						*pTmp1 = '/';
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
						pBufLast += 1;
						pbTmp += 1;

						InsertPattern(pbBuffer, pdwLen, pbTmp, stNum09[i].strNum, stNum09[i].nlenNum);
						pBufLast += stNum09[i].nlenNum;
						pbTmp += stNum09[i].nlenNum;
					}
				}

				*pTmp1 = ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, stNum[0].strNum, stNum[0].nlenNum);
				pBufLast += stNum[0].nlenNum;
				pbTmp += stNum[0].nlenNum;

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}


//-----------------------------(MaxHp*0.05)-----------------------------------------
void DAT_FindBracket1Power(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x08, 0x00 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

			}
			if(idNum == 0 && idFloat == 1 && idStr == 1)
			{
				BYTE pTmp1[1];
				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

//    (-EffParam2,-EffParam2/100.0)
//    (-EffParam1,-EffParam2)
void DAT_FindBracket2NegativeVar(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x03, 0x1a };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncStr[10];
			int nIncFloat[10];
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}
			if(idFloat == 1 && idStr == 2)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

			}
			else if(idFloat == 0 && idStr == 2)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

//   -EffParam1/100.0
void DAT_FindInBracketNegative1Divide(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x09, 0x00, 0x03 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncStr[10];
			int nIncFloat[10];
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}
			if(idFloat == 1 && idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

			}
			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

//   -EffParam1
void DAT_FindInBracketNegativeVar(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x03, 0x00, 0x19, 0x07 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;

			BYTE pstrStr[10][MAX_PATH] = { 0 };
			int nIncStr[10];
			int idStr = 0;

			nIncStr[idStr] = *pOccurrence & 0x7f;
			memcpy(pstrStr[idStr], pOccurrence + 1, nIncStr[idStr]);
			DeletePattern(pbBuffer, pdwLen, pOccurrence, 1 + nIncStr[idStr]);
			pBufLast -= (1 + nIncStr[idStr]);
			idStr += 1;

			if(idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

			}
			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}


//   (-EffParam1)
void DAT_FindBracketNegativeVar(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStart, PBYTE* ppEnd)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x1a, 0x03, 0x00 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = *ppEnd;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pStart;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncStr[10];
			int nIncFloat[10];
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			if(idFloat == 0 && idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

			}

			pbBuf = pbTmp;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	*ppEnd = pBufLast;
}

void DAT_FindPower(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x08, 0x19, 0x07 
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
			BYTE pTmp[MAX_PATH] = { 0 };
			int nSubStrLen = (*(pOccurrence + dwPatternSize) & 0x7f);
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize + 1);
			pBufLast -= (dwPatternSize + 1);
			pbBuf = pOccurrence + nSubStrLen;

			*pTmp = '*';
			InsertPattern(pbBuffer, pdwLen, pOccurrence + nSubStrLen, pTmp, 1);
			pBufLast += 1;
			pbBuf += 1;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

// EffParam2*3600*1000
void DAT_Find2Power(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x08, 0x08
	};

	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		if( !((*(pOccurrence + dwPatternSize) == 0x17 && *(pOccurrence + dwPatternSize + 1) == 0x03) || (*(pOccurrence + dwPatternSize) == 0x17 && *(pOccurrence + dwPatternSize + 1) == 0x05)) )
		{
			pbBuf += dwPatternSize;
			if( pbBuf >= pBufLast)
				break;
			continue;
		}
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int idNum = 0;
			int idStr = 0;
			while( 1 )
			{
				if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum += 1;
				}
				else if(*pbTmp == 0x19 && *(pbTmp + 1)== 0x07)
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if( !(*pbTmp == 0x0d && *(pbTmp + 1)== 0x0a) )
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

				if( (*pbTmp == 0x80 && *(pbTmp + 1) == 0x80) ||  (*pbTmp & 0x7f && *(pbTmp + 1) == 0x01 && *(pbTmp + 3) == 0x07) || (*pbTmp == 0x80 && *(pbTmp + 2) == 0x01 && *(pbTmp + 4) == 0x07) )
					break;

			}
			
			int i;
			for(i = idStr - 1; i >= 0; i--)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[i], nIncStr[i]);
				pBufLast += nIncStr[i];
				pbTmp += nIncStr[i];
				
				BYTE pTmp[MAX_PATH] = { 0 };
				*pTmp = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;
			}

			for(i = idNum - 1; i >= 0; i--)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[i], nIncNum[i]);
				pBufLast += nIncNum[i];
				pbTmp += nIncNum[i];

				if(i == 0) break;
				BYTE pTmp[MAX_PATH] = { 0 };
				*pTmp = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//  	(NpcId=3636||NpcId=3637)&&SelfTypeEffect(2072)
//		(NpcId=3508||NpcId=3509||NpcId=3510||NpcId=3519||NpcId=3520||NpcId=3521||NpcId=3522||NpcId=3523||NpcId=3524)&&SelfTypeEffect(2072)
void DAT_FindSomeOR1AND(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0x13, 0x14 };
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum1[10][MAX_PATH] = { 0 };
			BYTE pstrNum2[10][MAX_PATH] = { 0 };
			BYTE pstrStr1[10][MAX_PATH] = { 0 };
			BYTE pstrStr2[10][MAX_PATH] = { 0 };
			int nIncNum1[10];
			int nIncNum2[10];
			int nIncStr1[10];
			int nIncStr2[10];
			int idStr1 = 1;
			int idStr2 = 0;
			while( 1 )
			{
				if(*pbTmp == 0x18 && *(pbTmp + 1)== 0x07)
				{
					nIncStr1[0] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr1[0], pOccurrence + 3, nIncStr1[0]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr1[0]);
					pBufLast -= (3 + nIncStr1[0]);
				}
				else if(*pbTmp == 0x11 && *(pbTmp + 1)== 0x19 && *(pbTmp + 2)== 0x07)
				{
					nIncStr2[idStr2] = *(pOccurrence + 3) & 0x7f;
					memcpy(pstrStr2[idStr2], pOccurrence + 4, nIncStr2[idStr2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 4 + nIncStr2[idStr2]);
					pBufLast -= (4 + nIncStr2[idStr2]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum2[idStr2] = Hex4ToAsc(pOccurrence + 2, pstrNum2[idStr2]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
					}
					idStr2 += 1;
				}
				else if(*pbTmp == 0x19 && *(pbTmp + 1)== 0x07)
				{
					nIncStr1[idStr1] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr1[idStr1], pOccurrence + 3, nIncStr1[idStr1]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr1[idStr1]);
					pBufLast -= (3 + nIncStr1[idStr1]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum1[idStr1] = Hex4ToAsc(pOccurrence + 2, pstrNum1[idStr1]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
					}
					idStr1 += 1;
				}
				else if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
				{
					nIncNum1[0] = Hex4ToAsc(pOccurrence + 2, pstrNum1[0]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
				}
				else if(*pbTmp == 0x01 && *(pbTmp + 1)== 0x17 && *(pbTmp + 2)== 0x03)
				{
					break;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;
			}

			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pTmp = '(';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			int i;
			for(i = 1; i < idStr1; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr1[i], nIncStr1[i]);
				pBufLast += nIncStr1[i];
				pbTmp += nIncStr1[i];

				*pTmp = '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[i], nIncNum1[i]);
				pBufLast += nIncNum1[i];
				pbTmp += nIncNum1[i];
	
				if(idStr2 == 0 && i == idStr1 - 1)
					break;

				BYTE pTmp1[2] = { '|', '|' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			for(i = idStr2 -  1; i >= 0; i--)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr2[i], nIncStr2[i]);
				pBufLast += nIncStr2[i];
				pbTmp += nIncStr2[i];

				*pTmp = '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum2[i], nIncNum2[i]);
				pBufLast += nIncNum2[i];
				pbTmp += nIncNum2[i];

				if(i != 0)
				{
					BYTE pTmp1[2] = { '|', '|' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
					pBufLast += 2;
					pbTmp += 2;
				}
			}


			BYTE pTmp1[3] = { ')', '&', '&' };
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 3);
			pBufLast += 3;
			pbTmp += 3;

			InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr1[0], nIncStr1[0]);
			pBufLast += nIncStr1[0];
			pbTmp += nIncStr1[0];

			*pTmp = '(';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[0], nIncNum1[0]);
			pBufLast += nIncNum1[0];
			pbTmp += nIncNum1[0];

			*pTmp = ')';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;
			
			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//   NpcId=3508||NpcId=3509||NpcId=3510
void DAT_FindSomeOR(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0x14, 0x14 };
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum1[20][MAX_PATH] = { 0 };
			BYTE pstrNum2[20][MAX_PATH] = { 0 };
			BYTE pstrStr1[20][MAX_PATH] = { 0 };
			BYTE pstrStr2[20][MAX_PATH] = { 0 };
			int nIncNum1[20];
			int nIncNum2[20];
			int nIncStr1[20];
			int nIncStr2[20];
			int idStr1 = 0;
			int idStr2 = 0;
			while( 1 )
			{
				if(*pbTmp == 0x11 && *(pbTmp + 1)== 0x19 && *(pbTmp + 2)== 0x07)
				{
					nIncStr2[idStr2] = *(pOccurrence + 3) & 0x7f;
					memcpy(pstrStr2[idStr2], pOccurrence + 4, nIncStr2[idStr2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 4 + nIncStr2[idStr2]);
					pBufLast -= (4 + nIncStr2[idStr2]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum2[idStr2] = Hex4ToAsc(pOccurrence + 2, pstrNum2[idStr2]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
					}
					idStr2 += 1;
				}
				else if(*pbTmp == 0x19 && *(pbTmp + 1)== 0x07)
				{
					nIncStr1[idStr1] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr1[idStr1], pOccurrence + 3, nIncStr1[idStr1]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr1[idStr1]);
					pBufLast -= (3 + nIncStr1[idStr1]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum1[idStr1] = Hex4ToAsc(pOccurrence + 2, pstrNum1[idStr1]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
					}
					idStr1 += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

				if(*pbTmp == 0x01 && *(pbTmp + 1)== 0x17 && *(pbTmp + 2)== 0x03)
					break;
			}

			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			int i;
			for(i = 0; i < idStr1; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr1[i], nIncStr1[i]);
				pBufLast += nIncStr1[i];
				pbTmp += nIncStr1[i];

				*pTmp = '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[i], nIncNum1[i]);
				pBufLast += nIncNum1[i];
				pbTmp += nIncNum1[i];

				if(idStr2 == 0 && i == idStr1 - 1)
					break;

				BYTE pTmp1[2] = { '|', '|' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			for(i = idStr2 -  1; i >= 0; i--)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr2[i], nIncStr2[i]);
				pBufLast += nIncStr2[i];
				pbTmp += nIncStr2[i];

				*pTmp = '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum2[i], nIncNum2[i]);
				pBufLast += nIncNum2[i];
				pbTmp += nIncNum2[i];

				if(i != 0)
				{
					BYTE pTmp1[2] = { '|', '|' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
					pBufLast += 2;
					pbTmp += 2;
				}
			}

			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//   IsPet=0&&IsSmallPet=0&&IsUser=1&&HaveTypeEffect(8327)
void DAT_FindSomeAND(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0x13, 0x13 };
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum1[20][MAX_PATH] = { 0 };
			BYTE pstrNum2[20][MAX_PATH] = { 0 };
			BYTE pstrNum3[20][MAX_PATH] = { 0 };
			BYTE pstrStr1[20][MAX_PATH] = { 0 };
			BYTE pstrStr2[20][MAX_PATH] = { 0 };
			BYTE pstrStr3[20][MAX_PATH] = { 0 };
			int nIncNum1[20];
			int nIncNum2[20];
			int nIncNum3[20];
			int nIncStr1[20];
			int nIncStr2[20];
			int nIncStr3[20];
			int idStr1 = 0;
			int idStr2 = 0;
			int idStr3 = 0;
			while( 1 )
			{
				if(*pbTmp == 0x11 && *(pbTmp + 1)== 0x19 && *(pbTmp + 2)== 0x07)
				{
					nIncStr2[idStr2] = *(pOccurrence + 3) & 0x7f;
					memcpy(pstrStr2[idStr2], pOccurrence + 4, nIncStr2[idStr2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 4 + nIncStr2[idStr2]);
					pBufLast -= (4 + nIncStr2[idStr2]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum2[idStr2] = Hex4ToAsc(pOccurrence + 2, pstrNum2[idStr2]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
					}
					idStr2 += 1;
				}
				else if(*pbTmp == 0x19 && *(pbTmp + 1)== 0x07)
				{
					nIncStr1[idStr1] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr1[idStr1], pOccurrence + 3, nIncStr1[idStr1]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr1[idStr1]);
					pBufLast -= (3 + nIncStr1[idStr1]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum1[idStr1] = Hex4ToAsc(pOccurrence + 2, pstrNum1[idStr1]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
					}
					idStr1 += 1;
				}
				else if(*pbTmp == 0x18 && *(pbTmp + 1)== 0x07)
				{
					nIncStr3[idStr3] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr3[idStr3], pOccurrence + 3, nIncStr3[idStr3]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr3[idStr3]);
					pBufLast -= (3 + nIncStr3[idStr3]);
					idStr3 += 1;
				}
				else if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
				{
					nIncNum3[0] = Hex4ToAsc(pOccurrence + 2, pstrNum3[0]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

				if(*pbTmp == 0x01 && *(pbTmp + 1)== 0x17 && *(pbTmp + 2)== 0x03)
					break;
			}

			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			int i;
			for(i = 0; i < idStr1; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr1[i], nIncStr1[i]);
				pBufLast += nIncStr1[i];
				pbTmp += nIncStr1[i];

				*pTmp = '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[i], nIncNum1[i]);
				pBufLast += nIncNum1[i];
				pbTmp += nIncNum1[i];

				if(idStr2 == 0 && i == idStr1 - 1)
					break;

				BYTE pTmp1[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			for(i = idStr2 -  1; i >= 0; i--)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr2[i], nIncStr2[i]);
				pBufLast += nIncStr2[i];
				pbTmp += nIncStr2[i];

				*pTmp = '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum2[i], nIncNum2[i]);
				pBufLast += nIncNum2[i];
				pbTmp += nIncNum2[i];

				if(i == 0 && idStr3 == 0) break;

				BYTE pTmp1[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr3[0], nIncStr3[0]);
			pBufLast += nIncStr3[0];
			pbTmp += nIncStr3[0];

			*pTmp = '(';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum3[0], nIncNum3[0]);
			pBufLast += nIncNum3[0];
			pbTmp += nIncNum3[0];

			*pTmp = ')';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//    EffParam2*1000+10000
void DAT_Find1Power1Plus(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0x0b, 0x08 };
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum++;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

				if( (*pbTmp == 0x80 && *(pbTmp + 1) == 0x80) ||  (*pbTmp & 0x7f && *(pbTmp + 1) == 0x01 && *(pbTmp + 3) == 0x07) || (*pbTmp == 0x80 && *(pbTmp + 2) == 0x01 && *(pbTmp + 4) == 0x07) )
					break;
			}

			BYTE pTmp1[1];

			*pTmp1 = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
			pBufLast += 1;
			pbTmp += 1;

			if(idNum == 2 && idStr == 1)
			{
				BYTE pTmp1[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = '*';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];

				*pTmp1 = '+';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];
			}

			*pTmp1 = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
			pBufLast += 1;
			pbTmp += 1;

			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}



//-------------(100-EffParam1)/100.0--------------------
void DAT_Find1Minus1Divide(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0x09, 0x0C };
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			BYTE pstrFloat[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int nIncFloat[10];
			int idNum = 0;
			int idStr = 0;
			int idFloat = 0;

			while( 1 )
			{
				if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr += 1;
				}
				else if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum++;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x05 )
				{
					char pTmp1[MAX_PATH] = { 0 };
					DWORD dwTmp1 = *((PDWORD)(pOccurrence + 2));
					float f = *((float*)&dwTmp1);
					std::ostringstream ss;
					ss << f ;
					std::string s(ss.str());
					nIncFloat[idFloat] = s.length();
					memcpy(pstrFloat[idFloat], s.c_str(), nIncFloat[idFloat]);

					DeletePattern(pbBuffer, pdwLen, pOccurrence, 6);
					pBufLast -= 6;
					idFloat += 1;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

				if( (*pbTmp == 0x80 && *(pbTmp + 1) == 0x80) ||  (*pbTmp & 0x7f && *(pbTmp + 1) == 0x01 && *(pbTmp + 3) == 0x07) || (*pbTmp == 0x80 && *(pbTmp + 2) == 0x01 && *(pbTmp + 4) == 0x07) )
					break;
			}
			
			BYTE pTmp1[1];

			*pTmp1 = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
			pBufLast += 1;
			pbTmp += 1;
			
			if(idNum == 1 && idFloat == 1 && idStr == 1)
			{
				BYTE pTmp1[1];

				*pTmp1 = '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				*pTmp1 = '-';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp1 = ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp1 = '/';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrFloat[0], nIncFloat[0]);
				pBufLast += nIncFloat[0];
				pbTmp += nIncFloat[0];
			}

			*pTmp1 = ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 1);
			pBufLast += 1;
			pbTmp += 1;

			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//    IsPet=0&&IsSmallPet=0&&IsUser=1
//	  NpcIdEx(5759,5761,5763,5765,5767,5769,5771,5773)=0&&IsPet=0&&IsSmallPet=0	
void DAT_Find3AND(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x13, 0x13, 0x11, 0x11 
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum1[20][MAX_PATH] = { 0 };
			BYTE pstrNum2[20][MAX_PATH] = { 0 };
			BYTE pstrNum3[20][MAX_PATH] = { 0 };
			BYTE pstrNum4[50][MAX_PATH] = { 0 };
			BYTE pstrStr1[20][MAX_PATH] = { 0 };
			BYTE pstrStr2[20][MAX_PATH] = { 0 };
			BYTE pstrStr3[20][MAX_PATH] = { 0 };
			int nIncNum1[20];
			int nIncNum2[20];
			int nIncNum3[20];
			int nIncNum4[20];
			int nIncStr1[20];
			int nIncStr2[20];
			int nIncStr3[20];
			int idNum1 = 0;
			int idNum2 = 0;
			int idNum3 = 0;
			int idNum4 = 0;
			int idStr1 = 0;
			int idStr2 = 0;
			int idStr3 = 0;
			while( 1 )
			{
				if( *pbTmp == 0x11 && *(pbTmp + 1)== 0x19 && *(pbTmp + 2)== 0x07)
				{
					nIncStr2[idStr2] = *(pOccurrence + 3) & 0x7f;
					memcpy(pstrStr2[idStr2], pOccurrence + 4, nIncStr2[idStr2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 4 + nIncStr2[idStr2]);
					pBufLast -= (4 + nIncStr2[idStr2]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum2[idNum2] = Hex4ToAsc(pOccurrence + 2, pstrNum2[idNum2]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum2++;
					}
					idStr2++;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr1[idStr1] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr1[idStr1], pOccurrence + 3, nIncStr1[idStr1]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr1[idStr1]);
					pBufLast -= (3 + nIncStr1[idStr1]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum1[idNum1] = Hex4ToAsc(pOccurrence + 2, pstrNum1[idNum1]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum1++;
					}
					idStr1++;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr3[idStr3] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr3[idStr3], pOccurrence + 3, nIncStr3[idStr3]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr3[idStr3]);
					pBufLast -= (3 + nIncStr3[idStr3]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum3[idNum3] = Hex4ToAsc(pOccurrence + 2, pstrNum3[idNum3]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum3++;
					}
					idStr3++;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum4[idNum4] = Hex4ToAsc(pOccurrence + 2, pstrNum4[idNum4]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum4++;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if(pbTmp >= pBufLast)
					break;

				if(*pbTmp == 0x01 && *(pbTmp + 1)== 0x17 && *(pbTmp + 2)== 0x03)
					break;
			}

			BYTE pTmp[1] = { '"' };
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			if(idStr3 > 0)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr3[0], nIncStr3[0]);
				pBufLast += nIncStr3[0];
				pbTmp += nIncStr3[0];

				*pTmp =  '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				for(int i = 0; i < idNum4 ; i++)
				{
					InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum4[i], nIncNum4[i]);
					pBufLast += nIncNum4[i];
					pbTmp += nIncNum4[i];

					if(i < idNum4 - 1)
					{
						BYTE pTmp[1] = { ',' };
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
						pBufLast += 1;
						pbTmp += 1;
					}
				}

				*pTmp =  ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp =  '=' ;
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum3[0], nIncNum3[0]);
				pBufLast += nIncNum3[0];
				pbTmp += nIncNum3[0];

				BYTE pTmp2[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp2, 2);
				pBufLast += 2;
				pbTmp += 2;
			}
			for(int i = 0; i < idStr1; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr1[i], nIncStr1[i]);
				pBufLast += nIncStr1[i];
				pbTmp += nIncStr1[i];

				BYTE pTmp[1] = { '=' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[i], nIncNum1[i]);
				pBufLast += nIncNum1[i];
				pbTmp += nIncNum1[i];

				if( i == idStr1 - 1 && idStr2 == 0) break;
				BYTE pTmp2[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp2, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			for(int i = idStr2 - 1; i >= 0; i--)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr2[i], nIncStr2[i]);
				pBufLast += nIncStr2[i];
				pbTmp += nIncStr2[i];

				BYTE pTmp[1] = { '=' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum2[i], nIncNum2[i]);
				pBufLast += nIncNum2[i];
				pbTmp += nIncNum2[i];

				if(i == 0) break;
				BYTE pTmp2[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp2, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			*pTmp = '"';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pbTmp = ',';
			pbTmp += 1;
			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//   IsMyMaster=1&&IsAttackState=0
void DAT_Find1AND(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x13, 0x11, 0x11
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int idNum = 0;
			int idStr = 0;
			while( 1)
			{
				if( *pbTmp== 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum++;
					}
					idStr++;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}

				if( pbTmp >= pBufLast)
					break;

				if( (*pbTmp == 0x80 && *(pbTmp + 1) == 0x80) ||  (*pbTmp & 0x7f && *(pbTmp + 1) == 0x01 && *(pbTmp + 3) == 0x07) || (*pbTmp == 0x80 && *(pbTmp + 2) == 0x01 && *(pbTmp + 4) == 0x07) )
					break;
			}
			
			BYTE pTmp[1] = { 0 };
			*pTmp =  ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			for(int i = 0 ;i < idStr; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[i], nIncStr[i]);
				pBufLast += nIncStr[i];
				pbTmp += nIncStr[i];

				BYTE pTmp[1] = { '=' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[i], nIncNum[i]);
				pBufLast += nIncNum[i];
				pbTmp += nIncNum[i];

				if(i == idStr - 1) break;
				BYTE pTmp1[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
				pBufLast += 2;
				pbTmp += 2;
			}
			*pTmp =  ',';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;
			
			pbBuf = pbTmp;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

//   IsNpc=1 && Hp > 0
void DAT_FindEqualAndBigger(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x13, 0x11, 0x0f
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			int nIncNum[10];
			int nIncStr[10];
			int idNum = 0;
			int idStr = 0;
			while( 1)
			{
				if( *pbTmp== 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					if(*pbTmp == 0x17 && *(pbTmp + 1)== 0x03)
					{
						nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum++;
					}
					idStr++;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}

				if( pbTmp >= pBufLast)
					break;

				if(*pbTmp == 0x01 && *(pbTmp + 1)== 0x17 && *(pbTmp + 2)== 0x03)
					break;
			}

			if( idStr == 2 && idNum == 2)
			{
				BYTE pTmp[1];
				*pTmp =  ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp =  '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[0], nIncNum[0]);
				pBufLast += nIncNum[0];
				pbTmp += nIncNum[0];

				BYTE pTmp1[4] = { ' ', '&', '&', ' ' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 4);
				pBufLast += 4;
				pbTmp += 4;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[1], nIncStr[1]);
				pBufLast += nIncStr[1];
				pbTmp += nIncStr[1];

				BYTE pTmp2[3] = { ' ', '>', ' ' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp2, 3);
				pBufLast += 3;
				pbTmp += 3;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[1], nIncNum[1]);
				pBufLast += nIncNum[1];
				pbTmp += nIncNum[1];


				*pTmp =  ',';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;
			}
			pbBuf = pbTmp;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

// IsPet=0&&IsSmallPet=0&&(IsUser=0&&NpcIdEx(3895,3896,3897,3898,3899,3900,3901,3902,3903,3904,3905,3906,3907)=1)
void DAT_Find3ANDBracketSomeVar(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x13, 0x13, 0x13, 0x11, 0x11, 0x11
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum[20][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			int nIncNum[20];
			int nIncStr[10];
			int idNum = 0;
			int idStr = 0;
			while( 1 )
			{
				if( (idStr < 4) && ((*pbTmp == 0x19 && *(pbTmp + 1)== 0x07) || (*pbTmp == 0x18 && *(pbTmp + 1)== 0x07)))
				{
					nIncStr[idStr] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr[idStr], pOccurrence + 3, nIncStr[idStr]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr[idStr]);
					pBufLast -= (3 + nIncStr[idStr]);
					idStr++;
				}
				else if( (idNum < 17) && (*pbTmp == 0x17 && *(pbTmp + 1)== 0x03) )
				{
					nIncNum[idNum] = Hex4ToAsc(pOccurrence + 2, pstrNum[idNum]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum++;
				}
				else if( idStr < 4 || idNum < 17)
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}

				else
					break;
			}

			BYTE pTmp[1] = { '"' };
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			for(int i = 0; i < 4; i++)
			{

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[i], nIncStr[i]);
				pBufLast += nIncStr[i];
				pbTmp += nIncStr[i];

				if( i == 3)
				{
					
					BYTE pTmp[1] = { '(' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
					pBufLast += 1;
					pbTmp += 1;

					for(int j = 4; j < 17; j++)
					{
						InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[j], nIncNum[j]);
						pBufLast += nIncNum[j];
						pbTmp += nIncNum[j];

						if(j == 16) break;

						BYTE pTmp[2] = { ',' };
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
						pBufLast += 1;
						pbTmp += 1;
					}

					pTmp[0] = ')';
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
					pBufLast += 1;
					pbTmp += 1;
				}
				
				BYTE pTmp[1] = { '=' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum[i], nIncNum[i]);
				pBufLast += nIncNum[i];
				pbTmp += nIncNum[i];

				if( i < 3 )
				{
					BYTE pTmp1[2] = { '&', '&' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
					pBufLast += 2;
					pbTmp += 2;
				}
				if(i == 1)
				{
					BYTE pTmp[1] = { '(' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
					pBufLast += 1;
					pbTmp += 1;
				}
				if(i == 3)
				{
					BYTE pTmp[1] = { ')' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
					pBufLast += 1;
					pbTmp += 1;
				}

			}

			*pTmp = '"';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pbTmp = ',';
			pbTmp += 1;
			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

// IsPet=0&&IsSmallPet=0&&(IsUser=0||NpcIdEx(3981,3982,3983,3984,3985,3989,3990,3991)=1)
void DAT_Find2AND1ORBracketSomeVar(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x13, 0x13, 0x14, 0x11, 0x11, 0x11
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum1[20][MAX_PATH] = { 0 };
			BYTE pstrNum2[20][MAX_PATH] = { 0 };
			BYTE pstrNum3[20][MAX_PATH] = { 0 };
			BYTE pstrStr1[20][MAX_PATH] = { 0 };
			BYTE pstrStr2[20][MAX_PATH] = { 0 };
			int nIncNum1[20];
			int nIncNum2[20];
			int nIncNum3[20];
			int nIncStr1[20];
			int nIncStr2[20];
			int idNum1 = 0;
			int idNum2 = 0;
			int idNum3 = 0;
			int idStr1 = 0;
			int idStr2 = 0;
			while( 1 )
			{
				if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr1[idStr1] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr1[idStr1], pOccurrence + 3, nIncStr1[idStr1]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr1[idStr1]);
					pBufLast -= (3 + nIncStr1[idStr1]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum1[idNum1] = Hex4ToAsc(pOccurrence + 2, pstrNum1[idNum1]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum1++;
					}
					idStr1++;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr2[idStr2] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr2[idStr2], pOccurrence + 3, nIncStr2[idStr2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr2[idStr2]);
					pBufLast -= (3 + nIncStr2[idStr2]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum2[idNum2] = Hex4ToAsc(pOccurrence + 2, pstrNum2[idNum2]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum2++;
					}
					idStr2++;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum3[idNum3] = Hex4ToAsc(pOccurrence + 2, pstrNum3[idNum3]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum3++;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if(pbTmp >= pBufLast)
					break;
				
				if(*pbTmp == 0x01 && *(pbTmp + 1)== 0x17 && *(pbTmp + 2)== 0x03)
					break;
			}

			BYTE pTmp[1] = { '"' };
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			for(int i = 0; i < idStr1; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr1[i], nIncStr1[i]);
				pBufLast += nIncStr1[i];
				pbTmp += nIncStr1[i];

				BYTE pTmp[1] = { '=' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[i], nIncNum1[i]);
				pBufLast += nIncNum1[i];
				pbTmp += nIncNum1[i];

				if( i < idStr1 - 1)
				{
					BYTE pTmp1[2] = { '&', '&' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
					pBufLast += 2;
					pbTmp += 2;

					if( i == idStr1 - 2)
					{
						BYTE pTmp[1] = { '(' };
						InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
						pBufLast += 1;
						pbTmp += 1;
					}
				}
			}

			BYTE pTmp1[2] = { '|', '|' };
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp1, 2);
			pBufLast += 2;
			pbTmp += 2;

			InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr2[0], nIncStr2[0]);
			pBufLast += nIncStr2[0];
			pbTmp += nIncStr2[0];
			
			*pTmp =  '(';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;
			
			for(int i = 0; i < idNum3 ; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum3[i], nIncNum3[i]);
				pBufLast += nIncNum3[i];
				pbTmp += nIncNum3[i];

				if(i < idNum3 - 1)
				{
					BYTE pTmp[1] = { ',' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
					pBufLast += 1;
					pbTmp += 1;
				}
			}

			*pTmp =  ')';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pTmp =  '=' ;
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum2[0], nIncNum2[0]);
			pBufLast += nIncNum2[0];
			pbTmp += nIncNum2[0];
			
			*pTmp =  ')';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pTmp = '"';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pbTmp = ',';
			pbTmp += 1;
			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

// IsPet=0&&IsSmallPet=0&&IsUser=0&&NpcIdEx(3895,3896,3897,3898,3899,3900,3901,3902,3903,3904,3905,3906,3907)=0
void DAT_Find3ANDSomeVar(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x13, 0x13, 0x11, 0x13
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
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum1[20][MAX_PATH] = { 0 };
			BYTE pstrNum2[20][MAX_PATH] = { 0 };
			BYTE pstrNum3[20][MAX_PATH] = { 0 };
			BYTE pstrNum4[50][MAX_PATH] = { 0 };
			BYTE pstrStr1[20][MAX_PATH] = { 0 };
			BYTE pstrStr2[20][MAX_PATH] = { 0 };
			BYTE pstrStr3[20][MAX_PATH] = { 0 };
			int nIncNum1[20];
			int nIncNum2[20];
			int nIncNum3[20];
			int nIncNum4[20];
			int nIncStr1[20];
			int nIncStr2[20];
			int nIncStr3[20];
			int idNum1 = 0;
			int idNum2 = 0;
			int idNum3 = 0;
			int idNum4 = 0;
			int idStr1 = 0;
			int idStr2 = 0;
			int idStr3 = 0;
			while( 1 )
			{
				if( *pbTmp == 0x11 && *(pbTmp + 1)== 0x19 && *(pbTmp + 2)== 0x07)
				{
					nIncStr2[idStr2] = *(pOccurrence + 3) & 0x7f;
					memcpy(pstrStr2[idStr2], pOccurrence + 4, nIncStr2[idStr2]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 4 + nIncStr2[idStr2]);
					pBufLast -= (4 + nIncStr2[idStr2]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum2[idNum2] = Hex4ToAsc(pOccurrence + 2, pstrNum2[idNum2]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum2++;
					}
					idStr2++;
				}
				else if( *pbTmp == 0x19 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr1[idStr1] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr1[idStr1], pOccurrence + 3, nIncStr1[idStr1]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr1[idStr1]);
					pBufLast -= (3 + nIncStr1[idStr1]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum1[idNum1] = Hex4ToAsc(pOccurrence + 2, pstrNum1[idNum1]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum1++;
					}
					idStr1++;
				}
				else if( *pbTmp == 0x18 && *(pbTmp + 1)== 0x07 )
				{
					nIncStr3[idStr3] = *(pOccurrence + 2) & 0x7f;
					memcpy(pstrStr3[idStr3], pOccurrence + 3, nIncStr3[idStr3]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 3 + nIncStr3[idStr3]);
					pBufLast -= (3 + nIncStr3[idStr3]);
					if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
					{
						nIncNum3[idNum3] = Hex4ToAsc(pOccurrence + 2, pstrNum3[idNum3]);
						DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
						pBufLast -= (2 + 4);
						idNum3++;
					}
					idStr3++;
				}
				else if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum4[idNum4] = Hex4ToAsc(pOccurrence + 2, pstrNum4[idNum4]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum4++;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if(pbTmp >= pBufLast)
					break;
				if(*pbTmp == 0x01 && *(pbTmp + 1)== 0x17 && *(pbTmp + 2)== 0x03)
					break;
			}

			BYTE pTmp[1] = { '"' };
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			for(int i = 0; i < idStr1; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr1[i], nIncStr1[i]);
				pBufLast += nIncStr1[i];
				pbTmp += nIncStr1[i];

				BYTE pTmp[1] = { '=' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[i], nIncNum1[i]);
				pBufLast += nIncNum1[i];
				pbTmp += nIncNum1[i];
				
				BYTE pTmp2[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp2, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			for(int i = idStr2 - 1; i >= 0; i--)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr2[i], nIncStr2[i]);
				pBufLast += nIncStr2[i];
				pbTmp += nIncStr2[i];

				BYTE pTmp[1] = { '=' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum2[i], nIncNum2[i]);
				pBufLast += nIncNum2[i];
				pbTmp += nIncNum2[i];

				BYTE pTmp2[2] = { '&', '&' };
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp2, 2);
				pBufLast += 2;
				pbTmp += 2;
			}

			InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr3[0], nIncStr3[0]);
			pBufLast += nIncStr3[0];
			pbTmp += nIncStr3[0];

			*pTmp =  '(';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			for(int i = 0; i < idNum4 ; i++)
			{
				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum4[i], nIncNum4[i]);
				pBufLast += nIncNum4[i];
				pbTmp += nIncNum4[i];

				if(i < idNum4 - 1)
				{
					BYTE pTmp[1] = { ',' };
					InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
					pBufLast += 1;
					pbTmp += 1;
				}
			}

			*pTmp =  ')';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			if(idStr3 == 1)
			{
				*pTmp =  '=' ;
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum3[0], nIncNum3[0]);
				pBufLast += nIncNum3[0];
				pbTmp += nIncNum3[0];
			}

			*pTmp = '"';
			InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
			pBufLast += 1;
			pbTmp += 1;

			*pbTmp = ',';
			pbTmp += 1;
			pbBuf = pbTmp;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//  IsUser=1
void DAT_FindEqual(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x11, 0x19, 0x07 
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
			BYTE pTmp[MAX_PATH] = { 0 };
			int nSubStrLen = (*(pOccurrence + dwPatternSize) & 0x7f);
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize + 1);
			pBufLast -= (dwPatternSize + 1);

			*pTmp = '=';
			InsertPattern(pbBuffer, pdwLen, pOccurrence + nSubStrLen, pTmp, 1);
			pBufLast += 1;
			pbBuf += 1;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

//  HaveTypeEffect(8326)=0
void DAT_FindEqualWithBracket(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x11, 0x18, 0x07 
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

			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;
			PBYTE pbTmp = pOccurrence;
			BYTE pstrNum1[10][MAX_PATH] = { 0 };
			BYTE pstrNum2[10][MAX_PATH] = { 0 };
			BYTE pstrStr[10][MAX_PATH] = { 0 };
			int nIncNum1[10];
			int nIncNum2[10];
			int nIncStr[10];
			int idNum1 = 0;
			int idNum2 = 0;
			int idStr = 0;

			nIncStr[idStr] = *pOccurrence & 0x7f;
			memcpy(pstrStr[idStr], pOccurrence + 1, nIncStr[idStr]);
			DeletePattern(pbBuffer, pdwLen, pOccurrence, 1 + nIncStr[idStr]);
			pBufLast -= (1 + nIncStr[idStr]);
			if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
			{
				nIncNum2[idNum2] = Hex4ToAsc(pOccurrence + 2, pstrNum2[idNum2]);
				DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
				pBufLast -= (2 + 4);
				idNum2++;
			}
			idStr++;
			
			while( 1 )
			{
				if( *pbTmp == 0x17 && *(pbTmp + 1)== 0x03 )
				{
					nIncNum1[idNum1] = Hex4ToAsc(pOccurrence + 2, pstrNum1[idNum1]);
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 2 + 4);
					pBufLast -= (2 + 4);
					idNum1++;
				}
				else
				{
					DeletePattern(pbBuffer, pdwLen, pOccurrence, 1);
					pBufLast -= 1;
				}
				if( pbTmp >= pBufLast)
					break;

				if( (*pbTmp == 0x80 && *(pbTmp + 1) == 0x80) ||  (*pbTmp & 0x7f && *(pbTmp + 1) == 0x01 && *(pbTmp + 3) == 0x07) || (*pbTmp == 0x80 && *(pbTmp + 2) == 0x01 && *(pbTmp + 4) == 0x07) )
					break;
			}

			if(idStr == 1 && idNum1 == 1 && idNum2 == 1)
			{
				BYTE pTmp[1];

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrStr[0], nIncStr[0]);
				pBufLast += nIncStr[0];
				pbTmp += nIncStr[0];

				*pTmp =  '(';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum1[0], nIncNum1[0]);
				pBufLast += nIncNum1[0];
				pbTmp += nIncNum1[0];

				*pTmp =  ')';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				*pTmp =  '=';
				InsertPattern(pbBuffer, pdwLen, pbTmp, pTmp, 1);
				pBufLast += 1;
				pbTmp += 1;

				InsertPattern(pbBuffer, pdwLen, pbTmp, pstrNum2[0], nIncNum2[0]);
				pBufLast += nIncNum2[0];
				pbTmp += nIncNum2[0];
			}
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void DAT_PreProcess(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0x19, 0x07 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	int k = 0;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			*(pOccurrence + 1) = 0x18;
			pbBuf = pOccurrence + sizeof(Pattern);
		}
	} while (fFound);
}

void DAT_MakeComma(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x00, 0x00, 0x00, 0x01 };
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	int k = 0;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, dwPatternSize);
			pBufLast -= dwPatternSize;

			BYTE pTmp[1] = { ',' };
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, 1);
			pBufLast += 1;
			pbBuf += 1;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);

}

void DAT_StraightenStringSet(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0x18, 0x07 };
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
			if( ((*(pOccurrence - 1) & 0x7f) != 0)  && (*(pOccurrence - 1) & 0x80) ) //if first entry of string set 
			{
				int nSubStrNum; //The count of sub-string in string set
				PBYTE pStartPos, pEndPos = NULL; //Start and End point of string set

				nSubStrNum = *(pOccurrence - 1) & 0x7f;
				
				pStartPos = pOccurrence;
				
			
				PBYTE pTmpPos = pStartPos;
				while( 1 )
				{
					if( (*pTmpPos == 0x80 && *(pTmpPos + 1) == 0x80) ||  (*pTmpPos & 0x7f && *(pTmpPos + 1) == 0x01 && *(pTmpPos + 3) == 0x07) || (*pTmpPos == 0x80 && *(pTmpPos + 2) == 0x01 && *(pTmpPos + 4) == 0x07) )
						break;

					pTmpPos++;
				}
				pEndPos = pTmpPos;

				if((pEndPos == pBufLast))
				{
					MessageBox(NULL, "problem occur in finding the end point of sub string", "Infor", 0L);
					return;
				}
				
				PBYTE pOccurrence2;
				PBYTE pbBuf1 = pStartPos;
				PBYTE pBufLast1 = pEndPos;
				for(int i = 0 ;i < nSubStrNum; i++)
				{
					int nSubStrLen;
					PBYTE pSubStartP, pSubEndP;
					nSubStrLen = *(pbBuf1 + 3) & 0x7f;
					DeletePattern(pbBuffer, pdwLen, pbBuf1, 4);
					pBufLast1 -= 4;
					pBufLast -= 4;

					pOccurrence2 = std::search(pbBuf1, pBufLast1, Pattern, pPatternLast);
					pSubStartP = pbBuf1;
					pSubEndP = pOccurrence2;

					DAT_FindBracket2Plus2Power(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket1Divide(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBrakcketNegative1PowerInBracket(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket2Plus2Power1Divide(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket1Power1Divide(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracketMinus1Divide(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket1Divide1Power(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindInBracket2Plus3Power(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindInBracketPowerComma(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracketForFloat(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracketForLong(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracketForString1(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracketForString2(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindQuoteForString(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket1Plus1Minus(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket2Power1Divide(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket1Power(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracket2NegativeVar(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindBracketNegativeVar(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindInBracketNegativeVar(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindIn1Divide1Power(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindInBracketNegative1Divide(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindInBracket2Plus3Power1Divide(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindString(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindFloat(pbBuffer, pdwLen, pSubStartP, &pSubEndP);
					DAT_FindNumber(pbBuffer, pdwLen, pSubStartP, &pSubEndP, TRUE);
					

					PBYTE pbSubTmp = pSubEndP - 1;
					int nNumZero = 0;
					while( *pbSubTmp == 0 )
					{
						nNumZero++;
						pbSubTmp--;
					}
					if(nNumZero)
					{
						DeletePattern(pbBuffer, pdwLen, pSubEndP - nNumZero, nNumZero);
						pSubEndP -= nNumZero;
					}

					if( i < nSubStrNum - 1 )
					{
						BYTE pTmp1[1];
						*pTmp1 = ';';
						InsertPattern(pbBuffer, pdwLen, pSubEndP, pTmp1, 1);
						pSubEndP += 1;
					}
					pbBuf1 = pSubEndP;
					pBufLast1 += (pSubEndP - pOccurrence2);
					pBufLast += (pSubEndP - pOccurrence2);
				}
				pEndPos = pbBuf1;
				*(pStartPos - 1) = '"';
				BYTE pTmp[1] = { 0 };

				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pStartPos - 1, pTmp, 1);
				pEndPos += 1;
				pBufLast += 1;

				*pTmp = '"';
				InsertPattern(pbBuffer, pdwLen, pEndPos, pTmp, 1);
				pEndPos += 1;
				pBufLast += 1;

				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pEndPos, pTmp, 1);
				pEndPos += 1;
				pBufLast += 1;

				pbBuf = pEndPos;
			}
			else
				pbBuf = pOccurrence + dwPatternSize;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

void DAT_ClearAndInsertNumber1(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pbBuffer + *pdwLen;
	unsigned char Pattern[] = {
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00,	0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01 
	};

	DWORD PatternSize = sizeof(Pattern);
	BOOL fFound;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, Pattern + PatternSize);
		pOccurrence -= 2;
		fFound = (pOccurrence + 2 != pBufLast);
		if(fFound)
		{
			BYTE b1 = *pOccurrence;			
			BYTE b2 = *(pOccurrence + 1);
			BYTE b = b1 | b2;
			if(b == 0x00 || b == 0x01)
			{
				int nInc = 0;
				if( b1 )
					*(pOccurrence + nInc++) = '1';
				*(pOccurrence + nInc++) = ',';
				if( b2 )
					*(pOccurrence + nInc++) = '1';
				*(pOccurrence + nInc++) = ',';

				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, PatternSize);
				pBufLast -= PatternSize;
				pbBuf = pOccurrence + nInc;
			}
			else
				pbBuf += (PatternSize + 2);
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

void DAT_ClearAndInsertNumber2(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pbBuffer + *pdwLen;
	unsigned char Pattern[] = {
		0x00, 0x00, 0x00, 0x00, 0x01, 0x00,	0x00, 0x01, 0x00, 0x00, 0x01 
	};

	DWORD PatternSize = sizeof(Pattern);
	BOOL fFound;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, Pattern + PatternSize);
		pOccurrence -= 2;
		fFound = (pOccurrence + 2 != pBufLast);
		if(fFound)
		{
			BYTE b1 = *pOccurrence;			
			BYTE b2 = *(pOccurrence + 1);
			BYTE b = b1 | b2;
			if(b == 0x00 || b == 0x01)
			{
				int nInc = 0;
				if( b1 )
					*(pOccurrence + nInc++) = '1';
				*(pOccurrence + nInc++) = ',';
				if( b2 )
					*(pOccurrence + nInc++) = '1';
				*(pOccurrence + nInc++) = ',';

				DeletePattern(pbBuffer, pdwLen, pOccurrence + nInc, PatternSize);
				pBufLast -= PatternSize;
				pbBuf = pOccurrence + nInc;
			}
			else
				pbBuf += (PatternSize + 2);
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

void DAT_InsertNumBeforeCRLF(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pStartPos, PDWORD pdwInc)
{
	BYTE pBuf[MAX_PATH] = { 0 };
	BYTE pTmp[MAX_PATH] = { 0 };
	DWORD dwSize = 0;
	int nInc;
	PBYTE pTmpBuf = pBuf;

	*pTmp =  ',';
	InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, 1);
	pTmpBuf += 1;

	if(*(pStartPos + 39))
	{
		nInc = Hex4ToAsc(pStartPos + 39, pTmp);
		InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, nInc);
		pTmpBuf += nInc;
	}
	*pTmp =  ',';
	InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, 1);
	pTmpBuf += 1;

	if(*(pStartPos + 35))
	{
		nInc = Hex4ToAsc(pStartPos + 35, pTmp);
		InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, nInc);
		pTmpBuf += nInc;
	}

	*pTmp =  ',';
	InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, 1);
	pTmpBuf += 1;

	if(*(pStartPos + 43))
	{
		nInc = Hex4ToAsc(pStartPos + 43, pTmp);
		InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, nInc);
		pTmpBuf += nInc;
	}

	*pTmp =  ',';
	InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, 1);
	pTmpBuf += 1;

	if(*pStartPos)
	{
		nInc = Hex4ToAsc(pStartPos, pTmp);
		InsertPattern(pTmpBuf, &dwSize, (DWORD)0, pTmp, nInc);
		pTmpBuf += nInc;
	}

	InsertPattern(pbBuffer, pdwLen, pStartPos, pBuf, dwSize);

	*pdwInc = dwSize;

}

void DAT_Clear2(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pbBuffer + *pdwLen;
	unsigned char Pattern1[] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
	};
	DWORD PatternSize = sizeof(Pattern1);
	BOOL fFound;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern1, Pattern1 + PatternSize);
		fFound = (pOccurrence != pBufLast);
		if(fFound)
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, PatternSize);
			pBufLast -= PatternSize;
			pbBuf = pOccurrence;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);

}

void DAT_Clear1(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pbBuffer + *pdwLen;
	unsigned char Pattern[56] = {
		0x04, 0x00, 0x00, 0x00, 0x72, 0x6F, 0x6F, 0x74,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
		0x72, 0x6F, 0x6F, 0x74, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x72, 0x6F, 0x6F, 0x74 
	};
	DWORD PatternSize = sizeof(Pattern);
	BOOL fFound;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, Pattern + PatternSize);
		fFound = (pOccurrence != pBufLast);
		if(fFound)
		{
			DWORD dwInc = 0;
			DAT_InsertNumBeforeCRLF(pbBuffer, pdwLen, pOccurrence - 112, &dwInc);
			pOccurrence += dwInc;
			DeletePattern(pbBuffer, pdwLen, pOccurrence - 112, 112 + PatternSize + 130);
			pBufLast -= (112 + PatternSize + 130);
			pBufLast += dwInc;
			pbBuf = pOccurrence - 112;

			//Process till CRLF
			PBYTE pTmpPos = pbBuf;
			if(pTmpPos >= pBufLast)
				break;
			do 
			{
				if(*pTmpPos == 0x01 )
				{
					PBYTE pTmpPos1 = pTmpPos;
					while(*(++pTmpPos) == 0x00);
					DeletePattern(pbBuffer, pdwLen, pTmpPos1, pTmpPos - pTmpPos1);
					pBufLast -= (pTmpPos - pTmpPos1);
					pTmpPos = pTmpPos1;
					BYTE pTmp[MAX_PATH] = { 0 };
					*pTmp = ',';
					InsertPattern(pbBuffer, pdwLen, pTmpPos1, pTmp, 1);
					pBufLast += 1;
					pTmpPos += 1;
				}
				else if(*pTmpPos > 0x01)
				{
					if(*(pTmpPos + 1) == 0x00 && *(pTmpPos + 2) == 0x00 && *(pTmpPos + 3) == 0x00)
					{
						int nSubStrLen = *pTmpPos;
						DeletePattern(pbBuffer, pdwLen, pTmpPos, 4);
						pBufLast -= 4;
						BYTE pTmp[MAX_PATH] = { 0 };
						*pTmp = ',';
						InsertPattern(pbBuffer, pdwLen, pTmpPos + nSubStrLen, pTmp, 1);
						InsertPattern(pbBuffer, pdwLen, pTmpPos, pTmp, 1);
						pBufLast += 2;
						pTmpPos += (2 + nSubStrLen);
					}
				}
				else
				{
					PBYTE pTmpPos1 = pTmpPos;
					while(*(++pTmpPos) == 0x00);
					DeletePattern(pbBuffer, pdwLen, pTmpPos1, pTmpPos - pTmpPos1);
					pBufLast -= (pTmpPos - pTmpPos1);
					pTmpPos = pTmpPos1;
				}
			} while ( !(*pTmpPos == 0x0d && *(pTmpPos + 1) == 0x0a) );
			pbBuf = pTmpPos + 2;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

void DAT_FindCRLF(PBYTE pbBuffer, PDWORD pdwLen, int nPos1, BYTE b1, int nPos2, BYTE b2)
{
	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pbBuffer + *pdwLen;
	unsigned char Pattern[29] = {
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00 
	};

	Pattern[nPos1] = b1;
	Pattern[nPos2] = b2;
	DWORD PatternSize = sizeof(Pattern);
	BOOL fFound;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, Pattern + PatternSize);
		fFound = (pOccurrence != pBufLast);
		if(fFound)
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, PatternSize);
			pBufLast -= PatternSize;

			BYTE pTmp1[] = { 0x0d, 0x0a };
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp1, 2);
			pBufLast += 2;
			pbBuf = pOccurrence + 2;
			
			if(pbBuf == pBufLast) //if last CRLF
				break;

			BYTE pTmp[MAX_PATH] = { 0 };
			int nInc = Hex4ToAsc(pOccurrence + 2, pTmp);

			DeletePattern(pbBuffer, pdwLen, pOccurrence + 2, 4);
			pBufLast -= 4;
			InsertPattern(pbBuffer, pdwLen, pOccurrence + 2, pTmp, nInc);
			pbBuf += nInc;
			pBufLast += nInc;

			if(*pbBuf == 0x00 && *(pbBuf + 1) == 0x00 && *(pbBuf + 2) == 0x00 && *(pbBuf + 3) == 0x00 )
			{
				DeletePattern(pbBuffer, pdwLen, pbBuf, 4);
				pBufLast -= 4;
				BYTE pTmp[2] = { 0 };
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
				pbBuf += 1;
				pBufLast += 1;

				BYTE pTmp1[MAX_PATH] = { 0 };
				BYTE pTmp2[MAX_PATH] = { 0 };

				*pTmp1 = *pbBuf;
				*(pTmp1 + 1)= *(pbBuf + 1);
				nInc = Hex4ToAsc(pTmp1, pTmp2);
				DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
				pBufLast -= 2;
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
				pbBuf += nInc;
				pBufLast += nInc;
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
				pbBuf += 1;
				pBufLast += 1;

				*pTmp1 = *pbBuf;
				*(pTmp1 + 1)= *(pbBuf + 1);
				nInc = Hex4ToAsc(pTmp1, pTmp2);
				DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
				pBufLast -= 2;
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
				pbBuf += nInc;
				pBufLast += nInc;

				if(*pbBuf != 0)
				{
					InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
					pbBuf += 1;
					pBufLast += 1;

					*pTmp1 = *pbBuf;
					*(pTmp1 + 1)= *(pbBuf + 1);

					nInc = Hex4ToAsc(pTmp1, pTmp2);
					DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
					pBufLast -= 2;

					InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
					pbBuf += nInc;
					pBufLast += nInc;
				}
			}
			
			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
			pbBuf += 1;
			pBufLast += 1;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

void DAT_FindCRLFSpecial(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pbBuffer + *pdwLen;
	unsigned char Pattern[29] = {
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x00, 0x00 
	};

	DWORD PatternSize = sizeof(Pattern);
	BOOL fFound;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, Pattern + PatternSize);
		fFound = (pOccurrence != pBufLast);
		if(fFound)
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, PatternSize);
			pBufLast -= PatternSize;

			BYTE pTmp1[] = { 0x0d, 0x0a };
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp1, 2);
			pBufLast += 2;
			pbBuf = pOccurrence + 2;

			if(pbBuf == pBufLast) //if last CRLF
				break;

			BYTE pTmp[MAX_PATH] = { 0 };
			int nInc = Hex4ToAsc(pOccurrence + 2, pTmp);

			DeletePattern(pbBuffer, pdwLen, pOccurrence + 2, 4);
			pBufLast -= 4;
			InsertPattern(pbBuffer, pdwLen, pOccurrence + 2, pTmp, nInc);
			pbBuf += nInc;
			pBufLast += nInc;

			if(*pbBuf == 0x00 && *(pbBuf + 1) == 0x00 && *(pbBuf + 2) == 0x00 && *(pbBuf + 3) == 0x00 )
			{
				DeletePattern(pbBuffer, pdwLen, pbBuf, 4);
				pBufLast -= 4;
				BYTE pTmp[2] = { 0 };
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
				pbBuf += 1;
				pBufLast += 1;

				BYTE pTmp1[MAX_PATH] = { 0 };
				BYTE pTmp2[MAX_PATH] = { 0 };

				*pTmp1 = *pbBuf;
				*(pTmp1 + 1)= *(pbBuf + 1);
				nInc = Hex4ToAsc(pTmp1, pTmp2);
				DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
				pBufLast -= 2;
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
				pbBuf += nInc;
				pBufLast += nInc;
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
				pbBuf += 1;
				pBufLast += 1;

				*pTmp1 = *pbBuf;
				*(pTmp1 + 1)= *(pbBuf + 1);
				nInc = Hex4ToAsc(pTmp1, pTmp2);
				DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
				pBufLast -= 2;
				InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
				pbBuf += nInc;
				pBufLast += nInc;

				if(*pbBuf != 0)
				{
					InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
					pbBuf += 1;
					pBufLast += 1;

					*pTmp1 = *pbBuf;
					*(pTmp1 + 1)= *(pbBuf + 1);

					nInc = Hex4ToAsc(pTmp1, pTmp2);
					DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
					pBufLast -= 2;
					InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
					pbBuf += nInc;
					pBufLast += nInc;
				}
			}

			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
			pbBuf += 1;
			pBufLast += 1;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

void DAT_ClearX80(PBYTE pbBuffer, PDWORD pdwLen)
{
	unsigned char Pattern[18] = {
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80 
	};

	for(int i = 18; i > 0; i--)
	{
		PBYTE pbBuf = pbBuffer;
		PBYTE pBufLast = pbBuffer + *pdwLen;
		DWORD PatternSize = i;
		BOOL fFound;
		do 
		{
			PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, Pattern + PatternSize);
			fFound = (pOccurrence != pBufLast);
			if(fFound)
			{
				DeletePattern(pbBuffer, pdwLen, pOccurrence, PatternSize);
				pBufLast -= PatternSize;
				BYTE pTmp[MAX_PATH] = { 0 };
				*pTmp = ',';
				InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, 1);
				pbBuf = pOccurrence + 1;
				pBufLast += 1;
			}
			if( pbBuf >= pBufLast)
				break;
		} while (fFound);
	}
}

void DAT_Convert2Comma(PBYTE pbBuffer, PDWORD pdwLen, BYTE b)
{
	unsigned char Pattern[] = {
		0x04, 0x00, 0x00, 0x00 
	};
	Pattern[0] = b;
	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pbBuffer + *pdwLen;
	DWORD PatternSize = sizeof(Pattern);
	BOOL fFound;
	do 
	{
		PBYTE pOccurrence = std::search(pbBuf, pBufLast, Pattern, Pattern + PatternSize);
		fFound = (pOccurrence != pBufLast);
		if(fFound)
		{
			DeletePattern(pbBuffer, pdwLen, pOccurrence, PatternSize);
			pBufLast -= PatternSize;

			BYTE pTmp[MAX_PATH] = { 0 };
			*pTmp = ',';
			InsertPattern(pbBuffer, pdwLen, pOccurrence, pTmp, 1);
			pbBuf = pOccurrence + 1;
			pBufLast += 1;
		}
		if( pbBuf >= pBufLast)
			break;
	} while (fFound);
}

void DAT_FindFirstCRLF(PBYTE pbBuffer, PDWORD pdwLen)
{
	DWORD dwOffset;
	switch( nDatFormat )
	{
	case DAT_2:
		dwOffset = 30;
		break;
	}
	//base line
	DWORD dwNameNum = *(pbBuffer + 2);
	DeletePattern(pbBuffer, pdwLen, (DWORD)0, 3);

	DeletePattern(pbBuffer, pdwLen, dwNameNum, 4);

	BYTE pTmp1[2];
	*pTmp1 = ',';
	InsertPattern(pbBuffer, pdwLen, dwNameNum, pTmp1, 1);

	BYTE pTmp[MAX_PATH] = { 0 };
	int nInc = Hex4ToAsc(pbBuffer + dwNameNum + 1, pTmp);
	DeletePattern(pbBuffer, pdwLen, dwNameNum + 1, 4);
	InsertPattern(pbBuffer, pdwLen, dwNameNum + 1, pTmp, nInc);

	//first line
	BYTE pTmp2[] = { 0x0d, 0x0a };
	PBYTE pbBuf = pbBuffer + dwNameNum + 1 + nInc;

	InsertPattern(pbBuffer, pdwLen, dwNameNum + 1 + nInc, pTmp2, 2);

	pbBuf += 2;
	nInc = Hex4ToAsc(pbBuf, pTmp);

	DeletePattern(pbBuffer, pdwLen, pbBuf, 4);
	InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, nInc);
	pbBuf += nInc;

	if(*pbBuf == 0x00 && *(pbBuf + 1) == 0x00 && *(pbBuf + 2) == 0x00 && *(pbBuf + 3) == 0x00 )
	{
		DeletePattern(pbBuffer, pdwLen, pbBuf, 4);

		BYTE pTmp[2] = { 0 };
		*pTmp = ',';
		InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
		pbBuf += 1;

		BYTE pTmp1[MAX_PATH] = { 0 };
		BYTE pTmp2[MAX_PATH] = { 0 };

		*pTmp1 = *pbBuf;
		*(pTmp1 + 1)= *(pbBuf + 1);
		nInc = Hex4ToAsc(pTmp1, pTmp2);
		DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
		InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
		pbBuf += nInc;
		InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
		pbBuf += 1;

		*pTmp1 = *pbBuf;
		*(pTmp1 + 1)= *(pbBuf + 1);
		nInc = Hex4ToAsc(pTmp1, pTmp2);
		DeletePattern(pbBuffer, pdwLen, pbBuf, 2);
		InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
		pbBuf += nInc;

		if(*pbBuf != 0)
		{
			InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
			pbBuf += 1;

			*pTmp1 = *pbBuf;
			*(pTmp1 + 1)= *(pbBuf + 1);

			nInc = Hex4ToAsc(pTmp1, pTmp2);
			DeletePattern(pbBuffer, pdwLen, pbBuf, 2);

			InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp2, nInc);
			pbBuf += nInc;
		}
	}

	*pTmp = ',';
	InsertPattern(pbBuffer, pdwLen, pbBuf, pTmp, 1);
	pbBuf += 1;

}

void DAT_FindCRLF(PBYTE pbBuffer, PDWORD pdwLen)
{
	DAT_FindFirstCRLF(pbBuffer, pdwLen);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 16, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 17, 0x00);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 17, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 17, 0x02);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 21, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 25, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 26, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 0, 0x01, 28, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 17, 0x01, 26, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 17, 0x02, 26, 0x01);
	DAT_FindCRLF(pbBuffer, pdwLen, 25, 0x01, 27, 0x01);
	DAT_FindCRLFSpecial(pbBuffer, pdwLen);
}

//For effect
void DAT_Convert2(PBYTE pbBuffer, PDWORD pdwLen)
{
	DAT_FindCRLF(pbBuffer, pdwLen);
	DAT_PreProcess(pbBuffer, pdwLen);
	DAT_StraightenStringSet(pbBuffer, pdwLen);
	DAT_Find3ANDBracketSomeVar(pbBuffer, pdwLen);
	DAT_Find2AND1ORBracketSomeVar(pbBuffer, pdwLen);
	DAT_Find3ANDSomeVar(pbBuffer, pdwLen);
	DAT_Find3AND(pbBuffer, pdwLen);
	DAT_Find1AND(pbBuffer, pdwLen);
	DAT_FindEqualAndBigger(pbBuffer, pdwLen);
	DAT_FindQuoteForString(pbBuffer, pdwLen, NULL, NULL);
	DAT_FindSomeOR1AND(pbBuffer, pdwLen);
	DAT_Find1Minus1Divide(pbBuffer, pdwLen);
	DAT_FindSomeOR(pbBuffer, pdwLen);
	DAT_FindSomeAND(pbBuffer, pdwLen);
	DAT_Find1Power1Plus(pbBuffer, pdwLen);
	DAT_FindEqual(pbBuffer, pdwLen);
	DAT_FindEqualWithBracket(pbBuffer, pdwLen);
	DAT_FindPower(pbBuffer, pdwLen);
	DAT_Find2Power(pbBuffer, pdwLen);
	DAT_FindNumber(pbBuffer, pdwLen, NULL, NULL, FALSE);
	
	DAT_ClearX80(pbBuffer, pdwLen);
	DAT_Clear1(pbBuffer, pdwLen);
	DAT_Clear2(pbBuffer, pdwLen);
	DAT_ClearAndInsertNumber1(pbBuffer, pdwLen);
	DAT_ClearAndInsertNumber2(pbBuffer, pdwLen);
	DAT_MakeComma(pbBuffer, pdwLen);
}

