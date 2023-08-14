// Common.h : header file
//

#pragma once

void ConvertFloatToHex(float f_value, PSTR pStr);
float ConvertHexToFloat(PSTR pStr);
int Hex4ToAsc(PBYTE pBuf, PBYTE pStr);
void DeletePattern(PBYTE pbBuffer, PDWORD pdwLen, DWORD dwPos, DWORD dwSize);
void DeletePattern(PBYTE pbBuffer, PDWORD pdwLen, PBYTE pbPos, DWORD dwSize);
void InsertPattern(PBYTE pbBuffer, PDWORD pdwLen, DWORD dwPos, PBYTE pbBlock, DWORD dwSize);
void InsertPattern(PBYTE pbBuffer, PDWORD pdwLen, PBYTE dwPos, PBYTE pbBlock, DWORD dwSize);
