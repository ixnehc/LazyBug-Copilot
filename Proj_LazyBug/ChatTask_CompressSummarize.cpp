#include "stdh.h"
#include "ChatTask_CompressSummarize.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCompress.h"
#include "ChatAgent.h"
#include <algorithm>

#include "Utils_Context.h"

CChatTask_CompressSummarize::CChatTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName)
{
	_workingOpIndex = workingOpIndex;
	_summarizeApiName = summarizeApiName;
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

// 			std::wstring  newContent = utf8_to_widechar(result);
// 			std::wstring oldContent= utf8_to_widechar(compressor._opsCtrl->GetOps()[workingOp.srcIndex].contentUtf8);

			// 使用 Level_Partial 作为压缩等级
			const std::string* pContentToStore = &result;
			int resultTokenCount = Utils::EstimateTokenCount(result);
			// 如果压缩掉的token数不到400,仍然用原来的
			if (resultTokenCount + 400 > _originalTokenCount)
			{
				int srcIndex = workingOp.srcIndex;
				if (srcIndex >= 0 && srcIndex < (int)compressor._opsCtrl->GetOps().size())
				{
					pContentToStore = &compressor._opsCtrl->GetOps()[srcIndex].contentUtf8;
				}
			}
			workingOp.newCompressedContents[CChatOpsCompress::Level_Partial] = *pContentToStore;
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
	_originalTokenCount = Utils::EstimateTokenCount(textToCompress);

	if (textToCompress.empty())
	{
		_Fail();
		return;
	}

	// 使用传入的 summarize API 名称
	if (_summarizeApiName.empty())
	{
		_Fail();
		return;
	}

	LlmSessionSetting setting;
	if (g_llmLib.LoadLlmSetting(setting, _summarizeApiName,false, ""))
	{
		setting.api.tools.clear();
		setting.rulesFiles.clear();
		
		LlmSessionRequest request;
		
		// 构建精简提示词
		std::string prompt =
			"Please summarize the following text. Preserve the original tone and all key information,\n"
			"including file names, code symbols (function names, class names, variable names, etc.).\n"
			"Output only the summary. Do not include any additional text or explanation.\n"
			"Text to summarize:\n";
		prompt += textToCompress;
		
		request.AddUserMessage(prompt.c_str());
		request.isStreaming = true;

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
				if (output.fullContent.empty())
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
						_Succeed(output.fullContent);
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

void CChatTask_CompressSummarize::Interrupt()
{ 
	_requestInterrupt = true; 
	Update();
}
