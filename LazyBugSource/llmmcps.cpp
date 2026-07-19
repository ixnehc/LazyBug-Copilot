#include "stdh.h"
#include "LlmMcps.h"
#include "Utils_Mcp.h"
#include "stringparser/stringparser.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <windows.h>

#include "Utils_File.h"

// ============================================================================
// 内部辅助函数
// ============================================================================

CLlmMcps g_llmMcps;

using json = nlohmann::ordered_json;

// 去除字符串首尾空白
static std::string _Trim(const std::string& s)
{
	size_t start = 0;
	while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n'))
		start++;
	size_t end = s.size();
	while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n'))
		end--;
	return s.substr(start, end - start);
}

// 获取目录的修改时间
static FILETIME _GetDirectoryModTime(const std::wstring& wDirPath)
{
	FILETIME ft = { 0 };
	WIN32_FILE_ATTRIBUTE_DATA fileInfo;
	if (GetFileAttributesExW(wDirPath.c_str(), GetFileExInfoStandard, &fileInfo))
	{
		ft = fileInfo.ftLastWriteTime;
	}
	return ft;
}

// 删除指定 mcp 目录下的 .uid 文件
static void _DeleteMcpUidFile(const std::wstring& wMcpDirPath)
{
	std::wstring wUidPath = wMcpDirPath + L"\\.uid";
	DeleteFileW(wUidPath.c_str());
}

// 递归扫描目录，收集所有包含 MCP.json 的子目录信息（uid + 路径 + 目录修改时间）
struct _McpUidInfo
{
	WUID uid;
	std::wstring wDirPath;
	FILETIME dirModTime;
};

static void _CollectMcpUidInfos(const std::wstring& wRootPath, std::vector<_McpUidInfo>& outInfos, int maxDepth = 100)
{
	if (maxDepth <= 0)
		return;

	std::wstring wSearchPattern = wRootPath + L"\\*";

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(wSearchPattern.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		// 跳过 "." 和 ".."
		if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
			continue;

		// 跳过以 "." 开头的隐藏目录
		if (findData.cFileName[0] == L'.')
			continue;

		std::wstring wFullPath = wRootPath + L"\\" + findData.cFileName;

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 检查该目录下是否存在 MCP.json
			std::wstring wMcpJsonPath = wFullPath + L"\\MCP.json";
			DWORD mcpJsonAttrs = GetFileAttributesW(wMcpJsonPath.c_str());
			if (mcpJsonAttrs != INVALID_FILE_ATTRIBUTES && !(mcpJsonAttrs & FILE_ATTRIBUTE_DIRECTORY))
			{
			// MCP.json 存在，读取 .uid 文件
				std::wstring wUidPath = wFullPath + L"\\.uid";
				std::string uidContent;
				Utils::FileContentCodingFormat codingFmt;
				if (Utils::GetFileContentIntoUTF8(wUidPath.c_str(), uidContent, codingFmt))
				{
					try
					{
						WUID uid = std::stoull(_Trim(uidContent));
						if (uid != 0)
						{
							_McpUidInfo info;
							info.uid = uid;
							info.wDirPath = wFullPath;
							info.dirModTime = _GetDirectoryModTime(wFullPath);
							outInfos.push_back(info);
						}
					}
					catch (...) {}
				}
			}
			else
			{
				// 继续递归扫描子目录
				_CollectMcpUidInfos(wFullPath, outInfos, maxDepth - 1);
			}
		}
	} while (FindNextFileW(hFind, &findData));

	FindClose(hFind);
}

