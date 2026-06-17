#pragma once

class CLlmMcps
{
public:
	struct Mcp
	{
		enum class Type
		{
			None,
			BuiltIn,
			Global,
			Project,
		};
		Type tp;
		std::string name;
		std::string description;
		std::string folderPath;
		std::string command;            // 启动mcp server的命令 (从MCP.json解析)
		std::vector<std::string> args;  // 启动参数列表 (从MCP.json解析)
		bool enable;

		// ---- 运行时数据(从mcp server动态获取) ----
		struct Tool
		{
			std::string name;         // tool名称
			std::string description;  // tool描述
			std::string inputSchema;  // tool参数的JSON schema (原始JSON字符串)
		};
		std::vector<Tool> tools;      // 该mcp server提供的所有tools
		bool toolsLoaded = false;     // tools是否已从server加载
	};

	//先清除所有指定tp的Mcp, 再在指定目录下搜集所有的mcp信息,作为 tp 载入
	bool ReLoad(const char* rootPath, Mcp::Type tp);

	void Clear();

	//将所有enable且已加载运行时tools的Mcp信息,输出为符合LLM tool call格式的JSON字符串
	void Dump(std::string& str);

	std::vector<Mcp> _mcps;

};

extern CLlmMcps g_llmMcps;

// 解析 MCP.json 文件
// 从中提取 name, description, command, args
// outContent: 可选，填充 MCP.json 的完整内容
bool ParseMcpJson(const std::string& filePath, std::string& outName, std::string& outDescription,
                  std::string& outCommand, std::vector<std::string>& outArgs, std::string* outContent = nullptr);
