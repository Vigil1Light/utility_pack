// Eva.cpp : Defines the entry point for the console 
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

#include "common.h"

enum EVA_FORMAT
{
	EVA_1, EVA_2, EVA_3, EVA_4, EVA_5, EVA_6, EVA_7
};

EVA_FORMAT nEvaFormat;

void EVA_ReplaceX028001withXffffffff(PBYTE pbBuffer, PDWORD pdwLen)
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

void EVA_Process5Bytes(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	PBYTE pbBuf;
	BYTE Pattern[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	DWORD dwPatternSize = sizeof(Pattern);
	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	Pattern[18] = 0x01;
	Pattern[19] = 0x01;
	pbBuf = pbBuffer;
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
	//-----------------------------
	Pattern[20] = 0x01;
	pbBuf = pbBuffer;
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
	//-----------------------------
	Pattern[17] = 0x01;
	Pattern[18] = 0x01;
	Pattern[19] = 0x00;
	Pattern[20] = 0x00;
	pbBuf = pbBuffer;
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

void EVA_ReplaceFileName1(PBYTE pbBuffer, PDWORD pdwLen)
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

EVA_FORMAT EVA_GetFormat(PBYTE pbBuffer, PDWORD pdwLen)
{
	unsigned char hexData1[19] = {
		0x00, 0x00, 0x08, 0x45, 0x56, 0x5F, 0x41, 0x63,
		0x74, 0x6F, 0x72, 0x64, 0x00, 0x00, 0x00, 0x02,
		0x80, 0x01, 0x02 
	};
	PBYTE pOccurrence;
	pOccurrence = std::search(pbBuffer, pbBuffer + 19, hexData1, hexData1 + 19);
	if(pOccurrence != pbBuffer + 19)
		return EVA_1;

	BYTE Pattern1[] = { 0x02, 0xff, 0xff, 0xff, 0xff };
	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern1, Pattern1 + sizeof(Pattern1));
	if(pOccurrence != pbBuffer + *pdwLen)
		return EVA_2;

	BYTE Pattern2[] = { 0x01, 0xff, 0xff, 0xff, 0xff };
	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern2, Pattern2 + sizeof(Pattern2));
	if(pOccurrence != pbBuffer + *pdwLen)
		return EVA_3;

	BYTE Pattern3[] = { 0xbb, 0x08, 0xe6 };
	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern3, Pattern3 + sizeof(Pattern3));
	if(pOccurrence != pbBuffer + *pdwLen)
		return EVA_4;
	return EVA_6;

}

