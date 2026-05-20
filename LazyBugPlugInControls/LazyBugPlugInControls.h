// LazyBugPlugInControls.h : main header file for the LazyBugPlugInControls DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CLazyBugPlugInControlsApp
// See LazyBugPlugInControls.cpp for the implementation of this class
//

class CLazyBugPlugInControlsApp : public CWinApp
{
public:
	CLazyBugPlugInControlsApp();

// Overrides
public:
	virtual BOOL InitInstance();
	int ExitInstance(); // return app exit code

	DECLARE_MESSAGE_MAP()
};
