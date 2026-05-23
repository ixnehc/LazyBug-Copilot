#include "stdh.h"

#include "ChatOpsCompress.h"
#include "ChatOpsCtrl.h"
#include "Utils.h"
#include "Utils_Context.h"
#include "ChatInputTag.h"
#include "LlmTools.h"


//////////////////////////////////////////////////////////////////////////
// CChatOpsCompress::Op

int CChatOpsCompress::Op::GetCurrentTokens() const
{
	// 根据当前压缩等级获取有效内容
	std::string effectiveContent;
	if (currentLevel == Level_None)
	{
		effectiveContent = originalContent;
	}
	else if (currentLevel > Level_None)
	{
		auto it = compressedContents.find(static_cast<int>(currentLevel));
		if (it != compressedContents.end())
			effectiveContent = it->second;
		// Level_Full 时可能没有内容，effectiveContent 为空
	}

	// 根据类型估算 token
	switch (type)
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


//////////////////////////////////////////////////////////////////////////
// CChatOpsCompress

CChatOpsCompress::CChatOpsCompress()
{
}

CChatOpsCompress::~CChatOpsCompress()
{
}

void CChatOpsCompress::Init(CChatOpsCtrl* opsCtrl)
{
	_opsCtrl = opsCtrl;
}

void CChatOpsCompress::StartCompress(int reduceTokenCount)
{
	if (!_opsCtrl || reduceTokenCount <= 0)
	{
		_state = State_Failed;
		return;
	}

	_reduceTokenCount = reduceTokenCount;
	_reducedTokens = 0;
	_currentPass = 0;
	_state = State_Compressing;

	// 构建工作 Op 数组
	_BuildWorkingOps();

	// 检查是否有可压缩的内容
	if (_workingOps.empty())
	{
		_state = State_Finished;
		return;
	}
}

void CChatOpsCompress::Update()
{
	if (_state != State_Compressing)
		return;

	_updateStartTime = GetAbsTick();

	// 持续执行 Pass 直到超时、达标或完成
	while (_currentPass < _passCount)
	{
		// 检查是否已达到目标
		if (_reducedTokens >= _reduceTokenCount)
			break;

		// 执行当前 Pass
		_ExecutePass(_currentPass);

		// Pass 执行完毕后检查是否超时
		if (_IsUpdateTimeout())
			break;

		// 进入下一个 Pass
		_currentPass++;
	}

	// 所有 Pass 完成或达标或超时
	if (_currentPass >= _passCount || _reducedTokens >= _reduceTokenCount)
	{
		// 同步回原数组
		_SyncBackToOps();
		_state = State_Finished;
	}
}

void CChatOpsCompress::Cancel()
{
	if (_state == State_Idle || _state == State_Finished)
		return;

	_state = State_Idle;
	_workingOps.clear();
}

void CChatOpsCompress::DecompressAll()
{
	if (!_opsCtrl)
		return;

	for (auto& op : _opsCtrl->_ops)
	{
		op.currentCompressionLevel = 0;
	}
	_state = State_Idle;
}


void CChatOpsCompress::_BuildWorkingOps()
{
	_workingOps.clear();

	if (!_opsCtrl || _opsCtrl->_ops.empty())
		return;

	// 获取当前 Session 的起始索引
	int currentSessionBegin = _opsCtrl->_GetLastNotDisabledSessionBegin();
	if (currentSessionBegin < 0)
		currentSessionBegin = static_cast<int>(_opsCtrl->_ops.size());

	// 计算每个 Op 的 sessionAge
	// 遍历找出所有 session 边界
	std::vector<int> sessionBoundaries; // 每个 session 的起始索引
	for (int i = 0; i < static_cast<int>(_opsCtrl->_ops.size()); i++)
	{
		if (_opsCtrl->_ops[i].type == ChatOp::Op_BeginSession)
			sessionBoundaries.push_back(i);
	}

	// 为每个 Op 分配 sessionAge
	int sessionCount = static_cast<int>(sessionBoundaries.size());
	int currentSessionIndex = sessionCount - 1; // 最后一个 session 的索引

	for (int i = 0; i < static_cast<int>(sessionBoundaries.size()); i++)
	{
		if (sessionBoundaries[i] >= currentSessionBegin)
		{
			currentSessionIndex = i;
			break;
		}
	}

	// 构建工作数组
	_workingOps.reserve(_opsCtrl->_ops.size());
	for (int i = 0; i < static_cast<int>(_opsCtrl->_ops.size()); i++)
	{
		const ChatOp& srcOp = _opsCtrl->_ops[i];

		Op workOp;
		workOp.type = srcOp.type;
		workOp.originalContent = srcOp.contentUtf8;
		workOp.initialTokens = _EstimateOpTokens(srcOp);
		workOp.currentLevel = static_cast<CompressLevel>(srcOp.currentCompressionLevel);
		workOp.compressedContents = srcOp.compressedContents;

		// 计算 sessionAge
		int opSessionIndex = -1;
		for (int j = static_cast<int>(sessionBoundaries.size()) - 1; j >= 0; j--)
		{
			if (i >= sessionBoundaries[j])
			{
				opSessionIndex = j;
				break;
			}
		}

		if (opSessionIndex >= 0)
			workOp.sessionAge = currentSessionIndex - opSessionIndex;
		else
			workOp.sessionAge = 999; // 不在任何 session 中，设为很大

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
	if (!_opsCtrl || _workingOps.size() != _opsCtrl->_ops.size())
		return;

	for (size_t i = 0; i < _workingOps.size(); i++)
	{
		ChatOp& dstOp = _opsCtrl->_ops[i];
		const Op& workOp = _workingOps[i];

		dstOp.currentCompressionLevel = static_cast<int>(workOp.currentLevel);
		dstOp.compressedContents = workOp.compressedContents;
	}
	_opsCtrl->_ver++;
}

int CChatOpsCompress::_EstimateOpTokens(const ChatOp& op) const
{
	// 获取有效内容（考虑压缩级别）
	std::string effectiveContent = op.contentUtf8;
	if (op.currentCompressionLevel > 0)
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
	int tokensBefore = op.GetCurrentTokens();

	// 应用压缩
	op.currentLevel = level;
	if (!content.empty())
	{
		op.compressedContents[static_cast<int>(level)] = content;
	}

	// 计算压缩后的 token 数
	int tokensAfter = op.GetCurrentTokens();
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

bool CChatOpsCompress::_IsUpdateTimeout() const
{
	AbsTick elapsed = GetAbsTick() - _updateStartTime;
	return elapsed >= _updateTimeLimitMs;
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
			nlohmann::json parsed = nlohmann::json::parse(op.originalContent);

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
				_ApplyCompressToOp(op, Level_Full, "");
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
			nlohmann::json parsed = nlohmann::json::parse(op.originalContent);

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
				_ApplyCompressToOp(op, Level_Full, "");
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
			nlohmann::json parsed = nlohmann::json::parse(op.originalContent);
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
			_ApplyCompressToOp(_workingOps[info.index], Level_Full, "");
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

void CChatOpsCompress::_Pass_TruncateSearchResults(int startSessionAge, int endSessionAge)
{
	// TODO: 截断搜索结果
}

void CChatOpsCompress::_Pass_TruncateCmdResults(int startSessionAge, int endSessionAge)
{
	constexpr int MAX_LINES = 100;

	for (auto& op : _workingOps)
	{
		if (_reducedTokens >= _reduceTokenCount)
			return;

		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		if (op.type != ChatOp::Op_AddToolCallResult)
			continue;

		if (op.toolType != LlmToolType::CLI_Cmd &&
			op.toolType != LlmToolType::CLI_Bash &&
			op.toolType != LlmToolType::CLI_RunScript)
			continue;

		if (op.currentLevel != Level_None)
			continue;

		std::string truncated = _TruncateCmdResult(op.originalContent, MAX_LINES);

		if (truncated != op.originalContent)
		{
			_ApplyCompressToOp(op, Level_Partial, truncated);
		}
	}
}

void CChatOpsCompress::_Pass_ClearToolCallResult(int startSessionAge, int endSessionAge, const std::vector<LlmToolType>& toolTypes)
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

		if (op.currentLevel != Level_None)
			continue;

		_ApplyCompressToOp(op, Level_Full, "");
	}
}

void CChatOpsCompress::_Pass_ClearSearchOps(int startSessionAge, int endSessionAge)
{
	std::vector<LlmToolType> toolTypes = { LlmToolType::FindInFiles, LlmToolType::SearchFile };
	_Pass_ClearToolCallResult(startSessionAge, endSessionAge, toolTypes);
}

void CChatOpsCompress::_Pass_ClearFindSymbol(int startSessionAge, int endSessionAge)
{
	std::vector<LlmToolType> toolTypes = { LlmToolType::FindSymbolDefine };
	_Pass_ClearToolCallResult(startSessionAge, endSessionAge, toolTypes);
}


void CChatOpsCompress::_Pass_ClearToolCalls(int startSessionAge, int endSessionAge)
{
	// 清除所有 Op_AddToolCallResult 类型（不限 toolType）
	for (auto& op : _workingOps)
	{
		if (_reducedTokens >= _reduceTokenCount)
			return;

		if (op.sessionAge < startSessionAge || op.sessionAge > endSessionAge)
			continue;

		if (op.type != ChatOp::Op_AddToolCallResult)
			continue;

		if (op.currentLevel != Level_None)
			continue;

		_ApplyCompressToOp(op, Level_Full, "");
	}
}

void CChatOpsCompress::_Pass_ClearMessages(int startSessionAge, int endSessionAge)
{
	// TODO: 清除消息
}


void CChatOpsCompress::_ExecutePass(int pass)
{
	// 按列表顺序依次对应 pass 序号，可通过参数控制作用范围
	int currentPassIndex = 0;

#define _PASS(expr) \
	if (pass == currentPassIndex) { expr; return; } \
	currentPassIndex++;

	_PASS( _Pass_RemoveFailureFileEdit(0, 999) )     // 删除失败的 File Edit（所有 session）
	_PASS( _Pass_RemoveFailureCMD(0, 999) )          // 删除失败或空的 CLI（所有 session）
	_PASS( _Pass_RemoveCoveredLines(0, 999) )        // 删除被后续 ReadFile 覆盖的行（sessionAge >= 1）
//	_PASS( _Pass_RemoveIrrelavantSearchResult(1, 999) ) // 删除不相关的搜索结果（sessionAge >= 1）
	_PASS( _Pass_ClearThinking(1, 999) )             // 清除思考过程（sessionAge >= 1）
//	_PASS( _Pass_TruncateSearchResults(1, 999) )     // 截断搜索结果（sessionAge >= 1）
	_PASS( _Pass_ClearSearchOps(3, 999) )            // 清除搜索操作（sessionAge >= 3）
	_PASS(_Pass_ClearFindSymbol(3, 999))            // 清除搜索操作（sessionAge >= 3）
	_PASS( _Pass_TruncateCmdResults(3, 999) )        // 截断命令结果（sessionAge >= 1）
	_PASS( _Pass_ClearSearchOps(2, 999) )            // 清除搜索操作（sessionAge >= 3）
	_PASS(_Pass_ClearFindSymbol(2, 999))            // 清除搜索操作（sessionAge >= 3）
	_PASS( _Pass_TruncateCmdResults(2, 999) )        // 截断命令结果（sessionAge >= 1）

// 	_PASS( _Pass_ClearToolCalls(2, 999) )            // 清除工具调用（sessionAge >= 2）
// 	_PASS( _Pass_ClearMessages(3, 999) )             // 清除消息（sessionAge >= 3）

#undef _PASS
}
