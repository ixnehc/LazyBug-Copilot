// Package.h

#pragma once

#include <string>
#include <unordered_map>
#include <windows.h>

class COpenedDocStates
{
public:
	struct Entry
	{
		FILETIME modifiedTime;
	};

	void Add(const std::wstring& filePath);
	void Remove(const std::wstring& filePath);
	void Update(const std::wstring& filePath);

	bool IsSyncWithDisk(const std::wstring& filePath,FILETIME &fileTimeInDisk);

public:
	std::unordered_map<std::wstring, Entry> _entries;
};

extern COpenedDocStates g_openDocStates;
