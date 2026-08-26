#include "stdh.h"
#include "ChatTask_HistoryEmbeddingQuery.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "InputHintContext.h"
#include "CoreDefines.h"
#include "LlmSession.h"
#include "SolutionDBAPI.h"
#include "SolutionDBMsgs.h"

#include <algorithm>

// 声明外部函数
extern const char* GetOpenedDBFolderPath_utf8();


CChatTask_HistoryEmbeddingQuery::CChatTask_HistoryEmbeddingQuery(
	const std::string& apiName, const std::string& embeddingApiName)
{
	_apiName = apiName;
	_embeddingApiName = embeddingApiName;
}

void CChatTask_HistoryEmbeddingQuery::_Fail(const std::string& reason)
{
	(void)reason;
	if (_context && _context->inputHintCtx)
		_context->inputHintCtx->SetHistorySimilarChunks(std::vector<EmbeddingSimilarChunk>());
	_status = TaskStatus::Failure;
}

// ── Phase 1: PlanQueries ──

void CChatTask_HistoryEmbeddingQuery::_StartPlanQueries()
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

bool CChatTask_HistoryEmbeddingQuery::_ProcessPlanQueriesResponse(const std::string& llmOutput)
{
	std::string result = llmOutput;

	// 去除首尾空白
	size_t start = result.find_first_not_of(" \t\r\n");
	size_t end = result.find_last_not_of(" \t\r\n");
	if (start != std::string::npos && end != std::string::npos)
		result = result.substr(start, end - start + 1);
	else
		result.clear();

	if (result.empty())
		return false;

	// 提取 JSON 子串（处理可能的 markdown 包裹或多余文本）
	size_t braceStart = result.find('{');
	size_t braceEnd = result.rfind('}');
	if (braceStart == std::string::npos || braceEnd == std::string::npos || braceEnd <= braceStart)
		return false;

	std::string jsonStr = result.substr(braceStart, braceEnd - braceStart + 1);

	json parsed;
	try
	{
		parsed = json::parse(jsonStr);
	}
	catch (...)
	{
		return false;
	}

	if (!parsed.contains("queries") || !parsed["queries"].is_array())
		return false;

	for (const auto& q : parsed["queries"])
	{
		if (q.contains("text") && q["text"].is_string())
		{
			std::string text = q["text"].get<std::string>();
			if (!text.empty())
				_queries.push_back(std::move(text));
		}
		if (_queries.size() >= 5)
			break;
	}

	return !_queries.empty();
}

// ── Phase 2: RequestEmbedding ──

void CChatTask_HistoryEmbeddingQuery::_StartEmbeddingRequest()
{
	LlmSessionSetting setting;
	if (!g_llmLib.LoadLlmSetting(setting, _embeddingApiName, false, nullptr))
	{
		_Fail("Failed to load embedding API settings");
		return;
	}

	setting.api.tools.clear();
	_embeddingModelName = setting.api.model;

	if (!_llmChats[0]->RequestEmbedding(_queries[_currentQueryIndex], setting))
	{
		_Fail("Failed to send embedding request");
		return;
	}

	_hasStartedRequest = true;
}

// ── Phase 3: QuerySimilar ──

void CChatTask_HistoryEmbeddingQuery::_ExecuteSimilarityQuery()
{
	if (_requestInterrupt)
	{
		_phase = Phase::Done;
		return;
	}

	const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
	if (!dbFolderPath || dbFolderPath[0] == '\0')
	{
		_Fail("No opened DB folder");
		return;
	}

	SolutionDBMsg_SimilarChunks result;
	SolutionDB_QuerySimilarByVector(dbFolderPath, _currentEmbedding, _embeddingModelName.c_str(), 5, result);

	for (auto& c : result.chunks)
		_allChunks.push_back(std::move(c));

	_currentQueryIndex++;
	if (_currentQueryIndex < static_cast<int>(_queries.size()))
		_phase = Phase::RequestEmbedding;
	else
		_phase = Phase::MergeAndFormat;
}

// ── Phase 4: MergeAndFormat ──

void CChatTask_HistoryEmbeddingQuery::_MergeAndFormatChunks()
{
	if (_requestInterrupt)
	{
		_phase = Phase::Done;
		return;
	}

	// 将累积的 chunks 转换为 EmbeddingSimilarChunk 并写入 InputHintContext
	std::vector<EmbeddingSimilarChunk> chunks;
	chunks.reserve(_allChunks.size());
	for (const auto& c : _allChunks)
	{
		EmbeddingSimilarChunk esc;
		esc.filePath = c.filePath;
		esc.range = { c.startLine, c.endLine };
		esc.genTime = c.fileTime;
		esc.similarity = c.similarity;
		chunks.push_back(std::move(esc));
	}

	_context->inputHintCtx->SetHistorySimilarChunks(std::move(chunks));

	_phase = Phase::Done;
	_status = TaskStatus::Success;
}

// ── CChatTask overrides ──

void CChatTask_HistoryEmbeddingQuery::Start()
{
	_StartPlanQueries();
}

void CChatTask_HistoryEmbeddingQuery::Update()
{
	if (_llmChats.empty())
		return;

	switch (_phase)
	{
	case Phase::PlanQueries:
	{
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

		if (_ProcessPlanQueriesResponse(output.fullContent))
		{
			_hasStartedRequest = false;
			_currentQueryIndex = 0;
			_phase = Phase::RequestEmbedding;
		}
		else
		{
			_Fail("Failed to parse query plan");
		}
		break;
	}

	case Phase::RequestEmbedding:
	{
		if (!_hasStartedRequest)
		{
			_StartEmbeddingRequest();
		}
		else
		{
			if (!_llmChats[0]->HasActiveSession())
			{
				_Fail("No response from embedding API");
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
				_Fail(output.errorMessage.empty() ? "Embedding request failed" : output.errorMessage);
				return;
			}

			if (output.embedding.empty())
			{
				_Fail("Empty embedding response");
				return;
			}

			_currentEmbedding = std::move(output.embedding);
			_hasStartedRequest = false;
			_phase = Phase::QuerySimilar;
		}
		break;
	}

	case Phase::QuerySimilar:
	{
		_ExecuteSimilarityQuery();
		break;
	}

	case Phase::MergeAndFormat:
	{
		_MergeAndFormatChunks();
		break;
	}

	case Phase::Done:
		break;
	}
}

void CChatTask_HistoryEmbeddingQuery::Interrupt()
{
	_requestInterrupt = true;
	_status = TaskStatus::Failure;
}
