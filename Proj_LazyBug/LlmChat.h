#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <mutex>
#include <memory>

#include "stringparser/stringparser.h"

#include "LlmSession.h"

// 输出结构
struct LlmSessionOutput
{
	std::string fullContent;     // 完整内容
	std::string content;     // 内容
	std::string reasoning;     
	std::vector<LlmToolCall> updatedToolCalls;//发送过更新的tool calls
	bool isCompleted;        // 是否完成
	LlmSessionUsage usage;
	bool hasError;           // 是否有错误
	std::string errorMessage; // 错误消息
	
	LlmSessionOutput() 
		: isCompleted(false)
		, hasError(false)
	{}
};

// 前向声明
typedef void CURL;

class CLlmChat
{
public:
	CLlmChat();
	~CLlmChat();

	void Init();
	void Clear();

	// 请求问题，启动一个会话并立即返回
	bool Request(const LlmSessionRequest &request, const LlmSessionSetting&setting);

	// 处理当前会话，获取输出
	bool Process(LlmSessionOutput& output, bool interrupt = false);

	// 是否有活动的会话
	bool HasActiveSession() const;

private:
	std::unique_ptr<CLlmSession> m_activeSession;
	mutable std::mutex m_mutex;

	LlmSessionSetting m_setting;
	std::vector<std::unique_ptr<CLlmSession>> m_discardedSessions;
};