void EVA_ProcessX02ffffff(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x02, 0xff, 0xff, 0xff, 0xff };
	BYTE AddPattern[] = { 0x00, 0x00, 0x0b, 0x45, 0x56, 0x5f, 0x53, 0x6b, 0x65, 0x6c, 0x53, 0x6b, 0x69, 0x6e, 0xff, 0xff, 0xff, 0xff  };

	DWORD dwPatternSize = sizeof(Pattern);
	DWORD dwAddPatternSize = sizeof(AddPattern);

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
			memcpy(pOccurrence, AddPattern, dwAddPatternSize);
			*pdwLen -= dwPatternSize;
			*pdwLen += dwAddPatternSize;
			memcpy(pOccurrence + dwAddPatternSize, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + dwAddPatternSize;
			pBufLast -= dwPatternSize;
			pBufLast += dwAddPatternSize;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void EVA_ProcessX02ffffff(PBYTE pbBuffer, PDWORD pdwLen, BYTE bIn, BYTE bOut)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x22, 0xbb, 0x08, 0xe6, 0x02, 0xff, 0xff, 0xff, 0xff };
	BYTE AddPattern[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x45, 0x56, 0x5f, 0x53, 0x6b, 0x65, 0x6c, 0x53, 0x6b, 0x69, 0x6e, 0xff, 0xff, 0xff, 0xff  };

	Pattern[0] = bIn;
	AddPattern[0] = bOut;

	DWORD dwPatternSize = sizeof(Pattern);
	DWORD dwAddPatternSize = sizeof(AddPattern);

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
			memcpy(pOccurrence, AddPattern, dwAddPatternSize);
			*pdwLen -= dwPatternSize;
			*pdwLen += dwAddPatternSize;
			memcpy(pOccurrence + dwAddPatternSize, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + dwAddPatternSize;
			pBufLast -= dwPatternSize;
			pBufLast += dwAddPatternSize;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void EVA_ProcessX01ffffff(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x01, 0xff, 0xff, 0xff, 0xff };
	BYTE AddPattern[] = { 0x00, 0x00, 0x09, 0x45, 0x56, 0x5f, 0x56, 0x61, 0x53, 0x6b, 0x69, 0x6e, 0xff, 0xff, 0xff, 0xff  };

	DWORD dwPatternSize = sizeof(Pattern);
	DWORD dwAddPatternSize = sizeof(AddPattern);

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
			memcpy(pOccurrence, AddPattern, dwAddPatternSize);
			*pdwLen -= dwPatternSize;
			*pdwLen += dwAddPatternSize;
			memcpy(pOccurrence + dwAddPatternSize, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + dwAddPatternSize;
			pBufLast -= dwPatternSize;
			pBufLast += dwAddPatternSize;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void EVA_ProcessX01ffffff(PBYTE pbBuffer, PDWORD pdwLen, BYTE bIn, BYTE bOut)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x29, 0xbb, 0x08, 0xe6, 0x01, 0xff, 0xff, 0xff, 0xff };
	BYTE AddPattern[] = { 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 0x45, 0x56, 0x5f, 0x56, 0x61, 0x53, 0x6b, 0x69, 0x6e, 0xff, 0xff, 0xff, 0xff  };

	Pattern[0] = bIn;
	AddPattern[0] = bOut;

	DWORD dwPatternSize = sizeof(Pattern);
	DWORD dwAddPatternSize = sizeof(AddPattern);

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
			memcpy(pOccurrence, AddPattern, dwAddPatternSize);
			*pdwLen -= dwPatternSize;
			*pdwLen += dwAddPatternSize;
			memcpy(pOccurrence + dwAddPatternSize, pbTmp, dwBackUpSize);
			free(pbTmp);

			pbBuf = pOccurrence + dwAddPatternSize;
			pBufLast -= dwPatternSize;
			pBufLast += dwAddPatternSize;

			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
}

