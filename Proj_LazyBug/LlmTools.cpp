#include "stdh.h"
#include "LlmTools.h"
#include <stdexcept>

// 注意: 此实现需要 nlohmann/json 库.
// 请从 https://github.com/nlohmann/json 获取并将其包含到您的项目中.
#include "nlohmann/json.hpp"

#include "stringparser/stringparser.h"
#include "Utils.h"

namespace Utils {
	extern bool is_valid_utf8(const std::string& s);
}

using json = nlohmann::ordered_json;

void ExtractPartialArguments(const std::string& raw_args, const CLlmTools::ToolDefinition& toolDef, LlmToolCall& toolCall)
{
	toolCall.params_string.clear();
	toolCall.params_int.clear();

	for (const auto& paramDef : toolDef.params)
	{
		const auto& paramName = paramDef.name;

		std::string search_key = "\"" + paramName + "\":";
		size_t key_pos = raw_args.find(search_key);

		if (key_pos == std::string::npos) continue;

		size_t value_start_pos = key_pos + search_key.length();
		value_start_pos = raw_args.find_first_not_of(" \t\n\r", value_start_pos);

		if (value_start_pos == std::string::npos) continue;

		if (paramDef.type == "string")
		{
			if (raw_args[value_start_pos] == '"')
			{
				size_t string_start = value_start_pos + 1;
				size_t string_end = string_start;
				while ((string_end = raw_args.find('"', string_end)) != std::string::npos)
				{
					size_t backslashes = 0;
					size_t pos = string_end;
					while (pos > 0 && raw_args[pos - 1] == '\\') {
						backslashes++;
						pos--;
					}

					if (backslashes % 2 == 1) { // 奇数个反斜杠, 说明引号被转义
						string_end++;
						continue;
					}

					// 引号没有被转义, 这是字符串的结尾
					std::string val = raw_args.substr(string_start, string_end - string_start);
					toolCall.params_string[paramName] = val;
					break;
				}
				if (string_end == std::string::npos)
				{
					// 没有找到结束引号, 这是一个不完整的字符串
					toolCall.params_string[paramName] = raw_args.substr(string_start);
				}
			}
		}
		else if (paramDef.type == "integer")
		{
			if (isdigit(raw_args[value_start_pos]) || (raw_args[value_start_pos] == '-' && value_start_pos + 1 < raw_args.length() && isdigit(raw_args[value_start_pos + 1])))
			{
				size_t num_end_pos = value_start_pos + 1;
				while (num_end_pos < raw_args.length() && isdigit(raw_args[num_end_pos]))
				{
					num_end_pos++;
				}
				std::string num_str = raw_args.substr(value_start_pos, num_end_pos - value_start_pos);
				if (!num_str.empty())
				{
					try { toolCall.params_int[paramName] = std::stoi(num_str); }
					catch (const std::exception&) {}
				}
			}
		}
	}
}

// 尝试修补不完整的JSON字符串
std::string TryFixIncompleteJson(const std::string& incomplete_json)
{
	if (incomplete_json.empty()) return incomplete_json;
	
	std::string fixed = incomplete_json;
	
	// 计算各种括号和引号的状态
	int brace_count = 0;    // {}
	int bracket_count = 0;  // []
	bool in_string = false;
	bool escape_next = false;
	
	for (size_t i = 0; i < fixed.length(); i++)
	{
		char c = fixed[i];
		
		if (escape_next)
		{
			escape_next = false;
			continue;
		}
		
		if (c == '\\')
		{
			escape_next = true;
			continue;
		}
		
		if (c == '"')
		{
			in_string = !in_string;
		}
		else if (!in_string)
		{
			if (c == '{')
				brace_count++;
			else if (c == '}')
				brace_count--;
			else if (c == '[')
				bracket_count++;
			else if (c == ']')
				bracket_count--;
		}
	}
	
	// 如果在字符串内部结束，补齐引号
	if (in_string)
	{
		fixed += '"';
	}
	
	// 补齐缺失的右括号
	while (bracket_count > 0)
	{
		fixed += ']';
		bracket_count--;
	}
	
	// 补齐缺失的右花括号
	while (brace_count > 0)
	{
		fixed += '}';
		brace_count--;
	}
	
	return fixed;
}

//////////////////////////////////////////////////////////////////////////
//CLlmToolCallParser

void CLlmToolCallParser::Reset(LlmApiFormat fmt)
{
	_format = fmt;
	_partialToolCalls.clear();
	_anthropic_current_tool_call = {};
	_toolCalls.clear();
}

