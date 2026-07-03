#include "stdh.h"

#include "FileLocator.h"


#include "Utils.h"
#include "SolutionDBApi.h"

extern const char* GetOpenedDBFolderPath_utf8();

void CFileLacator::Request(const char* filePath, FileLocation loc)
{
	_fileLocate.filePath = filePath ? filePath : "";
	_fileLocate.fileLoc = loc;
	_hasRequest = true;
	_lastRequestTime = GetAbsTick();

	// 激活文件到 EmbeddingDB
	const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
	if (dbFolderPath && dbFolderPath[0] != '\0' && !_fileLocate.filePath.empty())
	{
		std::vector<std::string> filePaths;
		filePaths.push_back(_fileLocate.filePath);
		SolutionDB_ActivateFiles(dbFolderPath, filePaths);
	}
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

