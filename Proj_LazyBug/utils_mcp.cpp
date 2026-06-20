#include "stdh.h"
#include <fstream>
#include <sstream>
#include "Utils.h"
#include "Utils_Mcp.h"
#include "stringparser/stringparser.h"
#include "nlohmann/json.hpp"
#include "timer/wuid.h"
#include <string>
#include <unordered_set>

// 外部函数声明
extern const char* GetCurModuleFolderPath_utf8();
extern const char* GetOpenedDBFolderPath_utf8();

extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern std::string widechar_to_utf8(const wchar_t* str);

using json = nlohmann::ordered_json;

namespace Utils
{

std::string GetGlobalMcpsFolder()
{
	const char* dbRoot = GetDBRootFolder_utf8();
	if (!dbRoot || dbRoot[0] == '\0')
		return "";
	return std::string(dbRoot) + "\\_mcps";
}

std::string GetProjectMcpsFolder()
{
	const char* openedDB = GetOpenedDBFolderPath_utf8();
	if (!openedDB || openedDB[0] == '\0')
		return "";
	return std::string(openedDB) + "\\_mcps";
}

std::string GetMcpSettingFilePath()
{
	const char* openedDB = GetOpenedDBFolderPath_utf8();
	if (!openedDB || openedDB[0] == '\0')
		return "";
	return std::string(openedDB) + "\\_mcps\\.setting";
}

static std::string _TypeToString(CLlmMcps::Mcp::Type tp)
{
	switch (tp)
	{
	case CLlmMcps::Mcp::Type::Global:  return "Global";
	case CLlmMcps::Mcp::Type::Project: return "Project";
	default: return "";
	}
}

void EnableMcps(bool enable, const std::vector<WUID>& uids)
{
	if (uids.empty())
		return;

	std::string settingPath = GetMcpSettingFilePath();
	if (settingPath.empty())
		return;

	// 读取现有setting文件
	json j;
	std::string content;
	if (Utils::LoadFileContent(settingPath.c_str(), content))
	{
		try { j = json::parse(content); } catch (...) { j = json::array(); }
	}
	else
	{
		j = json::array();
	}

	if (!j.is_array())
		j = json::array();

	// 构建uid查找集合（字符串形式）
	std::unordered_set<std::string> uidSet;
	for (auto uid : uids)
		uidSet.insert(std::to_string(uid));

	// 在数组中查找并更新uid
	std::unordered_set<std::string> foundUids;
	for (auto& item : j)
	{
		if (!item.is_object() || !item.contains("uid") || !item["uid"].is_string())
			continue;
		std::string uidStr = item["uid"].get<std::string>();
		if (uidSet.count(uidStr))
		{
			item["enable"] = enable;
			foundUids.insert(uidStr);
		}
	}

	// 为未在数组中找到的uid添加新条目
	for (const auto& uidStr : uidSet)
	{
		if (!foundUids.count(uidStr))
		{
			json item;
			item["uid"] = uidStr;
			item["enable"] = enable;
			j.push_back(item);
		}
	}

	// 写回文件
	std::string newContent = j.dump(2);
	Utils::SaveFileContent(settingPath.c_str(), newContent);
}

void EnableMcpTools(bool enable, WUID mcpUid, const std::vector<std::string>& toolNames)
{
	if (toolNames.empty())
		return;

	std::string settingPath = GetMcpSettingFilePath();
	if (settingPath.empty())
		return;

	// 读取现有setting文件
	json j;
	std::string content;
	if (Utils::LoadFileContent(settingPath.c_str(), content))
	{
		try { j = json::parse(content); } catch (...) { j = json::array(); }
	}
	else
	{
		j = json::array();
	}

	if (!j.is_array())
		j = json::array();

	std::string uidStr = std::to_string(mcpUid);

	// 在数组中查找指定uid的mcp条目
	for (auto& item : j)
	{
		if (!item.is_object() || !item.contains("uid") || !item["uid"].is_string())
			continue;
		if (item["uid"].get<std::string>() != uidStr)
			continue;

		// 获取或创建disabledTools数组
		if (!item.contains("disabledTools") || !item["disabledTools"].is_array())
			item["disabledTools"] = json::array();

		auto& disabledTools = item["disabledTools"];
		std::unordered_set<std::string> existingTools;
		for (const auto& t : disabledTools)
		{
			if (t.is_string())
				existingTools.insert(t.get<std::string>());
		}

		if (enable)
		{
			// 启用工具：从disabledTools中移除
			json newDisabled;
			for (const auto& t : disabledTools)
			{
				if (t.is_string())
				{
					std::string toolName = t.get<std::string>();
					bool shouldRemove = false;
					for (const auto& name : toolNames)
					{
						if (toolName == name)
						{
							shouldRemove = true;
							break;
						}
					}
					if (!shouldRemove)
						newDisabled.push_back(t);
				}
			}
			item["disabledTools"] = newDisabled;
		}
		else
		{
			// 禁用工具：添加到disabledTools
			for (const auto& name : toolNames)
			{
				if (!existingTools.count(name))
					disabledTools.push_back(name);
			}
		}
		break; // 找到并处理完就退出
	}

	// 写回文件
	std::string newContent = j.dump(2);
	Utils::SaveFileContent(settingPath.c_str(), newContent);
}

WUID EnsureMcpUid(const std::string& mcpFolderPath)
{
	// 构造 .uid 文件路径
	std::string uidFilePath = mcpFolderPath + "\\.uid";

	// 尝试读取已有的 .uid 文件
	std::string content;
	if (Utils::LoadFileContent(uidFilePath.c_str(), content))
	{
		// 解析已有的 uid
		try
		{
			WUID uid = std::stoull(content);
			if (uid != 0)
				return uid;
		}
		catch (...) {}
	}

	// 不存在或无效，生成新的 uid
	WUID uid = GenWUID();
	std::string uidStr = std::to_string(uid);
	Utils::SaveFileContent(uidFilePath.c_str(), uidStr);
	return uid;
}

}
