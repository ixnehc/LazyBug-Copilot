// Proj_GuiLib.h : main header file for the Proj_GuiLib DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

#include "GuiLib.h"



// Initialize the DLL, register the classes etc
extern "C" AFX_EXT_API void WINAPI InitGuiLib();

