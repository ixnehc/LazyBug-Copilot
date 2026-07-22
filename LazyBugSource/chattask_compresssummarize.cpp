#include "stdh.h"
#include "ChatTask_CompressSummarize.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCompress.h"
#include "ChatAgent.h"
#include "ChatDialogA.h"
#include "LlmTools.h"
#include <algorithm>

#include "Utils_Context.h"
#include "TokenCalibrate.h"

extern const char* GetOpenedDBFolderPath_utf8();

// 获取压缩总结日志文件路径
std::string GetCompressSummarizeLogPath()
{
	std::string path = GetOpenedDBFolderPath_utf8();
	path += "\\_log\\compress_summarize.txt";
	return path;
}

// 追加压缩总结日志到文件
static void AppendCompressSummarize(const std::string& logStr)
{
	std::ofstream outFile;
	Utils::OpenOFStream(outFile, GetCompressSummarizeLogPath().c_str(), std::ios::app);
	if (outFile.is_open())
	{
		outFile << logStr << "\n\n";
		outFile.close();
	}
}

// 清空压缩总结日志文件
void ClearCompressSummarize()
{
	std::ofstream outFile;
	Utils::OpenOFStream(outFile, GetCompressSummarizeLogPath().c_str(), std::ios::trunc);
	if (outFile.is_open())
	{
		outFile.close();
	}
}

CChatTask_CompressSummarize::CChatTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName, CompressSummarizeMode mode)
{
	_workingOpIndex = workingOpIndex;
	_summarizeApiName = summarizeApiName;
	_mode = mode;
	_hasStartedRequest = false;
	_requestInterrupt = false;
}

void CChatTask_CompressSummarize::_Fail(const std::string& reason)
{
	_resultMessage = _MakeShortResultString(false, reason);
	
	// Immediate 模式不通知 compressor（通过 chatDialogA 直接操作 ops）
	if (_mode != CompressSummarizeMode::Immediate && _context && _context->chatAgent)
	{
		CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
		compressor._SetCompressSummarizeTip(false, _resultMessage, GetCompressSummarizeLogPath());
	}
	
	_status = TaskStatus::Failure;
}

void CChatTask_CompressSummarize::_Succeed(const std::string& result, const LlmSessionUsage& usage)
{
	int originalTokenCount = static_cast<int>(Utils::EstimateTokenCount(_textToCompress) * CTokenCalibrate::GetCalibrationFactor());
	int resultTokenCount = static_cast<int>(Utils::EstimateTokenCount(result) * CTokenCalibrate::GetCalibrationFactor());
	
	// 输出 log（追加到文件末尾）
	{
		std::string logStr = _MakeCompressLogString(_textToCompress, result, originalTokenCount, resultTokenCount, usage.fee);
		AppendCompressSummarize(logStr);
	}
	
	// 决定实际存储的内容（压缩量不够则跳过）
	const std::string* pContentToStore = &result;
	if (!(resultTokenCount + 50 <= originalTokenCount))
	{
		static std::string skip_compress = "<skip_compress>";
		pContentToStore = &skip_compress;
		resultTokenCount = originalTokenCount;
	}

	if (_mode == CompressSummarizeMode::Evaluation)
	{
		_resultMessage = _MakeShortResultString(true, "", originalTokenCount, resultTokenCount);
	}
	else if (_mode == CompressSummarizeMode::Immediate)
	{
		// 直接写入 CChatOpsCtrl::_ops 中对应的 op
		CChatOpsCtrl* pOpsCtrl = nullptr;
		if (_context && _context->chatDialogA)
			pOpsCtrl = &_context->chatDialogA->GetOpsCtrl();
		else if (_context && _context->chatOpsCtrl)
			pOpsCtrl = _context->chatOpsCtrl;
		if (pOpsCtrl)
			pOpsCtrl->SetOpCompressedContent(_workingOpIndex, CChatOpsCompress::Level_Partial, *pContentToStore);
		_resultMessage = _MakeShortResultString(true, "", originalTokenCount, resultTokenCount);
	}
	else  // Normal
	{
		// 将压缩结果存入 workingOp.newCompressedContents
		if (_context && _context->chatAgent)
		{
			CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
			if (_workingOpIndex >= 0 && _workingOpIndex < (int)compressor._workingOps.size())
			{
				CChatOpsCompress::Op& workingOp = compressor._workingOps[_workingOpIndex];
				workingOp.newCompressedContents[CChatOpsCompress::Level_Partial] = *pContentToStore;
				
				_resultMessage = _MakeShortResultString(true, "", originalTokenCount, resultTokenCount);
				compressor._SetCompressSummarizeTip(true, _resultMessage, GetCompressSummarizeLogPath());
			}
		}
	}
	_status = TaskStatus::Success;
}

