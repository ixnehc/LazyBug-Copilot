#include "stdh.h"
#include "CliWhitelist.h"
#include "Utils_CliWhitelist.h"
#include "Utils.h"

CCliWhitelist g_cliWhitelist;

// 外部函数声明
extern const char* GetCurModuleFolderPath_utf8();

void CCliWhitelist::_GetFilePaths(std::string& path1, std::string& path2) const
{
	path1 = std::string(Utils::GetDBRootFolder_utf8()) + "\\" + LAZYBUG_CLI_WHITELIST_FILENAME;
	path2 = std::string(GetCurModuleFolderPath_utf8()) + "\\" + LAZYBUG_CLI_WHITELIST_FILENAME;
}

bool CCliWhitelist::UpdateReload()
{
	std::string currentPath1, currentPath2;
	_GetFilePaths(currentPath1, currentPath2);

	// 获取两个文件的当前时间戳（-1表示文件不存在）
	time_t currentTime1 = Utils::IsFileExist(currentPath1.c_str()) ? Utils::GetFileTimeT(currentPath1.c_str()) : -1;
	time_t currentTime2 = Utils::IsFileExist(currentPath2.c_str()) ? Utils::GetFileTimeT(currentPath2.c_str()) : -1;

	// 检查路径是否变化，或者文件的修改时间是否变化
	bool changed = false;
	if (currentPath1 != _lastPath1 || currentPath2 != _lastPath2)
	{
		changed = true;
	}
	else if (currentTime1 != _lastTime1 || currentTime2 != _lastTime2)
	{
		changed = true;
	}

	if (!changed)
		return false;

	// 记录当前路径和时间戳
	_lastPath1 = currentPath1;
	_lastPath2 = currentPath2;
	_lastTime1 = currentTime1;
	_lastTime2 = currentTime2;

	// 重新加载白名单
	std::vector<std::string> rawList;
	Utils::LoadCliWhitelists(rawList);

	// 编译每行为正则表达式
	_compiledRegexes.clear();
	_compiledRegexes.reserve(rawList.size());
	for (const auto& line : rawList)
	{
		try
		{
			// 使用 std::regex::ECMAScript 语法，与白名单文件注释中描述的 regex 行为保持一致
			// 增加 std::regex::icase 支持大小写不敏感（Windows CLI 大小写不敏感）
			_compiledRegexes.emplace_back(line, std::regex::ECMAScript | std::regex::icase);
		}
		catch (const std::regex_error&)
		{
			// 忽略无法编译的正则表达式行
		}
	}

	return true;
}

bool CCliWhitelist::Check(const char* command) const
{
	if (command == nullptr || command[0] == '\0')
		return false;

	std::string cmd(command);

	for (const auto& regex : _compiledRegexes)
	{
		if (std::regex_match(cmd, regex))
			return true;
	}

	return false;
}