void EVA_PreProcess(PBYTE pbBuffer, PDWORD pdwLen)
{
	pbBuffer[11] = 0x64; //0xd8->0x64

	DWORD dwBackUpSize;
	PBYTE pbTmp1, pbTmp2;
	pbTmp1 = (PBYTE)malloc(43);
	memcpy(pbTmp1, pbBuffer + 24, 43);
	*(pbTmp1 + 4) = 0x01;
	dwBackUpSize = *pdwLen - 72;
	pbTmp2 = (PBYTE)malloc(dwBackUpSize);
	memcpy(pbTmp2, pbBuffer + 72, dwBackUpSize);
	memcpy(pbBuffer + 24, pbTmp2, dwBackUpSize);
	free(pbTmp2);
	*pdwLen -= 43;
	*pdwLen -= 5;

	//---------------------move from first part to end------------
	PBYTE pOccurrence;
	unsigned char Pattern[20] = { // Full...Dynamic
		0x04, 0x00, 0x00, 0x00, 0x46, 0x75, 0x6C, 0x6C,
		0x07, 0x00, 0x00, 0x00, 0x44, 0x79, 0x6E, 0x61,
		0x6D, 0x69, 0x63, 0x01 
	};

	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern, Pattern + sizeof(Pattern));
	if(pOccurrence != pbBuffer + *pdwLen)
	{
		InsertPattern(pbBuffer, pdwLen, pOccurrence - pbBuffer, pbTmp1, 43);
		free(pbTmp1);
		return;
	}

	unsigned char Pattern1[17] = {//Null...Null
		0x04, 0x00, 0x00, 0x00, 0x4E, 0x75, 0x6C, 0x6C,
		0x04, 0x00, 0x00, 0x00, 0x4E, 0x75, 0x6C, 0x6C,
		0x01 
	};

	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern1, Pattern1 + sizeof(Pattern1));
	if(pOccurrence != pbBuffer + *pdwLen)
	{
		*pbTmp1 = 0x02;
		*(pOccurrence + 4) = 0x46;
		InsertPattern(pbBuffer, pdwLen, pOccurrence - pbBuffer, pbTmp1, 43);
		free(pbTmp1);
		return;
	}
	unsigned char Pattern2[20] = {//NULL...Dynamic
		0x04, 0x00, 0x00, 0x00, 0x4E, 0x75, 0x6C, 0x6C,
		0x07, 0x00, 0x00, 0x00, 0x44, 0x79, 0x6E, 0x61,
		0x6D, 0x69, 0x63, 0x01 
	};
	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern2, Pattern2 + sizeof(Pattern2));
	if(pOccurrence != pbBuffer + *pdwLen)
	{
		*(pOccurrence + 4) = 0x46;
		InsertPattern(pbBuffer, pdwLen, pOccurrence - pbBuffer, pbTmp1, 43);
		free(pbTmp1);
		return;
	}
}

