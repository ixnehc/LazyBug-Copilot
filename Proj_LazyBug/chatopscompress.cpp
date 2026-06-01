#include "stdh.h"

#include "ChatOpsCompress.h"
#include "ChatOpsCtrl.h"
#include "Utils.h"
#include "Utils_Context.h"
#include "ChatInputTag.h"
#include "LlmTools.h"
#include "LlmLib.h"
#include "TokenCalibrate.h"
#include "Registry/Registry.h"

extern CLlmLib g_llmLib;
extern CCurrentUserRegistry g_reg;


//////////////////////////////////////////////////////////////////////////
// CChatOpsCompress - static functions

ChatOpCompressIntensity CChatOpsCompress::LoadIntensityForCurrentApi()
{
	std::string apiName = g_llmLib.GetMajorChatApi();
	if (apiName.empty())
		return ChatOpCompressIntensity::Low;

	int value = g_reg.ReadInt("CompressIntensities", apiName.c_str(), static_cast<int>(ChatOpCompressIntensity::Extreme));
	return static_cast<ChatOpCompressIntensity>(value);
}

void CChatOpsCompress::SaveIntensityForCurrentApi(ChatOpCompressIntensity intensity)
{
	std::string apiName = g_llmLib.GetMajorChatApi();
	if (apiName.empty())
		return;

	g_reg.WriteInt("CompressIntensities", apiName.c_str(), static_cast<int>(intensity));
}


//////////////////////////////////////////////////////////////////////////
// CChatOpsCompress::Op

const ChatOp& CChatOpsCompress::_GetSrcOp(const Op& op) const
{
	static const ChatOp s_empty;
	if (!_opsCtrl || op.srcIndex < 0 || op.srcIndex >= (int)_opsCtrl->_ops.size())
		return s_empty;
	return _opsCtrl->_ops[op.srcIndex];
}

