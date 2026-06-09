#include "stdh.h"
#include "ChatTask_CompressSummarize.h"

#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCompress.h"
#include "ChatAgent.h"
#include "LlmTools.h"
#include <algorithm>

#include "Utils_Context.h"

extern const char* GetOpenedDBFolderPath_utf8();

CChatTask_CompressSummarize::CChatTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName, bool isSessionMode)
{
	_workingOpIndex = workingOpIndex;
	_summarizeApiName = summarizeApiName;
	_isSessionMode = isSessionMode;
	_hasStartedRequest = false;
	_requestInterrupt = false;
}

void CChatTask_CompressSummarize::_Fail(const std::string& reason)
{
	_resultMessage = _MakeShortResultString(false, reason);
	
	// 设置提示消息到 CChatOpsCompress
	if (_context && _context->chatAgent)
	{
		CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
		std::string logPath = GetOpenedDBFolderPath_utf8();
		logPath += "\\_log\\compress_summarize.txt";
		compressor._SetCompressSummarizeTip(false, _resultMessage, logPath);
	}
	
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
			const std::string* pContentToStore = &result;
			int resultTokenCount = Utils::EstimateTokenCount(result);
			
			// 输出 log
			{
				std::string logStr = _MakeCompressLogString(_originalContent, result, _originalTokenCount, resultTokenCount);
				std::string path = GetOpenedDBFolderPath_utf8();
				path += "\\_log\\compress_summarize.txt";
				std::ofstream outFile;
				Utils::OpenOFStream(outFile, path.c_str(), std::ios::app);
				if (outFile.is_open())
				{
					outFile << logStr << "\n\n";
					outFile.close();
				}
			}
			
			// 如果压缩掉的token数不到400,仍然用原来的
			bool bSuccess = (resultTokenCount + 400 <= _originalTokenCount);
			if (!bSuccess)
			{
				int srcIndex = workingOp.srcIndex;
				if (srcIndex >= 0 && srcIndex < (int)compressor._opsCtrl->GetOps().size())
				{
					pContentToStore = &compressor._opsCtrl->GetOps()[srcIndex].contentUtf8;
				}
			}
			workingOp.newCompressedContents[CChatOpsCompress::Level_Partial] = *pContentToStore;
			
			// 准备简短结果信息
			_resultMessage = _MakeShortResultString(bSuccess, "", _originalTokenCount, resultTokenCount);
			
			// 设置提示消息到 CChatOpsCompress
			std::string logPath = GetOpenedDBFolderPath_utf8();
			logPath += "\\_log\\compress_summarize.txt";
			compressor._SetCompressSummarizeTip(bSuccess, _resultMessage, logPath);
		}
	}
	_status = TaskStatus::Success;
}

std::string CChatTask_CompressSummarize::_MakeCompressLogString(const std::string& originalContent, const std::string& compressedContent, int originalTokens, int compressedTokens)
{
	std::string logStr;
	
	// 标题信息
	logStr += "========================================\n";
	logStr += "[CompressSummarize] ";
	logStr += _isSessionMode ? "Session Mode" : "Single Op Mode";
	logStr += "\n\n";
	
	// 压缩结果摘要
	logStr += "=== Summary ===\n";
	logStr += "Original tokens: " + std::to_string(originalTokens) + "\n";
	logStr += "Compressed tokens: " + std::to_string(compressedTokens) + "\n";
	int reduced = originalTokens - compressedTokens;
	int percent = originalTokens > 0 ? (reduced * 100 / originalTokens) : 0;
	logStr += "Reduced: " + std::to_string(reduced) + " tokens (" + std::to_string(percent) + "%)\n";
	logStr += "Result: " + std::string(compressedTokens + 400 <= originalTokens ? "SUCCESS" : "SKIPPED (insufficient reduction)") + "\n";
	logStr += "\n";
	
	// 原始内容
	logStr += "=== Original Content ===\n";
	logStr += originalContent;
	logStr += "\n\n";
	
	// 压缩后内容
	logStr += "=== Compressed Content ===\n";
	logStr += compressedContent;
	logStr += "\n";
	
	return logStr;
}

std::string CChatTask_CompressSummarize::_MakeShortResultString(bool success, const std::string& reason, int originalTokens, int compressedTokens)
{
	std::string str;
	str += "[CompressSummarize] ";
	str += _isSessionMode ? "Session Mode - " : "Single Op Mode - ";
	
	if (success)
	{
		int reduced = originalTokens - compressedTokens;
		int percent = originalTokens > 0 ? (reduced * 100 / originalTokens) : 0;
		str += "Success. Reduced " + std::to_string(reduced) + " tokens (" + std::to_string(percent) + "%)";
	}
	else
	{
		str += "Failed";
		if (!reason.empty())
			str += ": " + reason;
	}
	
	return str;
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
// 		return true;
		return toolType == LlmToolType::ReplaceInFile ||
			   toolType == LlmToolType::CLI_Bash ||
			   toolType == LlmToolType::CLI_Cmd ||
			   toolType == LlmToolType::CLI_RunScript ||
			   toolType == LlmToolType::Question;
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
//				collectedContent += "[AI Message]\n";
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
		_Fail("No context or chat agent");
		return;
	}

	CChatOpsCompress& compressor = _context->chatAgent->GetCompressor();
	
	// 验证索引有效性
	if (_workingOpIndex < 0 || _workingOpIndex >= (int)compressor._workingOps.size())
	{
		_Fail("Invalid working op index");
		return;
	}

	std::string textToCompress;

	if (_isSessionMode)
	{
		// Session 模式：收集整个 session 的内容
		textToCompress = _CollectSessionContent();
		if (textToCompress.empty())
		{
			_Fail("Session content is empty");
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
			_Fail("Invalid source op index");
			return;
		}

		const ChatOp& srcOp = compressor._opsCtrl->GetOps()[srcIndex];
		textToCompress = srcOp.contentUtf8;
	}

	_originalTokenCount = Utils::EstimateTokenCount(textToCompress);
	_originalContent = textToCompress;

	if (textToCompress.empty())
	{
		_Fail("Content to compress is empty");
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
		prompt += textToCompress;
		
		request.AddUserMessage(prompt.c_str());
		request.isStreaming = true;

		if (!_llmChat->Request(request, setting))
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
						_Succeed(output.fullContent);
					}
					else
					{
						_Fail("Interrupted by user");
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


