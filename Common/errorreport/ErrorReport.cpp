#include "stdh.h"
#include ".\ErrorReport.h"
//#include "resource.h"
#include "windowsx.h"

#include <DbgHelp.h>

#ifndef _T
#define _T(a) a
#endif

#define PRODUCT_NAME	_T("IxEngine Tools")

#ifdef _DEBUG
#define	ERRREP_SENDER	_T("ErrRep_D.exe")
#else
#define	ERRREP_SENDER	_T("ErrRep.exe")
#endif

static TCHAR			ErrReportPath[MAX_PATH + 1];
static TCHAR			DumpFileName[MAX_PATH + 1];
static TCHAR			ProgPath[MAX_PATH + 1];
static HANDLE			hProcess;
static DWORD			ProcessId;
static HMODULE			MainModule;
static TCHAR			Buffer[0x2000]; // 防止错误发生时, 申请内存会失败

static LPCTSTR			ReportText = _T("程序发生了未预知的错误. 我们对给您造成的不便感到抱歉.\n")
									_T("错误信息文件被保存在bin\\dump目录下,请将此文件提交给相关的程序员.\n\n");

static LONG CALLBACK MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo);
static void ShowErrReportDlg();
static bool GenerateDump2;

typedef BOOL
(WINAPI* TMiniDumpWriteDump)(
				  IN HANDLE hProcess,
				  IN DWORD ProcessId,
				  IN HANDLE hFile,
				  IN MINIDUMP_TYPE DumpType,
				  IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
				  IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
				  IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
				  );

static TMiniDumpWriteDump WriteDump;

bool InitErrorReport(LPCTSTR StorePath /* = NULL */, bool GenDump2 /* = false */, bool IsSvr /* = false */)
{
	HMODULE hDbgHlp = LoadLibrary(_T("dbghelp.dll"));
	if (hDbgHlp == NULL)
		return false;

	WriteDump = (TMiniDumpWriteDump )GetProcAddress(hDbgHlp, "MiniDumpWriteDump");
	if (WriteDump == NULL)
		return false;

	GenerateDump2 = GenDump2;

	TCHAR* c;
	hProcess = GetCurrentProcess();
	ProcessId = GetCurrentProcessId();
	MainModule = GetModuleHandle(NULL);

	GetModuleFileName(MainModule, ProgPath, MAX_PATH);

	if (StorePath == NULL) {
		lstrcpy(ErrReportPath, ProgPath);
		c = ErrReportPath + lstrlen(ErrReportPath);
		while (c != ErrReportPath) {
			if (*c == _T('\\')) {
				*c = 0;
				break;
			}

			c --;
		}
	} else 
		lstrcpy(ErrReportPath, StorePath);

	SetErrorMode(SEM_NOGPFAULTERRORBOX);		
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

	return true;
}

static LONG CALLBACK MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo)
{
	// 为了防止破坏出错现场信息, 将局部变量移出为全局变量
// 	sym_engine::stack_trace(L_CRITICAL,ExceptionInfo->ContextRecord,"未处理的异常.");

	ShowErrReportDlg();


	static _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
	static HANDLE hDumpFile;
	static BOOL Result;

	static SYSTEMTIME st;

	wsprintf(DumpFileName, _T("%s\\dump"), ErrReportPath);
	CreateDirectory(DumpFileName,NULL);
	GetLocalTime(&st);
	wsprintf(DumpFileName, _T("%s\\dump\\%04d-%02d-%02d_%02d.%02d.%02d.%03d.dmp"), ErrReportPath, st.wYear, 
		st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	hDumpFile = CreateFile(DumpFileName, GENERIC_WRITE, 0, NULL, 
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDumpFile != INVALID_HANDLE_VALUE) {
		
		ExInfo.ThreadId = ::GetCurrentThreadId();
		ExInfo.ExceptionPointers = ExceptionInfo;
		ExInfo.ClientPointers = FALSE;

		Result = WriteDump(hProcess, ProcessId, hDumpFile, MiniDumpNormal, 
			&ExInfo, NULL, NULL);
		CloseHandle(hDumpFile);

		if (!Result) {

			// _ftprintf(stderr, _T("cannot dump file\n"));
			return EXCEPTION_EXECUTE_HANDLER;
		}

		if (GenerateDump2) {
			wsprintf(Buffer, _T("%s2"), DumpFileName);
			hDumpFile = CreateFile(Buffer, GENERIC_WRITE, 0, NULL, 
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hDumpFile != INVALID_HANDLE_VALUE) {
				WriteDump(hProcess, ProcessId, hDumpFile, MiniDumpWithDataSegs, 
					&ExInfo, NULL, NULL);
				CloseHandle(hDumpFile);
			}
		}


	}
	
	return EXCEPTION_EXECUTE_HANDLER;
}


static void ShowErrReportDlg()
{
	MessageBox(NULL, ReportText, PRODUCT_NAME, MB_OK);
}
