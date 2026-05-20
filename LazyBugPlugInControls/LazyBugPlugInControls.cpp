// LazyBugPlugInControls.cpp : Defines the initialization routines for the DLL.
//

#include "stdh.h"
#include "framework.h"
#include "LazyBugPlugInControls.h"

#include "Registry/Registry.h"
#include "stringparser/stringparser.h"

#include "../Proj_LazyBug/SolutionDBApi.h"

#include <shlwapi.h>
#include <DbgHelp.h>
#include <time.h>
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "DbgHelp.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CCurrentUserRegistry g_reg("IxSoftware", "IxEngine");

HINSTANCE g_hInstance;
char g_szDllDir[MAX_PATH] = { 0 };           // 保存 DLL 所在目录 (ANSI)
std::string g_szDllDirUtf8;                   // 保存 DLL 所在目录 (UTF-8)

BOOL GetWndClippedRect(CWnd* pWnd, CRect& rc, BOOL bConsiderChildren)
{
	CRgn rgn;
	CRect rcT;
	pWnd->GetClientRect(&rcT);
	rgn.CreateRectRgnIndirect(&rcT);
	CDC* pDC;
	if (bConsiderChildren)
		pDC = pWnd->GetDCEx(NULL, DCX_PARENTCLIP | DCX_CACHE | DCX_CLIPSIBLINGS | DCX_CLIPCHILDREN);
	else
		pDC = pWnd->GetDCEx(NULL, DCX_PARENTCLIP | DCX_CACHE | DCX_CLIPSIBLINGS);
	//	CDC *pDC=pWnd->GetDC();
	if (!pDC)
		return FALSE;
	pDC->GetClipBox(&rc);
	//	CPoint pt=pDC->GetViewportOrg();
	//	CSize sz=pDC->GetViewportExt();
	pWnd->ReleaseDC(pDC);

	pWnd->ClientToScreen(&rc);
	return TRUE;
}



BOOL CheckWndDescendant(CWnd* pWnd, CWnd* pWndToCheck)
{
	if (pWndToCheck == pWnd)
		return FALSE;
	CWnd* wnd = pWndToCheck;
	while (wnd)
	{
		if (wnd == pWnd)
			return TRUE;
		wnd = wnd->GetParent();
	}
	return FALSE;
}


//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

// CLazyBugPlugInControlsApp

BEGIN_MESSAGE_MAP(CLazyBugPlugInControlsApp, CWinApp)
END_MESSAGE_MAP()


// CLazyBugPlugInControlsApp construction

CLazyBugPlugInControlsApp::CLazyBugPlugInControlsApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CLazyBugPlugInControlsApp object

CLazyBugPlugInControlsApp theApp;

ULONG_PTR g_gdiplusToken;

// Minidump support
LONG WINAPI MyUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
	// 获取当前时间
	time_t now = time(NULL);
	struct tm localTime;
	localtime_s(&localTime, &now);
	
	// 生成dump文件名: DllDir\LazyBugPlugInControls_YYYYMMDD_HHMMSS.dmp
	char szDumpPath[MAX_PATH] = {0};
	sprintf_s(szDumpPath, MAX_PATH, "%s\\LazyBugPlugInControls_%04d%02d%02d_%02d%02d%02d.dmp",
		g_szDllDir,
		localTime.tm_year + 1900,
		localTime.tm_mon + 1,
		localTime.tm_mday,
		localTime.tm_hour,
		localTime.tm_min,
		localTime.tm_sec);
	
	// 创建dump文件
	HANDLE hFile = CreateFileA(szDumpPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION mdei;
		mdei.ThreadId = GetCurrentThreadId();
		mdei.ExceptionPointers = pExceptionInfo;
		mdei.ClientPointers = FALSE;
		
		// 写入minidump (使用MiniDumpNormal获取基本信息,或MiniDumpWithFullMemory获取完整内存)
		MiniDumpWriteDump(
			GetCurrentProcess(),
			GetCurrentProcessId(),
			hFile,
			MiniDumpNormal,  // 可改为 MiniDumpWithFullMemory 获取更完整信息
			&	mdei,
			NULL,
			NULL);
		
		CloseHandle(hFile);
	}
	
	// 返回EXCEPTION_EXECUTE_HANDLER表示已处理,程序将终止
	// 返回EXCEPTION_CONTINUE_SEARCH继续搜索其他handler
	return EXCEPTION_EXECUTE_HANDLER;
}

// CLazyBugPlugInControlsApp initialization

BOOL CLazyBugPlugInControlsApp::InitInstance()
{
	CWinApp::InitInstance();

	AfxOleInit();

	g_hInstance = m_hInstance;

	// 计算 DLL 所在目录 (使用宽字符版本，避免中文路径问题)
	wchar_t wszDllDir[MAX_PATH] = { 0 };
	GetModuleFileNameW(m_hInstance, wszDllDir, MAX_PATH);
	PathRemoveFileSpecW(wszDllDir);
	
	// 转换为 ANSI 格式 (用于兼容旧代码)
	WideCharToMultiByte(CP_ACP, 0, wszDllDir, -1, g_szDllDir, MAX_PATH, NULL, NULL);
	
	// 转换为 UTF-8 格式
	g_szDllDirUtf8 = widechar_to_utf8(wszDllDir);

	// 设置未处理异常过滤器,用于生成minidump
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	// 启用C++异常转SEH异常（捕获未处理的C++异常）
	_set_se_translator([](unsigned int code, EXCEPTION_POINTERS* pExc) {
		throw std::exception("SEH exception converted");
	});

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Status v = Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, NULL);

	SolutionDB_EnsureConnected();

	return TRUE;
}

int CLazyBugPlugInControlsApp::ExitInstance()
{
	Gdiplus::GdiplusShutdown(g_gdiplusToken);
	AfxOleTerm();

	return CWinApp::ExitInstance();
}

// 导出函数：返回 DLL 所在目录 (UTF-8 格式)
const char* GetCurModuleFolderPath_utf8()
{
	return g_szDllDirUtf8.c_str();
}
