#include "stdh.h"

#include "LlmSession.h"
#include "LlmFormatter.h"
#include <curl/curl.h>
#include <sstream>
#include <map>
#include <mutex>
#include <thread>
#include <deque>

#include "Utils.h"
#include "LlmTools.h"
#include "utils_image.h"
#include <fstream>

extern const char* GetOpenedDBFolderPath();

bool IsPrompCachingEnabled()
{
	return true;//XXXXX
}


std::deque<std::string> g_requests;

// 保存最近的请求到文件
void SaveRecentRequests(const std::string& filename = "recent_requests.txt")
{
	std::string path = GetOpenedDBFolderPath();
	path += "\\_log\\" + filename;
	std::ofstream outFile(path.c_str());
	if (outFile.is_open())
	{
		for (const auto& request : g_requests)
		{
			outFile << request << "\n\n";
		}
		outFile.close();
	}
}

std::deque<std::string> g_receives;

// 保存最近的请求到文件
void SaveRecentReceives(const std::string& filename = "recent_receives.txt")
{
	std::string path = GetOpenedDBFolderPath();
	path += "\\_log\\" + filename;
	std::ofstream outFile(path.c_str());
	if (outFile.is_open())
	{
		for (const auto& receive : g_receives)
		{
			outFile << receive << "\n\n";
		}
		outFile.close();
	}
}



// 定义 LLM 返回消息的数据结构
struct LlmMessage
{
	std::string role;
	std::string content;
	std::string reasoning;
	std::string finish_reason;
	bool is_delta = false;  // 标记是否是增量数据
};


// 定义 LLM 返回的数据结构
struct LlmResponse
{
	std::string id;
	std::string object;
	int64_t created = 0;
	std::string model;
	std::vector<LlmMessage> choices;
	// toolCalls are now handled by the parser statefully.
	struct
	{
		int prompt_tokens_ = 0;
		int prompt_tokens_equivalent = 0;
		int completion_tokens = 0;
		int total_tokens = 0;
		float cost = 0.0f;
	} usage;
	std::string error_message;
	std::string error_type;
	int error_code = 0;
	bool is_streaming = false;  // 标记是否是流式响应
	bool is_finished = false;   // 标记流式响应是否结束
};

//////////////////////////////////////////////////////////////////////////
//LlmSessionRequest

void LlmSessionRequest::_ProcessReasoning(const LlmSessionSetting& setting)
{
	// 检查是否是Kimi（Moonshot AI）
	bool isKimi = (setting.apiFormat == LlmApiFormat::Kimi);
	bool isDeepSeek = (setting.apiFormat == LlmApiFormat::DeepSeek);

	// 需要处理的情况：reasoning后面紧跟一个tool call
	for (size_t i = 0; i < entries.size(); ++i)
	{
		if (entries[i].tp != Entry::Reasoning)
			continue;

		std::string reasoningContent = entries[i].content;
		size_t nextIndex = i + 1;

		if (nextIndex < entries.size() && entries[nextIndex].tp == Entry::ToolCallAndResult)
		{
			if (isKimi||isDeepSeek)
			{
				// Kimi方式：将reasoning_content添加到tool call的JSON中
				try
				{
					json toolCallJson = json::parse(entries[nextIndex].content);
					if (toolCallJson.is_array() && !toolCallJson.empty() && toolCallJson[0].is_object())
					{
						// 添加到第一个元素的reasoning_content字段
						toolCallJson[0]["reasoning_content"] = reasoningContent;
						entries[nextIndex].content = toolCallJson.dump();
					}
				}
				catch (const json::parse_error&)
				{
					// 解析失败，不添加
				}
			}
			else
			{
				// 默认方式：将reasoning转化为assistant message
				std::string newContent = "[thinking...]" + reasoningContent;

				// 检查前面是否是assistant message
				if (i > 0 && entries[i - 1].tp == Entry::Assistant)
				{
					// 合并到前一个assistant message
					entries[i - 1].content += "\n"+newContent;
				}
				else
				{
					// 将当前reasoning转化为assistant message
					entries[i].tp = Entry::Assistant;
					entries[i].content = newContent;
					// 不删除，只改变类型
					continue;
				}
			}
		}
		// 标记为删除（转为None类型）
		entries[i].tp = Entry::None;
	}

	// 移除所有None类型的entries（即原来的reasoning，已处理的）
	entries.erase(
		std::remove_if(entries.begin(), entries.end(),
			[](const Entry& e) { return e.tp == Entry::None; }),
		entries.end()
	);

	// 如果是Kimi，检查所有tool call message，确保reasoning_content字段被填了一个值
	if (isKimi)
	{
		for (size_t i = 0; i < entries.size(); ++i)
		{
			if (entries[i].tp != Entry::ToolCallAndResult)
				continue;

			try
			{
				json toolCallJson = json::parse(entries[i].content);
				if (toolCallJson.is_array() && !toolCallJson.empty() && toolCallJson[0].is_object())
				{
					// 检查第一个元素是否有reasoning_content字段
					if (!toolCallJson[0].contains("reasoning_content"))
					{
						// 如果没有，填入"..."
						toolCallJson[0]["reasoning_content"] = "...";
						entries[i].content = toolCallJson.dump();
					}
				}
			}
			catch (const json::parse_error&)
			{
				// 解析失败，跳过
			}
		}
	}
}


