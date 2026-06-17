#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <string>
#include <algorithm>
#include <map>
#include <unordered_set>
#include <mutex>
#include <memory>
#include <atomic>

#include "stringparser/stringparser.h"

#include "LlmLib.h"
#include "LlmTools.h"

typedef int AiChatQuestionID;

struct LlmSessionSetting
{
	bool IsValid()
	{
		return !apiEndpoint_.empty();
	}
	bool ExistRuleFiles() const
	{
		for (const auto& file : rulesFiles)
		{
			if (!file.empty())
				return true;
		}
		return false;
	}

	LlmApi api;
	std::string apiKey;      // API密钥
	std::string apiEndpoint_; // API端点，如"https://api.openai.com/v1/chat/completions"
	LlmApiFormat apiFormat;
	LlmApiCacheControlType apiCacheControlType;
	std::vector<std::string> rulesFiles;
	std::shared_ptr<const std::string> skillsDump;  // 预先 dump 好的 skills 字符串，由外部传入
	int timeoutSeconds;      // 请求超时时间（秒）

	LlmSessionSetting() : timeoutSeconds(3000)
	{}
};


struct LlmSessionRequest
{
	LlmSessionRequest()
	{
		isStreaming = true;
	}

	struct Entry
	{
		Entry()
		{
			tp = None;
			cacheControl = false;
		}
		enum Type
		{
			None,
			System,
			User,
			Assistant,
			ToolCallAndResult,
			Reasoning,
		};
		Type tp;
		std::string content;
		std::string reasoningContent;//只在tp为Assistant时有效
		bool cacheControl;
		std::string mimeType;    // 非空表示 content 是 base64 图片数据
	};

	void Clear() { entries.clear();	prompt.clear();isStreaming = true; }

	void CommitMessages(json& messages, const LlmSessionSetting& setting);

	void AddUserMessage(const char* str)	{		_Add(Entry::User, str);	}
	void AddUserMessageOfImage(const char* str, const char* mimeType);
	void AddAssistMessage(const char* str) { _Add(Entry::Assistant, str); }
	void AddSystemMessage(const char* str) { _Add(Entry::System, str); }
	void AddReasoningMessage(const char* str) { _Add(Entry::Reasoning, str); }
	void AddToolCallResult(const char* jsonStr) { _Add(Entry::ToolCallAndResult, jsonStr); }//str为OpenAI 格式的tool call json字串
	void AddCacheControl()
	{
		if (entries.size() > 0)
			entries[entries.size() - 1].cacheControl = true;
	}
	void SetPrompt(const char* str)	{		prompt = str;	}

	void _Add(Entry::Type tp, const char*str)
	{
		Entry e;
		e.tp = tp;
		e.content = str;
		entries.push_back(e);
	}

	void _ProcessReasoning(const LlmSessionSetting& setting);
	void _ProcessReasoning2(const LlmSessionSetting& setting);

	static void CommitUserMessage(json& messages, const char* str, const LlmSessionSetting& setting, const char* mimeType = nullptr);
	static void CommitImageMessage(json& messages, const char* base64Data, const char* mimeType, const LlmSessionSetting& setting);
	static void CommitAssistMessage(json& messages, const char* str, const char* reasoningContent, const LlmSessionSetting& setting);
	static void CommitSystemMessage(json& messages, const char* str, const LlmSessionSetting& setting);
	static void CommitToolCallResult(json& messages, const char* jsonStr, const LlmSessionSetting& setting);//jsonStr为OpenAI 格式的tool call json字串
	static void CommitCacheControl(json& messages, const LlmSessionSetting& setting);

	std::vector<Entry> entries;
	std::string prompt;
	bool isStreaming;
};

struct LlmSessionUsage
{
	LlmSessionUsage()
	{
		Zero();
	}
	void Zero()
	{
		fee = 0.0f;
		inputToken_ = 0;
		inputToken_CacheRead = 0;
		inputToken_CacheWrite = 0;
		outputToken = 0;
	}
	void Accumulate(const LlmSessionUsage &other)
	{
		fee += other.fee;
		inputToken_ += other.inputToken_;
		inputToken_CacheRead += other.inputToken_CacheRead;
		inputToken_CacheWrite += other.inputToken_CacheWrite;
		outputToken += other.outputToken;
	}