void CLlmToolCallParser::Update(json& deltaResponseJson)
{
//	if (_format == LlmApiFormat::OpenAI)
	if(true)
	{
		if (!deltaResponseJson.contains("choices") || deltaResponseJson["choices"].empty()) return;

		auto& choice = deltaResponseJson["choices"][0];
		if (!choice.contains("delta")) return;
		auto& delta = choice["delta"];

		if (delta.contains("tool_calls"))
		{
			for (const auto& call_delta : delta["tool_calls"])
			{
				size_t index = call_delta["index"].get<size_t>();

				if (_partialToolCalls.size() <= index) _partialToolCalls.resize(index + 1);
				if (_toolCalls.size() <= index) _toolCalls.resize(index + 1);

				auto& parsing_call = _partialToolCalls[index];
				auto& final_tool_call = _toolCalls[index];

				if (call_delta.contains("id") && !call_delta["id"].is_null())
				{
					std::string id = call_delta["id"];
					if (!id.empty())
						parsing_call.id = std::move(id);
					final_tool_call.id = parsing_call.id;
				}
				
				if (call_delta.contains("thoughtSignature") && !call_delta["thoughtSignature"].is_null())
				{
					final_tool_call.thoughtSignature += call_delta["thoughtSignature"].get<std::string>();
				}

				if (call_delta.contains("function"))
				{
					auto& function_delta = call_delta["function"];
					if (function_delta.contains("name") && !function_delta["name"].is_null())
					{
						parsing_call.name = function_delta["name"];
						final_tool_call.tp = g_llmTools.GetToolTypeByName(parsing_call.name);
					}
					if (function_delta.contains("arguments") && !function_delta["arguments"].is_null())
					{
						parsing_call.arguments += function_delta["arguments"].get<std::string>();
						final_tool_call.raw_arguments = parsing_call.arguments;
					}
				}

				if (!parsing_call.arguments.empty())
				{
					const auto* toolDef = g_llmTools.GetToolDefinition(final_tool_call.tp);
					if (!toolDef) continue;

					json args_json;
					if (json::accept(parsing_call.arguments))
						args_json= json::parse(parsing_call.arguments);
					else
					{
						std::string fixed_json = TryFixIncompleteJson(parsing_call.arguments);
						if (json::accept(fixed_json))
							args_json = json::parse(fixed_json);
					}

                    if (!args_json.empty())
					{
						final_tool_call.params_string.clear();
						final_tool_call.params_int.clear();
						for (const auto& el : args_json.items())
						{
							auto para_it = std::find_if(toolDef->params.begin(), toolDef->params.end(),
								[&el](const CLlmTools::ToolParam& p) { return p.name == el.key(); });
							if (para_it != toolDef->params.end())
							{
								if (para_it->type == "string" && el.value().is_string())
								{
									final_tool_call.params_string[el.key()] = el.value().get<std::string>();
								}
								else if (para_it->type == "integer" && el.value().is_number_integer())
								{
									final_tool_call.params_int[el.key()] = el.value().get<int>();
								}
							}
						}
					}
				}
			}
		}

		if (choice.contains("finish_reason") && (choice["finish_reason"] == "tool_calls"|| choice["finish_reason"] == "stop"))
		{
			for (auto& tool_call : _toolCalls)
			{
				if (tool_call.tp != LlmToolType::None)
				{
					tool_call.isComplete = true;
				}
			}
		}
	}
}