void EVA_ProcessAfterRoot(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern[] = { 0x72, 0x6f, 0x6f, 0x74};//"root"
	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	PBYTE pOccurrence, pOccurrenceLast = NULL;
	do 
	{
		pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			pOccurrenceLast = pOccurrence;
			pbBuf = pOccurrence + dwPatternSize;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);
	if(pOccurrenceLast == NULL) return;
	DWORD dwPos = pOccurrenceLast - pbBuffer + dwPatternSize;
	BYTE pTmp[16] = { 0 };
	int i = 0;
	do
	{
		*(pTmp + i) = *(pOccurrenceLast + 4 + i);
		i++;
	}while( *(pOccurrenceLast + 4 + i) != 0 );
	
	if( i == 3)
	{
		DWORD dwBackUpSize = *pdwLen - 24;
		PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
		memcpy(pbTmp, pbBuffer + 24, dwBackUpSize);
		memcpy(pbBuffer + 20, pTmp, 3);
		*(pbBuffer + 23) = 0x00;
		*(pbBuffer + 24) = 0x00;
		memcpy(pbBuffer + 25, pbTmp, dwBackUpSize);
		free(pbTmp);
		*pdwLen += 1;
	}
	else if( i == 2)
	{
		if(pTmp[0] == 0x01 &&  pTmp[1] == 0x01)
		{
			DWORD dwBackUpSize = *pdwLen - 21;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pbBuffer + 21, dwBackUpSize);
			memcpy(pbBuffer + 20, pTmp, 2);
			*(pbBuffer + 20) = 0x02;
			memcpy(pbBuffer + 22, pbTmp, dwBackUpSize);
			free(pbTmp);
			*pdwLen += 1;
			DeletePattern(pbBuffer, pdwLen, 25, 4);
			nEvaFormat = EVA_5; 
		}
		else
		{
			DWORD dwBackUpSize = *pdwLen - 21;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pbBuffer + 21, dwBackUpSize);
			memcpy(pbBuffer + 20, pTmp, 2);
			memcpy(pbBuffer + 22, pbTmp, dwBackUpSize);
			free(pbTmp);
			*pdwLen += 1;
		}
	}
	else if( i == 1)
	{
		if(*(pOccurrenceLast + 4) == 0x04)
		{
			DWORD dwBackUpSize = *pdwLen - 21;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pbBuffer + 21, dwBackUpSize);
			memcpy(pbBuffer + 22, pbTmp, dwBackUpSize);
			free(pbTmp);

			unsigned char hexData[9] = {
				0x00, 0x00, 0x00, 0x00, 0x01, 0x23, 0xBB, 0x08,
				0xE6 
			};
			BYTE bTmp = 0x2e;
			pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, hexData, hexData + sizeof(hexData));
			if(pOccurrence != pbBuffer + *pdwLen)
			{
				bTmp = *(pOccurrence + sizeof(hexData));
			}
			*(pbBuffer + 20) = 0x01;
			*(pbBuffer + 21) = bTmp;
			*(pbBuffer + 22) = 0x00;
			*(pbBuffer + 23) = 0x00;
			*(pbBuffer + 24) = 0x00;
			*pdwLen += 1;

		}
		else
		{
			DWORD dwBackUpSize = *pdwLen - 21;
			PBYTE pbTmp = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp, pbBuffer + 21, dwBackUpSize);
			memcpy(pbBuffer + 20, pTmp, 2);
			memcpy(pbBuffer + 22, pbTmp, dwBackUpSize);
			free(pbTmp);
			*pdwLen += 1;
		}
	}
}
void EVA_DecryptOne(PBYTE pbBuf, PDWORD pdwSize)
{
	PBYTE pbTmp = (PBYTE)malloc(*pdwSize);
	memset(pbTmp, 0, *pdwSize);
	DWORD 	dwCont = 0;
	DWORD 	dwNum = 0;
	DWORD	dwTargetPos = 0;

	while(dwCont < *pdwSize)
	{
		BYTE bTmp;
		bTmp = pbBuf[dwCont];
		if(bTmp == 0x01 && dwCont + 33 + 9 <= *pdwSize && *(pbBuf + dwCont + 33) == 0x01 && *(pbBuf + dwCont + 33 + 9) == 0x01)
		{
			dwNum++;
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont + 34, 8);
			dwTargetPos += 8;
			
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont, 34);
			dwTargetPos += 34;
			dwCont += 34;

			dwCont += 8;
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont, 1);
			dwTargetPos += 1;
		}
		else if(bTmp == 0x02 && dwCont + 63 + 9 <= *pdwSize && *(pbBuf + dwCont + 63) == 0x01 && *(pbBuf + dwCont + 63 + 9) == 0x01)
		{
			dwNum++;
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont + 64, 8);
			dwTargetPos += 8;

			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont, 64);
			dwTargetPos += 64;
			dwCont += 64;

			dwCont += 8;
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont, 1);
			dwTargetPos += 1;
		}
		else if(bTmp == 0x03 && dwCont + 93 + 9 <= *pdwSize && *(pbBuf + dwCont + 93) == 0x01 && *(pbBuf + dwCont + 93 + 9) == 0x01)
		{
			dwNum++;
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont + 64, 8);
			dwTargetPos += 8;
			
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont, 94);
			dwTargetPos += 94;
			dwCont += 94;

			dwCont += 8;
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont, 1);
			dwTargetPos += 1;
		}
		else
		{
			memcpy(pbTmp + dwTargetPos, pbBuf + dwCont, 1);
			dwTargetPos += 1;
		}
		dwCont++;
	}
	memcpy(pbBuf, pbTmp, dwTargetPos);
	free(pbTmp);
	*pdwSize = dwTargetPos;

	//*(pbBuf + 10) = 0xae; 
	//*(pbBuf + 11) = 0x07; 
	DWORD dwPos;
	if(nEvaFormat == EVA_2)
		dwPos = 26;
	else if(nEvaFormat == EVA_5)
		dwPos = 14;
	DeletePattern(pbBuf, pdwSize, dwPos, 4);
}

