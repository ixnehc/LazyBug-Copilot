#include "stdh.h"
#include "AssetWatcher.h"

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

		char fileName[MAX_PATH + 1] = { 0 };
		WideCharToMultiByte(CP_ACP, 0, pNotifyInfo->FileName, pNotifyInfo->FileNameLength / 2, 
			fileName, MAX_PATH, NULL, NULL);
		cfi2.name = fileName;

		files.Add(cfi2);

		done = (pNotifyInfo->NextEntryOffset == 0);
		pEntry += pNotifyInfo->NextEntryOffset;
	}

	return files.GetCount();
}

CAssetWatcher::CAssetWatcher() : _hDir(INVALID_HANDLE_VALUE), _hNotifyEvent(NULL)
{
}

CAssetWatcher::~CAssetWatcher()
{
	Stop();
}

BOOL CAssetWatcher::Start(const char* targetDirectory, DWORD dwNotifyFilter)
{
	if (_hDir != INVALID_HANDLE_VALUE || !targetDirectory)	// Is watching
		return FALSE;

	_hDir = CreateFile(targetDirectory, 
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

void CAssetWatcher::Stop()
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

const char* CAssetWatcher::GetWatchDirectory() const
{
	return _dir.c_str();
}

int CAssetWatcher::FetchChangedFiles(const ChangedFileInformation*& files, int& count)
{
	static std::vector<ChangedFileInformation> sChangedFileVector;
	static CChangedFileList sChangedFiles;

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

	// List the changed files
	ChangedFileInformation cfi;
	int nCount = sChangedFiles.GetCount();
	for (int i = 0; i < nCount; i++)
	{
		cfi.action = sChangedFiles[i].action;
		cfi.name = sChangedFiles[i].name.c_str();
		sChangedFileVector.push_back(cfi);
	}

	if (nCount > 0)
	{
		files = &sChangedFileVector[0];		
	}

	count = nCount;

	return nCount;
}

BOOL CAssetWatcher::_SetupWatcher()
{
	BOOL bRet = FALSE;
	if (_hDir != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesReturned;

		memset(&_ovl, 0, sizeof(OVERLAPPED));
		_ovl.hEvent = _hNotifyEvent;

		memset(_notifyInfors._pNotifyResult, 0, AW_NOTIFYRESULT_LENGTH);

		bRet = ReadDirectoryChangesW(_hDir, 
			_notifyInfors._pNotifyResult, 
			AW_NOTIFYRESULT_LENGTH, 
			TRUE, 
			_dwNotifyFilter, 
			&dwBytesReturned, 
			&_ovl, 
			NULL);

	}

	return bRet;
}