void CLlmToolCallParser::GetIncompleteResult(std::vector<LlmToolCall>& result)
{
	for (int i = 0; i < _toolCalls.size(); i++)
	{
		if (!_toolCalls[i].isComplete)
		{
			result.push_back(_toolCalls[i]);
		}
	}
}
void CLlmToolCallParser::FetchCompleteResult(std::vector<LlmToolCall>& result)
{
	for (int i = 0; i < _toolCalls.size(); i++)
	{
		if (_toolCalls[i].isComplete && !_toolCalls[i].isFetched)
		{
			result.push_back(_toolCalls[i]);
            _toolCalls[i].isFetched=true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//CLlmToolCall
bool LlmToolCall::ExistParam(const char* param)
{
    if (params_string.find(param) != params_string.end())
        return true;
    if (params_int.find(param) != params_int.end())
        return true;
    return false;
}

bool LlmToolCall::GetStringParam(const char* param, std::string& value)
{
	value = "";
	auto it = params_string.find(param);
	if (it != params_string.end())
	{
		value = it->second;
		return true;
	}
	return false;
}

bool LlmToolCall::GetIntParam(const char* param, int& value)
{
	auto it = params_int.find(param);
	if (it != params_int.end())
	{
		value = it->second;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//CLlmTools
void CLlmTools::Init()
{
	Clear();

	// 定义 SearchSymbolImplement 工具
	BeginTool(LlmToolType::FindSymbolDefine, "FindSymbol");
	AppendToolDesc("Search for the implementation of a symbol (e.g., function, class, variable) in the codebase. Symbols can be nested using '::' or '.' as a separator. Examples: 'MyClass::myMethod', 'Namespace::Class::function','Namespace.Class.m_Member'. Currently supports c/c++/c# symbols");
	AddToolPara_String("symbols", "The symbol(s) to search for. Multiple symbols can be separated by '|' to search for each symbol independently. Example: 'MyClass::methodA|MyClass::methodB|classC'", true);
	EndTool();

	// 定义 FindInFiles 工具
	BeginTool(LlmToolType::FindInFiles, "Grep");
	AppendToolDesc("Search for text across all files in the codebase. For each search result, the symbol(function name,class name,...) including the searched text could also be returned, which could be used in the tool FindSymbol. ");
	AppendToolDesc("Note: Wildcards or regular expression are not supported. It only supports 'Match case' and 'Match whole word' matching.");
	AddToolPara_String("keywords", "The keyword or text to search for. Multiple keywords can be separated by '|' to search for each keyword independently. Example: 'functionA|functionB|classC'", true);
// 	AddToolPara_Integer("maxResults", "Maximum number of results to return (default: 100)", false);
	EndTool();

	// 定义 SearchFile 工具
	BeginTool(LlmToolType::SearchFile, "Glob");
	AppendToolDesc("Search for file pathes in the codebase by keywords (case-insensitive). It matches against the full file path. Support wildcards ('*' matches any chars not including path separator, '**' matches across directories, '?' matches a single char). Without wildcards, it performs a substring inclusion search.");
	AppendToolDesc(" Examples:");
	AppendToolDesc(" 1. 'MyClass.h' -> Finds any path containing 'myclass.h' as a substring.");
	AppendToolDesc(" 2. 'AStar*.cpp' -> Finds cpp files where the file name constains 'astar'.");
	AppendToolDesc(" 3. '\\AStar*.cpp' -> Specifically finds cpp files where the FILE NAME starts with 'astar' (the '\\' matches the directory separator).");
	AppendToolDesc(" 4. 'src\\utils\\*.h' -> Finds all .h files under all 'src\\utils' folder.");
	
	AddToolPara_String("keywords", "The keyword(s) to search for in file paths. Multiple keywords can be separated by '|'. Example: 'abc.jpg|\\Main*.cpp|project\\sample.h'", true);
	EndTool();

	// 定义 ReadFile 工具
	BeginTool(LlmToolType::ReadFile, "ReadFile");
	AppendToolDesc("This tool allows you to read the content of a file in the codebase.");
	AddToolPara_String("filePath", "The full path of the file to read.", true);
	AddToolPara_Integer("startLine", "The starting line number to read from (default: 1).", false);
	AddToolPara_Integer("endLine", "The ending line number to read to. If not specified, reads the entire file.", false);
	EndTool();

	// 定义 CLI_Cmd 工具
	BeginTool(LlmToolType::CLI_Cmd, "Cmd");
	AppendToolDesc("Execute a Windows cmd.exe command and return the output.");
	AppendToolDesc("Note: cmd.exe does not support multi-line commands.");
	AddToolPara_String("command", "The command line command to execute.", true);
	AddToolPara_String("desc", "A brief description of what this command does.", true);
	AddToolPara_String("workingDir", "The working directory for the command execution. If not specified, uses the current directory.", false);
	AddToolPara_Integer("timeout", "Timeout in milliseconds for the command execution (default: 30000).", false);
	EndTool();

	// 定义 CLI_Bash 工具
	BeginTool(LlmToolType::CLI_Bash, "Bash");
	AppendToolDesc("Execute a bash command via bash.exe (WSL) and return the output.");
	AddToolPara_String("command", "The bash command to execute.", true);
	AddToolPara_String("desc", "A brief description of what this command does.", true);
	AddToolPara_String("workingDir", "The working directory for the command execution. If not specified, uses the current directory.", false);
	AddToolPara_Integer("timeout", "Timeout in milliseconds for the command execution (default: 30000).", false);
	EndTool();

	// 定义 CLI_RunScript 工具
	BeginTool(LlmToolType::CLI_RunScript, "RunPythonScript");
	AppendToolDesc("Execute a Python script and return the output.");
	AddToolPara_String("command", "The python script content to execute.", true);
	AddToolPara_String("desc", "A brief description of what this script does.", true);
	AddToolPara_String("workingDir", "The working directory for the script execution. If not specified, uses the current directory.", false);
	AddToolPara_Integer("timeout", "Timeout in milliseconds for the script execution (default: 30000).", false);
	EndTool();

	// 定义 ReplaceInFile 工具
	BeginTool(LlmToolType::ReplaceInFile, "EditFile");
	AppendToolDesc("This tool allows you to edit a file by replacing a range of lines with new content. You can use this tool to create a new file by specifying a new file path and leaving the oldLines empty");
	AddToolPara_String("filePath", "The full path of the file to edit.", true);
	AddToolPara_String("oldLines", "The lines that will be removed", true);
	AddToolPara_String("newLines", "The lines that will be added and replace the old lines", true);
	EndTool();

	// 定义 Question 工具
	BeginTool(LlmToolType::Question, "AskQuestion");
	AppendToolDesc("Ask the user a question with options. Use this tool when you need to get brief information or choice from the user.");
	AppendToolDesc("Note: If 'options' is empty, the user will manually input a free-text answer.");
	AddToolPara_String("question", "The question to ask the user.", true);
	AddToolPara_String("options", "Comma-separated list of options for the user to choose from. If empty, the user will manually input a free-text answer.", false);
	EndTool();

	// 定义 QueryFinish 工具
	BeginTool(LlmToolType::QueryFinish, "QueryFinish");
	AppendToolDesc("Request user confirmation before ending the entire conversation. Use this tool when you believe the task is complete and the conversation should end.");
	AppendToolDesc("If the user confirms, no further messages should be sent and the conversation will end directly.");
	EndTool();

	// 定义 CreateSkill 工具
	BeginTool(LlmToolType::CreateSkill, "CreateSkill");
	AppendToolDesc("Create a new skill. A skill is a reusable capability that can be invoked by the AI assistant.Never use this tool to update an existing skill");
	AddToolPara_String("name", "The name of the skill to create.", true);
	AddToolPara_String("type", "The type of the skill. Valid values: 'Global' (available across all projects) or 'Project' (only available in current project).", true);
	AddToolPara_String("description", "A brief description of what this skill does.", false);
	AddToolPara_String("content", "The content of the skill (optional). This typically includes instructions or templates.", false);
	EndTool();

}

void CLlmTools::Clear()
{
	_tools.clear();
	_pCurrentToolDef = nullptr;
}

void CLlmTools::BeginTool(LlmToolType tp, const char* name)
{
	if (_pCurrentToolDef)
	{
		throw std::runtime_error("Must call EndTool() before beginning a new tool.");
	}

	auto it = _tools.find(tp);
	if (it != _tools.end())
	{
		throw std::runtime_error("Tool with this type already exists.");
	}

	ToolDefinition def;
	def.type = tp;
	def.name = name;
	_tools[tp] = def;
	_pCurrentToolDef = &_tools[tp];
}

void CLlmTools::AppendToolDesc(const char* desc)
{
	if (!_pCurrentToolDef)
	{
		throw std::runtime_error("Must call BeginTool() first.");
	}
	_pCurrentToolDef->description += desc;
}

void CLlmTools::AddToolPara_String(const char* name, const char* description, bool isRequired)
{
	if (!_pCurrentToolDef)
	{
		throw std::runtime_error("Must call BeginTool() first.");
	}
	ToolParam param;
	param.name = name;
	param.description = description;
	param.type = "string";
	_pCurrentToolDef->params.push_back(param);

	if (isRequired)
	{
		_pCurrentToolDef->required_params.push_back(name);
	}
}

void CLlmTools::AddToolPara_Integer(const char* name, const char* description, bool isRequired)
{
	if (!_pCurrentToolDef)
	{
		throw std::runtime_error("Must call BeginTool() first.");
	}
	ToolParam param;
	param.name = name;
	param.description = description;
	param.type = "integer";
	_pCurrentToolDef->params.push_back(param);

	if (isRequired)
	{
		_pCurrentToolDef->required_params.push_back(name);
	}
}

void CLlmTools::EndTool()
{
	_pCurrentToolDef = nullptr;
}

extern bool IsPrompCachingEnabled();
void CLlmTools::FillToolsJson(const std::vector<LlmToolType>& toolTypes, json& requestJson)
{
	if (toolTypes.empty())
	{
		return;
	}

	json tools_array = json::array();

    for (int i=0;i<toolTypes.size();i++)
	{
        LlmToolType toolType = toolTypes[i];
		auto it = _tools.find(toolType);
		if (it == _tools.end())
		{
			continue;
		}

		const ToolDefinition& def = it->second;
		json tool_def;

		json properties = json::object();
		for (const auto& param : def.params)
		{
			json para_def;
			para_def["type"] = param.type;
			para_def["description"] = param.description;
			properties[param.name] = para_def;
		}

		json parameters_obj;
		parameters_obj["type"] = "object";
		parameters_obj["properties"] = properties;
		if (!def.required_params.empty())
		{
			parameters_obj["required"] = def.required_params;
		}

		if (true)
		{
			tool_def["type"] = "function";
			json function_obj;
			function_obj["name"] = def.name;
			function_obj["description"] = def.description;
			function_obj["parameters"] = parameters_obj;
			//如果it是最后一个元素，则设置cache_control为ephemeral
// 			if (false)
			if (IsPrompCachingEnabled())
			{
				if (i == toolTypes.size() - 1)
				{
					json cache_control_obj;
					cache_control_obj["type"] = "ephemeral";
					tool_def["cache_control"] = cache_control_obj;
				}
			}
			tool_def["function"] = function_obj;
		}

		tools_array.push_back(tool_def);
	}

	if (!tools_array.empty())
		requestJson["tools"] = tools_array;
}

LlmToolType CLlmTools::GetToolTypeByName(const std::string& name)
{
	for (const auto& pair : _tools)
	{
		if (pair.second.name == name)
		{
			return pair.first;
		}
	}
	return LlmToolType::None;
}

const char* CLlmTools::GetToolTypeName(LlmToolType tp)
{
	auto it = _tools.find(tp);
	if (it != _tools.end())
	{
		return it->second.name.c_str();
	}
	return "";
}


const CLlmTools::ToolDefinition* CLlmTools::GetToolDefinition(LlmToolType tp)
{
	auto it = _tools.find(tp);
	if (it != _tools.end())
	{
		return &it->second;
	}
	return nullptr;
}

std::string CLlmTools::MakeToolCallResultString(const LlmToolCall& toolCall, const char* result)
{
	using json = nlohmann::json;
	
	const char* tool_name = GetToolTypeName(toolCall.tp);
	if (!tool_name)
		tool_name = "";

	// 构建工具调用的参数
	json arguments = json::object();
	for (const auto& param : toolCall.params_string)
	{
		arguments[param.first] = param.second;
	}
	for (const auto& param : toolCall.params_int)
	{
		arguments[param.first] = param.second;
	}

	// 处理 result 的编码问题
	std::string processed_result = result ? result : "";
	if (!Utils::is_valid_utf8(processed_result))
	{
		// 尝试转换为 UTF-8
		processed_result = local_to_utf8(processed_result);
		
		// 转换后再次检查
		if (!Utils::is_valid_utf8(processed_result))
		{
			processed_result = "invalid coding string";
		}
	}

	// OpenAI格式：包含工具调用和结果的完整消息数组
	json messages = json::array();
		
	// 添加工具调用消息
	json tool_call_message = json{
		{"role", "assistant"},
// 			{"content", nullptr},
//  			{"reasoning_content", "chain-of-thought reasoning"},
		{"tool_calls", json::array({
			json{
				{"id", toolCall.id},
				{"thoughtSignature", toolCall.thoughtSignature},
				{"type", "function"},
				{"function", json{
					{"name", tool_name},
					{"arguments", arguments.dump()}
				}}
			}
		})}
	};
	messages.push_back(tool_call_message);

	// 添加工具结果消息
	json tool_result_message = json{
		{"role", "tool"},
		{"tool_call_id", toolCall.id},
		{"name", tool_name},
		{"content", processed_result}
	};
	messages.push_back(tool_result_message);

// 		// 添加工具调用消息
// 		json tool_call_message = json{
// 			{"role", "assistant"},
// 			{"content", json::array({
// 				json{
// 					{"type", "tool_use"},
// 					{"id", toolCall.id},
// 					{"name", tool_name},
// 					{"input", arguments.dump()}
// 				}
// 			})}
// 		};
// 		messages.push_back(tool_call_message);
// 
// 		// 添加工具结果消息
// 		json tool_result_message = json{
// 			{"role", "user"},
// 			{"content", json::array({
// 				json{
// 					{"type", "tool_result"},
// 					{"tool_use_id", toolCall.id},
// 					{"content", result}
// 				}
// 			})}
// 		};
// 		messages.push_back(tool_result_message);

	// 添加异常保护，避免 dump 失败
	try
	{
		std::string s = messages.dump();
		return s;
	}
	catch (const std::exception&)
	{
		return "{}";
	}
}

bool CLlmTools::ParseToolCallResultString(const char* jsonString, LlmToolCall& toolCall, std::string& result)
{
	toolCall = {}; // Reset toolCall
	result.clear();

	using json = nlohmann::json;
	if (!json::accept(jsonString))
	{
		return false;
	}
	
	json data = json::parse(jsonString);

	// 尝试解析 OpenAI 格式 (消息数组)
	if (data.is_array() && !data.empty())
	{
		// 查找工具调用消息和工具结果消息
		json tool_call_msg, tool_result_msg;
		bool found_call = false, found_result = false;

		for (const auto& msg : data)
		{
			if (msg.contains("role"))
			{
				std::string role = msg.value("role", "");
				if (role == "assistant" && msg.contains("tool_calls"))
				{
					tool_call_msg = msg;
					found_call = true;
				}
				else if (role == "tool" && msg.contains("tool_call_id"))
				{
					tool_result_msg = msg;
					found_result = true;
				}
			}
		}

		if (found_call && found_result)
		{
			// 解析工具调用信息
			if (tool_call_msg["tool_calls"].is_array() && !tool_call_msg["tool_calls"].empty())
			{
				const auto& call = tool_call_msg["tool_calls"][0];
				toolCall.id = call.value("id", "");
					
				if (call.contains("function"))
				{
					const auto& func = call["function"];
					std::string tool_name = func.value("name", "");
					toolCall.tp = GetToolTypeByName(tool_name);
						
					// 解析参数
					if (func.contains("arguments"))
					{
						std::string args_str = func.value("arguments", "");
						toolCall.raw_arguments = args_str;
							
						if (json::accept(args_str))
						{
							json args = json::parse(args_str);
							const auto* toolDef = GetToolDefinition(toolCall.tp);
							if (toolDef)
							{
								for (const auto& el : args.items())
								{
									auto para_it = std::find_if(toolDef->params.begin(), toolDef->params.end(),
										[&el](const CLlmTools::ToolParam& p) { return p.name == el.key(); });
									if (para_it != toolDef->params.end())
									{
										if (para_it->type == "string" && el.value().is_string())
										{
											toolCall.params_string[el.key()] = el.value().get<std::string>();
										}
										else if (para_it->type == "integer" && el.value().is_number_integer())
										{
											toolCall.params_int[el.key()] = el.value().get<int>();
										}
									}
								}
							}
						}
						else
						{
							// 如果解析失败，尝试部分解析
							const auto* toolDef = GetToolDefinition(toolCall.tp);
							if (toolDef)
							{
								ExtractPartialArguments(args_str, *toolDef, toolCall);
							}
						}
					}
				}
			}

			// 解析工具结果
			if (tool_result_msg.contains("content") && tool_result_msg["content"].is_string())
			{
				result = tool_result_msg.value("content", "");
			}

			toolCall.isComplete = true;
			return true;
		}
	}
	
	return false;
}

bool CLlmTools::GetFilePathFromToolCallResultString(const char* jsonString, std::string& filePath)
{
	LlmToolCall toolCall;
	std::string result;

	if(false==ParseToolCallResultString(jsonString, toolCall, result))
		return false;

	if (toolCall.tp == LlmToolType::ReplaceInFile)
	{
		filePath = toolCall.params_string["filePath"];
		return true;
	}

	return false;
}

void CLlmTools::OmitFileContentInToolCallResultString(std::string& jsonString)
{
	LlmToolCall toolCall;
	std::string result;

	if (false == ParseToolCallResultString(jsonString.c_str(), toolCall, result))
		return;

	if (toolCall.tp == LlmToolType::ReplaceInFile)
	{
		toolCall.params_string["oldLines"]="[omitted...]";
		toolCall.params_string["newLines"] = "[omitted...]";

		jsonString = MakeToolCallResultString(toolCall, result.c_str());
	}
}


CLlmTools g_llmTools;
