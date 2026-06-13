#include "stdh.h" 
#include "FileChangeAttach.h"

#include "stringparser/stringparser.h"

#include "Utils.h"

#include "../LazyBugPlugInControls/LazyBugPlugInControlsExport.h"

#include "PackageState.h"

#include "CommandFilter.h"

#include <comutil.h>

std::wstring CFileChangeAttach::GetFullPath(const FileChange& fileChange)
{
	std::wstring pathW = utf8_to_widechar(fileChange.lowerCaseFullPath);

	return pathW;
}

bool CFileChangeAttach::CheckReadOnly(const std::wstring& filePath)
{
	DWORD dwAttrs = GetFileAttributesW(filePath.c_str());
	if (dwAttrs == INVALID_FILE_ATTRIBUTES)
		return false;

	return (dwAttrs & FILE_ATTRIBUTE_READONLY) != 0;
}

void CFileChangeAttach::EnableWritable(const std::wstring& filePath, bool enable)
{
	DWORD dwAttrs = GetFileAttributesW(filePath.c_str());	
	if (dwAttrs == INVALID_FILE_ATTRIBUTES)
		return;

	if (enable)
		SetFileAttributesW(filePath.c_str(), dwAttrs & (~FILE_ATTRIBUTE_READONLY));
	else
		SetFileAttributesW(filePath.c_str(), dwAttrs | FILE_ATTRIBUTE_READONLY);
}


bool CFileChangeAttach::Attach(const FileChange& fileChange, FILETIME fileTimeOfChange)
{
	if (!_change.IsEmpty())
		return false;
	CComPtr<IServiceProvider> sp=g_ps.pServiceProvider;
	if (!sp)
		return false;

	CComPtr < IVsUIShellOpenDocument> pOpenDoc = g_ps.pUIShellOpenDocument;
	if (!pOpenDoc)
		return false;

	std::wstring pathW = GetFullPath(fileChange);

	if (!_change.IsEmpty())
		Detach();

	CComPtr<IVsTextView> pVsTextView = Util_GetTextViewForFile(pathW.c_str());

	if (!pVsTextView)
		return false;

	std::string oldContent;
	if (!fileChange.oldContent.empty())
	{
		oldContent.resize(fileChange.oldContent.size());
		memcpy((void*)oldContent.c_str(), &fileChange.oldContent[0], fileChange.oldContent.size());
	}
	std::string newContent;
	if (!fileChange.newContent.empty())
	{
		newContent.resize(fileChange.newContent.size());
		memcpy((void*)newContent.c_str(), &fileChange.newContent[0], fileChange.newContent.size());
	}

	_comparingContent.Clear();
	MakeCodeComparing_Lines(oldContent, newContent, _comparingContent);

	// 在替换缓冲区之前，保存断点、书签和原始文本
	Util_GetText(pVsTextView, _originalText);
	_savedBreakpoints = Util_SaveBreakpointsForFile(pathW);
	_savedBookmarks = Util_SaveBookmarksForFile(pathW);

	Util_SetComparingContent(pathW,pVsTextView, _comparingContent);
	_commandFilter=Util_AddCommandFilter(pVsTextView);
// 	Util_DisableReloadFile(pathW.c_str());

	Util_SetFileChangeBorder(pVsTextView, true); // 显示橘黄色边框

	Util_NavigateNextDiff(pathW, _comparingContent, true, false);

	_pVsTextView = pVsTextView;
	_change = fileChange;
	_filePath = pathW;
	_fileTimeWhenAttach = fileTimeOfChange;

	return true;
}

void CFileChangeAttach::Detach()
{
	if (_change.IsEmpty())
		return;

	Util_SetFileChangeBorder(_pVsTextView, false); // 移除橘黄色边框

	if (_commandFilter)
	{
		_pVsTextView->RemoveCommandFilter(_commandFilter);
		_commandFilter->Release();
	}
// 	Util_EnableReloadFile(_filePath.c_str());

	// 保存当前位置信息
	long oldLine = 0;
	long oldFirstVisibleLine = 0;
	std::string oldText;
	
	if (_pVsTextView)
	{
		// 获取当前光标位置
		ViewCol col;
		_pVsTextView->GetCaretPos(&oldLine, &col);

		oldFirstVisibleLine = Util_GetFirstVisibleLine(_pVsTextView);
		Util_GetText(_pVsTextView,oldText);
	}

	Util_ClearUserData(_pVsTextView);

	// 重新加载文件
	Util_ReloadFile(_filePath.c_str());
	
	// 重新获取文本视图（因为重新加载后可能会改变）
	CComPtr<IVsTextView> pNewTextView = Util_GetTextViewForFile(_filePath);
	if (pNewTextView)
	{
		std::string newText;

		Util_GetText(_pVsTextView, newText);

		std::deque<CodeDiffLine> diffs;
		CompareCodeStrings(oldText, newText, diffs);

		// 根据比较内容找到最接近的行位置
		long newLine = 0;
		long newFirstVisibleLine = 0;
		newLine = ClosestNewLineFromOldLine(oldLine,diffs);
		newFirstVisibleLine = ClosestNewLineFromOldLine(oldFirstVisibleLine, diffs);

		// 恢复光标位置
		if (newLine >= 0)
			pNewTextView->SetCaretPos(newLine, 0);
		if (newFirstVisibleLine)
			Util_SetFirstVisibleLine(pNewTextView, newFirstVisibleLine);
	}

	// 恢复断点和书签
	{
		std::vector<int> lineMap;

		// 使用 Attach 前保存的原始文本与重新加载后的文本做比较，建立行号映射
		std::string reloadedText;
		CComPtr<IVsTextView> pReloadedView = Util_GetTextViewForFile(_filePath);
		if (pReloadedView)
			Util_GetText(pReloadedView, reloadedText);

		if (!_originalText.empty() && !reloadedText.empty())
		{
			// 有原始文本和重新加载后的文本，做 diff 比较建立精确映射
			std::deque<CodeDiffLine> diffsForMap;
			CompareCodeStrings(_originalText, reloadedText, diffsForMap);

			int oldLineCount = 1;
			for (size_t i = 0; i < _originalText.size(); i++)
			{
				if (_originalText[i] == '\n')
					oldLineCount++;
			}
			lineMap.resize(oldLineCount, -1);
			for (int i = 0; i < oldLineCount; i++)
				lineMap[i] = ClosestNewLineFromOldLine(i, diffsForMap);
		}
		// 若 lineMap 为空，C# 端将按 1:1 映射处理

		Util_RestoreBreakpoints(_filePath, _savedBreakpoints, lineMap);
		Util_RestoreBookmarks(_filePath, _savedBookmarks, lineMap);
	}

	_pVsTextView = NULL;

	_change.Clear();
	_comparingContent.Clear();
	_filePath.clear();
	_fileTimeWhenAttach = Util_GetZeroFileTime();
	_savedBreakpoints.clear();
	_savedBookmarks.clear();
	_originalText.clear();
}
void CFileChangeAttach::Validate()
{

}


