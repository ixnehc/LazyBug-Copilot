// Proj_LazyBugService.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "stdh.h"
#include <windows.h>
#include <tlhelp32.h>
#include <tchar.h>

#include "../LazyBugSource/SolutionDBService.h"

#include "stringparser/stringparser.h"

// 全局互斥量句柄
HANDLE g_hMutex = NULL;

// 使用命名互斥量确保只有一个同名进程实例运行
void CheckAndExitIfSameProcessExists()
{
    // 获取当前进程名
    TCHAR szFileName[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, szFileName, MAX_PATH);
    
    // 提取文件名（不含路径）
    LPCTSTR pszProcessName = _tcsrchr(szFileName, _T('\\'));
    if (pszProcessName == NULL)
        pszProcessName = szFileName;
    else
        pszProcessName++;

    // 创建基于进程名的命名互斥量
    // 使用Global\前缀确保在整个系统中有效
    TCHAR szMutexName[MAX_PATH];
    _stprintf_s(szMutexName, _T("Global\\%s_Mutex"), pszProcessName);

    // 尝试创建命名互斥量
    g_hMutex = CreateMutex(NULL, TRUE, szMutexName);
    
    if (g_hMutex == NULL)
    {
        // 创建失败，直接退出
        ExitProcess(0);
    }

    // 检查互斥量是否已经存在
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        // 互斥量已存在，说明已有同名进程在运行
        CloseHandle(g_hMutex);
        g_hMutex = NULL;
        ExitProcess(0);
    }

    // 互斥量创建成功，可以继续运行
    // g_hMutex保持打开状态，直到进程结束
}

int main()
{
    // 检查是否有同名进程在运行
    CheckAndExitIfSameProcessExists();

	CSolutionDBService solutoinDBService;

	solutoinDBService.Start();

	while (1)
	{
		AbsTick tStart = GetAbsTick();
		if (!solutoinDBService.Update())
			break;

		const AbsTick frameTime = 20;

		AbsTick tEnd = GetAbsTick();
		if (tEnd < tStart + frameTime)
			Sleep((DWORD)((tStart + frameTime)-tEnd ));
	}


    // 程序结束前关闭互斥量句柄
    if (g_hMutex != NULL)
    {
        CloseHandle(g_hMutex);
        g_hMutex = NULL;
    }

    return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