void LlmSessionRequest::CommitMessages(json& messages, const LlmSessionSetting& setting)
{
	_ProcessReasoning(setting);

	for (int i = 0;i < entries.size();i++)
	{
		switch(entries[i].tp)
		{
			case LlmSessionRequest::Entry::User:
				CommitUserMessage(messages,entries[i].content.c_str(),setting, entries[i].mimeType.c_str());
				break;	
			case LlmSessionRequest::Entry::Assistant:
				CommitAssistMessage(messages,entries[i].content.c_str(),setting);
				break;
			case LlmSessionRequest::Entry::System:
				CommitSystemMessage(messages,entries[i].content.c_str(),setting);
				break;
			case LlmSessionRequest::Entry::ToolCallAndResult:
				CommitToolCallResult(messages,entries[i].content.c_str(),setting);
				break;
		}
		if(entries[i].cacheControl)
		{
			CommitCacheControl(messages,setting);
		}
	}
}

static void CommitMessage(json& messages, const char* role, const char* content, const LlmSessionSetting& setting)
{
	if (setting.api.providerTypeName == "DeepSeek")
//		(setting.api.providerTypeName == "Moonshot AI") )
// 		(setting.api.providerTypeName == "GLM"))
	{//deepseek不支持复杂类型的message
		json message;
		message["role"] = role;
		message["content"] = content;
		messages.push_back(message);
	}
	else
	{
		// 检查最后一个消息是否可以合并
		if (!messages.empty() && messages.back().is_object() && messages.back().contains("role") && messages.back()["role"] == role && messages.back().contains("content") && messages.back()["content"].is_array())
		{
			// 添加一个新的 text content part
			json content_part;
			content_part["type"] = "text";
			content_part["text"] = content;
			messages.back()["content"].push_back(content_part);
		}
		else
		{
			// 创建一个新消息
			json message;
			message["role"] = role;
			message["content"] = json::array();

			json content_part;
			content_part["type"] = "text";
			content_part["text"] = content;
			message["content"].push_back(content_part);

			messages.push_back(message);
		}
	}
}

void LlmSessionRequest::CommitUserMessage(json& messages,const char* str,const LlmSessionSetting& setting, const char* mimeType)
{
	if (mimeType && *mimeType != '\0')
	{
		// 带图片的情况
		CommitImageMessage(messages, str, mimeType, setting);
	}
	else
	{
		// 原有纯文本逻辑
		CommitMessage(messages, "user", str, setting);
	}
}

void LlmSessionRequest::CommitImageMessage(json& messages, const char* base64Data, const char* mimeType, const LlmSessionSetting& setting)
{
	// DeepSeek 不支持图片，降级为纯文本提示
	if (setting.api.providerTypeName == "DeepSeek")
//		(setting.api.providerTypeName == "Moonshot AI") )
	{
		CommitMessage(messages, "user", "[图片内容]", setting);
		return;
	}

	// 检查最后一个消息是否为 user 且可以合并
	if (!messages.empty() && messages.back().is_object() && 
		messages.back().contains("role") && messages.back()["role"] == "user" &&
		messages.back().contains("content") && messages.back()["content"].is_array())
	{
		// 合并到最后一个 user 消息
		json image_part;
		image_part["type"] = "image_url";
		image_part["image_url"]["url"] = std::string("data:") + mimeType + ";base64," + base64Data;
		messages.back()["content"].push_back(image_part);
	}
	else
	{
		// 创建新的 user 消息
		json message;
		message["role"] = "user";
		message["content"] = json::array();

		// 图片部分
		json image_part;
		image_part["type"] = "image_url";
		image_part["image_url"]["url"] = std::string("data:") + mimeType + ";base64," + base64Data;
		message["content"].push_back(image_part);

		messages.push_back(message);
	}
}

