// test.h : main header file for the test DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CtestApp
// See test.cpp for the implementation of this class
//

class CtestApp : public CWinApp
{
public:
	CtestApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
