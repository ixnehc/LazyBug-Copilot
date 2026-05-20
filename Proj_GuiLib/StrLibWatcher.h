
#pragma once
#include "GuiLib.h"

#include "filewatcher/FileWatcher.h"

class IFileSystem;

class GuiLib_Api CStrLibWatcher
{
public:
	CStrLibWatcher()
	{
		_pFS=NULL;
	}
	void Init(IFileSystem *pFS);
	void Clear();
	void Update();
protected:
	IFileSystem *_pFS;
	CFileWatcher _watcher;
};