void LlmSessionRequest::AddUserMessageOfImage(const char* str, const char* mimeType)
{
	Entry e;
	e.tp = Entry::User;
	e.content = str;
	e.mimeType = mimeType ? mimeType : "";
	entries.push_back(e);
}

void LlmSessionRequest::CommitAssistMessage(json& messages,const char* str,const LlmSessionSetting& setting)
{
	CommitMessage(messages, "assistant", str, setting);
}

void LlmSessionRequest::CommitSystemMessage(json& messages,const char* str,const LlmSessionSetting& setting)
{
	CommitMessage(messages, "system", str, setting);
}

void LlmSessionRequest::CommitToolCallResult(json& messages, const char* jsonStr, const LlmSessionSetting& setting)
{
	try
	{
		json parsedJson = json::parse(jsonStr);
		if (!parsedJson.is_array() || parsedJson.empty())
			return;

		if ((setting.api.providerTypeName == "DeepSeek")|| (setting.api.providerTypeName == "Moonshot AI"))
		{//deepseek不支持复杂类型的message
			messages.insert(messages.end(), parsedJson.begin(), parsedJson.end());
			return;
		}

		const json& assistant_msg = parsedJson[0];
		if (!assistant_msg.is_object() || !assistant_msg.contains("role") || assistant_msg["role"] != "assistant" || !assistant_msg.contains("tool_calls"))
		{
			// 如果格式不符合预期，直接追加所有消息
			messages.insert(messages.end(), parsedJson.begin(), parsedJson.end());
			return;
		}

		// 检查最后一个消息是否为 assistant 消息,且不是一个tool call
		if (!messages.empty() && messages.back().is_object() &&
			messages.back().contains("role") && messages.back()["role"] == "assistant" &&
			messages.back().contains("content") && !messages.back()["content"].is_null() &&
			!messages.back().contains("tool_calls"))
		{
			//我们要保证两个assistant不连续,所以额外添加一个user 消息
			if (setting.apiFormat != LlmApiFormat::Anthropic_)
				CommitUserMessage(messages, "ok", setting);
			messages.insert(messages.end(), parsedJson.begin(), parsedJson.end());
			return;
		}

		// 检查最后一个消息是否为 assistant 角色，且是一个tool call,以便合并
		if (!messages.empty() && messages.back().is_object() && messages.back().contains("role") && messages.back()["role"] == "assistant")
		{
			json& lastMessage = messages.back();

			// 确保 tool_calls 存在且为数组
			if (!lastMessage.contains("tool_calls") || !lastMessage["tool_calls"].is_array())
			{
				lastMessage["tool_calls"] = json::array();
			}

			// 合并 tool_calls
			const auto& new_tool_calls = assistant_msg["tool_calls"];
			if (new_tool_calls.is_array())
			{
				lastMessage["tool_calls"].insert(lastMessage["tool_calls"].end(), new_tool_calls.begin(), new_tool_calls.end());
			}
			
			// 根据OpenAI规范，如果存在tool_calls，content应为null
			lastMessage["content"] = nullptr;

			// 追加其余消息（即 tool result）
			if (parsedJson.size() > 1)
			{
				messages.insert(messages.end(), parsedJson.begin() + 1, parsedJson.end());
			}
		}
		else
		{
			// 如果最后一个消息不是 assistant，则直接追加
			messages.insert(messages.end(), parsedJson.begin(), parsedJson.end());
		}
	}
	catch (const json::parse_error&)
	{
		// 解析失败则不执行任何操作
	}
}


void LlmSessionRequest::CommitCacheControl(json& messages,const LlmSessionSetting& setting)
{
	// 确保 messages 是数组格式
	if (!messages.is_array()) 
		return;
	
	if (messages.size() < 1) 
		return;

	if ((setting.apiCacheControlType != LlmApiCacheControlType::Anthropic_))
		return;

	size_t cacheIndex = messages.size() - 1;
	
	if (cacheIndex < messages.size() && messages[cacheIndex].is_object())
	{
		if (messages[cacheIndex].contains("tool_call_id"))//a tool call result
		{
			json cache_control_obj;
			cache_control_obj["type"] = "ephemeral";
			messages[cacheIndex]["cache_control"] = cache_control_obj;
		}
		else
		{
			auto& content = messages[cacheIndex]["content"];
			if (content.is_array() && !content.empty())
				content[content.size() - 1]["cache_control"]["type"] = "ephemeral";
		}
	}
}
  

