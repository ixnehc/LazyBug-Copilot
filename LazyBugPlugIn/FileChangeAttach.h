#pragma once

#include "codediff/CodeDiff.h"

#include "../Proj_LazyBug/FileChange.h"


class CCommandFilter;
class CFileChangeAttach
{
public:
	bool IsEmpty()	{		return _change.IsEmpty();	}
	bool Attach(const FileChange& fileChange, FILETIME fileTimeOfChange);
	void Detach();
	void Validate();

	static std::wstring GetFullPath(const FileChange& fileChange);
	static bool CheckReadOnly(const std::wstring& filePath);
	static void EnableWritable(const std::wstring& filePath,bool enable);

private:
	int FindClosestLineAfterReload(int originalLine);

public:
	FileChange _change;
	CodeComparingLines _comparingContent;
	std::wstring _filePath;
	FILETIME _fileTimeWhenAttach;//Attach?,???????

	CCommandFilter* _commandFilter;
	CComPtr<IVsTextView> _pVsTextView;
};