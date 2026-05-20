// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

	// Modify the following defines if you have to target a platform prior to the ones specified below.
	// Refer to MSDN for the latest info on corresponding values for different platforms.
	#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
	#define WINVER 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
	#endif

	#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
	#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target Windows 2000 or later.
	#endif						

	#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
	#define _WIN32_WINDOWS 0x0501 // Change this to the appropriate value to target Windows Me or later.
	#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

//Copied from #include <BaseTsd.h>
#ifdef _WIN64
typedef __int64 SHANDLE_PTR;
#else
typedef long SHANDLE_PTR;
#endif
//

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
//#include <afxole.h>         // MFC OLE classes
//#include <afxodlgs.h>       // MFC OLE dialog classes
//#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
//#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
//#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <GdiPlus.h>

using namespace Gdiplus;

#pragma comment( lib, "gdiplus.lib" )



#define _XTP_EXCLUDE_GRAPHICLIBRARY

//#define _XTP_EXCLUDE_CONTROLS

//#define _XTP_EXCLUDE_COMMANDBARS

#define _XTP_EXCLUDE_DOCKINGPANE

//#define _XTP_EXCLUDE_PROPERTYGRID

//#define _XTP_EXCLUDE_REPORTCONTROL

#define _XTP_EXCLUDE_CALENDAR

#define _XTP_EXCLUDE_TASKPANEL

#define _XTP_EXCLUDE_SHORTCUTBAR

#define _XTP_EXCLUDE_SKINFRAMEWORK

#define _XTP_EXCLUDE_RIBBON

#define _XTP_EXCLUDE_SYNTAXEDIT

#include <XTToolkitPro.h>       // Xtreme Toolkit support


#include "resource.h"

#include "commondefines/general.h"

#include "math/imath_all.h"

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"
#include "gds/GProp.h"

#include <vector>
#include <map>
#include <string>
#include <tchar.h>

#include "stringparser/stringparser.h"