int CChatOpsCompress::_GetOpCurrentTokens(const Op& op) const
{
	// 根据当前压缩等级获取有效内容
	const ChatOp& srcOp = _GetSrcOp(op);
	static const std::string s_empty;
	const std::string* effectiveContent = &s_empty;
	if (op.currentLevel == Level_None)
	{
		effectiveContent = &srcOp.contentUtf8;
	}
	else if (op.currentLevel > Level_None)
	{
		// 优先查本次新增的压缩内容
		auto it = op.newCompressedContents.find(static_cast<int>(op.currentLevel));
		if (it != op.newCompressedContents.end())
		{
			effectiveContent = &it->second;
		}
		else
		{
			// 再查原始 ChatOp 中已有的历史压缩内容
			auto it2 = srcOp.compressedContents.find(static_cast<int>(op.currentLevel));
			if (it2 != srcOp.compressedContents.end())
				effectiveContent = &it2->second;
			// Level_Full / Level_Remove 时可能没有内容，effectiveContent 指向空串
		}
	}

	// 根据类型估算 token
	switch (op.type)
	{
	case ChatOp::Op_AddUserMessage:
	{
		std::string plainText = ExtractPlainTextUtf8(*effectiveContent);
		return Utils::EstimateTokenCount(plainText);
	}
	case ChatOp::Op_AddStreamingAIMessage:
	case ChatOp::Op_AddStreamingAIMessage_Thinking:
	case ChatOp::Op_AddToolCallResult:
		return Utils::EstimateTokenCount(*effectiveContent);
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
// CChatOpsCompress

CChatOpsCompress::CChatOpsCompress()
{
	Zero();
}

CChatOpsCompress::~CChatOpsCompress()
{
}

void CChatOpsCompress::Init(CChatOpsCtrl* opsCtrl, CChatAgent* chatAgent)
{
	_opsCtrl = opsCtrl;

	ChatTaskContext ctx;
	ctx.chatOpsCtrl = opsCtrl;
	ctx.chatAgent = chatAgent;
	_taskMgr.Init(ctx);

}

void CChatOpsCompress::Clear()
{
	_CancelCompress();
	_env.Clear();
	Zero();
}

void CChatOpsCompress::_CollectEnv(Env& env)
{
	env.intensity = LoadIntensityForCurrentApi();
	env.tokenCalibrate = CTokenCalibrate::GetCalibrationFactor();
	env.opsVer = _opsCtrl->GetVer();
	env.isValid = true;
}


void CChatOpsCompress::_StartCompress(int reduceTokenCount, bool allowSummarize)
{
	if (!_opsCtrl || reduceTokenCount <= 0)
	{
		_state = State_Idle;
		return;
	}

	_reduceTokenCount = reduceTokenCount;
	_reducedTokens = 0;
	_currentPass = 0;
	_state = State_Compressing;
	_allowSummarize = allowSummarize;
	_summarized.clear();

	// 构建工作 Op 数组
	_BuildWorkingOps();

	// 检查是否有可压缩的内容
	if (_workingOps.empty()) 
	{
		_state = State_Idle;
		return;
	}

	_CollectEnv(_workingEnv);
}

bool CChatOpsCompress::IsSummarizing()
{
	if (IsCompressing() && _taskMgr.IsRunning())
		return true;
	return false;
}


void CChatOpsCompress::UpdateCompressTriggering()
{
	if (IsCompressing())
		return;

	Env env;
	_CollectEnv(env);
	if (env.Equals(_env))
		return;

	bool forceRecompress = false;
	if (env.intensity != _env.intensity)
		forceRecompress = true;

	_TryTrigger(false, forceRecompress);
	_UpdateCompress();//立即更新一次

	_CollectEnv(_env);
}


void CChatOpsCompress::UpdateCompress()
{
	_taskMgr.Update();
	if (_taskMgr.IsRunning())
		return;

	_UpdateCompress();
}


void CChatOpsCompress::_UpdateCompress()
{
	if (_state != State_Compressing)
		return;

	_compressStartTime = GetAbsTick();

	// 持续执行 Pass 直到超时、达标或完成
	while (_currentPass < _passCount)
	{
		// 检查是否已达到目标
		if (_reducedTokens >= _reduceTokenCount)
			break;

		// 执行当前 Pass
		_ExecutePass(_currentPass);

		if (_taskMgr.IsRunning())
			return;

		// Pass 执行完毕后检查是否超时
		if (_IsCompressTimeout())
			break;

		// 进入下一个 Pass
		_currentPass++;
	}

	// 所有 Pass 完成或达标或超时
	if (_currentPass >= _passCount || _reducedTokens >= _reduceTokenCount)
	{
		// 同步回原数组
		_SyncBackToOps();
		_state = State_Idle;
		_env = _workingEnv;
		_env.opsVer = _opsCtrl->GetVer();
		_workingEnv.Clear();
		_summarized.clear();
	}
}

void CChatOpsCompress::_CancelCompress()
{
	if (_state == State_Idle )
		return;

	_taskMgr.Shutdown();

	_state = State_Idle;
	_workingOps.clear();
	_workingEnv.Clear();
	_summarized.clear();
}

void CChatOpsCompress::_DecompressAll()
{
	if (!_opsCtrl)
		return;

	for (auto& op : _opsCtrl->_ops)
	{
		op.currentCompressionLevel = 0;
	}

	if (!_opsCtrl->_ops.empty())
		_opsCtrl->_ver++;
}


void CChatOpsCompress::_BuildWorkingOps()
{
	_workingOps.clear();

	if (!_opsCtrl || _opsCtrl->_ops.empty())
		return;

	// 获取 disable 边界，忽略 disabled 的 op
	int disableAfterIndex = _opsCtrl->_GetDisableAfterIndex();

	// 计算每个 Op 的 sessionAge
	// 基于 Op_EndSession 划分：最后一个 EndSession 之前的 op age=1，倒数第二 age=2，以此类推
	std::vector<int> endSessionIndices; // 所有 Op_EndSession 的索引（忽略 disabled）
	for (int i = 0; i < disableAfterIndex; i++)
	{
		if (_opsCtrl->_ops[i].type == ChatOp::Op_EndSession)
			endSessionIndices.push_back(i);
	}

	// 构建工作数组（忽略 disabled 的 op）
	_workingOps.reserve(disableAfterIndex);
	for (int i = 0; i < disableAfterIndex; i++)
	{
		const ChatOp& srcOp = _opsCtrl->_ops[i];

		Op workOp;
		workOp.type = srcOp.type;
		workOp.srcIndex = i;
		workOp.initialTokens = _EstimateOpTokens(srcOp,true);
		workOp.currentLevel = static_cast<CompressLevel>(0);

		// 计算 sessionAge：基于 Op_EndSession
		// 最后一个 EndSession 之前的 op age=1，倒数第二 age=2，以此类推
		int endCount = static_cast<int>(endSessionIndices.size());
		int age = 0; // 默认值：在所有 EndSession 之后（当前进行中的 session）
		for (int j = endCount - 1; j >= 0; j--)
		{
			if (i <= endSessionIndices[j])
			{
				age = endCount - j; // 倒数第 (endCount-j) 个 EndSession
			}
		}
		workOp.sessionAge = age;

		// 解析 ToolCall 类型
		if (srcOp.type == ChatOp::Op_AddToolCallResult)
		{
			// 从 content 中解析 tool type
			// ToolCallResult 格式: [assistant_msg, tool_result_msg]
			// 需要解析出 tool name 然后映射到 LlmToolType
			workOp.toolType = CLlmTools::ParseToolTypeFromToolCallResultString(srcOp.contentUtf8);
		}

		_workingOps.push_back(workOp);
	}
}


void CChatOpsCompress::_SyncBackToOps()
{
	if (!_opsCtrl)
		return;

	if (_workingOps.size()<=0)
		return;

	// 获取 disable 边界，只同步未被 disabled 的 op
	int disableAfterIndex = _opsCtrl->_GetDisableAfterIndex();
	if (static_cast<int>(_workingOps.size()) != disableAfterIndex)
		return;

	for (int i = 0; i < disableAfterIndex; i++)
	{
		ChatOp& dstOp = _opsCtrl->_ops[i];
		const Op& workOp = _workingOps[i];

		dstOp.currentCompressionLevel = static_cast<int>(workOp.currentLevel);
		for (const auto& kv : workOp.newCompressedContents)
			dstOp.compressedContents.insert_or_assign(kv.first, kv.second);
	}
	_opsCtrl->_ver++;
}

int CChatOpsCompress::_EstimateOpTokens(const ChatOp& op, bool useUncompressed) const
{
	// 获取有效内容（考虑压缩级别）
	std::string effectiveContent = op.contentUtf8;
	if (!useUncompressed && op.currentCompressionLevel > 0)
	{
		auto it = op.compressedContents.find(op.currentCompressionLevel);
		if (it != op.compressedContents.end())
			effectiveContent = it->second;
	}

	switch (op.type)
	{
	case ChatOp::Op_AddUserMessage:
	{
		std::string plainText = ExtractPlainTextUtf8(effectiveContent);
		return Utils::EstimateTokenCount(plainText);
	}
	case ChatOp::Op_AddStreamingAIMessage:
	case ChatOp::Op_AddStreamingAIMessage_Thinking:
	case ChatOp::Op_AddToolCallResult:
		return Utils::EstimateTokenCount(effectiveContent);
	default:
		return 0;
	}
}

int CChatOpsCompress::_ApplyCompressToOp(Op& op, CompressLevel level, const std::string& content)
{
	// 计算压缩前的 token 数
	int tokensBefore = _GetOpCurrentTokens(op);

	// 应用压缩
	op.currentLevel = level;
	if (!content.empty())
	{
		op.newCompressedContents[static_cast<int>(level)] = content;
	}

	// 计算压缩后的 token 数
	int tokensAfter = _GetOpCurrentTokens(op);
	int reduced = tokensBefore - tokensAfter;

	// 累加到总减少量
	_reducedTokens += reduced;

	return reduced;
}

std::wstring CChatOpsCompress::_TruncateSearchResult(const std::wstring& content, int maxLines)
{
	try
	{
		std::string utf8Content = widechar_to_utf8(content.c_str());
		nlohmann::json parsed = nlohmann::json::parse(utf8Content);

		if (!parsed.is_array() || parsed.size() < 2)
			return content;

		auto& toolResultMsg = parsed[1];
		if (!toolResultMsg.contains("content"))
			return content;

		std::string resultContent = toolResultMsg["content"].get<std::string>();

		// 按行截断
		std::vector<std::string> lines;
		size_t pos = 0, prev = 0;
		while ((pos = resultContent.find('\n', prev)) != std::string::npos)
		{
			lines.push_back(resultContent.substr(prev, pos - prev));
			prev = pos + 1;
		}
		if (prev < resultContent.length())
			lines.push_back(resultContent.substr(prev));

		if (static_cast<int>(lines.size()) <= maxLines)
			return content;

		// 截断
		std::string truncated;
		for (int i = 0; i < maxLines; i++)
		{
			truncated += lines[i] + "\n";
		}
		truncated += "\n... [output truncated, showing first " + std::to_string(maxLines) + " lines]";

		toolResultMsg["content"] = truncated;
		return utf8_to_widechar(parsed.dump().c_str());
	}
	catch (...)
	{
		return content;
	}
}

std::string CChatOpsCompress::_TruncateCmdResult(const std::string& content, int maxLines)
{
	try
	{
		nlohmann::json parsed = nlohmann::json::parse(content);

		if (!parsed.is_array() || parsed.size() < 2)
			return content;

		auto& toolResultMsg = parsed[1];
		if (!toolResultMsg.contains("content"))
			return content;

		std::string resultContent = toolResultMsg["content"].get<std::string>();

		// 按行截断（保留尾部）
		std::vector<std::string> lines;
		size_t pos = 0, prev = 0;
		while ((pos = resultContent.find('\n', prev)) != std::string::npos)
		{
			lines.push_back(resultContent.substr(prev, pos - prev));
			prev = pos + 1;
		}
		if (prev < resultContent.length())
			lines.push_back(resultContent.substr(prev));

		if (static_cast<int>(lines.size()) <= maxLines)
			return content;

		// 截断（保留尾部）
		std::string truncated;
		truncated += "... [output truncated, showing last " + std::to_string(maxLines) + " lines]\n";
		for (int i = static_cast<int>(lines.size()) - maxLines; i < static_cast<int>(lines.size()); i++)
		{
			truncated += lines[i] + "\n";
		}

		toolResultMsg["content"] = truncated;
		return parsed.dump();
	}
	catch (...)
	{
		return content;
	}
}

bool CChatOpsCompress::_IsCompressTimeout() const
{
	AbsTick elapsed = GetAbsTick() - _compressStartTime;
	return elapsed >= _compressTimeLimitMs;
}




//////////////////////////////////////////////////////////////////////////
// Pass 实现

void CChatOpsCompress::_Pass_RemoveFailureFileEdit(int startSessionAge, int endSessionAge)
{
	const char* SUCCESS_MARKER = "Successfully made the replacement in the file!";

	for (auto& op : _workingOps)
	{
		// 检查是否已达到目标
		if (_reducedTokens >= _reduceTokenCount)
			return;

		// 检查 sessionAge 范围
		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		// 检查是否是 FileEdit 类型的 ToolCallResult
		if (op.type != ChatOp::Op_AddToolCallResult)
			continue;
		if (op.toolType != LlmToolType::ReplaceInFile)
			continue;
		if (op.currentLevel != Level_None)
			continue;  // 已压缩过

		// 解析 JSON 检查是否失败
		try
		{
			nlohmann::json parsed = nlohmann::json::parse(_GetSrcOp(op).contentUtf8);

			if (!parsed.is_array() || parsed.size() < 2)
				continue;

			// 获取 tool result 的 content
			auto& toolResultMsg = parsed[1];
			if (!toolResultMsg.contains("content"))
				continue;

			std::string resultContent = toolResultMsg["content"].get<std::string>();

			// 检查是否成功，不包含成功标记则视为失败
			if (resultContent.find(SUCCESS_MARKER) == std::string::npos)
			{
				// 失败的 FileEdit，完全清除
				_ApplyCompressToOp(op, Level_Remove, "");
			}
		}
		catch (...)
		{
			// JSON 解析失败，跳过
		}
	}
}

void CChatOpsCompress::_Pass_RemoveFailureCMD(int startSessionAge, int endSessionAge)
{
	for (auto& op : _workingOps)
	{
		// 检查是否已达到目标
		if (_reducedTokens >= _reduceTokenCount)
			return;

		// 检查 sessionAge 范围
		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		// 检查是否是 CLI 类型的 ToolCallResult
		if (op.type != ChatOp::Op_AddToolCallResult)
			continue;
		if (op.toolType != LlmToolType::CLI_Cmd &&
			op.toolType != LlmToolType::CLI_Bash &&
			op.toolType != LlmToolType::CLI_RunScript)
			continue;
		if (op.currentLevel != Level_None)
			continue;  // 已压缩过

		// 解析 JSON 检查是否失败
		try
		{
			nlohmann::json parsed = nlohmann::json::parse(_GetSrcOp(op).contentUtf8);

			if (!parsed.is_array() || parsed.size() < 2)
				continue;

			// 获取 tool result 的 content
			auto& toolResultMsg = parsed[1];
			if (!toolResultMsg.contains("content"))
				continue;

			std::string resultContent = toolResultMsg["content"].get<std::string>();


			// 检查是否失败：以 Error: 开头、包含 Command failed、或 Task interrupted
			bool isFailure = false;
			if (resultContent.find("Error:") == 0)
				isFailure = true;
			else if (resultContent.find("Command failed") == 0)
				isFailure = true;
			else if (resultContent.find("Task interrupted") == 0)
				isFailure = true;
			else if (resultContent.find("Command execution was rejected by user") == 0)
				isFailure = true;

			if (isFailure)
			{
				// 失败的 CLI，完全清除
				_ApplyCompressToOp(op, Level_Remove, "");
			}
		}
		catch (...)
		{
			// JSON 解析失败，跳过
		}
	}
}

void CChatOpsCompress::_Pass_RemoveCoveredLines(int startSessionAge, int endSessionAge)
{
	// 删除被后续 ReadFile 覆盖的行
	// 规则1: 如果一个 ReadFile 只读取了文件的部分行，但之后有一个 ReadFile 读取了整个文件，则清除
	// 规则2: 同一个 session 内，如果一个 ReadFile 的行范围被后续 ReadFile 的行范围包含，
	//        且两者之间没有 ReplaceInFile，则清除

	// 首先收集所有 ReadFile 的 Op 索引和信息
	struct ReadFileInfo
	{
		int index;              // 在 _workingOps 中的索引
		int sessionAge;         // session 年龄
		std::string filePath;   // 文件路径
		int startLine;          // 起始行（-1 表示未指定，从头开始）
		int endLine;            // 结束行（-1 表示未指定，读到末尾）
		bool isFullFile;        // 是否读取整个文件

		// 判断 this 的范围是否包含 other 的范围
		bool ContainsRange(int otherStart, int otherEnd) const
		{
			int thisStart = startLine;
			int thisEnd = endLine;  // -1 表示到末尾，能包含任何有限值

			if (otherStart < thisStart)
				return false;

			if (thisEnd < 0)
				return true;  // this 读到末尾，能包含任何 otherEnd

			if (otherEnd < 0)
				return false;  // other 读到末尾，this 无法包含

			return otherEnd <= thisEnd;
		}
	};
	std::vector<ReadFileInfo> readFileOps;

	for (int i = 0; i < static_cast<int>(_workingOps.size()); i++)
	{
		auto& op = _workingOps[i];

		// 检查 sessionAge 范围
		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		if (op.type != ChatOp::Op_AddToolCallResult)
			continue;
		if (op.toolType != LlmToolType::ReadFile)
			continue;
		if (op.currentLevel != Level_None)
			continue;

		// 解析 JSON 获取文件路径和行范围
		try
		{
			nlohmann::json parsed = nlohmann::json::parse(_GetSrcOp(op).contentUtf8);
			if (!parsed.is_array() || parsed.size() < 1)
				continue;

			auto& firstMsg = parsed[0];
			if (!firstMsg.contains("tool_calls") || !firstMsg["tool_calls"].is_array())
				continue;

			auto& toolCalls = firstMsg["tool_calls"];
			if (toolCalls.empty() || !toolCalls[0].contains("function"))
				continue;

			auto& func = toolCalls[0]["function"];
			if (!func.contains("arguments"))
				continue;

			// 解析 arguments JSON 字符串
			std::string argsStr = func["arguments"].get<std::string>();
			nlohmann::json args = nlohmann::json::parse(argsStr);

			if (!args.contains("filePath"))
				continue;

			std::string filePath = args["filePath"].get<std::string>();
			StringLower(filePath);

			// 解析行范围
			int startLine = -1;  // -1 表示未指定，默认为 1
			int endLine = -1;    // -1 表示未指定，读到末尾

			if (args.contains("startLine") && args["startLine"].is_number())
				startLine = args["startLine"].get<int>();

			if (args.contains("endLine") && args["endLine"].is_number())
				endLine = args["endLine"].get<int>();

			// 判断是否读取整个文件
			bool isFullFile = (!args.contains("startLine") && !args.contains("endLine"));

			ReadFileInfo info;
			info.index = i;
			info.sessionAge = op.sessionAge;
			info.filePath = filePath;
			info.startLine = startLine;
			info.endLine = endLine;
			info.isFullFile = isFullFile;
			readFileOps.push_back(info);
		}
		catch (...)
		{
			// JSON 解析失败，跳过
		}
	}

	// 辅助函数：检查两个索引之间是否有 ReplaceInFile
	auto hasReplaceInFileBetween = [this](int fromIdx, int toIdx) -> bool
	{
		for (int i = fromIdx + 1; i < toIdx; i++)
		{
			if (_workingOps[i].toolType == LlmToolType::ReplaceInFile)
				return true;
		}
		return false;
	};

	// 对于每个 ReadFile Op，检查是否应该被清除
	for (const auto& info : readFileOps)
	{
		// 检查是否已达到目标
		if (_reducedTokens >= _reduceTokenCount)
			return;

		bool shouldRemove = false;

		// 检查之后的所有 ReadFile Op
		for (const auto& laterInfo : readFileOps)
		{
			// 必须在当前 Op 之后
			if (laterInfo.index <= info.index)
				continue;

			// 必须是同一个文件
			if (laterInfo.filePath != info.filePath)
				continue;

			// 规则1: 之后有读取整个文件的 Op
			if (laterInfo.isFullFile)
			{
				shouldRemove = true;
				break;
			}

			// 规则2: 同一个 session 内，行范围被包含，且中间没有 ReplaceInFile
			if (laterInfo.sessionAge == info.sessionAge)
			{
				if (laterInfo.ContainsRange(info.startLine, info.endLine))
				{
					if (!hasReplaceInFileBetween(info.index, laterInfo.index))
					{
						shouldRemove = true;
						break;
					}
				}
			}
		}

		if (shouldRemove)
		{
			_ApplyCompressToOp(_workingOps[info.index], Level_Remove, "");
		}
	}
}

void CChatOpsCompress::_Pass_RemoveIrrelavantSearchResult(int startSessionAge, int endSessionAge)
{
	// TODO: 删除不相关的搜索结果
}

void CChatOpsCompress::_Pass_ClearThinking(int startSessionAge, int endSessionAge)
{
	for (auto& op : _workingOps)
	{
		// 检查是否已达到目标
		if (_reducedTokens >= _reduceTokenCount)
			return;

		// 检查 sessionAge 范围
		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		// 检查是否是 Thinking 类型的 Op
		if (op.type != ChatOp::Op_AddStreamingAIMessage_Thinking)
			continue;
		if (op.currentLevel != Level_None)
			continue;  // 已压缩过

		// 完全清除 thinking 内容
		_ApplyCompressToOp(op, Level_Partial, ".");
	}
}

void CChatOpsCompress::_Pass_TruncateCmdResults(int startSessionAge, int endSessionAge)
{
	std::vector<LlmToolType> cliTypes = {
		LlmToolType::CLI_Cmd,
		LlmToolType::CLI_Bash,
		LlmToolType::CLI_RunScript
	};
	_Pass_TruncateToolCallResult(startSessionAge, endSessionAge, cliTypes);
}

void CChatOpsCompress::_Pass_TruncateToolCallResult(int startSessionAge, int endSessionAge, const std::vector<LlmToolType>& toolTypes, CompressLevel level)
{
	for (auto& op : _workingOps)
	{
		if (_reducedTokens >= _reduceTokenCount)
			return;

		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		if (op.type != ChatOp::Op_AddToolCallResult)
			continue;

		// 检查是否匹配任一 toolType
		bool matched = false;
		for (LlmToolType toolType : toolTypes)
		{
			if (op.toolType == toolType)
			{
				matched = true;
				break;
			}
		}
		if (!matched)
			continue;

		if (op.currentLevel >= level)
			continue;

		// 检查是否有预存的简化版内容
		// 优先查本次新增，再查原始 ChatOp 历史内容
		const std::string* partialContent = nullptr;
		auto it = op.newCompressedContents.find(static_cast<int>(level));
		if (it != op.newCompressedContents.end())
		{
			partialContent = &it->second;
		}
		else
		{
			const ChatOp& srcOp = _GetSrcOp(op);
			auto it2 = srcOp.compressedContents.find(static_cast<int>(level));
			if (it2 != srcOp.compressedContents.end())
				partialContent = &it2->second;
		}

		if (!partialContent || partialContent->empty() || *partialContent == _GetSrcOp(op).contentUtf8)
			continue;

		_ApplyCompressToOp(op, level, *partialContent);
	}
}

void CChatOpsCompress::_Pass_TruncateFindSymbol(int startSessionAge, int endSessionAge)
{
	_Pass_TruncateToolCallResult(startSessionAge, endSessionAge, { LlmToolType::FindSymbolDefine });
}

void CChatOpsCompress::_Pass_TruncateReadFile(int startSessionAge, int endSessionAge)
{
	_Pass_TruncateToolCallResult(startSessionAge, endSessionAge, { LlmToolType::ReadFile });
}

void CChatOpsCompress::_Pass_TruncateFindInFiles(int startSessionAge, int endSessionAge)
{
	_Pass_TruncateToolCallResult(startSessionAge, endSessionAge, { LlmToolType::FindInFiles });
}

void CChatOpsCompress::_Pass_TruncateReplaceInFile(int startSessionAge, int endSessionAge)
{
	_Pass_TruncateToolCallResult(startSessionAge, endSessionAge, { LlmToolType::ReplaceInFile });
}

void CChatOpsCompress::_Pass_ClearReplaceInFile(int startSessionAge, int endSessionAge)
{
	_Pass_TruncateToolCallResult(startSessionAge, endSessionAge, { LlmToolType::ReplaceInFile },CompressLevel::Level_Full);
}

void CChatOpsCompress::_Pass_RemoveToolCallResult(int startSessionAge, int endSessionAge, const std::vector<LlmToolType>& toolTypes)
{
	if (toolTypes.empty())
		return;

	for (auto& op : _workingOps)
	{
		if (_reducedTokens >= _reduceTokenCount)
			return;

		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		if (op.type != ChatOp::Op_AddToolCallResult)
			continue;

		// 检查 toolType 是否在指定列表中
		bool matched = false;
		for (const auto& toolType : toolTypes)
		{
			if (op.toolType == toolType)
			{
				matched = true;
				break;
			}
		}
		if (!matched)
			continue;

		if (op.currentLevel >= Level_Remove)
			continue;

		_ApplyCompressToOp(op, Level_Remove, "");
	}
}

void CChatOpsCompress::_Pass_RemoveSearchOps(int startSessionAge, int endSessionAge)
{
	std::vector<LlmToolType> toolTypes = { LlmToolType::FindInFiles, LlmToolType::SearchFile };
	_Pass_RemoveToolCallResult(startSessionAge, endSessionAge, toolTypes);
}

void CChatOpsCompress::_Pass_RemoveFindSymbol(int startSessionAge, int endSessionAge)
{
	std::vector<LlmToolType> toolTypes = { LlmToolType::FindSymbolDefine };
	_Pass_RemoveToolCallResult(startSessionAge, endSessionAge, toolTypes);
}


void CChatOpsCompress::_Pass_ClearMessages(int startSessionAge, int endSessionAge)
{
	// TODO: 清除消息
}

void CChatOpsCompress::_Pass_SummarizeMessage(int startSessionAge, int endSessionAge)
{
	for (size_t i = 0; i < _workingOps.size(); ++i)
	{
		Op& op = _workingOps[i];

		// 检查是否已达到目标
		if (_reducedTokens >= _reduceTokenCount)
			return;

		// 检查 sessionAge 范围
		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		// 仅处理 AI 消息
		if (op.type != ChatOp::Op_AddStreamingAIMessage)
			continue;
		if (op.currentLevel != Level_None)
			continue;  // 已压缩过

		// 查找已有的压缩内容（优先本次新增，再查原始 ChatOp 历史内容）
		const std::string* partialContent = nullptr;
		auto it = op.newCompressedContents.find(static_cast<int>(Level_Partial));
		if (it != op.newCompressedContents.end())
		{
			partialContent = &it->second;
		}
		else
		{
			const ChatOp& srcOp = _GetSrcOp(op);
			auto it2 = srcOp.compressedContents.find(static_cast<int>(Level_Partial));
			if (it2 != srcOp.compressedContents.end())
				partialContent = &it2->second;
		}

		// 如果已有压缩内容，则直接应用压缩
		if (partialContent && !partialContent->empty() && *partialContent != _GetSrcOp(op).contentUtf8)
		{
			std::wstring oldContent = utf8_to_widechar(_GetSrcOp(op).contentUtf8);
			std::wstring newContent = utf8_to_widechar(*partialContent);
			_ApplyCompressToOp(op, Level_Partial, "");
			continue;
		}

		if(!_allowSummarize)
			continue;

		// 否则：内容数据大于 50 字节才值得压缩，启动 task 进行压缩
		const ChatOp& srcOp = _GetSrcOp(op);
		if (srcOp.contentUtf8.size() <= 50)
			continue;

		// 每个 op 在一次 compress 过程中只尝试摘要一次
		if (_summarized.count(static_cast<int>(i)) > 0)
			continue;
		_summarized.insert(static_cast<int>(i));

		// 启动异步压缩 task（结果会写回 op.newCompressedContents，下次 pass 时应用）
		_taskMgr.AddTask_CompressSummarize(static_cast<int>(i));
	}
}


void CChatOpsCompress::_ExecutePass(int pass)
{
	// 按列表顺序依次对应 pass 序号，可通过参数控制作用范围
	int currentPassIndex = 0;

#define _PASS(expr) \
	if (pass == currentPassIndex) { expr; return; } \
	currentPassIndex++;

	// ============================================================
	// Pass 顺序原则：信息损失从小到大；同类操作 sessionAge 从大到小（先精简最旧的）
	// ============================================================

	// ---- 阶段1：清除无效/冗余内容（信息损失 ~0，本就不该保留）----
	_PASS(_Pass_RemoveFailureFileEdit(0, 999));   // 失败的文件编辑
	_PASS(_Pass_RemoveFailureCMD(0, 999));        // 失败的命令执行
	_PASS(_Pass_RemoveCoveredLines(0, 999));      // 被后续 ReadFile 覆盖的行

	// ---- 阶段2：清除思考过程（低损失，非最终结果）----
	_PASS(_Pass_ClearThinking(1, 999));

	// ---- 阶段3：摘要化 AI 消息（LLM 语义压缩，损失最小，优先于截断）----
	_PASS(_Pass_SummarizeMessage(4, 999));

	// ---- 阶段4：截断工具结果（中等损失，从最旧的 session 开始）----
	// 第一批：sessionAge >= 3（较旧）
	_PASS(_Pass_TruncateCmdResults(3, 999));
	_PASS(_Pass_TruncateFindSymbol(3, 999));
	_PASS(_Pass_TruncateFindInFiles(3, 999));
	_PASS(_Pass_TruncateReadFile(3, 999));
	_PASS(_Pass_TruncateReplaceInFile(3, 999));

	_PASS(_Pass_SummarizeMessage(3, 999));

	// 第二批：sessionAge >= 2（次新也开始截断）
	_PASS(_Pass_TruncateCmdResults(2, 999));
	_PASS(_Pass_TruncateFindSymbol(2, 999));
	_PASS(_Pass_TruncateFindInFiles(2, 999));
	_PASS(_Pass_TruncateReadFile(2, 999));
	_PASS(_Pass_TruncateReplaceInFile(2, 999));

	_PASS(_Pass_SummarizeMessage(2, 999));

	// ---- 阶段5：进一步清除/删除可恢复内容（高损失）----
	_PASS(_Pass_ClearReplaceInFile(3, 999));      // 完全清除文件替换结果
	_PASS(_Pass_RemoveFindSymbol(3, 999));        // 删除符号查找结果
	_PASS(_Pass_RemoveSearchOps(3, 999));         // 删除搜索结果

	// ---- 阶段6：Extreme 模式 - 激进精简（极高损失，含当前 session）----
	if (_workingEnv.intensity >= ChatOpCompressIntensity::Extreme)
	{
//		_PASS(_Pass_TruncateCmdResults(1, 999));
		_PASS(_Pass_TruncateFindSymbol(1, 999));
		_PASS(_Pass_TruncateFindInFiles(1, 999));
// 		_PASS(_Pass_TruncateReadFile(1, 999));
		_PASS(_Pass_TruncateReplaceInFile(1, 999));

		_PASS(_Pass_ClearReplaceInFile(2, 999));
		_PASS(_Pass_RemoveFindSymbol(2, 999));
		_PASS(_Pass_RemoveSearchOps(2, 999));
	}


#undef _PASS
}

  
bool CChatOpsCompress::_TryTrigger(bool allowSummarize,bool forceRecompress)
{
	if (!_opsCtrl)
		return false;

	if (IsCompressing())
		return false;

	Env env;
	_CollectEnv(env);

	// 根据 intensity 直接设置 threshold 和 targetTokens
	int threshold = 0;
	int targetTokens = 0;

	switch (env.intensity)
	{
	case ChatOpCompressIntensity::None:
		threshold = 500000;
		targetTokens = 200000;
		break;
	case ChatOpCompressIntensity::Low:
		threshold = 200000;
		targetTokens = 100000;
		break;
	case ChatOpCompressIntensity::Medium:
		threshold = 100000;
		targetTokens = 50000;
		break;
	case ChatOpCompressIntensity::High:
		threshold = 50000;
		targetTokens = 30000;
		break;
	case ChatOpCompressIntensity::Extreme:
		threshold = 30000;
		targetTokens = 10000;
		break;
	default: 
		return false;
	} 
	  
	if (threshold <= 0 || targetTokens <= 0)
		return false;

	if (env.tokenCalibrate <= 0.0f)
		return false;
	  
	int currentUncompressedTokens = (int)(((float)_opsCtrl->GetUncompressedEstimateTokens())*env.tokenCalibrate);

	if (currentUncompressedTokens <= threshold)
	{
		_DecompressAll();
		return false;
	}

	//如果当前的token数没有超过threshold,我们可以不压缩
	//除非强制要求重新压缩
	//注意:重新压缩会从未压缩数据基础上压缩到targetTokens,可能会比当前的token数多
	if (!forceRecompress)
	{
		int currentTokens = (int)(((float)_opsCtrl->GetEstimateTokens()) * env.tokenCalibrate);

		if (currentTokens < threshold * 9 / 10)
			return false;
	}

	int reduceTokens = currentUncompressedTokens - targetTokens;
	reduceTokens = (int)(((float)reduceTokens) / env.tokenCalibrate);

	if (reduceTokens <= 0)
		return false;

	_StartCompress(reduceTokens,allowSummarize);
	return true;
}

