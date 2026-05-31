#include "stdh.h"
#include "ChatTask_CompressSummarize.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCompress.h"
#include "ChatAgent.h"
#include <algorithm>

CChatTask_CompressSummarize::CChatTask_CompressSummarize(int workingOpIndex)
{
	_workingOpIndex = workingOpIndex;
	_hasStartedRequest = false;
	_requestInterrupt = false;
}

void CChatTask_CompressSummarize::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_CompressSummarize::_Succeed(const std::string& result)
{
	// 将压缩结果存入 workingOp.newCompressedContents
	if (_context && _context->chatAgent)
	{
		CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
		if (_workingOpIndex >= 0 && _workingOpIndex < (int)compressor._workingOps.size())
		{
			CChatOpsCompress::Op& workingOp = compressor._workingOps[_workingOpIndex];
			// 使用 Level_Partial 作为压缩等级
			workingOp.newCompressedContents[CChatOpsCompress::Level_Partial] = result;
		}
	}
	_status = TaskStatus::Success;
}

void CChatTask_CompressSummarize::Start()
{
	if (!_context || !_context->chatAgent)
	{
		_Fail();
		return;
	}

	CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
	
	// 验证索引有效性
	if (_workingOpIndex < 0 || _workingOpIndex >= (int)compressor._workingOps.size())
	{
		_Fail();
		return;
	}

	// 获取原始内容
	CChatOpsCompress::Op& workingOp = compressor._workingOps[_workingOpIndex];
	int srcIndex = workingOp.srcIndex;
	if (srcIndex < 0 || srcIndex >= (int)compressor._opsCtrl->GetOps().size())
	{
		_Fail();
		return;
	}

	const ChatOp& srcOp = compressor._opsCtrl->GetOps()[srcIndex];
	const std::string& textToCompress = srcOp.contentUtf8;

	if (textToCompress.empty())
	{
		_Fail();
		return;
	}

	// 查找可用于压缩摘要的API
	std::string apiName = g_llmLib.GetSummarizeApi();
	if (apiName.empty())
	{
		_Fail();
		return;
	}

	LlmSessionSetting setting;
	if (g_llmLib.LoadLlmSetting(setting, apiName, ""))
	{
		setting.api.tools.clear();
		setting.rulesFiles.clear();
		
		LlmSessionRequest request;
		
		// 构建精简提示词
		std::string prompt = u8R"(You are a context-compression assistant. Your task is to condense the conversation content below into a concise summary that preserves all information essential for continuing the work.

Compression guidelines:
- Retain the user's original intent, requirements, and any explicit constraints or preferences.
- Preserve key technical details: file paths, symbol names (functions, classes, members, variables), code snippets, signatures, and configuration values that may be referenced later.
- Keep track of decisions made, the current state of the task, and any pending or unresolved items.
- Remove redundant, repetitive, or purely conversational filler that adds no actionable value.
- Do not invent, infer, or add information that is not present in the original text.
- Maintain the chronological/logical order so the flow of work remains clear.

Output only the compressed summary text. Do not include any preamble, explanation, or commentary.

Content to compress:
)" + textToCompress;
		
		request.AddUserMessage(prompt.c_str());
		request.isStreaming = false;

		if (!_llmChat->Request(request, setting))
		{
			_Fail();
			return;
		}
		_hasStartedRequest = true;
		return;
	}

	_Fail();
}

void CChatTask_CompressSummarize::Update()
{
	if (!_llmChat)
		return;

	// 检查LLM会话状态
	if (_llmChat->HasActiveSession())
	{
		LlmSessionOutput output;
		
		if (_llmChat->Process(output, _requestInterrupt))
		{
			// 检查会话是否完成
			if (output.isCompleted)
			{
				if (output.content.empty())
				{
					_Fail();
				}
				else
				{
					if (output.hasError)
					{
						_Fail();
					}
					else if (!_requestInterrupt)
					{
						_Succeed(output.content);
					}
					else
					{
						_Fail();
					}
				}
			}
		}
	}
	else if (_hasStartedRequest)
	{
		_Fail();
	}
}