
// Live-Equestre-Pro.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLiveEquestreProApp:
// See Live-Equestre-Pro.cpp for the implementation of this class
//

class CLiveEquestreProApp : public CWinApp
{
public:
	CLiveEquestreProApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CLiveEquestreProApp theApp;