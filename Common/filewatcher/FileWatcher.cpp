#include "stdh.h"
#include "FileWatcher.h"

#include "../stringparser/stringparser.h"

int CFileNotifyInformations::GetChangedFiles(CChangedFileList& files)
{
	const FILE_NOTIFY_INFORMATION* pNotifyInfo;

	const char* pEntry = (const char*) _pNotifyResult;

	bool done = false;
	while (!done)
	{
		pNotifyInfo = (FILE_NOTIFY_INFORMATION*) pEntry;

		ChangedFileInformation2 cfi2;
		cfi2.action = ChangedFileAction(pNotifyInfo->Action);

		cfi2.name = widechar_to_utf8(pNotifyInfo->FileName);

		files.Add(cfi2);

		done = (pNotifyInfo->NextEntryOffset == 0);
		pEntry += pNotifyInfo->NextEntryOffset;
	}

	return files.GetCount();
}

CFileWatcher::CFileWatcher() : _hDir(INVALID_HANDLE_VALUE), _hNotifyEvent(NULL)
{
}

CFileWatcher::~CFileWatcher()
{
	Stop();
}

BOOL CFileWatcher::IsStarted()
{
	return _hDir!=INVALID_HANDLE_VALUE;
}


BOOL CFileWatcher::Start(const char* targetDirectory, DWORD dwNotifyFilter)
{
	if (_hDir != INVALID_HANDLE_VALUE || !targetDirectory)	// Is watching
		return FALSE;

	_hDir = CreateFileW(utf8_to_widechar(targetDirectory).c_str(),
		FILE_LIST_DIRECTORY, 
		FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, 
		NULL, 
		OPEN_EXISTING, 
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, 
		NULL);
	if (_hDir == INVALID_HANDLE_VALUE)
		return FALSE;

	if (!_notifyInfors.Initialize())
	{
		CloseHandle(_hDir);
		_hDir = INVALID_HANDLE_VALUE;
		return FALSE;
	}

	_hNotifyEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	_dir = targetDirectory;

	_dwNotifyFilter = dwNotifyFilter;

	if (!_SetupWatcher())
	{
		Stop();
	}

	return TRUE;
}

void CFileWatcher::Stop()
{
	if (_hDir != INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hDir);
		_hDir = INVALID_HANDLE_VALUE;

		_notifyInfors.Cleanup();

		_dir.clear();

		CloseHandle(_hNotifyEvent);
		_hNotifyEvent = NULL;
	}
}

void CFileWatcher::SetSuffixFilter(const std::vector<std::string>& suffixes)
{
	_suffixFilters.clear();
	for (const auto& suffix : suffixes)
	{
		std::string lowerSuffix = suffix;
		StringLower(lowerSuffix);
		_suffixFilters.push_back(lowerSuffix);
	}
}

void CFileWatcher::ClearSuffixFilter()
{
	_suffixFilters.clear();
}

BOOL CFileWatcher::_IsFileMatchSuffix(const char* fileName) const
{
	// 如果没有设置过滤器，则匹配所有文件
	if (_suffixFilters.empty())
		return TRUE;

	std::string fileSuffix = GetFileSuffix(fileName);
	StringLower(fileSuffix);

	for (const auto& suffix : _suffixFilters)
	{
		if (fileSuffix == suffix)
			return TRUE;
	}

	return FALSE;
}

const char* CFileWatcher::GetWatchDirectory() const
{
	return _dir.c_str();
}

int CFileWatcher::FetchChangedFiles(const ChangedFileInformation*& files)
{
	thread_local static std::vector<ChangedFileInformation> sChangedFileVector;
	thread_local static CChangedFileList sChangedFiles;

	// Release the old filename list
	sChangedFileVector.clear();
	sChangedFiles.Clear();

	DWORD dwBytesReturned = 0;
	BOOL bRet = GetOverlappedResult(_hNotifyEvent, &_ovl, &dwBytesReturned, FALSE);
	while (bRet && dwBytesReturned > 0)
	{
		_notifyInfors.GetChangedFiles(sChangedFiles);		

		bRet = _SetupWatcher();
		if (bRet)
			bRet = GetOverlappedResult(_hNotifyEvent, &_ovl, &dwBytesReturned, FALSE);
	}

	// List the changed files (with suffix filtering)
	ChangedFileInformation cfi;
	int nCount = sChangedFiles.GetCount();
	for (int i = 0; i < nCount; i++)
	{
		// 应用后缀过滤
		if (_IsFileMatchSuffix(sChangedFiles[i].name.c_str()))
		{
			cfi.action = sChangedFiles[i].action;
			cfi.name = sChangedFiles[i].name.c_str();
			sChangedFileVector.push_back(cfi);
		}
	}

	int filteredCount = static_cast<int>(sChangedFileVector.size());
	if (filteredCount > 0)
	{
		files = sChangedFileVector.data();
	}

	return filteredCount;
}

BOOL CFileWatcher::_SetupWatcher()
{
	BOOL bRet = FALSE;
	if (_hDir != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesReturned;

		memset(&_ovl, 0, sizeof(OVERLAPPED));
		_ovl.hEvent = _hNotifyEvent;

		memset(_notifyInfors._pNotifyResult, 0, FW_NOTIFYRESULT_LENGTH);

		bRet = ReadDirectoryChangesW(_hDir, 
			_notifyInfors._pNotifyResult, 
			FW_NOTIFYRESULT_LENGTH, 
			TRUE, 
			_dwNotifyFilter, 
			&dwBytesReturned, 
			&_ovl, 
			NULL);

	}

	return bRet;
}