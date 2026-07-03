#pragma once
#include <unordered_set>
#include <unordered_map>
#include <windows.h>
#include "timer/wuid.h"
#include "nlohmann/json.hpp"

using json = nlohmann::ordered_json;

class CLlmMcps
{
public:
	// MCP 连接设置（HTTP 或 stdio 模式）
	struct McpConnectSetting
	{
		std::string url;              // HTTP MCP server 的 URL
		std::string command;          // 启动mcp server的命令 (stdio模式)
		std::vector<std::string> args;  // 启动参数列表 (stdio模式)
		std::unordered_map<std::string, std::string> env;  // 自定义环境变量 (stdio模式)

		// 判断是否为 HTTP 模式
		bool IsHttpMode() const { return !url.empty(); }
		// 判断是否为 stdio 模式
		bool IsStdioMode() const { return !command.empty(); }
	};

	struct Mcp
	{
		enum class Type
		{
			None,
			Global,
			Project,
		};
		Type tp;
		WUID uid = 0;                // 唯一标识
		std::string name;//这个名字在同一个type下的所有mcp中是唯一的,注意这个name其实是folderPath的folder name,不是从文件里读出来的
		std::string legalName;       // name的合法版本，只包含字母数字"-""_"，用于生成唯一tool名称
		std::string description;

		// 根据name生成legalName（只保留字母数字"-""_"，为空则设为"mcp"）
		void GenerateLegalName()
		{
			legalName.clear();
			for (char c : name)
			{
				if (std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_')
					legalName.push_back(c);
			}
			if (legalName.empty())
				legalName = "mcp";
		}
		std::string folderPath;
		McpConnectSetting connect;      // MCP 连接设置（HTTP 或 stdio 模式）
		FILETIME fileTime = { 0 };      // MCP.json 文件的修改时间
		bool enable = false;            // mcp默认不启用

		std::unordered_set<std::string> disabledTools;  // 被禁用的tool名称集合

		// ---- 运行时数据(从mcp server动态获取) ----
		struct Tool
		{
			std::string name;         // tool名称
			std::string description;  // tool描述
			std::string inputSchema;  // tool参数的JSON schema (原始JSON字符串)
		};
		std::vector<Tool> tools;      // 该mcp server提供的所有tools
		bool toolsLoaded = false;     // tools是否已从server加载
		std::string lastError;        // 最后一次操作的错误信息

		// 判断tool是否启用(不在disabledTools中即为启用)
		bool IsToolEnabled(const std::string& toolName) const;
	};

	//先清除所有指定tp的Mcp, 再在指定目录下搜集所有的mcp信息,作为 tp 载入
	bool ReLoad(const char* rootPath, Mcp::Type tp);

	//从setting文件读取mcp及tool的enable状态
	//setting文件格式: [{"uid":"123456","enable":true,"disabledTools":["tool1","tool2"]}, ...]
	bool ReLoadSettings(const char* settingPath);

	//检查并重新载入文件(MCP.json)修改过的mcp，如果有变化则增加版本号
	void UpdateReLoadOutdated();

	const Mcp::Tool* FindTool(WUID uid, const char* toolName);

	void Clear();

	// 工具别名信息：用于解决不同mcp中tool名称冲突
	struct ToolAliasInfo
	{
		WUID mcpUid;               // 所属mcp的唯一标识
		std::string realToolName;  // tool的真实名称（非别名）
	};

	//将所有enable且已加载运行时tools的Mcp信息,填充到requestJson的"tools"数组中
	//只输出mcp.enable && tool.enable 的tool
	//如果requestJson里已有"tools",则追加到已有数组中
	//同时重建_toolsAlias查询表，确保tool名称唯一（冲突时生成别名）
	void FillToolsJson(json& requestJson);

	//根据别名查找tool信息（返回nullptr表示未找到）
	const ToolAliasInfo* FindToolByAlias(const std::string& alias) const;

	std::vector<Mcp> _mcps;
	std::unordered_map<std::string, ToolAliasInfo> _toolsAlias;  // 别名 -> (mcpName, realToolName)
	int _ver = 0;  // 版本号，每次ReLoad或ReLoadSettings加1

};

extern CLlmMcps g_llmMcps;

// 解析 MCP.json 文件
// 从中提取 description, command/args/env (stdio模式) 或 url (http模式)
// outContent: 可选，填充 MCP.json 的完整内容
bool ParseMcpJson(const wchar_t* filePath, std::string& outDescription,
                  std::string& outCommand, std::vector<std::string>& outArgs,
                  std::unordered_map<std::string, std::string>& outEnv,
                  std::string& outUrl,
                  std::string* outContent = nullptr);
