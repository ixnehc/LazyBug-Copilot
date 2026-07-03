#pragma once

#include "timer/timer.h"

struct FileLocate
{
	FileLocate()
	{
	}
	bool IsEmpty()
	{
		return filePath.empty();
	}
	std::string filePath;
	FileLocation fileLoc;
};

class CFileLacator
{
public:
	CFileLacator()
	{
		_hasRequest = false;
		_lastRequestTime = 0;
	}

	bool HasRequest()	{		return _hasRequest;	}

	void Request(const char* filePath, FileLocation fileLoc);

	//取走最近一次新的Request产生的时间,返回0表示没有新的FileChange产生
	AbsTick FetchRequest(FileLocate &locate);

protected:

	FileLocate _fileLocate;
	
	bool _hasRequest;
	AbsTick _lastRequestTime;
	
};