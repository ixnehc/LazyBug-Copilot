#include "stdh.h"
#include "ChatTask_InputEmbeddingQuery.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "InputHintContext.h"
#include "SolutionDBAPI.h"
#include "SolutionDBMsgs.h"
#include "Utils_File.h"

#include <algorithm>
#include <cstring>

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
	const std::string& chatOps = ctx->GetChatOpsContent();
	if (!chatOps.empty())
	{
		// 反向查找最后一个 "User: "
		const std::string marker = "User: ";
		size_t userPos = chatOps.rfind(marker);
		if (userPos != std::string::npos)
		{
			size_t msgStart = userPos + marker.size();
			size_t nlPos = chatOps.find('\n', msgStart);
			std::string lastUserMsg = (nlPos == std::string::npos)
				? chatOps.substr(msgStart)
				: chatOps.substr(msgStart, nlPos - msgStart);

			// 若该消息非输入纯文本的子串，则追加
			if (!lastUserMsg.empty() && queryText.find(lastUserMsg) == std::string::npos)
			{
				queryText += "\n";
				queryText += lastUserMsg;
			}
		}
	}

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
						_Fail(output.errorMessage.empty() ? "Embedding request failed" : output.errorMessage);
						return;
					}

					if (output.embedding.empty())
					{
						_Fail("Empty embedding response");
						return;
					}

					_embedding = std::move(output.embedding);
					_phase = Phase::Query;
				}
			}
		}
		else if (_hasStartedRequest)
		{
			_Fail("No response from embedding API");
			return;
		}
	}
	else if (_phase == Phase::Query)
	{
		const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
		if (!dbFolderPath || dbFolderPath[0] == '\0')
		{
			_Fail("No opened DB folder");
			return;
		}

		SolutionDBMsg_SimilarChunks result;
		SolutionDB_QuerySimilarByVector(dbFolderPath, _embedding, _modelName.c_str(), 5, result);

		// 读取每个 chunk 的文件内容并拼接为文本，按行数限制总量
		const int kMaxTotalLines = 200;
		int remainingLines = kMaxTotalLines;
		std::string similarText;

		for (const auto& c : result.chunks)
		{
			if (remainingLines <= 0)
				break;

			// 检查文件是否在 embedding 生成后被修改过，若不匹配则跳过
			time_t curFileTime = Utils::GetFileTimeT(c.filePath.c_str());
			if (curFileTime != c.fileTime)
				continue;

			// range 是 [startLine, endLine)，GetFilePartIntoUTF8 接受闭区间 [start, end]
			int startLine = c.startLine;
			int endLine = c.endLine - 1;
			if (endLine < startLine)
				endLine = startLine;

			// 超过剩余行数限制时截断行范围
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

			// 格式: [File: path Lstart-end similarity: 0.xx]\n<content>\n\n
			char header[512];
			std::snprintf(header, sizeof(header),
				"[File: %s L%d-L%d similarity: %.2f]\n",
				c.filePath.c_str(), startLine, endLine + 1, c.similarity);

			std::string entry = header;
			entry += chunkContent;
			entry += "\n\n";

			similarText += entry;
		}

		_context->inputHintCtx->SetSimilarChunks(std::move(similarText));

		_phase = Phase::Done;
		_status = TaskStatus::Success;
	}
}

void CChatTask_InputEmbeddingQuery::Interrupt()
{
	_requestInterrupt = true;
	_status = TaskStatus::Failure;
}
