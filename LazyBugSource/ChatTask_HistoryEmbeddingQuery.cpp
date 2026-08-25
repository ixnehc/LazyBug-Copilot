#include "stdh.h"
#include "ChatTask_HistoryEmbeddingQuery.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "InputHintContext.h"
#include "LlmSession.h"


CChatTask_HistoryEmbeddingQuery::CChatTask_HistoryEmbeddingQuery(const std::string& apiName)
{
	_apiName = apiName;
}

void CChatTask_HistoryEmbeddingQuery::_Fail(const std::string& reason)
{
	(void)reason;
	_status = TaskStatus::Failure;
}

void CChatTask_HistoryEmbeddingQuery::Start()
{
	if (!_context || !_context->inputHintCtx)
	{
		_Fail("No InputHintContext");
		return;
	}

	// 读取聊天上下文
	const std::string& chatOpsContent = _context->inputHintCtx->GetChatOpsContent();
	if (chatOpsContent.empty())
	{
		_Fail("Empty chatOpsContent");
		return;
	}

	// 加载 API 设置（不加载 chat rules）
	LlmSessionSetting setting;
	if (!g_llmLib.LoadLlmSetting(setting, _apiName, false, nullptr))
	{
		_Fail("Failed to load API settings");
		return;
	}

	setting.api.tools.clear();
	setting.api.thinkingMode = LlmThinkingMode::Disable;

	// 构造 system prompt: 指示 LLM 从聊天历史中提取检索 query
	const char* systemPrompt =
		"You are a code search query planner. Analyze the following chat history between a user and an AI assistant. "
		"Generate 2-5 search queries for finding relevant code in a C/C++ project codebase.\n\n"
		"Output JSON only (no markdown, no explanation):\n"
		"{\n"
		"  \"queries\": [\n"
		"    {\"text\": \"concise natural language query for semantic code search\", \"priority\": 1-10}\n"
		"  ],\n"
		"  \"identifiers\": [\"ClassName\", \"functionName\", \"variableName\"],\n"
		"  \"files\": [\"filename.cpp\", \"filename.h\"]\n"
		"}\n\n"
		"Rules:\n"
		"- Focus on the most recent topics in the conversation\n"
		"- Extract exact class names, function names, variable names, file paths mentioned in the chat\n"
		"- Each query should be a concise natural language description suitable for semantic code search\n"
		"- Only include identifiers that actually appear in the chat history\n"
		"- Output JSON only, no other text";

	// 构造 user message: 聊天历史
	std::string userMsg = "Chat history:\n";
	userMsg += chatOpsContent;

	LlmSessionRequest request;
	request.AddSystemMessage(systemPrompt);
	request.AddUserMessage(userMsg.c_str());
	request.isStreaming = true;
	request.allowMcpTools = false;

	if (!_llmChats[0]->Request(request, setting))
	{
		_Fail("Failed to send LLM request");
		return;
	}

	_hasStartedRequest = true;
	_phase = Phase::PlanQueries;
	_status = TaskStatus::Running;
}

void CChatTask_HistoryEmbeddingQuery::Update()
{
	if (_llmChats.empty())
		return;

	if (_phase != Phase::PlanQueries)
		return;

	if (!_llmChats[0]->HasActiveSession())
	{
		if (_hasStartedRequest)
			_Fail("LLM session ended unexpectedly");
		return;
	}

	LlmSessionOutput output;
	if (!_llmChats[0]->Process(output, _requestInterrupt))
		return;

	if (!output.isCompleted)
		return;

	if (_requestInterrupt)
	{
		_Fail("Interrupted");
		return;
	}

	if (output.hasError)
	{
		_Fail(output.errorMessage.empty() ? "LLM request failed" : output.errorMessage);
		return;
	}

	// 将 LLM 原始输出存入 InputHintContext
	std::string result = output.fullContent;

	// 去除首尾空白
	size_t start = result.find_first_not_of(" \t\r\n");
	size_t end = result.find_last_not_of(" \t\r\n");
	if (start != std::string::npos && end != std::string::npos)
		result = result.substr(start, end - start + 1);
	else
		result.clear();

	if (result.empty())
	{
		_Fail("Empty query plan response");
		return;
	}

	_context->inputHintCtx->SetHistoryQueryPlan(std::move(result));

	_phase = Phase::Done;
	_status = TaskStatus::Success;
}

void CChatTask_HistoryEmbeddingQuery::Interrupt()
{
	_requestInterrupt = true;
	_status = TaskStatus::Failure;
}