void EVA_DecryptAll(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[] = {
		0x01, 0x01, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00 
	};

	DWORD dwPatternSize = sizeof(Pattern);

	PBYTE pBufferLast = pbBuffer + *pdwLen;
	PBYTE pPatternLast = Pattern + dwPatternSize;

	PBYTE pbBuf = pbBuffer;
	PBYTE pBufLast = pBufferLast;
	PBYTE pOccurrence;
	do 
	{
		pOccurrence = std::search(pbBuf, pBufLast, Pattern, pPatternLast);
		fFound = (pOccurrence != pBufLast);
		if( fFound )
		{
			BYTE Pattern2[] = { 0x02, 0xff, 0xff, 0xff, 0xff  };//search End pattern
			DWORD dwPatternSize2 = sizeof(Pattern2);
			PBYTE pPatternLast2 = Pattern2 + dwPatternSize2;
			PBYTE pOccurrence2 = std::search(pOccurrence, pBufLast, Pattern2, pPatternLast2);
			PBYTE pOccurrenceLast = NULL;
			if(pOccurrence2 == pBufLast)
			{
				pbBuf = pOccurrence;
				unsigned char Pattern4[] = {
					0x01, 0x38, 0x02, 0x00, 0x00 
				};
				Pattern4[0] = *(pbBuffer + 20);
				Pattern4[1] = *(pbBuffer + 21);
				Pattern4[2] = *(pbBuffer + 22);
				do 
				{
					PBYTE pOccurrence1 = std::search(pbBuf, pbBuffer + *pdwLen, Pattern4, Pattern4 + sizeof(Pattern4));
					fFound = (pOccurrence1 != pbBuffer + *pdwLen);
					if( fFound )
					{
						pOccurrenceLast = pOccurrence1;
						pbBuf = pOccurrence1 + sizeof(Pattern4);
						if( pbBuf >= pbBuffer + *pdwLen)
							break;
					}
					pOccurrence2 = pOccurrenceLast;
				} while (fFound);
			}
			DWORD dwBackUpSize = *pdwLen - (pOccurrence2 - pbBuffer) - dwPatternSize2;
			PBYTE pbTmp2 = (PBYTE)malloc(dwBackUpSize);
			memcpy(pbTmp2, pOccurrence2 + dwPatternSize2, dwBackUpSize);

			DWORD dwSize = (pOccurrence2 - pOccurrence) + dwPatternSize2;
			PBYTE pbTmp1 = (PBYTE)malloc(dwSize + 1000);
			memcpy(pbTmp1, pOccurrence, dwSize);
			*pdwLen -= dwSize;
			EVA_DecryptOne(pbTmp1, &dwSize);
			*pdwLen += dwSize;
			memcpy(pOccurrence, pbTmp1, dwSize);
			memcpy(pOccurrence + dwSize, pbTmp2, dwBackUpSize);
			free(pbTmp1);
			free(pbTmp2);
			pbBuf = pOccurrence + dwSize;
			pBufLast = pbBuf + dwBackUpSize;
			if( pbBuf >= pBufLast)
				break;
		}
	} while (fFound);

}

void EVA_ProcessFromBB08E6ToXBB08E6(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pOccurrence;
	DWORD dwPosInsert = 25;
	//DeletePattern(pbBuffer, pdwLen, dwPosInsert, 4);

	unsigned char Pattern2[23] = {
		0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x65,
		0x66, 0x66, 0x65, 0x63, 0x74, 0x06, 0x00, 0x00,
		0x00, 0x65, 0x66, 0x66, 0x65, 0x63, 0x74 
	};
	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern2, Pattern2 + sizeof(Pattern2));
	pOccurrence -= 1;
	DWORD dwPosDelete = pOccurrence - pbBuffer;
	unsigned char Pattern3[] = {
		0xBB, 0x08, 0xE6 
	};
	PBYTE pOccurrence1 = std::search(pOccurrence, pbBuffer + *pdwLen, Pattern3, Pattern3 + sizeof(Pattern3));
	pOccurrence1 -= 1;

	DWORD dwSize = pOccurrence1 - pOccurrence + 4;

	PBYTE pbTmp = (PBYTE)malloc(dwSize);
	memset(pbTmp, 0, dwSize);
	memcpy(pbTmp, pOccurrence, dwSize - 4);

	DeletePattern(pbBuffer, pdwLen, dwPosDelete, dwSize);
	InsertPattern(pbBuffer, pdwLen, dwPosInsert, pbTmp, dwSize);
	free(pbTmp);
}

