#pragma once

#include <string>
#include <vector>
#include <set>
#include "SolutionDBDefines.h"

// 目录监视条目
struct DirWatchEntry
{
	std::string directoryPath;           // 绝对路径（lowercased）
	std::vector<std::string> extensions; // 扩展名列表（小写，不含点号），如 {"cpp","h","hpp"}
	std::set<std::string> files;         // 当前该目录下匹配的文件集合（lowercased 全路径）
	FileSourceMask sourceBit = 0;        // 在 sourceMask 中的位
	bool recursive = false;
	bool enabled = true;                 // 是否启用该目录监视
};

bool LoadDirWatchConfig(const char* fullPath, std::vector<DirWatchEntry>& entries);
bool SaveDirWatchConfig(const char* fullPath, const std::vector<DirWatchEntry>& entries);