// Proj_WorldEditor2.h : main header file for the Proj_WorldEditor2 application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CLazyBugApp:
// See Proj_WorldEditor2.cpp for the implementation of this class
//

class CLazyBugApp : public CWinApp
{
public:
	CLazyBugApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	virtual int ExitInstance();
	afx_msg void OnFileNew();
 	afx_msg void OnFileOpen();
	afx_msg void OnAppAbout();
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

protected:
	BOOL _Open(const char *path);

	HINSTANCE   _hDllScintilla;

public:
};

extern CLazyBugApp theApp;