void EVA_ProcessFromPos25ToX0100000000000001(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pOccurrence;
	DWORD dwPosInsert = 25;
	//DeletePattern(pbBuffer, pdwLen, dwPosInsert, 4);

	unsigned char Pattern2[23] = {
		0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x65,
		0x66, 0x66, 0x65, 0x63, 0x74, 0x06, 0x00, 0x00,
		0x00, 0x65, 0x66, 0x66, 0x65, 0x63, 0x74 
	};
	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern2, Pattern2 + sizeof(Pattern2));
	if(pOccurrence == pbBuffer + *pdwLen)
	{
		unsigned char Pattern2[19] = {//wait...wait
			0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x77,
			0x61, 0x69, 0x74, 0x04, 0x00, 0x00, 0x00, 0x77,
			0x61, 0x69, 0x74 
		};

		pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern2, Pattern2 + sizeof(Pattern2));
	}
	pOccurrence -= 1;
	DWORD dwPosDelete = pOccurrence - pbBuffer;
	unsigned char Pattern3[] = {
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01 
	};
	BOOL fFound;
	PBYTE pOccurrenceLast = NULL;
	PBYTE pbBuf = pbBuffer, pOccurrence1;
	do 
	{
		pOccurrence1 = std::search(pbBuf, pbBuffer + *pdwLen, Pattern3, Pattern3 + sizeof(Pattern3));
		fFound = (pOccurrence1 != pbBuffer + *pdwLen);
		if( fFound )
		{
			pOccurrenceLast = pOccurrence1;
			pbBuf = pOccurrence1 + sizeof(Pattern3);
			if( pbBuf >= pbBuffer + *pdwLen)
				break;
		}
	} while (fFound);


	DWORD dwSize = pOccurrenceLast - pOccurrence + sizeof(Pattern3);

	PBYTE pbTmp = (PBYTE)malloc(dwSize);
	memcpy(pbTmp, pOccurrence, dwSize);

	DeletePattern(pbBuffer, pdwLen, dwPosDelete, dwSize);
	InsertPattern(pbBuffer, pdwLen, dwPosInsert, pbTmp, dwSize);

	free(pbTmp);
}