std::string CChatTask_CompressSummarize::_MakeCompressLogString(const std::string& originalContent, const std::string& compressedContent, int originalTokens, int compressedTokens, float cost)
{
	std::string logStr;
	
	// 标题信息
	logStr += "################################################################################\n";
	logStr += "##                                                                            ##\n";
	logStr += "##                     [CompressSummarize] Session Mode                       ##\n";
	logStr += "##                                                                            ##\n";
	logStr += "################################################################################\n";
	logStr += "\n";
	
	// 压缩结果摘要
	logStr += "==============================================================================\n";
	logStr += "=                                   SUMMARY                                  =\n";
	logStr += "==============================================================================\n";
	int estimatedTokens = static_cast<int>(Utils::EstimateTokenCount(_textToCompress) * CTokenCalibrate::GetCalibrationFactor());
	logStr += "  Session tokens:                " + std::to_string(originalTokens) + "\n";
	logStr += "  Tokens of content to compress: " + std::to_string(estimatedTokens) + "\n";
	logStr += "  Compressed tokens:             " + std::to_string(compressedTokens) + "\n";
	int reduced = originalTokens - compressedTokens;
	int percent = originalTokens > 0 ? (reduced * 100 / originalTokens) : 0;
	logStr += "  Reduced:                       " + std::to_string(reduced) + " tokens (" + std::to_string(percent) + "%)\n";
	logStr += "  Result:                        " + std::string(compressedTokens + 50 <= originalTokens ? "SUCCESS" : "SKIPPED (insufficient reduction)") + "\n";
	logStr += "  Summarize API:                 " + _summarizeApiName + "\n";
	char costBuffer[64];
	sprintf_s(costBuffer, 64, "  Cost:                          $%.6f\n", cost);
	logStr += costBuffer;
	logStr += "\n\n";
	
	// 压缩后内容（先显示）
	logStr += "==============================================================================\n";
	logStr += "=                             COMPRESSED CONTENT                             =\n";
	logStr += "==============================================================================\n";
	logStr += compressedContent;
	logStr += "\n\n\n";
	
	// 原始内容（后显示）
	logStr += "==============================================================================\n";
	logStr += "=                              ORIGINAL CONTENT                              =\n";
	logStr += "==============================================================================\n";
	logStr += originalContent;
	logStr += "\n";
	
	return logStr;
}

std::string CChatTask_CompressSummarize::_MakeShortResultString(bool success, const std::string& reason, int originalTokens, int compressedTokens)
{
	std::string str;
	
	if (success)
	{
		int reduced = originalTokens - compressedTokens;
		str += "context reduced: " + std::to_string(reduced) + " tokens (" + 
		       std::to_string(originalTokens) + " -> " + std::to_string(compressedTokens) + "), view detail";
	}
	else
	{
		str += "context compressing failed";
		if (!reason.empty())
			str += " : " + reason;
	}
	
	return str;
}

std::string CChatTask_CompressSummarize::_CollectSessionContent()
{
	if (!_context || !_context->chatAgent)
		return "";

	CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();

	// 获取 _workingOpIndex 对应的 srcIndex
	if (_workingOpIndex < 0 || _workingOpIndex >= (int)compressor._workingOps.size())
		return "";

	int targetSrcIndex = compressor._workingOps[_workingOpIndex].srcIndex;

	std::string collectedContent;
	compressor._opsCtrl->CollectUncompressedSessionAIContent(targetSrcIndex, CChatOpsCompress::GetSessionSummarizeToolTypes(), collectedContent);

	return collectedContent;
}

