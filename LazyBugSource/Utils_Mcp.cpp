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
	case CLlmMcps::Mcp::Type::Dynamic: return "Dynamic";
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

void EnableMcpFullyAndReload(WUID mcpUid)
{
	if (mcpUid == 0)
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
	bool found = false;

	for (auto& item : j)
	{
		if (!item.is_object() || !item.contains("uid") || !item["uid"].is_string())
			continue;
		if (item["uid"].get<std::string>() == uidStr)
		{
			item["enable"] = true;
			item["disabledTools"] = json::array();
			found = true;
			break;
		}
	}

	if (!found)
	{
		json item;
		item["uid"] = uidStr;
		item["enable"] = true;
		item["disabledTools"] = json::array();
		j.push_back(item);
	}

	// 写回文件
	std::string newContent = j.dump(2);
	Utils::SaveFileContent(settingPath.c_str(), newContent);

	// ReloadSettings
	::g_llmMcps.ReLoadSettings(settingPath.c_str());
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

// 递归查找 json 中的 cmd/command+args 或 url
bool FindCmdAndArgs(const json& j, std::string& outDescription,
	std::string& outCommand, std::vector<std::string>& outArgs,
	std::unordered_map<std::string, std::string>& outEnv,
	std::string& outUrl)
{
	if (!j.is_object())
		return false;

	// 检查当前对象是否有 url (HTTP 模式)
	if (j.contains("url") && j["url"].is_string())
	{
		outUrl = j["url"].get<std::string>();
		outCommand.clear();
		outArgs.clear();
		outEnv.clear();

		// 同时提取 description（如果存在）
		outDescription.clear();
		if (j.contains("description") && j["description"].is_string())
			outDescription = j["description"].get<std::string>();

		return true;
	}

	// 检查当前对象是否有 command/cmd 和 args (stdio 模式)
	std::string cmd;
	bool hasCmd = false;
	if (j.contains("command") && j["command"].is_string())
	{
		cmd = j["command"].get<std::string>();
		hasCmd = true;
	}
	else if (j.contains("cmd") && j["cmd"].is_string())
	{
		cmd = j["cmd"].get<std::string>();
		hasCmd = true;
	}

	if (hasCmd && j.contains("args") && j["args"].is_array())
	{
		// 找到有效的 cmd + args 组合
		outCommand = cmd;
		outUrl.clear();
		outArgs.clear();
		for (const auto& arg : j["args"])
		{
			if (arg.is_string())
				outArgs.push_back(arg.get<std::string>());
		}

		// 同时提取 description（如果存在）
		outDescription.clear();
		if (j.contains("description") && j["description"].is_string())
			outDescription = j["description"].get<std::string>();

		// 提取 env（如果存在）
		outEnv.clear();
		if (j.contains("env") && j["env"].is_object())
		{
			for (json::const_iterator it = j["env"].begin(); it != j["env"].end(); ++it)
			{
				if (it.value().is_string())
					outEnv[it.key()] = it.value().get<std::string>();
			}
		}

		return true;
	}

	// 递归检查所有子对象
	for (json::const_iterator it = j.begin(); it != j.end(); ++it)
	{
		if (FindCmdAndArgs(it.value(), outDescription, outCommand, outArgs, outEnv, outUrl))
			return true;
	}

	return false;
}

// 从 JSON 字符串解析 MCP 配置
bool ParseMcpConfigFromString(const std::string& jsonStr,
	std::string& outDescription, std::string& outCommand,
	std::vector<std::string>& outArgs,
	std::unordered_map<std::string, std::string>& outEnv,
	std::string& outUrl)
{
	json j;
	try
	{
		j = json::parse(jsonStr);
	}
	catch (const json::exception&)
	{
		return false;
	}

	return FindCmdAndArgs(j, outDescription, outCommand, outArgs, outEnv, outUrl);
}

static std::string _EscapeMdTableCell(const std::string& s)
{
	std::string out;
	out.reserve(s.size());
	for (char c : s)
	{
		if (c == '|')
			out += "\\|";
		else if (c == '\n')
			out += "<br>";
		else if (c == '\r')
			; // skip
		else
			out += c;
	}
	return out;
}

static std::string _GetSchemaTypeString(const json& prop)
{
	if (!prop.contains("type"))
		return "any";

	const auto& typeField = prop["type"];
	if (typeField.is_string())
	{
		std::string typeStr = typeField.get<std::string>();
		if (typeStr == "array" && prop.contains("items") && prop["items"].is_object())
		{
			std::string itemType = _GetSchemaTypeString(prop["items"]);
			if (itemType != "any")
				return "array of " + itemType;
		}
		return typeStr;
	}

	if (typeField.is_array())
	{
		std::string typeStr;
		for (const auto& t : typeField)
		{
			if (!t.is_string())
				continue;
			if (!typeStr.empty())
				typeStr += " / ";
			typeStr += t.get<std::string>();
		}
		return typeStr.empty() ? "any" : typeStr;
	}

	return "any";
}

void MakeMcpToolDescription(const CLlmMcps::Mcp::Tool& tool, std::string& desc)
{
	desc.clear();

	desc += "## ";
	desc += tool.name.empty() ? "Unnamed Tool" : tool.name;
	desc += "\n\n";

	if (!tool.description.empty())
	{
		desc += tool.description;
		desc += "\n\n";
	}

	if (tool.inputSchema.empty())
	{
		desc += "*No parameters.*\n";
		return;
	}

	json schema;
	try
	{
		schema = json::parse(tool.inputSchema);
	}
	catch (const json::exception&)
	{
		desc += "*Failed to parse parameter schema.*\n";
		return;
	}

	if (!schema.is_object() || !schema.contains("properties") || !schema["properties"].is_object())
	{
		desc += "*No parameters.*\n";
		return;
	}

	const auto& properties = schema["properties"];
	if (properties.empty())
	{
		desc += "*No parameters.*\n";
		return;
	}

	std::unordered_set<std::string> requiredSet;
	if (schema.contains("required") && schema["required"].is_array())
	{
		for (const auto& req : schema["required"])
		{
			if (req.is_string())
				requiredSet.insert(req.get<std::string>());
		}
	}

	desc += "### Parameters\n\n";
	desc += "| Parameter | Type | Required | Description |\n";
	desc += "|-----------|------|----------|-------------|\n";

	for (auto it = properties.begin(); it != properties.end(); ++it)
	{
		const std::string& paramName = it.key();
		const auto& paramDef = it.value();

		std::string paramType = _GetSchemaTypeString(paramDef);
		std::string required = requiredSet.count(paramName) > 0 ? "Yes" : "No";

		std::string paramDesc;
		if (paramDef.contains("description") && paramDef["description"].is_string())
			paramDesc = paramDef["description"].get<std::string>();

		desc += "| ";
		desc += _EscapeMdTableCell(paramName);
		desc += " | ";
		desc += _EscapeMdTableCell(paramType);
		desc += " | ";
		desc += required;
		desc += " | ";
		desc += _EscapeMdTableCell(paramDesc);
		desc += " |\n";
	}
}

}