//////////////////////////////////////////////////////////////////////////
//CLlmSession

std::vector<std::string> g_responses;
// 解析 LLM 返回的 JSON 数据
void parseLlmResponse(const char* response, CLlmToolCallParser &toolCallParser, LlmResponse& result)
{
//	g_responses.push_back(std::string(response));
	try
	{
		json j = json::parse(response);

		// 清空之前的增量内容，除非是流式响应的中间块
		if (!result.is_streaming)
		{
			result.choices.clear();
		}

		// 检查是否是错误响应
		if (j.contains(u8"error"))
		{
			result.error_message = j[u8"error"].value(u8"message", u8"");
			result.error_type = j[u8"error"].value(u8"type", "");
			// 处理 code 可能是整数或字符串的情况
			if (j[u8"error"].contains(u8"code"))
			{
				const auto& codeVal = j[u8"error"][u8"code"];
				if (codeVal.is_string())
				{
					// 如果是字符串，尝试转换为整数
					try
					{
						result.error_code = std::stoi(codeVal.get<std::string>());
					}
					catch (...)
					{
						result.error_code = 0; // 转换失败时保持默认值
					}
				}
				else if (codeVal.is_number())
				{
					result.error_code = codeVal.get<int>();
				}
			}
			if (j[u8"error"].contains(u8"metadata"))
			{
				if (j[u8"error"][u8"metadata"].contains(u8"raw"))
					result.error_message = j[u8"error"][u8"metadata"].value(u8"raw", u8"");
			}
			return;
		}

		// 更新基本字段
		result.id = j.value(u8"id", result.id);
		result.object = j.value(u8"object", result.object);
		result.created = j.value(u8"created", result.created);
		result.model = j.value(u8"model", result.model);

		// 检查是否是流式响应
		result.is_streaming = (result.object.find(u8"chat.completion.chunk") != std::string::npos);

		// 将json块送入解析器进行状态更新
		toolCallParser.Update(j);

		// 解析 choices 数组
		if (j.contains(u8"choices"))
		{
			for (const auto& choice : j[u8"choices"])
			{
				LlmMessage message;
				message.is_delta = false;

				// 处理标准消息或增量消息
				if (choice.contains(u8"message"))
				{
					message.role = choice[u8"message"].value(u8"role", u8"");
					message.content = choice[u8"message"].value(u8"content", "");
				}
				else if (choice.contains(u8"delta"))
				{
					message.is_delta = true;
					message.role = choice[u8"delta"].value(u8"role", u8"assistant");
					if (choice[u8"delta"].contains(u8"content") && !choice[u8"delta"][u8"content"].is_null())
					{
						message.content = choice[u8"delta"][u8"content"].get<std::string>();
					}
					else
					{
						message.content = u8"";
					}
					if (choice[u8"delta"].contains(u8"reasoning") && !choice[u8"delta"][u8"reasoning"].is_null())
					{
						message.reasoning = choice[u8"delta"][u8"reasoning"].get<std::string>();
					}
					else
					{
						if (choice[u8"delta"].contains(u8"reasoning_content") && !choice[u8"delta"][u8"reasoning_content"].is_null())
						{
							message.reasoning = choice[u8"delta"][u8"reasoning_content"].get<std::string>();
						}
						else
							message.reasoning = u8"";
					}

				}

				// 检查 finish_reason 是否存在且为字符串
				if (choice.contains(u8"finish_reason") && choice[u8"finish_reason"].is_string())
				{
					message.finish_reason = choice[u8"finish_reason"].get<std::string>();
				}
				else
				{
					// 如果 finish_reason 不存在、为 null 或不是字符串，则保持为空
					// message.finish_reason 在 LlmMessage 构造时已初始化为空
				}

				if (choice.contains(u8"usage"))
				{
					// 检查usage是否为null
					if (!choice[u8"usage"].is_null())
					{
						result.usage.prompt_tokens_ = choice[u8"usage"].value(u8"prompt_tokens", result.usage.prompt_tokens_);
						result.usage.prompt_tokens_equivalent = choice[u8"usage"].value(u8"prompt_tokens_equivalent", result.usage.prompt_tokens_);
						result.usage.completion_tokens = choice[u8"usage"].value(u8"completion_tokens", result.usage.completion_tokens);
						result.usage.total_tokens = choice[u8"usage"].value(u8"total_tokens", result.usage.total_tokens);
						result.usage.cost = choice[u8"usage"].value(u8"cost", result.usage.cost);
					}
				}

				// 检查流式响应是否结束 (基于有效的 finish_reason)
				if (!message.finish_reason.empty())
				{
					result.is_finished = true;
				}

				// 对于流式响应，更新或添加消息
				if (result.is_streaming)
				{
					if (choice.value(u8"index", 0) < result.choices.size())
					{
						// 更新现有消息
						auto& existing = result.choices[choice.value(u8"index", 0)];
							existing.content += message.content;
						existing.finish_reason = message.finish_reason;
					}
					else
					{
						// 添加新消息
						result.choices.push_back(message);
					}
				}
				else
				{
					// 非流式响应直接添加
					result.choices.push_back(message);
				}
			}
		}

		// 解析 usage 数据
		if (j.contains(u8"usage"))
		{
			// 检查usage是否为null
			if (!j[u8"usage"].is_null())
			{
				result.usage.prompt_tokens_ = j[u8"usage"].value(u8"prompt_tokens", result.usage.prompt_tokens_);
				result.usage.prompt_tokens_equivalent = j[u8"usage"].value(u8"prompt_tokens_equivalent", result.usage.prompt_tokens_equivalent);
				result.usage.completion_tokens = j[u8"usage"].value(u8"completion_tokens", result.usage.completion_tokens);
				result.usage.total_tokens = j[u8"usage"].value(u8"total_tokens", result.usage.total_tokens);
				result.usage.cost = j[u8"usage"].value(u8"cost", result.usage.cost);
			}
			// 如果usage为null，保持默认值
		}

	}
	catch (const json::parse_error& e)
	{
		result.error_message = "JSON parse error: " + std::string(e.what());
		result.error_type = "parse_error";
		result.error_code = -1;
	}
	catch (const std::exception& e)
	{
		result.error_message = "Error: " + std::string(e.what());
		result.error_type = "unknown_error";
		result.error_code = -2;
	}
}

