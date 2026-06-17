#include "stdh.h"
#include "LlmMcps.h"
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

// 移除字符串开头的UTF-8 BOM和不可读字符
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

bool ParseMcpJson(const std::string& filePath, std::string& outName, std::string& outDescription,
                  std::string& outCommand, std::vector<std::string>& outArgs, std::string* outContent)
{
	std::ifstream file;
	Utils::OpenIFStream(file, filePath.c_str());

	if (!file.is_open())
		return false;

	// 读取全部内容
	std::ostringstream ss;
	ss << file.rdbuf();
	std::string content = ss.str();
	file.close();

	// 处理BOM
	content = _RemoveBOMAndUnreadable(content);

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

	if (!j.is_object())
		return false;

	// 提取 name
	outName.clear();
	if (j.contains("name") && j["name"].is_string())
		outName = j["name"].get<std::string>();

	// 提取 description
	outDescription.clear();
	if (j.contains("description") && j["description"].is_string())
		outDescription = j["description"].get<std::string>();

	// 提取 command
	outCommand.clear();
	if (j.contains("command") && j["command"].is_string())
		outCommand = j["command"].get<std::string>();

	// 提取 args
	outArgs.clear();
	if (j.contains("args") && j["args"].is_array())
	{
		for (const auto& arg : j["args"])
		{
			if (arg.is_string())
				outArgs.push_back(arg.get<std::string>());
		}
	}

	return !outName.empty() && !outCommand.empty();
}

// 递归扫描目录，收集所有包含 MCP.json 的子目录
static void _ScanMcps(const std::wstring& wRootPath, std::vector<CLlmMcps::Mcp>& outMcps, CLlmMcps::Mcp::Type tp, int maxDepth = 3)
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
				std::string mcpJsonPath = widechar_to_utf8(wMcpJsonPath.c_str());
				std::string name, description, command;
				std::vector<std::string> args;
				if (ParseMcpJson(mcpJsonPath, name, description, command, args))
				{
					CLlmMcps::Mcp mcp;
					mcp.tp = tp;
					mcp.name = name;
					mcp.description = description;
					mcp.folderPath = fullPath;
					mcp.command = command;
					mcp.args = args;

					// 检查同一目录下是否存在 .enable 文件
					std::wstring wEnableFilePath = wFullPath + L"\\.enable";
					DWORD enableAttrs = GetFileAttributesW(wEnableFilePath.c_str());
					mcp.enable = (enableAttrs != INVALID_FILE_ATTRIBUTES && !(enableAttrs & FILE_ATTRIBUTE_DIRECTORY));

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
					mcp.description = "";
					mcp.folderPath = fullPath;
					mcp.command = "";
					mcp.enable = false;
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

	// 1. 清除所有指定 tp 的 mcp
	_mcps.erase(
		std::remove_if(_mcps.begin(), _mcps.end(),
			[tp](const Mcp& m) { return m.tp == tp; }),
		_mcps.end()
	);

	// 2. 检查 rootPath 是否存在
	DWORD attrs = GetFileAttributesW(wRootPath.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
		return false;

	// 3. 扫描目录，搜集所有 mcp 信息
	std::vector<Mcp> newMcps;
	_ScanMcps(wRootPath, newMcps, tp);

	// 4. 将搜集到的 mcp 加入 _mcps
	_mcps.insert(_mcps.end(), newMcps.begin(), newMcps.end());

	return true;
}

void CLlmMcps::Clear()
{
	_mcps.clear();
}

void CLlmMcps::Dump(std::string& str)
{
	str.clear();

	// 优先级: Project > Global > BuiltIn
	auto priority = [](Mcp::Type tp) -> int {
		switch (tp)
		{
		case Mcp::Type::Project:  return 3;
		case Mcp::Type::Global:   return 2;
		case Mcp::Type::BuiltIn:  return 1;
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

	// 构造符合LLM tool call格式的JSON数组
	json tools_array = json::array();

	for (const auto* mcp : deduped)
	{
		for (const auto& tool : mcp->tools)
		{
			json tool_def;
			tool_def["type"] = "function";

			json function_obj;
			function_obj["name"] = tool.name;
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

	str = tools_array.dump();
}
