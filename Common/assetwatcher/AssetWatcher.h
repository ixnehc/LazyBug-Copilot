#ifndef __AssetWatcher_H__
#define __AssetWatcher_H__
#include "multithread/Win32ThreadSupports.h"
#include <vector>
#include <string>

const int AW_NOTIFYRESULT_LENGTH	= 8192;

enum WatchNotifyFilter
{
	WNF_CHANGE_FILE_NAME = FILE_NOTIFY_CHANGE_FILE_NAME, 
	WNF_CHANGE_DIR_NAME	= FILE_NOTIFY_CHANGE_DIR_NAME, 
	WNF_CHANGE_ATTRIBUTES = FILE_NOTIFY_CHANGE_ATTRIBUTES, 
	WNF_CHANGE_SIZE	= FILE_NOTIFY_CHANGE_SIZE, 
	WNF_CHANGE_LAST_WRITE = FILE_NOTIFY_CHANGE_LAST_WRITE, 
	WNF_CHANGE_LAST_ACCESS = FILE_NOTIFY_CHANGE_LAST_ACCESS, 
	WNF_CHANGE_CREATION = FILE_NOTIFY_CHANGE_CREATION, 
	WNF_CHANGE_SECURITY = FILE_NOTIFY_CHANGE_SECURITY
};

enum ChangedFileAction
{
	FA_ADDED = FILE_ACTION_ADDED, 
	FA_REMOVED = FILE_ACTION_REMOVED,
	FA_MODIFIED = FILE_ACTION_MODIFIED,
	FA_RENAMED_OLD_NAME	= FILE_ACTION_RENAMED_OLD_NAME,
	FA_RENAMED_NEW_NAME	= FILE_ACTION_RENAMED_NEW_NAME,
};

struct ChangedFileInformation
{
	ChangedFileAction action;
	const char* name;
};

struct ChangedFileInformation2
{
	ChangedFileAction action;
	std::string name;
};

class CChangedFileList
{
public:
	void Add(const ChangedFileInformation2& cfi2)
	{
		int index = Find(cfi2);
		if (index == -1)
		{
			_changedFiles.push_back(cfi2);
		}
	}
	int GetCount() const
	{
		return static_cast<int>(_changedFiles.size());
	}
	const ChangedFileInformation2& Get(int i) const
	{
		return _changedFiles[i];
	}
	const ChangedFileInformation2& operator [](int i) const
	{
		return _changedFiles[i];
	}
	int Find(const ChangedFileInformation2& cfi2)
	{
		int index = -1;
		int count = static_cast<int>(_changedFiles.size());
		for (int i = 0; i < count; i++)
		{
			if ((_changedFiles[i].action == cfi2.action) && 
				(_changedFiles[i].name == cfi2.name))
			{
				index = i;
				break;
			}
		}
		return index;
	}
	void Clear()
	{
		_changedFiles.clear();
	}

private:
	std::vector<ChangedFileInformation2> _changedFiles;
};

class CFileNotifyInformations
{
public:
	CFileNotifyInformations() : _pNotifyResult(NULL)
	{ 
	}
	~CFileNotifyInformations()
	{
		if (_pNotifyResult)
			delete []_pNotifyResult;
	}

public:
	BOOL Initialize()
	{
		_pNotifyResult = new char[AW_NOTIFYRESULT_LENGTH];
		return (_pNotifyResult != NULL);
	}
	void Cleanup()
	{
		if (_pNotifyResult)
		{
			delete []_pNotifyResult;
			_pNotifyResult = NULL;
		}
	}

public:
	int GetChangedFiles(CChangedFileList& files);

public:
	char* _pNotifyResult;
};

class CAssetWatcher
{
public:
	CAssetWatcher();
	~CAssetWatcher();

public:
	BOOL Start(const char* targetDirectory, DWORD dwNotifyFilter = WNF_CHANGE_LAST_WRITE);
	void Stop();

	const char* GetWatchDirectory() const;

	int FetchChangedFiles(const ChangedFileInformation*& files, int& count);

protected:
	BOOL _SetupWatcher();

private:
	std::string _dir;
	HANDLE _hDir;
	DWORD _dwNotifyFilter;
	CFileNotifyInformations _notifyInfors;	
	OVERLAPPED _ovl;
	HANDLE _hNotifyEvent;
};
#endif