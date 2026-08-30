#include "stdh.h"
#include "ChatTask_InputEmbeddingQuery.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "InputHintContext.h"
#include "CoreDefines.h"
#include "SolutionDBAPI.h"
#include "SolutionDBMsgs.h"

#include <algorithm>

// 声明外部函数
extern std::string widechar_to_utf8(const wchar_t* str);
extern const char* GetOpenedDBFolderPath_utf8();


CChatTask_InputEmbeddingQuery::CChatTask_InputEmbeddingQuery(const std::string& embeddingApiName)
{
	_embeddingApiName = embeddingApiName;
}

void CChatTask_InputEmbeddingQuery::_Fail(const std::string& reason)
{
	(void)reason;
	if (_context && _context->inputHintCtx)
		_context->inputHintCtx->SetInputSimilarChunks(std::vector<EmbeddingSimilarChunk>(), 0);
	_status = TaskStatus::Failure;
}

std::string CChatTask_InputEmbeddingQuery::_BuildQueryText()
{
	if (!_context || !_context->inputHintCtx)
		return "";

	const InputHintContext* ctx = _context->inputHintCtx;

	// 1. 拼接输入纯文本（去掉光标标记）
	std::wstring inputPlain;
	if (!ctx->GetBeforeCaretLines().empty())
		inputPlain += ctx->GetBeforeCaretLines() + L"\n";

	std::wstring caretLine = ctx->GetCaretLine();
	// 去掉光标标记 \x2038
	size_t caretPos = caretLine.find(L'\x2038');
	if (caretPos != std::wstring::npos)
		caretLine.erase(caretPos, 1);
	inputPlain += caretLine;

	if (!ctx->GetAfterCaretLines().empty())
		inputPlain += L"\n" + ctx->GetAfterCaretLines();

	std::string queryText = widechar_to_utf8(inputPlain.c_str());

	// 2. 从 chatOpsContent 中提取最近一条用户消息
// 	const std::string& chatOps = ctx->GetChatOpsContent();
// 	if (!chatOps.empty())
// 	{
// 		// 反向查找最后一个 "User: "
// 		const std::string marker = "User: ";
// 		size_t userPos = chatOps.rfind(marker);
// 		if (userPos != std::string::npos)
// 		{
// 			size_t msgStart = userPos + marker.size();
// 			size_t nlPos = chatOps.find('\n', msgStart);
// 			std::string lastUserMsg = (nlPos == std::string::npos)
// 				? chatOps.substr(msgStart)
// 				: chatOps.substr(msgStart, nlPos - msgStart);
// 
// 			// 若该消息非输入纯文本的子串，则追加
// 			if (!lastUserMsg.empty() && queryText.find(lastUserMsg) == std::string::npos)
// 			{
// 				queryText += "\n";
// 				queryText += lastUserMsg;
// 			}
// 		}
// 	}

	return queryText;
}

void CChatTask_InputEmbeddingQuery::Start()
{
	if (!_context || !_context->inputHintCtx)
	{
		_Fail("No InputHintContext");
		return;
	}

	// 构造查询文本
	_queryText = _BuildQueryText();
	if (_queryText.empty())
	{
		_Fail("Empty query text");
		return;
	}

	// 加载 embedding API 设置（不加载 chat rules）
	LlmSessionSetting setting;
	if (!g_llmLib.LoadLlmSetting(setting, _embeddingApiName, false, nullptr))
	{
		_Fail("Failed to load embedding API settings");
		return;
	}

	setting.api.tools.clear();

	// 记录模型名，用于后续 QuerySimilar 匹配
	_modelName = setting.api.model;

	// 发送异步 embedding 请求
	if (!_llmChats[0]->RequestEmbedding(_queryText, setting))
	{
		_Fail("Failed to send embedding request");
		return;
	}

	_hasStartedRequest = true;
	_phase = Phase::Embedding;
	_status = TaskStatus::Running;
}

void CChatTask_InputEmbeddingQuery::Update()
{
	if (_llmChats.empty())
		return;

	if (_phase == Phase::Embedding)
	{
		if (_llmChats[0]->HasActiveSession())
		{
			LlmSessionOutput output;
			if (_llmChats[0]->Process(output, _requestInterrupt))
			{
				if (output.isCompleted)
				{
					if (_requestInterrupt)
					{
						_Fail("Interrupted");
						return;
					}

					if (output.hasError)
					{
						if (_context && _context->inputHintCtx)
							_context->inputHintCtx->SetLastEmbeddingRequestStatus(EmbeddingRequestStatus{false, time(nullptr)});
						_Fail(output.errorMessage.empty() ? "Embedding request failed" : output.errorMessage);
						return;
					}

					if (output.embedding.empty())
					{
						if (_context && _context->inputHintCtx)
							_context->inputHintCtx->SetLastEmbeddingRequestStatus(EmbeddingRequestStatus{false, time(nullptr)});
						_Fail("Empty embedding response");
						return;
					}

					if (_context && _context->inputHintCtx)
						_context->inputHintCtx->SetLastEmbeddingRequestStatus(EmbeddingRequestStatus{true, time(nullptr)});
					_embedding = std::move(output.embedding);
					_phase = Phase::Query;
				}
			}
		}
		else if (_hasStartedRequest || _requestInterrupt)
		{
			if (!_requestInterrupt && _context && _context->inputHintCtx)
				_context->inputHintCtx->SetLastEmbeddingRequestStatus(EmbeddingRequestStatus{false, time(nullptr)});
			_Fail("No response from embedding API");
			return;
		}
	}
	else if (_phase == Phase::Query)
	{
		if (_requestInterrupt)
		{
			_Fail("Interrupted");
			return;
		}

		const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
		if (!dbFolderPath || dbFolderPath[0] == '\0')
		{
			_Fail("No opened DB folder");
			return;
		}

		SolutionDBMsg_EmbeddingDBVersion versionResult = SolutionDB_GetEmbeddingDBVersion(dbFolderPath);
		uint64_t embeddingDBVersion = versionResult.success ? versionResult.version : 0;

		SolutionDBMsg_SimilarChunks result;
		SolutionDB_QuerySimilarByVector(dbFolderPath, _embedding, _modelName.c_str(), 5, result);

		// 将 chunk 转换为 EmbeddingSimilarChunk 并写入 InputHintContext
		std::vector<EmbeddingSimilarChunk> chunks;
		chunks.reserve(result.chunks.size());
		for (const auto& c : result.chunks)
		{
			EmbeddingSimilarChunk esc;
			esc.filePath = c.filePath;
			esc.range = { c.startLine, c.endLine };
			esc.genTime = c.fileTime;
			esc.similarity = c.similarity;
			chunks.push_back(std::move(esc));
		}

		_context->inputHintCtx->SetInputSimilarChunks(std::move(chunks), embeddingDBVersion);

		_phase = Phase::Done;
		_status = TaskStatus::Success;
	}
}

void CChatTask_InputEmbeddingQuery::Interrupt()
{
	// 只置中断标记，保持 Running 状态，让后续 Update() 走完收尾流程（清理会话并结束任务）
	_requestInterrupt = true;
}
