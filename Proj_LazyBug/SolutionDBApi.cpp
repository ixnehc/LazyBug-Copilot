#include "stdh.h"
#include <memory>
#include <tlhelp32.h>
#include "SolutionDBClient.h"
#include "SolutionDBMsgs.h"
#include "stringparser/stringparser.h"

#include "SolutionDBApi.h"

#include "Utils.h"

CSolutionDBClient g_solutionDBClient;


void StartLazyBugService()
{
	extern const char* GetCurModuleFolderPath_utf8();

	std::string currentPath = GetCurModuleFolderPath_utf8();
	// 构建LazyBugService.exe的完整路径
	std::string strServicePath = currentPath;
	strServicePath += _T("\\"LAZYBUG_SERVICE_EXENAME);

	// 检查文件是否存在
	if (!Utils::IsFileExist(strServicePath.c_str()))
	{
// 		// 文件不存在，记录日志或显示错误
// 		OutputDebugString(_T("LazyBugService.exe not found in current directory\n"));
		return;
	}

	// 设置启动信息
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;  // 使用wShowWindow标志
	si.wShowWindow = SW_HIDE;           // 隐藏窗口

	ZeroMemory(&pi, sizeof(pi));

	// 创建新进程，不继承句柄，隐藏窗口
	if (!CreateProcessW(
		utf8_to_widechar(strServicePath).c_str(),    // 应用程序路径
		NULL,              // 命令行参数
		NULL,              // 进程安全属性
		NULL,              // 线程安全属性
		FALSE,             // 不继承句柄
		CREATE_NO_WINDOW,  // 不创建控制台窗口
		NULL,              // 使用父进程的环境变量
		utf8_to_widechar(currentPath).c_str(),      // 使用当前目录作为工作目录
		&si,               // 启动信息
		&pi))              // 进程信息
	{
		// 创建进程失败
// 		DWORD dwError = GetLastError();
// 		CString strError;
// 		strError.Format(_T("Failed to start LazyBugService.exe, error code: %d\n"), dwError);
// 		OutputDebugString(strError);
		return;
	}

	// 关闭进程和线程句柄，使其成为独立进程
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void StartEverythingService()
{
	// 检查是否已经有everything.exe在运行
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return;

	bool bEverythingRunning = false;
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (Process32First(hSnapshot, &pe32))
	{
		do
		{
			if (_tcsicmp(pe32.szExeFile, _T(EVERYTHING_EXENAME)) == 0)
			{
				bEverythingRunning = true;
				break;
			}
		} while (Process32Next(hSnapshot, &pe32));
	}

	CloseHandle(hSnapshot);

	// 如果Everything已经在运行，则不执行任何操作
	if (bEverythingRunning)
		return;

	extern const char* GetCurModuleFolderPath_utf8();

	std::string currentPath = GetCurModuleFolderPath_utf8();
	// 构建everything.exe的完整路径
	std::string strEverythingPath = currentPath;
	strEverythingPath += _T("\\"EVERYTHING_EXENAME);

	// 检查文件是否存在
	if (!Utils::IsFileExist(strEverythingPath.c_str()))
	{
		return;
	}

	// 设置启动信息
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW;  // 使用wShowWindow标志
	si.wShowWindow = SW_HIDE;           // 隐藏窗口

	ZeroMemory(&pi, sizeof(pi));

	// 构建命令行参数：-startup 隐藏主窗口，-no-tray 不显示托盘图标
	std::string strCmdLine = _T("");
	strCmdLine += _T("\"");
	strCmdLine += strEverythingPath;
	strCmdLine += _T("\" -startup");

	// CreateProcessW may modify the command line, so we need a non-const buffer
	std::wstring wstrCmdLine = utf8_to_widechar(strCmdLine);

	// 创建新进程，不继承句柄，隐藏窗口
	if (!CreateProcessW(
		NULL,              // 应用程序路径（在命令行中指定）
		&wstrCmdLine[0],   // 命令行参数 (non-const buffer)
		NULL,              // 进程安全属性
		NULL,              // 线程安全属性
		FALSE,             // 不继承句柄
		CREATE_NO_WINDOW,  // 不创建控制台窗口
		NULL,              // 使用父进程的环境变量
		utf8_to_widechar(currentPath).c_str(),      // 使用当前目录作为工作目录
		&si,               // 启动信息
		&pi))              // 进程信息
	{
		// 创建进程失败
		return;
	}

	// 关闭进程和线程句柄，使其成为独立进程
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

void SolutionDB_EnsureConnected()
{
	if (!g_solutionDBClient.IsConnected())
	{
		g_solutionDBClient.Disconnect();

#ifndef LOCAL_SOLUTIONDB_SERVICE
		StartLazyBugService();
#endif

//		StartEverythingService();

		SolutionDB_Connect();
	}
}


void SolutionDB_Connect()
{
	for (int i = 0;i < 10;i++)
	{
		if (g_solutionDBClient.Connect())
			break;
		Sleep(100);
	}
}

void SolutionDB_Disconnect()
{
	g_solutionDBClient.Disconnect();
}


SolutionDBMsg_Opened SolutionDB_Open(const char* slnPath)
{
	std::string slnName;
	ConvertFullPathToName(slnPath, slnName);
	RemoveFileSuffix(slnName);

	std::string dbRoot = Utils::GetDBRootFolder_utf8();
	std::string pathFolder = dbRoot + "\\" + slnName;

	SolutionDBMsg_RequestOpen request;
	request.dbFolderPath = pathFolder;
	request.slnPath = slnPath ? slnPath : "";

	FuturePipeMsg msg = g_solutionDBClient.SendMessage(request);

	SolutionDBMsg_Opened result;
	msg.WaitAndFetch(result);

	return std::move(result);
}

void SolutionDB_QueryNameItems(const char* dbFolderPath, const char* query, SolutionDBMsg_NameItems& result)
{
	SolutionDBMsg_QueryNameItems request;
	request.dbFolderPath = dbFolderPath;
	request.query = query;

	FuturePipeMsg msg = g_solutionDBClient.SendMessage(request);

	msg.WaitAndFetch(result);
}

bool SolutionDB_CollectRefs(const char* dbFolderPath, const CppSymbol::CollectRefsParam& collectRefsParam)
{
	SolutionDBMsg_CollectRefs request;
	request.dbFolderPath = dbFolderPath;
	request.collectRefParam = collectRefsParam;

	FuturePipeMsg msg = g_solutionDBClient.SendMessage(request);

	SolutionDBMsg_Refs result;
	msg.WaitAndFetch(result);
	return result.success;
}

void SolutionDB_FindSymbolDefines(const char* dbFolderPath, const char* symbolName, int maxResult, SolutionDBMsg_SymbolDefines& result)
{
	SolutionDBMsg_FindSymbolDefine request;
	request.dbFolderPath = dbFolderPath;
	request.symbolName = symbolName;
	request.maxResult = maxResult;

	FuturePipeMsg msg = g_solutionDBClient.SendMessage(request);

	msg.WaitAndFetch(result);
}

void SolutionDB_FindInFiles(const char* dbFolderPath, const char* keyword, int maxResults, SolutionDBMsg_FindInFilesResults& result)
{
	SolutionDBMsg_FindInFiles request;
	request.dbFolderPath = dbFolderPath;
	request.keyword = keyword;
	request.maxResults = maxResults;

	FuturePipeMsg msg = g_solutionDBClient.SendMessage(request);

	msg.WaitAndFetch(result);
}

void SolutionDB_SearchFile(const char* dbFolderPath, const char* keyword, int maxResults, SolutionDBMsg_SearchFileResult& result)
{
	SolutionDBMsg_SearchFile request;
	request.dbFolderPath = dbFolderPath;
	request.keyword = keyword;
	request.maxResults = maxResults;

	FuturePipeMsg msg = g_solutionDBClient.SendMessage(request);

	msg.WaitAndFetch(result);
}

