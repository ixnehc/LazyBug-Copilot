// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0600		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0600		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0600 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off MFC's hiding of some common and often safely ignored warning messages
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <XTToolkitPro.h>	// Xtreme Toolkit Pro components

#include "commondefines/general.h"

#include "math/imath_all.h"
#include "timer/timer.h"

#include "Resource.h"

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;


#define always_assert(condition) \
    do { \
        if (!(condition)) { \
            *((volatile int*)0) = 0;  /* 故意访问空指针 */ \
        } \
    } while(0)

// 替换 TRACE 的宏
#define MY_TRACE(fmt, ...) do { \
    static wchar_t buf[64*1024]; \
    swprintf_s(buf, fmt, __VA_ARGS__); \
    OutputDebugStringW(buf); \
} while(0)

//#define LOCAL_SOLUTIONDB_SERVICE

#include "LazyBugConfig.h"

#include "CoreDefines.h"

// 禁用 WebView2 默认右键上下文菜单
// 定义此宏将禁用所有 WebView2 控件的右键菜单
//#define DISABLE_WEBVIEW_CONTEXTMENU

// #include "class/class.h"
// #include "stringparser/stringparser.h"
// #include "editor/editor.h"
// #include "datapacket/DataPacket.h"
// #include "gds/GObj.h"
// #include "gds/GStub.h"
// #include "gds/GProp.h"
// #include "timer/profiler.h"
// 
//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
