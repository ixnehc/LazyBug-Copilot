//
// LazyBugChangelistsWindow.h
//
// This file contains the implementation of a new tool window
//

#pragma once

#include <AtlWin.h>
#include <VSLWindows.h>

#include "PackageState.h"

#include <vector>

struct CodeComparingLines;
class CCommandFilter;
struct SolutionDump;
struct SolutionDumpTimeStamps;

extern HRESULT Util_IsFileOpenInEditor(const wchar_t* filePath);
extern HRESULT Util_OpenFileInEditor(const wchar_t* filePath, int line = -1);
extern CComPtr<IVsTextView> Util_GetTextViewForFile(const std::wstring& filePath);
extern bool Util_SetComparingContent(const std::wstring& filePath,CComPtr<IVsTextView> pVsTextView, const CodeComparingLines& comparaingContent);
extern CCommandFilter* Util_AddCommandFilter(CComPtr<IVsTextView> pVsTextView);
extern int Util_GetFirstVisibleLine(CComPtr<IVsTextView> pVsTextView);
extern void Util_SetFirstVisibleLine(CComPtr<IVsTextView> pVsTextView,int line);
extern HRESULT Util_ReloadFile(const wchar_t* filePath);
extern void Util_ClearUserData(CComPtr<IVsTextView> pVsTextView);
extern void Util_SetFileChangeBorder(CComPtr<IVsTextView> pVsTextView, bool show);
extern void Util_NavigateNextDiff(const std::wstring& filePath, const CodeComparingLines& content, bool isNext,bool allowCycle);
extern bool Util_IsFileDirty(const wchar_t* filePath);
extern bool Util_ExistFile(const char* filePath);
extern bool Util_SaveFileContent(const std::wstring& wpath, const std::vector<BYTE>& content);
extern bool Util_LoadFileContent(const std::wstring& wpath, std::vector<BYTE>& content);
extern FILETIME Util_GetZeroFileTime();
extern bool Util_EqualFileTime(const FILETIME& a, const FILETIME& b);
extern FILETIME Util_GetFileTime(const std::wstring& wpath);
extern void Util_SetFileTime(const std::wstring& wpath, FILETIME&t);
std::wstring Util_ToLower(const std::wstring& str);
extern int Util_GetText(CComPtr<IVsTextView> pVsTextView, std::string& utf8Text);

// 断点/书签 保存与恢复（通过 C# COM 服务实现）
extern std::string Util_SaveBreakpointsForFile(const std::wstring& filePath);
extern std::string Util_SaveBookmarksForFile(const std::wstring& filePath);
extern void Util_RestoreBreakpoints(const std::wstring& filePath, const std::string& bpData, const std::vector<int>& lineMap);
extern void Util_RestoreBookmarks(const std::wstring& filePath, const std::string& bmData, const std::vector<int>& lineMap);

struct GenerateSlnDumpProgress
{
	GenerateSlnDumpProgress()
	{
		Zero();
	}
	void Zero()
	{
		cur = total = 0;
	}
	bool IsEmpty()
	{
		return cur == 0 && total == 0;
	}

	bool IsDone()
	{
		return cur >= total;
	}
	int cur;
	int total;
};
extern bool Util_GenerateSolutionDump2(const char* dbFolder, CComPtr<IVsSolution> pSolution, SolutionDump& dmp, int sliceTime, GenerateSlnDumpProgress& progress);

extern bool Util_GenerateSolutionDumpTimeStamps(SolutionDump& dmp,SolutionDumpTimeStamps &timeStamps);
extern bool Util_CheckSolutionDumpOutOfDate(SolutionDumpTimeStamps& timeStamps);