void ParseLine(std::string& line, CLlmSession* session)
{
	// 移除可能存在的前导 "data: "
	if (line.rfind("data: ", 0) == 0)
	{
		line = line.substr(6);
	}

	// 去除首尾空格
	line.erase(0, line.find_first_not_of(" \t\r\n"));
	line.erase(line.find_last_not_of(" \t\r\n") + 1);

	// 跳过空行
	if (line.empty()) 
		return;

	// 检查流结束标记
	if (line == "[DONE]")
	{
		// 可以在这里设置一个标志，表示流已正常结束，
		// 但通常 curl_easy_perform 的完成就表示结束了。
		// m_isCompleted 会在 RequestThreadFunction 结尾设置。
		return;
	}

	if (line == ": OPENROUTER PROCESSING")
		return;

	// 解析 JSON 数据块
	LlmResponse response;
	parseLlmResponse(line.c_str(), session->m_toolCallParser,response);

	// 处理解析结果
	if (!response.error_message.empty())
	{
		// 记录或处理解析错误
		session->m_hasError = true; // 可能需要更精细的错误处理
		session->m_errorMessage = response.error_message;
	}
	else if (!response.choices.empty())
	{
		for (int i = 0;i < response.choices.size();i++)
		{
			session->m_deltaAnswer += response.choices[i].content;
			session->m_answer += response.choices[i].content;

			session->m_deltaReasoning += response.choices[i].reasoning;
			session->m_reasoning += response.choices[i].reasoning;

		}
	}
	session->m_usage.inputToken_ += response.usage.prompt_tokens_;
	session->m_usage.inputToken_equivalent += response.usage.prompt_tokens_equivalent;
	session->m_usage.outputToken += response.usage.completion_tokens;
	session->m_usage.fee += response.usage.cost;
}

