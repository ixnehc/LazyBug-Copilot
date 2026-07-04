#include "stdh.h"

#include "OpenedDocStates.h"
#include "Utils.h"

void COpenedDocStates::Add(const std::wstring& filePath)
{
	std::wstring lowerPath = Util_ToLower(filePath);
	Entry entry;
	entry.modifiedTime = Util_GetFileTime(filePath);
	_entries[lowerPath] = entry;
}

void COpenedDocStates::Remove(const std::wstring& filePath)
{
	std::wstring lowerPath = Util_ToLower(filePath);
	_entries.erase(lowerPath);
}

void COpenedDocStates::Update(const std::wstring& filePath)
{
	std::wstring lowerPath = Util_ToLower(filePath);
	auto it = _entries.find(lowerPath);
	if (it != _entries.end())
	{
		it->second.modifiedTime = Util_GetFileTime(filePath);
	}
	else
	{
		// 如果文件不存在，添加它
		Add(filePath);
	}
}

bool COpenedDocStates::IsSyncWithDisk(const std::wstring& filePath, FILETIME& fileTimeInDisk)
{
	// 获取文件系统上的最新修改时间
	FILETIME currentFileTime = Util_GetFileTime(filePath);
	fileTimeInDisk = currentFileTime;

	std::wstring lowerPath = Util_ToLower(filePath);
	auto it = _entries.find(lowerPath);
	if (it == _entries.end())
	{
		// 文件未在记录中，
		// 有两种可能,一种文件已经打开,但我们没有捕获到事件,这种情况下,如果文件修改后一段时间后,必然会重载
		// 另一种文件确实没打开,这种情况下,我们认为已经同步了.
		// 所以我们要在这里等待一小段时间
		
		// 获取当前系统时间
		FILETIME currentTime;
		GetSystemTimeAsFileTime(&currentTime);
		
		// 将FILETIME转换为LARGE_INTEGER以便计算
		LARGE_INTEGER current, disk;
		current.HighPart = currentTime.dwHighDateTime;
		current.LowPart = currentTime.dwLowDateTime;
		disk.HighPart = currentFileTime.dwHighDateTime;
		disk.LowPart = currentFileTime.dwLowDateTime;
		
		// 2秒对应的FILETIME单位 (2 * 10,000,000 = 20,000,000，以100纳秒为单位)
		const LONGLONG twoSecondsInFileTime = 20000000LL;//FetchFileChangeOpenDocumentRequest()里的延迟要大于这个值
		
		// 如果当前时间大于文件时间2秒以上，返回true（不同步）
		if (current.QuadPart > disk.QuadPart + twoSecondsInFileTime)
		{
			return true;
		}
		return false;
	}
	
	// 比较记录的修改时间和当前文件系统的修改时间
	return Util_EqualFileTime(it->second.modifiedTime, currentFileTime);
}


COpenedDocStates g_openDocStates;

