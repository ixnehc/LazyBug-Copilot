// Package.h

#pragma once

#include <atlstr.h>
#include <VSLCommandTarget.h>
#include <vsshell.h> // For IVsSolutionEvents etc.

#include "RDTEventsListener.h"

struct PackageState
{
	PackageState()
	{
		dwSolutionEventsCookie = VSCOOKIE_NIL;
		requestDetachFileChange = false;
		pRDTEventsListener = NULL;
	}
	CComPtr<IServiceProvider>pServiceProvider;
	CComPtr < IVsUIShellOpenDocument> pUIShellOpenDocument;

	CComPtr<IVsTextManager> pTextManager;

	CComPtr<IVsSolution>        pSolution;
	VSCOOKIE dwSolutionEventsCookie; 

	CComObject<CRDTEventsListener>* pRDTEventsListener;

	bool requestDetachFileChange;


};

extern PackageState g_ps;