// 扫描并删除重复的 uid（保留目录修改时间最旧的那个）
static void _ScanRemoveDuplicateMcpUids(const std::wstring& wRootPath)
{
	// 1. 收集所有 mcp 目录的 uid 信息
	std::vector<_McpUidInfo> infos;
	_CollectMcpUidInfos(wRootPath, infos);

	// 2. 按 uid 分组
	std::unordered_map<WUID, std::vector<_McpUidInfo>> uidGroups;
	for (const auto& info : infos)
	{
		uidGroups[info.uid].push_back(info);
	}

	// 3. 处理重复的 uid
	for (auto& pair : uidGroups)
	{
		if (pair.second.size() > 1)
		{
			// 找出目录修改时间最旧的那个
			FILETIME oldestTime = pair.second[0].dirModTime;
			size_t oldestIndex = 0;
			for (size_t i = 1; i < pair.second.size(); i++)
			{
				if (CompareFileTime(&pair.second[i].dirModTime, &oldestTime) < 0)
				{
					oldestTime = pair.second[i].dirModTime;
					oldestIndex = i;
				}
			}

			// 删除其他重复的 .uid 文件
			for (size_t i = 0; i < pair.second.size(); i++)
			{
				if (i != oldestIndex)
				{
					_DeleteMcpUidFile(pair.second[i].wDirPath);
				}
			}
		}
	}
}

static std::string _RemoveBOMAndUnreadable(const std::string& str)
{
	const char* data = str.data();
	size_t len = str.size();
	size_t start = 0;

	// 跳过UTF-8 BOM (\xEF\xBB\xBF)
	if (len >= 3 && (unsigned char)data[0] == 0xEF && (unsigned char)data[1] == 0xBB && (unsigned char)data[2] == 0xBF)
	{
		start = 3;
	}

	// 跳过开头的不可读字符（控制字符，但保留换行符和制表符）
	while (start < len && (unsigned char)data[start] < 0x20 && data[start] != '\t')
	{
		start++;
	}

	return str.substr(start);
}

// 递归查找并列的 cmd/command+args 或 url (已移至 Utils::FindCmdAndArgs)

bool ParseMcpJson(const wchar_t* filePath, std::string& outDescription,
	std::string& outCommand, std::vector<std::string>& outArgs,
	std::unordered_map<std::string, std::string>& outEnv,
	std::string& outUrl,
	std::string* outContent)
{
	// 读取文件内容
	std::string content;
	Utils::FileContentCodingFormat codingFmt;
	if (!Utils::GetFileContentIntoUTF8(filePath, content, codingFmt))
		return false;

	// 如果请求了完整内容，填充
	if (outContent)
		*outContent = content;

	// 解析JSON
	json j;
	try
	{
		j = json::parse(content);
	}
	catch (const json::exception&)
	{
		return false;
	}

	// 递归查找 cmd/command+args 或 url
	return Utils::FindCmdAndArgs(j, outDescription, outCommand, outArgs, outEnv, outUrl);
}

// 递归扫描目录，收集所有包含 MCP.json 的子目录
static void _ScanMcps(const std::wstring& wRootPath, std::vector<CLlmMcps::Mcp>& outMcps, CLlmMcps::Mcp::Type tp, int maxDepth = 100)
{
	if (maxDepth <= 0)
		return;

	std::wstring wSearchPattern = wRootPath + L"\\*";

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(wSearchPattern.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		// 跳过 "." 和 ".."
		if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
			continue;

		// 跳过以 "." 开头的隐藏目录
		if (findData.cFileName[0] == L'.')
			continue;

		std::wstring wFullPath = wRootPath + L"\\" + findData.cFileName;
		std::string fullPath = widechar_to_utf8(wFullPath.c_str());

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 检查该目录下是否存在 MCP.json
			std::wstring wMcpJsonPath = wFullPath + L"\\MCP.json";
			DWORD mcpJsonAttrs = GetFileAttributesW(wMcpJsonPath.c_str());
			if (mcpJsonAttrs != INVALID_FILE_ATTRIBUTES && !(mcpJsonAttrs & FILE_ATTRIBUTE_DIRECTORY))
			{
			// MCP.json 存在，解析它
				std::string description, command, url;
				std::vector<std::string> args;
				std::unordered_map<std::string, std::string> env;
				if (ParseMcpJson(wMcpJsonPath.c_str(), description, command, args, env, url))
				{
					CLlmMcps::Mcp mcp;
					mcp.tp = tp;
					mcp.name = widechar_to_utf8(findData.cFileName);  // 使用 folder name
					mcp.GenerateLegalName();                          // 生成合法名称
					mcp.uid = Utils::EnsureMcpUid(fullPath);          // 确保 uid 存在
					mcp.description = description;
				mcp.folderPath = fullPath;
				mcp.connect.url = url;
				mcp.connect.command = command;
				mcp.connect.args = args;
				mcp.connect.env = env;
					mcp.fileTime = Utils::GetFileTime(wMcpJsonPath.c_str());  // 记录文件时间
					// enable 默认为 false，由 ReLoadSettings() 从设置文件读取
					outMcps.push_back(mcp);
				}
			}
			else
			{
				// 记录当前 outMcps 的大小，用于判断递归扫描是否找到 mcp
				size_t prevSize = outMcps.size();

				// 继续递归扫描子目录
				_ScanMcps(wFullPath, outMcps, tp, maxDepth - 1);

				// 如果递归扫描后没有找到任何 mcp，则该目录为空目录，添加一个 name 为空的 Mcp
				if (outMcps.size() == prevSize)
				{
					CLlmMcps::Mcp mcp;
					mcp.tp = tp;
					mcp.name = "";  // name 为空
					mcp.GenerateLegalName();  // 会生成 "mcp"
					mcp.description = "";
					mcp.folderPath = fullPath;
					mcp.connect.command = "";
					// enable 默认为 false
					outMcps.push_back(mcp);
				}
			}
		}
	} while (FindNextFileW(hFind, &findData));

	FindClose(hFind);
}