void EVA_ProcessFromPos25ToLastX02FFFFFFFF(PBYTE pbBuffer, PDWORD pdwLen)
{
	PBYTE pOccurrence;
	DWORD dwPosInsert = 25;//pOccurrence - pbBuffer;

	//DeletePattern(pbBuffer, pdwLen, dwPosInsert, 4);

	unsigned char Pattern2[23] = {//attack...attack
		0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x61,
		0x74, 0x74, 0x61, 0x63, 0x6B, 0x06, 0x00, 0x00,
		0x00, 0x61, 0x74, 0x74, 0x61, 0x63, 0x6B 
	};
	pOccurrence = std::search(pbBuffer, pbBuffer + *pdwLen, Pattern2, Pattern2 + sizeof(Pattern2));
	if(pOccurrence == pbBuffer + *pdwLen)
		return;
	pOccurrence -= 1;
	DWORD dwPosDelete = pOccurrence - pbBuffer;

	BOOL fFound;
	PBYTE pbBuf = pOccurrence;
	PBYTE pOccurrence1, pOccurrenceLast = NULL;

	pbBuf = pOccurrence;
	if(pOccurrenceLast == NULL)
	{
		unsigned char Pattern4[] = {
			0x01, 0x38, 0x02, 0x00, 0x00 
		};
		Pattern4[0] = *(pbBuffer + 20);
		Pattern4[1] = *(pbBuffer + 21);
		Pattern4[2] = *(pbBuffer + 22);
		do 
		{
			pOccurrence1 = std::search(pbBuf, pbBuffer + *pdwLen, Pattern4, Pattern4 + sizeof(Pattern4));
			fFound = (pOccurrence1 != pbBuffer + *pdwLen);
			if( fFound )
			{
				pOccurrenceLast = pOccurrence1;
				pOccurrenceLast += 1;
				pbBuf = pOccurrence1 + sizeof(Pattern4);
				if( pbBuf >= pbBuffer + *pdwLen)
					break;
			}
		} while (fFound);
	}

	pbBuf = pOccurrence;
	if(pOccurrenceLast == NULL)
	{
		unsigned char Pattern3[] = {
			0x02, 0xff, 0xff, 0xff, 0xff 
		};
		do 
		{
			pOccurrence1 = std::search(pbBuf, pbBuffer + *pdwLen, Pattern3, Pattern3 + sizeof(Pattern3));
			fFound = (pOccurrence1 != pbBuffer + *pdwLen);
			if( fFound )
			{
				pOccurrenceLast = pOccurrence1;
				pbBuf = pOccurrence1 + sizeof(Pattern3);
				if( pbBuf >= pbBuffer + *pdwLen)
					break;
			}
		} while (fFound);

	}
	DWORD dwSize = pOccurrenceLast - pOccurrence;

	PBYTE pbTmp = (PBYTE)malloc(dwSize);
	memcpy(pbTmp, pOccurrence, dwSize);

	DeletePattern(pbBuffer, pdwLen, dwPosDelete, dwSize);
	InsertPattern(pbBuffer, pdwLen, dwPosInsert, pbTmp, dwSize);
	free(pbTmp);
}

void EVA_FoundX0138AndInsertX00000000(PBYTE pbBuffer, PDWORD pdwLen)
{
	unsigned char Pattern1[10] = {
		0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
		0x38, 0x00 
	};
	Pattern1[0] = *(pbBuffer + 20);
	Pattern1[1] = *(pbBuffer + 21);
	Pattern1[2] = *(pbBuffer + 22);

	PBYTE pOccurrence1 = std::search(pbBuffer, pbBuffer + *pdwLen , Pattern1, Pattern1 + sizeof(Pattern1));
	if(pOccurrence1 == pbBuffer + *pdwLen)
		return;
	DWORD dwPos = pOccurrence1 - pbBuffer + 8; 

	unsigned char Pattern2[] = {
		0x00, 0x00, 0x00, 0x00
	};
	InsertPattern(pbBuffer, pdwLen, dwPos, Pattern2, 4);
}

