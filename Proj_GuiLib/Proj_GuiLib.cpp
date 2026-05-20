// Proj_GuiLib.cpp : Defines the initialization routines for the DLL.
//

#include "stdh.h"
#include "Proj_GuiLib.h"

#include "Registry/Registry.h"
#include "Log/LogDump.h"

#include "timer/profiler.h"
#include "strlib/strlib.h"
//#include "vld/vld.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif 

/////////////////////////////////////////////////////////////////////////////
// Initialization of MFC Extension DLL

#include "afxdllx.h"    // standard MFC Extension DLL routines

static AFX_EXTENSION_MODULE NEAR extensionDLL = { NULL, NULL };

HINSTANCE g_hInstance;
ULONG_PTR g_gdiplusToken;

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Extension DLL one-time initialization - do not allocate memory here,
		//   use the TRACE or ASSERT macros or call MessageBox
		if (!AfxInitExtensionModule(extensionDLL, hInstance))
			return 0;

		g_hInstance=hInstance;

		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);
		// Other initialization could be done here, as long as
		// it doesn't result in direct or indirect calls to AfxGetApp.
		// This extension DLL doesn't need to access the app object
		// but to be consistent with testdll1.dll, this DLL requires
		// explicit initialization as well (see below).

		// This allows for greater flexibility later in development.
	}
	if (dwReason==DLL_PROCESS_DETACH)
	{
		Gdiplus::GdiplusShutdown(g_gdiplusToken);
	}
	return 1;   // ok
}

// Exported DLL initialization is run in context of running application
extern "C" void WINAPI InitGuiLib()
{
	// create a new CDynLinkLibrary for this app
	new CDynLinkLibrary(extensionDLL);
	// nothing more to do
}


CCurrentUserRegistry g_reg("Passion Entertainment","IxEngine");


GuiLib_Api void RegisterGuiLogHandler(LogHandler handler)
{
	RegisterLogHandler(handler);
}

GuiLib_Api ProfilerMgr *GetGuiProfilerMgr()
{
	ProfilerMgr *mgr=GetProfilerMgr();
	mgr->SetName("GuiLib");
	return mgr;
}

GuiLib_Api void SetGuiStrLib(CStrLib *strlib)
{
	::StrLib_Set(strlib);
}