// ============================================================================
// CLlmMcps 公共方法
// ============================================================================

bool CLlmMcps::ReLoad(const char* rootPath, Mcp::Type tp)
{
	if (!rootPath || rootPath[0] == '\0')
		return false;

	// 转换为宽字符
	std::wstring wRootPath = utf8_to_widechar(rootPath);

	// 1. 清除所有指定 tp 的 mcp (保留 Dynamic)
	_mcps.erase(
		std::remove_if(_mcps.begin(), _mcps.end(),
			[tp](const Mcp& m) { return m.tp == tp; }),
		_mcps.end()
	);

	// 2. 检查 rootPath 是否存在
	DWORD attrs = GetFileAttributesW(wRootPath.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
		return false;

	// 3. 先删除重复的 uid
	_ScanRemoveDuplicateMcpUids(wRootPath);

	// 4. 扫描目录，搜集所有 mcp 信息
	std::vector<Mcp> newMcps;
	_ScanMcps(wRootPath, newMcps, tp);

	// 5. 将搜集到的 mcp 加入 _mcps
	_mcps.insert(_mcps.end(), newMcps.begin(), newMcps.end());

	_ver++;
	return true;
}

void CLlmMcps::UpdateReLoadOutdated()
{
	bool anyUpdated = false;

	for (auto& mcp : _mcps)
	{
		// 跳过空 name 的 mcp
		if (mcp.name.empty())
			continue;

		// 动态添加的 mcp 无文件, 跳过
		if (mcp.tp == Mcp::Type::Dynamic)
			continue;

		// 构造 MCP.json 路径
		std::wstring wMcpJsonPath = utf8_to_widechar(mcp.folderPath.c_str()) + L"\\MCP.json";

		// 获取当前文件时间
		FILETIME currentTime = Utils::GetFileTime(wMcpJsonPath.c_str());

		// 比较文件时间
		if (CompareFileTime(&currentTime, &mcp.fileTime) != 0)
		{
			// 文件有变化，重新载入
			std::string description, command, url;
			std::vector<std::string> args;
			std::unordered_map<std::string, std::string> env;
			if (ParseMcpJson(wMcpJsonPath.c_str(), description, command, args, env, url))
			{
				mcp.description = description;
				mcp.connect.url = url;
				mcp.connect.command = command;
				mcp.connect.args = args;
				mcp.connect.env = env;
				mcp.fileTime = currentTime;
				mcp.toolsLoaded = false;  // 需要重新加载 tools
				mcp.tools.clear();
				mcp.lastError.clear();
				anyUpdated = true;
			}
		}
	}

	if (anyUpdated)
		_ver++;
}

void CLlmMcps::Clear()
{
	_mcps.clear();
}

// Type转字符串
static std::string _TypeToString(CLlmMcps::Mcp::Type tp)
{
	switch (tp)
	{
	case CLlmMcps::Mcp::Type::Global:   return "Global";
	case CLlmMcps::Mcp::Type::Project:  return "Project";
	case CLlmMcps::Mcp::Type::Dynamic:  return "Dynamic";
	default:                            return "";
	}
}

// 字符串转Type
static CLlmMcps::Mcp::Type _StringToType(const std::string& str)
{
	if (str == "Global")   return CLlmMcps::Mcp::Type::Global;
	if (str == "Project")  return CLlmMcps::Mcp::Type::Project;
	return CLlmMcps::Mcp::Type::None;
}

bool CLlmMcps::Mcp::IsToolEnabled(const std::string& toolName) const
{
	return disabledTools.find(toolName) == disabledTools.end();
}

bool CLlmMcps::ReLoadSettings(const char* settingPath)
{
	if (!settingPath || settingPath[0] == '\0')
		return false;

	// 读取文件内容
	std::string content;
	Utils::FileContentCodingFormat codingFmt;
	if (!Utils::GetFileContentIntoUTF8(settingPath, content, codingFmt))
		return false;

	// 解析JSON
	json j;
	try
	{
		j = json::parse(content);
	}
	catch (const json::exception&)
	{
		return false;
	}

	// 新格式: 直接是一个数组
	if (!j.is_array())
		return false;

	// 遍历数组中的每个mcp设置
	for (const auto& mcpSetting : j)
	{
		if (!mcpSetting.is_object() || !mcpSetting.contains("uid") || !mcpSetting["uid"].is_string())
			continue;

		WUID mcpUid = 0;
		try { mcpUid = std::stoull(mcpSetting["uid"].get<std::string>()); }
		catch (...) { continue; }

		if (mcpUid == 0)
			continue;

		// 查找对应uid的mcp
		for (auto& mcp : _mcps)
		{
			if (mcp.uid == mcpUid)
			{
				// 读取mcp的enable状态
				if (mcpSetting.contains("enable") && mcpSetting["enable"].is_boolean())
					mcp.enable = mcpSetting["enable"].get<bool>();

				mcp.disabledTools.clear();
				// 读取disabledTools数组
				if (mcpSetting.contains("disabledTools") && mcpSetting["disabledTools"].is_array())
				{
					for (const auto& toolName : mcpSetting["disabledTools"])
					{
						if (toolName.is_string())
							mcp.disabledTools.insert(toolName.get<std::string>());
					}
				}
				break;
			}
		}
	}

	_ver++;
	return true;
}

void CLlmMcps::FillToolsJson(json& requestJson)
{
	// 重建别名查询表
	_toolsAlias.clear();

	// 用于检测名称冲突的集合（包括真实名称和已生成的别名）
	std::unordered_set<std::string> usedNames;

	// 优先级: Project > Global
	auto priority = [](Mcp::Type tp) -> int {
		switch (tp)
		{
		case Mcp::Type::Dynamic:  return 3;
		case Mcp::Type::Project:  return 2;
		case Mcp::Type::Global:   return 1;
		default:                  return 0;
		}
	};

	// 同名mcp按优先级覆盖，只保留最高优先级的
	std::vector<const Mcp*> deduped;
	std::unordered_map<std::string, size_t> nameToIndex;

	for (const auto& mcp : _mcps)
	{
		// 跳过未启用或未加载运行时tools的mcp
		if (!mcp.enable || !mcp.toolsLoaded || mcp.tools.empty())
			continue;

		auto it = nameToIndex.find(mcp.name);
		if (it == nameToIndex.end())
		{
			nameToIndex[mcp.name] = deduped.size();
			deduped.push_back(&mcp);
		}
		else
		{
			const Mcp* existing = deduped[it->second];
			if (priority(mcp.tp) > priority(existing->tp))
			{
				deduped[it->second] = &mcp;
			}
		}
	}

	if (deduped.empty())
		return;

	// 如果requestJson已有"tools"数组,则追加;否则新建
	json tools_array;
	if (requestJson.contains("tools") && requestJson["tools"].is_array())
		tools_array = requestJson["tools"];
	else
		tools_array = json::array();

	for (const auto* mcp : deduped)
	{
		for (const auto& tool : mcp->tools)
		{
		// 跳过未启用的tool
			if (!mcp->IsToolEnabled(tool.name))
				continue;

			// 生成唯一名称：legalName-toolName，冲突时加数字后缀
			std::string aliasBase = mcp->legalName + "-" + tool.name;
			std::string uniqueName = aliasBase;
			int suffix = 1;
			while (usedNames.count(uniqueName) > 0)
			{
				uniqueName = aliasBase + "-" + std::to_string(suffix++);
			}
			usedNames.insert(uniqueName);

			// 记录别名映射关系（即使没冲突也记录，方便统一查询）
			ToolAliasInfo info;
			info.mcpUid = mcp->uid;
			info.realToolName = tool.name;
			_toolsAlias[uniqueName] = info;

			json tool_def;
			tool_def["type"] = "function";

			json function_obj;
			function_obj["name"] = uniqueName;
			function_obj["description"] = tool.description;

			// 解析inputSchema为JSON对象
			if (!tool.inputSchema.empty())
			{
				try
				{
					json schema = json::parse(tool.inputSchema);
					function_obj["parameters"] = schema;
				}
				catch (const json::exception&)
				{
					// 解析失败，使用空object
					function_obj["parameters"] = json::object();
				}
			}
			else
			{
				function_obj["parameters"] = json::object();
			}

			tool_def["function"] = function_obj;
			tools_array.push_back(tool_def);
		}
	}

	if (!tools_array.empty())
		requestJson["tools"] = tools_array;
}

const CLlmMcps::Mcp::Tool* CLlmMcps::FindTool(WUID uid, const char* toolName)
{
	if (!toolName)
		return nullptr;

	for (const auto& mcp : _mcps)
	{
		if (mcp.uid != uid)
			continue;

		for (const auto& tool : mcp.tools)
		{
			if (tool.name == toolName)
				return &tool;
		}
		break;
	}
	return nullptr;
}

const CLlmMcps::ToolAliasInfo* CLlmMcps::FindToolByAlias(const std::string& alias) const
{
	auto it = _toolsAlias.find(alias);
	if (it != _toolsAlias.end())
		return &it->second;
	return nullptr;
} 

WUID CLlmMcps::AddDynamicMcp(const char* name, const McpConnectSetting& connect, const char* description)
{
	if (!name || name[0] == '\0')   
		return 0;
	 
	Mcp mcp;
	mcp.tp = Mcp::Type::Dynamic;
	mcp.uid = GenWUID();
	mcp.name = name;
	mcp.GenerateLegalName();
	mcp.description = description ? description : "";
	mcp.connect = connect;
	mcp.enable = true;  // 动态添加的 mcp 默认启用

	_mcps.push_back(std::move(mcp));
	_ver++;
	return _mcps.back().uid;
}
 
void CLlmMcps::RemoveDynamicMcp(WUID uid)
{
	for (auto it = _mcps.begin(); it != _mcps.end(); ++it)
	{
		if (it->uid == uid && it->tp == Mcp::Type::Dynamic)
		{
			_mcps.erase(it);
			_ver++;
			return;
		}
	}
}

const CLlmMcps::Mcp* CLlmMcps::FindMcpByName(const std::string& name) const
{
	// 按优先级查找: Dynamic > Project > Global
	for (const auto& mcp : _mcps)
	{
		if (mcp.tp == Mcp::Type::Dynamic && mcp.name == name)
			return &mcp;
	}
	for (const auto& mcp : _mcps)
	{
		if (mcp.tp == Mcp::Type::Project && mcp.name == name)
			return &mcp;
	}
	for (const auto& mcp : _mcps)
	{
		if (mcp.tp == Mcp::Type::Global && mcp.name == name)
			return &mcp;
	}
	return nullptr;
}
 
bool CLlmMcps::IsSameConnect(const McpConnectSetting& a, const McpConnectSetting& b)
{
	if (a.IsHttpMode() || b.IsHttpMode())
	{
		// HTTP 模式: 比较 url
		return a.url == b.url;
	}
	else
	{
		// stdio 模式: 比较 command + args + env + workingDir
		if (a.command != b.command)
			return false;
		if (a.args != b.args)
			return false;
		if (a.env != b.env)
			return false;
		if (a.workingDir != b.workingDir)
			return false;
		return true;
	}
}