void EVA_DeleteFromX02FFFFFFFFToX0101(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern1[] = { 0x02, 0xff, 0xff, 0xff, 0xff };

	DWORD dwPos;
	
	PBYTE pbBuf = pbBuffer, pOccurrence1 ,pOccurrenceLast = NULL;
	do 
	{
		pOccurrence1 = std::search(pbBuf, pbBuffer + *pdwLen , Pattern1, Pattern1 + sizeof(Pattern1));
		fFound = (pOccurrence1 != pbBuffer + *pdwLen);
		if( fFound )
		{
			pOccurrenceLast = pOccurrence1;
			pbBuf = pOccurrence1 + sizeof(Pattern1);
			if( pbBuf >= pbBuffer + *pdwLen)
				break;
		}
	} while (fFound);

	if(pOccurrenceLast == NULL)
		return;
	pOccurrence1 = pOccurrenceLast + sizeof(Pattern1);
	dwPos = pOccurrence1 - pbBuffer; 

	pbBuf = pOccurrence1;
	unsigned char Pattern2[] = {
		0x01, 0x38, 0x02, 0x00, 0x00 
	};
	Pattern2[0] = *(pbBuffer + 20);
	Pattern2[1] = *(pbBuffer + 21);
	Pattern2[2] = *(pbBuffer + 22);
	if(pOccurrenceLast == NULL)
	{
		do 
		{
			pOccurrence1 = std::search(pbBuf, pbBuffer + *pdwLen, Pattern2, Pattern2 + sizeof(Pattern2));
			fFound = (pOccurrence1 != pbBuffer + *pdwLen);
			if( fFound )
			{
				pOccurrenceLast = pOccurrence1;
				pOccurrenceLast += 1;
				pbBuf = pOccurrence1 + sizeof(Pattern2);
				if( pbBuf >= pbBuffer + *pdwLen)
					break;
			}
		} while (fFound);
	}

	if(pOccurrenceLast != NULL)
	{
		DeletePattern(pbBuffer, pdwLen, dwPos, pOccurrenceLast - pOccurrence1 + sizeof(Pattern2));
	}
}

void EVA_Process0x01X40x00X3(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	BYTE Pattern1[] = { 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00 };

	PBYTE pbBuf = pbBuffer, pOccurrence1 ,pOccurrenceLast = NULL;
	do 
	{
		pOccurrence1 = std::search(pbBuf, pbBuffer + *pdwLen , Pattern1, Pattern1 + sizeof(Pattern1));
		fFound = (pOccurrence1 != pbBuffer + *pdwLen);
		if( fFound )
		{
			pOccurrenceLast = pOccurrence1;
			pbBuf = pOccurrence1 + sizeof(Pattern1);
			if( pbBuf >= pbBuffer + *pdwLen)
				break;
		}
	} while (fFound);

	if(pOccurrenceLast == NULL)
		return;
	BYTE pbBlock[] = {0x00, 0x00, 0x00, 0x00};
	InsertPattern(pbBuffer, pdwLen, pOccurrenceLast - pbBuffer + 3, pbBlock, 4);
}


void EVA_ProcessX000B00(PBYTE pbBuffer, PDWORD pdwLen)
{
	BOOL fFound;
	unsigned char Pattern[9] = {
		0x01, 0x00, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x00,
		0x01 
	};
	PBYTE pbBuf = pbBuffer, pOccurrence;
	do 
	{
		pOccurrence = std::search(pbBuf, pbBuffer + *pdwLen , Pattern, Pattern + sizeof(Pattern));
		fFound = (pOccurrence != pbBuffer + *pdwLen);
		if( fFound )
		{
			DWORD dwPos = pOccurrence - pbBuffer + 3;
			DeletePattern(pbBuffer, pdwLen, dwPos, 5);
			pbBuf = pOccurrence + sizeof(Pattern);
			if( pbBuf >= pbBuffer + *pdwLen)
				break;
		}
	} while (fFound);
	//Delete
	//unsigned char hexData[35] = {
	//	0x41, 0x64, 0x64, 0x0A, 0x00, 0x00, 0x00, 0x43,
	//	0x61, 0x6D, 0x65, 0x72, 0x61, 0x20, 0x4E, 0x6F,
	//	0x72, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//	0x00, 0x00, 0x00 
	//};
	//pbBuf = pbBuffer;
	//do 
	//{
	//	pOccurrence = std::search(pbBuf, pbBuffer + *pdwLen , hexData, hexData + sizeof(hexData));
	//	fFound = (pOccurrence != pbBuffer + *pdwLen);
	//	if( fFound )
	//	{
	//		DWORD dwPos = pOccurrence - pbBuffer + 18;
	//		DeletePattern(pbBuffer, pdwLen, dwPos, 17);
	//		pbBuf = pOccurrence;
	//		if( pbBuf >= pbBuffer + *pdwLen)
	//			break;
	//	}
	//} while (fFound);


};
