#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include "nlohmann/json.hpp"

#include "LlmLibDefines.h"


struct LlmToolCall
{
	LlmToolCall()
	{
		tp = LlmToolType::None;
		isComplete = false;
	}
	bool IsValid() const
	{
		if (!id.empty())
		{
			if (tp == LlmToolType::Mcp)
			{
				// MCP工具: 有id即可,参数通过raw_arguments传递
				if (!mcpName.empty())
					return true;
			}
			else if (tp != LlmToolType::None)
			{
				// 内置工具
				if (!params_int.empty())
					return true;
				if (!params_string.empty())
					return true;
				if (IsComplete())
					return true;
			}
		}
		return false;
	}
	bool IsComplete() const
	{
		return isComplete;
	}
	bool ExistParam(const char* param);
	bool GetStringParam(const char* param, std::string& value);
	bool GetIntParam(const char* param, int& value);

	LlmToolType tp;
	std::string id;
	std::string mcpName;  // MCP工具名称(仅当tp==Mcp时有效)
	std::unordered_map<std::string, std::string> params_string;
	std::unordered_map<std::string, int> params_int;
	std::string raw_arguments;
	std::string thoughtSignature;
	bool isComplete;
};

//持续接受来自LLM的steaming的response,构建一个完整的toolcall
class CLlmToolCallParser
{
public:
	void Reset(LlmApiFormat fmt);
	void Update(json& deltaResponseJson);//处理流式数据
	void GetIncompleteResult(std::vector<LlmToolCall>& result);//得到当前收到的不完整的tool call
	void FetchCompleteResult(std::vector<LlmToolCall>& result);//取走完整的tool call

protected:
	LlmApiFormat _format = LlmApiFormat::Unknown;

	struct PartialToolCall {
		std::string id;
		std::string name;
		std::string arguments; // Stores the partial/complete JSON string of arguments
	};

	struct LlmToolCallEx :LlmToolCall
	{
		LlmToolCallEx()
		{
			isFetched = false;
		}
		bool isFetched;
	};

	std::vector<LlmToolCallEx> _toolCalls;

	// For OpenAI, which can have multiple tool calls in parallel
	std::vector<PartialToolCall> _partialToolCalls;

	// For Anthropic, which sends tool calls sequentially
	PartialToolCall _anthropic_current_tool_call;
};

class CLlmTools
{
public:
	struct ToolParam 
	{
		std::string name;
		std::string type;
		std::string description;
	};

	struct ToolDefinition 
	{
		LlmToolType type = LlmToolType::None;
		std::string name;
		std::string description;
		std::vector<ToolParam> params;
		std::vector<std::string> required_params;
	};

	void Init();
	void Clear();

	//Tools 定义相关
	void BeginTool(LlmToolType tp, const char* name);//开始一个tool的定义
	void AppendToolDesc(const char* desc);//添加一段字串到tool的描述信息
	void AddToolPara_String(const char* name, const char* description, bool isRequired);//添加字符串参数
	void AddToolPara_Integer(const char* name, const char* description, bool isRequired);//添加整数参数
	void EndTool();//结束tool的定义

	//填充tool数据到Json对象
	void FillToolsJson(const std::vector<LlmToolType>& toolTypes, json& requestJson);

	//构造用来返回给llm的一次tool call的json字符串,包括调用及结果
	std::string MakeToolCallResultString(const LlmToolCall& toolCall, const char* result);
	//根据json字串,解析其内容为一个LlmToolCall和result,会自动识别其格式
	bool ParseToolCallResultString(const char* jsonString, LlmToolCall& toolCall, std::string& result);
	//如果这个ToolCallResultString里面包含文件内容信息,则返回这个文件的文件路径
	bool GetFilePathFromToolCallResultString(const char* jsonString, std::string& filePath);
	//如果这个tool call里包含一些修改文件的内容信息,把它们用"omitted"代替,这些内容一般会包含在已经修改过的文件内容里,如果我们要直接发送这些文件内容,tool call里的这些信息不需要再发送
	void OmitFileContentInToolCallResultString(std::string& jsonString);

	LlmToolType GetToolTypeByName(const std::string& name);
	const char* GetToolTypeName(LlmToolType tp);
	const ToolDefinition* GetToolDefinition(LlmToolType tp);

	// 从 ToolCallResult 字符串解析 LlmToolType
	static LlmToolType ParseToolTypeFromToolCallResultString(const std::string& content);

private:
	std::map<LlmToolType, ToolDefinition> _tools;
	ToolDefinition* _pCurrentToolDef = nullptr;
};

extern CLlmTools g_llmTools;