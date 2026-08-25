#include "stdh.h"
#include "ChatTask_HistoryEmbeddingQuery.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "InputHintContext.h"
#include "LlmSession.h"
#include "SolutionDBAPI.h"
#include "SolutionDBMsgs.h"
#include "Utils_File.h"

#include <algorithm>
#include <cstring>

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
		_context->inputHintCtx->SetHistorySimilarChunks(std::string());
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

	// 按 (filePath, startLine, endLine) 排序后去重，保留最高 similarity
	std::sort(_allChunks.begin(), _allChunks.end(), [](const auto& a, const auto& b) {
		if (a.filePath != b.filePath) return a.filePath < b.filePath;
		if (a.startLine != b.startLine) return a.startLine < b.startLine;
		if (a.endLine != b.endLine) return a.endLine < b.endLine;
		return a.similarity > b.similarity;
	});

	std::vector<SolutionDBMsg_SimilarChunks::Chunk> deduped;
	for (const auto& c : _allChunks)
	{
		if (!deduped.empty())
		{
			const auto& last = deduped.back();
			if (last.filePath == c.filePath && last.startLine == c.startLine && last.endLine == c.endLine)
				continue;
		}
		deduped.push_back(c);
	}

	// 按 similarity 降序排序
	std::sort(deduped.begin(), deduped.end(), [](const auto& a, const auto& b) {
		return a.similarity > b.similarity;
	});

	// 逐 chunk 校验文件时间并拼接，按行数限制总量
	const int kMaxTotalLines = 200;
	int remainingLines = kMaxTotalLines;
	std::string similarText;

	for (const auto& c : deduped)
	{
		if (remainingLines <= 0)
			break;

		// 检查文件是否在 embedding 生成后被修改过
		time_t curFileTime = Utils::GetFileTimeT(c.filePath.c_str());
		if (curFileTime != c.fileTime)
			continue;

		// range 是 [startLine, endLine)，GetFilePartIntoUTF8 接受闭区间 [start, end]
		int startLine = c.startLine;
		int endLine = c.endLine - 1;
		if (endLine < startLine)
			endLine = startLine;

		int chunkLines = endLine - startLine + 1;
		if (chunkLines > remainingLines)
		{
			endLine = startLine + remainingLines - 1;
			chunkLines = remainingLines;
		}

		Utils::FileContentCodingFormat codingFmt;
		int totalLineCount = 0;
		std::string chunkContent;
		if (!Utils::GetFilePartIntoUTF8(c.filePath.c_str(), startLine, endLine, chunkContent, codingFmt, totalLineCount))
			continue;

		remainingLines -= chunkLines;

		char header[512];
		std::snprintf(header, sizeof(header),
			"[File: %s L%d-L%d similarity: %.2f]\n",
			c.filePath.c_str(), startLine, endLine + 1, c.similarity);

		similarText += header;
		similarText += chunkContent;
		similarText += "\n\n";
	}

	_context->inputHintCtx->SetHistorySimilarChunks(std::move(similarText));

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