void CChatTask_CompressSummarize::Start()
{
	if (!_context)
	{
		_Fail("No context");
		return;
	}

	if (_mode == CompressSummarizeMode::Evaluation || _mode == CompressSummarizeMode::Immediate)
	{
		// Evaluation/Immediate 模式：通过 chatDialogA 或 chatOpsCtrl 获取 opsCtrl
		CChatOpsCtrl* pOpsCtrl = nullptr;
		if (_context->chatDialogA)
			pOpsCtrl = &_context->chatDialogA->GetOpsCtrl();
		else if (_context->chatOpsCtrl)
			pOpsCtrl = _context->chatOpsCtrl;
		if (!pOpsCtrl)
		{
			_Fail("No chatOpsCtrl available");
			return;
		}

		// 验证索引有效性
		const std::vector<ChatOp>& ops = pOpsCtrl->GetOps();
		if (_workingOpIndex < 0 || _workingOpIndex >= (int)ops.size())
		{
			_Fail("Invalid op index");
			return;
		}

		// 收集 session 内容
		std::string textToCompress;
		pOpsCtrl->CollectUncompressedSessionAIContent(_workingOpIndex, CChatOpsCompress::GetSessionSummarizeToolTypes(), textToCompress);
		
		if (textToCompress.empty())
		{
			_Succeed("<skip_compress>", LlmSessionUsage());
			return;
		}

		_textToCompress = textToCompress;
	}
	else
	{
		// 正常模式：使用 chatAgent->GetCompressor()
		if (!_context->chatAgent)
		{
			_Fail("No chat agent");
			return;
		}

		CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
		
		// 验证索引有效性
		if (_workingOpIndex < 0 || _workingOpIndex >= (int)compressor._workingOps.size())
		{
			_Fail("Invalid working op index");
			return;
		}

		// 收集整个 session 的内容
		std::string textToCompress = _CollectSessionContent();
		if (textToCompress.empty())
		{
			_Succeed("<skip_compress>", LlmSessionUsage());
			return;
		}

		_textToCompress = textToCompress;
	}

	if (_textToCompress.empty())
	{
		_Succeed("<skip_compress>", LlmSessionUsage());
		return;
	}

	// 使用传入的 summarize API 名称
	if (_summarizeApiName.empty())
	{
		_Fail("Summarize API name is empty");
		return;
	}

	LlmSessionSetting setting;
	if (g_llmLib.LoadLlmSetting(setting, _summarizeApiName,false, ""))
	{
		setting.api.tools.clear();
		setting.rulesFiles.clear();
//  		setting.api.thinkingMode = LlmThinkingMode::Disable;
		
		LlmSessionRequest request;
		
		// 构建精简提示词
		std::string prompt;
		prompt =
			"Please summarize the following text. Preserve the original tone and all key information,\n"
			"including file names, code symbols (function names, class names, variable names, etc.).\n"
			"Never include detailed code snippets\n"
			"Output only the summarized content. Do not include any additional text or explanation.\n"
 			"Never start with \"This is the summary,...\" or something like that\n"
			"Text to summarize:\n";
		prompt += _textToCompress;
		
		request.AddUserMessage(prompt.c_str());
		request.isStreaming = true;

		if (!_llmChats[0]->Request(request, setting))
		{
			_Fail("Failed to send LLM request");
			return;
		}
		_hasStartedRequest = true;
		return;
	}

	_Fail("Failed to load LLM setting");
}

void CChatTask_CompressSummarize::Update()
{
	if (_llmChats.empty())
		return;

	// 检查LLM会话状态
	if (_llmChats[0]->HasActiveSession())
	{
		LlmSessionOutput output;
		
		if (_llmChats[0]->Process(output, _requestInterrupt))
		{
			// 检查会话是否完成
			if (output.isCompleted)
			{
				if(_requestInterrupt)
					_Fail("Interrupted");
				else
				{
					if (output.fullContent.empty())
					{
						_Fail("LLM returned empty content");
					}
					else
					{
						if (output.hasError)
						{
							_Fail("LLM error: " + output.errorMessage);
						}
						else if (!_requestInterrupt)
						{
							_Succeed(output.fullContent, output.usage);
						}
						else
						{
							_Fail("Interrupted by user");
						}
					}
				}
			}
		}
	}
	else if (_hasStartedRequest)
	{
		_Fail("LLM session ended unexpectedly");
	}
}

void CChatTask_CompressSummarize::Interrupt()
{ 
	_requestInterrupt = true; 
	Update();
}