void ParseRawLine(std::string& line, CLlmSession* session)
{
	if (session->m_settings.apiFormat == LlmApiFormat::Anthropic_)
	{
		session->m_bufferLines.push_back(line);
		std::vector<std::string> convertedLines;
		CLlmFormatter::ProcessLlmResponseFromAnthropicFormat(session->m_bufferLines, convertedLines, session->m_settings.api);
		for (int i = 0;i < convertedLines.size();i++)
			ParseLine(convertedLines[i],session);
	}
	else if (session->m_settings.apiFormat == LlmApiFormat::Gemini_)
	{
		session->m_bufferLines.push_back(line);
		std::vector<std::string> convertedLines;
		CLlmFormatter::ProcessLlmResponseFromGeminiFormat(session->m_bufferLines, convertedLines, session->m_settings.api);
		for (int i = 0;i < convertedLines.size();i++)
			ParseLine(convertedLines[i],session);
	}
	else if (session->m_settings.apiFormat == LlmApiFormat::OpenAI_ ||
	         session->m_settings.apiFormat == LlmApiFormat::Kimi ||
	         session->m_settings.apiFormat == LlmApiFormat::GLM ||
	         session->m_settings.apiFormat == LlmApiFormat::DeepSeek)
	{
		session->m_bufferLines.push_back(line);
		std::vector<std::string> convertedLines;
		CLlmFormatter::ProcessLlmResponseFromOpenAiCompatibleFormat(session->m_bufferLines, convertedLines, session->m_settings.api);
		for (int i = 0;i < convertedLines.size();i++)
			ParseLine(convertedLines[i],session);
	}
	else
		ParseLine(line,session);
}

// 回调函数用于处理HTTP响应
size_t LlmWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    size_t realsize = size * nmemb;
    CLlmSession* session = static_cast<CLlmSession*>(userp);
    
    if (!session)
    {
        return 0;  // 出错
    }

	std::lock_guard<std::mutex> lock(session->m_mutex);
    
    // 将新数据块附加到缓冲区
	std::string t;
	t.append(static_cast<char*>(contents), realsize);

	session->m_buffer += t;

	g_receives.push_back(t);

	SaveRecentReceives();

    // 处理缓冲区中的完整行
    size_t pos = 0;
    std::string line;
    while ((pos = session->m_buffer.find('\n')) != std::string::npos)
    {
		line = session->m_buffer.substr(0, pos);
		session->m_buffer.erase(0, pos + 1);

		ParseRawLine(line, session);
    }
    
    return realsize;
}

//////////////////////////
// CLlmSession 实现
//////////////////////////

CLlmSession::CLlmSession(const LlmSessionSetting& settings)
	: m_settings(settings),
	m_isCompleted(false),
	m_hasError(false),
	m_interruptRequested(false)
{
	m_toolCallParser.Reset(settings.apiFormat);
}

CLlmSession::~CLlmSession()
{
}

bool CLlmSession::Request(const LlmSessionRequest& request)
{
    m_request = request;
    m_isCompleted = false;
    m_hasError = false;
    m_errorMessage = "";
    m_answer = "";
    m_reasoning = "";
    m_deltaAnswer = "";
    m_deltaReasoning = "";
    m_buffer = "";
    m_interruptRequested = false;


    // 启动请求线程
    std::thread requestThread(RequestThreadFunction, this);
    requestThread.detach();
    
    return true;
}

bool CLlmSession::IsCompleted()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_isCompleted;
}

bool CLlmSession::GetAnswer(std::string& answer)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    answer = m_answer;
    return !m_answer.empty();
}

bool CLlmSession::GetTokenUsage(LlmSessionUsage& usage)
{
	usage.Zero();
	if (!m_isCompleted)
		return false;
	usage = m_usage;
	return true;
}


bool CLlmSession::FetchDeltaAnswer(std::string& delta, std::string& deltaReasoning)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	delta= m_deltaAnswer;
	deltaReasoning = m_deltaReasoning;
	m_deltaAnswer = "";
	m_deltaReasoning = "";
	return !delta.empty();
}

//取得发生过更新的Tool call(如果这个tool call没有更新,比如已经完整了,就不会返回)
bool CLlmSession::GetUpdatedToolCalls(std::vector<LlmToolCall>& toolCalls)
{
	toolCalls.clear();

	std::lock_guard<std::mutex> lock(m_mutex);
	m_toolCallParser.GetIncompleteResult(toolCalls);
	m_toolCallParser.FetchCompleteResult(toolCalls);
	return !toolCalls.empty();
}


bool CLlmSession::HasError()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_hasError;
}

std::string CLlmSession::GetErrorMessage()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_errorMessage;
}

void CLlmSession::Interrupt()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_interruptRequested = true;
}

void CLlmSession::Process()
{
    // 仅处理一些轮询操作，目前没有需要处理的内容
}

// cURL 进度回调函数
static int LlmProgressCallback(void *clientp,   double dltotal,   double dlnow,   double ultotal,   double ulnow)
{
    CLlmSession* session = static_cast<CLlmSession*>(clientp);
    if (session && session->m_interruptRequested)
    {
        return 1; // 返回非零值中止 cURL 操作
    }
    return 0; // 继续操作
}

