#pragma once

#include <vector>
#include <string>
#include <regex>
#include <time.h>

class CCliWhitelist
{
public:
	// 检查两个cli whitelist文件（路径或日期）是否有变化，如有变化则重新加载并编译为正则表达式
	// 返回是否发生了重新加载
	bool UpdateReload();

	// 检查传入的命令行是否匹配白名单中的任意正则表达式
	// 返回true表示在白名单中（允许执行），false表示不在白名单中
	bool Check(const char* command) const;

private:
	// 构建两个白名单文件的路径
	void _GetFilePaths(std::string& path1, std::string& path2) const;

	// 记录上次加载时的文件时间戳和路径（-1表示文件不存在）
	time_t _lastTime1 = -1;
	time_t _lastTime2 = -1;

	// 记录上次加载时的文件路径（用于检测路径是否变化）
	std::string _lastPath1;
	std::string _lastPath2;

	// 编译好的正则表达式列表
	std::vector<std::regex> _compiledRegexes;
};

extern CCliWhitelist g_cliWhitelist;
