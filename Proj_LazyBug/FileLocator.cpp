#include "stdh.h"

#include "FileLocator.h"


#include "Utils.h"

void CFileLacator::Request(const char* filePath, FileLocation loc)
{
	_fileLocate.filePath = filePath ? filePath : "";
	_fileLocate.fileLoc = loc;
	_hasRequest = true;
	_lastRequestTime = GetAbsTick();
}


AbsTick CFileLacator::FetchRequest(FileLocate& locate)
{
	if (_hasRequest)
	{
		AbsTick result = _lastRequestTime;
		_hasRequest = false; // 取走后重置标志
		locate = _fileLocate;
		return result;
	}
	return 0;
}

