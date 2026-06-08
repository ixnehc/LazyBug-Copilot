#include "stdh.h"
#include "ChatTask_CompressSummarize.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCompress.h"
#include "ChatAgent.h"
#include "LlmTools.h"
#include <algorithm>

#include "Utils_Context.h"

CChatTask_CompressSummarize::CChatTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName, bool isSessionMode)
{
	_workingOpIndex = workingOpIndex;
	_summarizeApiName = summarizeApiName;
	_isSessionMode = isSessionMode;
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
			//XXXXXXXXXXXXXXXXXXXX
			std::wstring  newContent = utf8_to_widechar(result);
			std::wstring oldContent= utf8_to_widechar(compressor._opsCtrl->GetOps()[workingOp.srcIndex].contentUtf8);

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

std::string CChatTask_CompressSummarize::_CollectSessionContent()
{
	if (!_context || !_context->chatAgent)
		return "";

	CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
	const std::vector<ChatOp>& srcOps = compressor._opsCtrl->GetOps();

	// 获取 _workingOpIndex 对应的 srcIndex
	if (_workingOpIndex < 0 || _workingOpIndex >= (int)compressor._workingOps.size())
		return "";

	int targetSrcIndex = compressor._workingOps[_workingOpIndex].srcIndex;
	if (targetSrcIndex < 0 || targetSrcIndex >= (int)srcOps.size())
		return "";

	// 找到 session 边界：向前找 Op_BeginSession，向后找 Op_EndSession
	int sessionBeginIndex = -1;
	int sessionEndIndex = -1;

	// 向前找 Op_BeginSession
	for (int i = targetSrcIndex; i >= 0; i--)
	{
		if (srcOps[i].type == ChatOp::Op_BeginSession)
		{
			sessionBeginIndex = i;
			break;
		}
	}

	// 向后找 Op_EndSession
	for (int i = targetSrcIndex; i < (int)srcOps.size(); i++)
	{
		if (srcOps[i].type == ChatOp::Op_EndSession)
		{
			sessionEndIndex = i;
			break;
		}
	}

	// 如果没有找到 session 边界，返回空
	if (sessionBeginIndex < 0 || sessionEndIndex < 0)
		return "";

	// 定义需要收集的 ToolCall 类型
	auto isTargetToolType = [](LlmToolType toolType) {
		return true;
// 		return toolType == LlmToolType::ReplaceInFile ||
// 			   toolType == LlmToolType::CLI_Bash ||
// 			   toolType == LlmToolType::CLI_Cmd ||
// 			   toolType == LlmToolType::CLI_RunScript ||
// 			   toolType == LlmToolType::Question;
	};

	// 收集 session 内的内容
	std::string collectedContent;
	for (int i = sessionBeginIndex; i <= sessionEndIndex; i++)
	{
		const ChatOp& op = srcOps[i];

		if (op.type == ChatOp::Op_AddStreamingAIMessage)
		{
			// 收集 AI 消息内容
			if (!op.contentUtf8.empty())
			{
				collectedContent += "[AI Message]\n";
				collectedContent += op.contentUtf8;
				collectedContent += "\n\n";
			}
		}
		else if (op.type == ChatOp::Op_AddToolCallResult)
		{
			// 解析 ToolCall 类型并判断是否需要收集
			LlmToolType toolType = CLlmTools::ParseToolTypeFromToolCallResultString(op.contentUtf8);
			if (isTargetToolType(toolType))
			{
				if (!op.contentUtf8.empty())
				{
					collectedContent += "[ToolCall: ";
					collectedContent += g_llmTools.GetToolTypeName(toolType);
					collectedContent += "]\n";
					collectedContent += op.contentUtf8;
					collectedContent += "\n\n";
				}
			}
		}
	}

	return collectedContent;
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

	std::string textToCompress;

	if (_isSessionMode)
	{
		// Session 模式：收集整个 session 的内容
		textToCompress = _CollectSessionContent();
		if (textToCompress.empty())
		{
			_Fail();
			return;
		}
	}
	else
	{
		// 原有模式：单个 Op 的内容
		CChatOpsCompress::Op& workingOp = compressor._workingOps[_workingOpIndex];
		int srcIndex = workingOp.srcIndex;
		if (srcIndex < 0 || srcIndex >= (int)compressor._opsCtrl->GetOps().size())
		{
			_Fail();
			return;
		}

		const ChatOp& srcOp = compressor._opsCtrl->GetOps()[srcIndex];
		textToCompress = srcOp.contentUtf8;
	}

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
		std::string prompt;
		prompt =
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
