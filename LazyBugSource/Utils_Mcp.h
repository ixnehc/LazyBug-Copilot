#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include <functional>
#include "timer/timer.h"

#include "Utils_File.h"

#include "LlmMcps.h"

namespace Utils
{

std::string GetGlobalMcpsFolder();
std::string GetProjectMcpsFolder();
std::string GetMcpSettingFilePath();

// 修改setting文件，启用/禁用指定mcp（uid全局唯一，无需传type）
void EnableMcps(bool enable, const std::vector<WUID>& uids);

// 修改setting文件，启用/禁用指定mcp的工具
// enable = true: 从disabledTools中移除该tool（启用该工具）
// enable = false: 将该tool添加到disabledTools（禁用该工具）
void EnableMcpTools(bool enable, WUID mcpUid, const std::vector<std::string>& toolNames);

// 修改setting文件，启用指定mcp并启用其所有tools，然后ReloadSettings
void EnableMcpFullyAndReload(WUID mcpUid);
WUID EnsureMcpUid(const std::string& mcpFolderPath);

void MakeMcpToolDescription(const CLlmMcps::Mcp::Tool& tool, std::string& desc);

// 递归查找 json 中的 cmd/command+args 或 url
// 从 json 对象中提取 description, command/args/env (stdio模式) 或 url (http模式)
bool FindCmdAndArgs(const json& j, std::string& outDescription,
	std::string& outCommand, std::vector<std::string>& outArgs,
	std::unordered_map<std::string, std::string>& outEnv,
	std::string& outUrl);

// 从 JSON 字符串解析 MCP 配置 (与 ParseMcpJson 相同的提取逻辑, 但输入是字符串而非文件)
bool ParseMcpConfigFromString(const std::string& jsonStr,
	std::string& outDescription, std::string& outCommand,
	std::vector<std::string>& outArgs,
	std::unordered_map<std::string, std::string>& outEnv,
	std::string& outUrl);

}