int g_instances = 0;
void CLlmSession::RequestThreadFunction(CLlmSession* session)
{
    if (!session)
    {
        return;
    }

    LlmSessionRequest& request = session->m_request;
    const LlmSessionSetting& settings = session->m_settings;
    
  
    // 初始化CURL
    CURL* curl = curl_easy_init();
    if (!curl)
    {
        std::lock_guard<std::mutex> lock(session->m_mutex);
        session->m_hasError = true;
        session->m_errorMessage = "Failed to initialize CURL";
        session->m_isCompleted = true;
        return;
    }

    // 设置请求参数
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

    // 添加API密钥
	std::string authHeader = "Authorization: Bearer " + settings.apiKey;
	if (settings.apiFormat==LlmApiFormat::Anthropic_)
		authHeader = "x-api-key: " + settings.apiKey;
	else if (settings.apiFormat == LlmApiFormat::Gemini_)
		authHeader = "x-goog-api-key: " + settings.apiKey;

	headers = curl_slist_append(headers, authHeader.c_str());

	if (settings.apiFormat == LlmApiFormat::Anthropic_)
		headers = curl_slist_append(headers, "anthropic-version: 2023-06-01");

    // 构建请求体JSON
	json requestJson;

    // 获取模型名称
    std::string modelName = !settings.api.model.empty() ? 
                           settings.api.model : "gpt-3.5-turbo";
    requestJson["model"] = modelName;

	// 添加 tools
	if (!settings.api.tools.empty())
	{
		g_llmTools.FillToolsJson(settings.api.tools, requestJson);
	}

	if (request.prompt.empty())
	{

		// 构建消息
		json messages = json::array();

		if (!settings.rulesFile.empty())
		{
			std::string context;
			std::string fileContent;
			Utils::FileContentCodingFormat codingFmt;
			if (Utils::GetFileContentIntoUTF8(settings.rulesFile.c_str(), fileContent,codingFmt))
			{
				context = u8"Please follow these rules when answering the questions:\n" + fileContent;
				CommitMessage(messages, "system", context.c_str(), settings);
				if (IsPrompCachingEnabled())
					LlmSessionRequest::CommitCacheControl(messages, settings);

				//  			if (!IsPrompCachingEnabled())
				// 				messages.push_back({ {"role", "system"}, {"content", context} });
				// 			else
				// 			{
				// 				json cache_control_obj;
				// 				cache_control_obj["type"] = "ephemeral";
				// 
				// 				messages.push_back({ {"role", "system"}, {"content", context}, {"cache_control",cache_control_obj } });
				// 			}
			}
		}

		request.CommitMessages(messages, settings);

		requestJson["messages"] = messages;
	}
	else
	{
		requestJson["prompt"] = request.prompt;
	}

	// 处理 thinkingMode 设置
	if (settings.api.thinkingMode == LlmThinkingMode::Disable)
	{
		if (settings.api.providerTypeName == "OpenRouter")
		{
			requestJson["reasoning"]["effort"] = "low";
		}
		else
		{
			requestJson["thinking"] = { {"type", "disabled"} };
		}
	}
	else if (settings.api.thinkingMode == LlmThinkingMode::Enable)
	{
		if (settings.api.providerTypeName == "OpenRouter")
		{
			requestJson["reasoning"]["effort"] = "high";
		}
		else
		{
			requestJson["thinking"] = { {"type", "enabled"}, {"budget_tokens", 16000} };
		}
	}
	// LlmThinkingMode::Auto 时不进行特殊处理，使用 API 默认行为

	if (settings.api.providerTypeName == "OpenRouter")
	{
		if (settings.api.purpose[0] == LlmApiPurpose::Complete)
		{
			requestJson["provider"]["sort"] = "latency";
//			requestJson["provider"]["quantizations"] = { "fp8","fp16","fp32","unknown"};
		}
		if (settings.api.openRouterOptions.only.size() > 0)
		{
			requestJson["provider"]["only"] = settings.api.openRouterOptions.only;
		}
	}

// 	if (settings.api.providerTypeName!="Ubisoft LiteLLM")
// 		requestJson["usage"] = {	{"include", true}	};

    // 添加 stream 参数
	if (request.isStreaming)
	{
		requestJson["stream"] = true;
		if (settings.api.providerTypeName == "Aliyun")
			requestJson["stream_options"] = { {"include_usage", true} };
	}
	else
		requestJson["stream"] = false;

	// 添加 temperature 参数
    requestJson["temperature"] = 1.0;
// 	requestJson["top_k"] = 40.0f;
// 	requestJson["top_p"] = 0.95f;
// 	requestJson["min_p"] = 0.05f;
// 	requestJson["repetition_penalty"] = 1.1f;
//	requestJson["stop"] = { "<|editable_region_end|>" };

    // 添加 max_tokens 参数
	if (settings.api.maxToken > 0)
		requestJson["max_tokens"] = settings.api.maxToken;

 	std::string requestBodyOpenAI = requestJson.dump();

	if (settings.apiFormat==LlmApiFormat::Anthropic_)
		CLlmFormatter::ConvertLlmRequestToAnthoropicFormat(requestJson);
	else if (settings.apiFormat==LlmApiFormat::Gemini_)
		CLlmFormatter::ConvertLlmRequestToGeminiFormat(requestJson);
	else 
		CLlmFormatter::ConvertLlmRequestToOpenAiCompatibleFormat(requestJson, settings.apiFormat);

    std::string requestBody = requestJson.dump();

	g_requests.push_back(requestBody);

	SaveRecentRequests();

    // 获取API端点
    std::string apiEndpoint = settings.apiEndpoint_.empty() ?
                             "https://api.openai.com/v1/chat/completions" :
                             settings.apiEndpoint_;
    
	// Gemini 需要特殊处理：将 API key 添加到 URL 参数中
	if (settings.apiFormat == LlmApiFormat::Gemini_)
	{
		// 如果 endpoint 中没有包含 model，需要添加
		// 格式: https://api.ofox.ai/gemini/v1beta/models/{model}:generateContent
		
		// 检查是否已经包含 :generateContent 或 :streamGenerateContent
		if (apiEndpoint.find(":generateContent") == std::string::npos && 
		    apiEndpoint.find(":streamGenerateContent") == std::string::npos)
		{
			// 需要添加模型名称和方法
			std::string modelName = !settings.api.model.empty() ? settings.api.model : "gemini-pro";
			
			// 如果 endpoint 以 / 结尾，去掉
			if (!apiEndpoint.empty() && apiEndpoint.back() == '/')
				apiEndpoint.pop_back();
			
			// 如果 endpoint 不包含 /models/，添加完整路径
			if (apiEndpoint.find("/models/") == std::string::npos)
			{
				apiEndpoint += "/models/" + modelName;
			}
			
			// 添加方法（流式或非流式）
			if (request.isStreaming)
				apiEndpoint += ":streamGenerateContent";
			else
				apiEndpoint += ":generateContent";
		}
		
		// 如果是流式请求，添加 alt=sse 参数
// 		if (request.isStreaming)
// 		{
// 			apiEndpoint += "&alt=sse";
// 		}
	}
    
    // 设置CURL选项
    curl_easy_setopt(curl, CURLOPT_URL, apiEndpoint.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
//	curl_easy_setopt(curl, CURLOPT_PROXY, "socks5h://127.0.0.1:1080");
//	curl_easy_setopt(curl, CURLOPT_PROXY, "http://127.0.0.1:8080");
//	curl_easy_setopt(curl, CURLOPT_PROXY, "socks5h://127.0.0.1:7890");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, LlmWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, session); // 传递session指针给LlmWriteCallback
    
    // 设置进度回调
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, LlmProgressCallback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, session);

    // 设置超时
    if (settings.timeoutSeconds > 0)
    {
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, settings.timeoutSeconds);
    }
    
    // 发送请求
    CURLcode res = curl_easy_perform(curl);

	if (!session->m_buffer.empty())
	{
		std::string line = std::move(session->m_buffer);
		ParseRawLine(line, session);
	}

	if (session->m_usage.inputToken_ <= 0)
	{
		int v = 0;
		v++;
	}
    
    // 处理结果
    {
        std::lock_guard<std::mutex> lock(session->m_mutex);
        
        if (res != CURLE_OK)
        {
            // 处理错误
            session->m_hasError = true;
            if (res == CURLE_ABORTED_BY_CALLBACK)
            {
                session->m_errorMessage = "Request stopped by user.";
            }
            else
            {
                session->m_errorMessage = curl_easy_strerror(res);
            }
        }

		g_instances--;

        // 标记为完成
        session->m_isCompleted = true;
    }
    
    // 清理
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

}