	// Session Cost 格式化函数（JSON格式）
	std::string FormatToCostText() const
	{
		json j;
		j["fee"] = fee;
		j["in"] = inputToken_;
		j["cr"] = inputToken_CacheRead;
		j["cw"] = inputToken_CacheWrite;
		j["out"] = outputToken;
		return j.dump();
	}

	// Session Cost 解析函数，返回是否为旧格式(Legacy)
	static std::pair<LlmSessionUsage, bool> ParseFromCostText(const std::string& costText)
	{
		LlmSessionUsage usage;
		bool isLegacy = false;

		if (costText.empty())
			return { usage, isLegacy };

		// 尝试 JSON 格式解析
		try
		{
			auto j = json::parse(costText);
			if (j.is_object())
			{
				usage.fee = j.value("fee", 0.0f);
				usage.inputToken_ = j.value("in", 0);
				usage.inputToken_CacheRead = j.value("cr", 0);
				usage.inputToken_CacheWrite = j.value("cw", 0);
				usage.outputToken = j.value("out", 0);
				return { usage, isLegacy };
			}
		}
		catch (...) {}

		// 旧格式兼容：$price(inputToken -> outputToken)
		isLegacy = true;
		size_t dollarPos = costText.find('$');
		size_t leftParenPos = costText.find('(');
		size_t arrowPos = costText.find(" -> ");
		size_t rightParenPos = costText.find(')');

		if (dollarPos == std::string::npos || leftParenPos == std::string::npos ||
			arrowPos == std::string::npos || rightParenPos == std::string::npos ||
			dollarPos >= leftParenPos || leftParenPos >= arrowPos || arrowPos >= rightParenPos)
		{
			return { usage, isLegacy };
		}

		try
		{
			std::string priceStr = costText.substr(dollarPos + 1, leftParenPos - dollarPos - 1);
			usage.fee = std::stof(priceStr);

			std::string inputTokenStr = costText.substr(leftParenPos + 1, arrowPos - leftParenPos - 1);
			usage.inputToken_ = std::stoi(inputTokenStr);

			std::string outputTokenStr = costText.substr(arrowPos + 4, rightParenPos - arrowPos - 4);
			usage.outputToken = std::stoi(outputTokenStr);
		}
		catch (...)
		{
			usage.Zero();
		}

		return { usage, isLegacy };
	}

	float fee;
	int inputToken_;//Uncached
	int inputToken_CacheRead;
	int inputToken_CacheWrite;
	int outputToken;
};

// 前向声明
typedef void CURL;

// LLM会话类
class CLlmSession
{
public:
	CLlmSession(const LlmSessionSetting& settings);
	~CLlmSession();
	
	// 发送请求
	bool Request(const LlmSessionRequest &request);
	
	// 停止当前请求
	void Interrupt();

	// 检查是否完成
	bool IsCompleted();
	
	// 获取回答
	bool GetAnswer(std::string& answer);
	bool FetchDeltaAnswer(std::string& delta,std::string &deltaReasoning);
	bool GetUpdatedToolCalls(std::vector<LlmToolCall>& toolCalls);
	bool GetTokenUsage(LlmSessionUsage &usage);

	// 获取错误信息
	bool HasError();
	std::string GetErrorMessage();
	
	// 处理函数
	void Process();
	
	// 互斥锁 - 公开，以便回调函数使用
	std::mutex m_mutex;
	
public:
	LlmSessionSetting m_settings;        // 设置
	LlmSessionRequest m_request;
	CLlmToolCallParser m_toolCallParser;

	std::string m_answer;            // 完整回答
	std::string m_deltaAnswer;            
	std::string m_reasoning;            // 完整回答
	std::string m_deltaReasoning;
	std::string m_errorMessage;      // 错误消息
	bool m_isCompleted;              // 是否完成
	bool m_hasError;                 // 是否有错误
	std::atomic<bool> m_interruptRequested; // 添加停止请求标志

	LlmSessionUsage m_usage;
	
	// 添加一个缓冲区来处理不完整的流数据块
	std::string m_buffer; 
	std::deque<std::string> m_bufferLines;

	// 发送请求到LLM
	static void RequestThreadFunction(CLlmSession* session);
	
};
