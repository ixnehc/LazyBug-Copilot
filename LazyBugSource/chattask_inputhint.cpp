#include "stdh.h"
#include "ChatTask_InputHint.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCtrl.h"
#include "ChatDialogA.h"
#include "utils_file.h"
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <cctype>


extern const char* GetOpenedDBFolderPath_utf8();

CChatTask_InputHint::CChatTask_InputHint(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect)
{
	_originalInputContent = Utils::BuildInputContent(content);
	_apiName = apiName;
	_hasStartedRequest = false;
	_requestInterrupt = false;
	_anchorRect = anchorRect;

	_checkCompleteStarted = false;
	_inputHintFinished = false;
	_checkCompleteFinished = false;
	_isInputComplete = false;
	_hintValid = false;


	// 将光标的 token 位置转换为 plainContent 中的字符位置
	// token 规则: 普通字符 = 1 token, 每个 tag = 1 token(与 CChatInput 的编号一致)
	_caretPlainPos = -1;
	if (caretTokenPos >= 0)
	{
		const auto& plain = _originalInputContent.plainContent;
		const auto& segs = _originalInputContent.tagSegments;
		size_t pos = 0;
		size_t segIdx = 0;
		int tokenIdx = 0;
		while (pos <= plain.size())
		{
			if (tokenIdx == caretTokenPos)
			{
				_caretPlainPos = (int)pos;
				break;
			}
			if (pos >= plain.size())
				break;

			// 当前位置是否落在某个 tag 区间的起点
			if (segIdx < segs.size() && pos == segs[segIdx].startPos)
			{
				pos = segs[segIdx].endPos;  // 整个 tag 前进(1 token)
				segIdx++;
			}
			else
			{
				pos++;  // 普通字符(1 token)
			}
			tokenIdx++;
		}
	}
}


void CChatTask_InputHint::_Fail(const std::string& reason)
{
	if (_context && _context->chatDialogA)
		_context->chatDialogA->HideHint();
	_status = TaskStatus::Failure;
}



std::string CChatTask_InputHint::_CollectChatContextFromOps()
{
	if (!_context || !_context->chatOpsCtrl)
		return "";

	const std::vector<ChatOp>& ops = _context->chatOpsCtrl->GetOps();

	// 从 disable 边界开始往前找（跳过被 disabled 的消息）
	int disableAfterIndex = _context->chatOpsCtrl->GetDisableAfterIndex();
	int startIdx = disableAfterIndex - 1;
	if (startIdx >= (int)ops.size())
		startIdx = (int)ops.size() - 1;

	std::string result;
	const int MAX_LEN = 8000;
	std::unordered_set<std::wstring> seenStreamingIds;

	for (int i = startIdx; i >= 0; i--)
	{
		const ChatOp& op = ops[i];
		std::string prefix;
		const std::string* pContent = nullptr;

		if (op.type == ChatOp::Op_AddUserMessage)
		{
			prefix = "User: ";
			pContent = &op.contentUtf8;
		}
		else if (op.type == ChatOp::Op_AddStreamingAIMessage)
		{
			if (op.contentUtf8.empty())
				continue;
			if (!seenStreamingIds.insert(op.messageId).second)
				continue;
			prefix = "Assistant: ";
			pContent = &op.contentUtf8;
		}
		else
		{
			continue;
		}

		std::string entry = prefix + *pContent;
		if (!result.empty())
			entry += "\n";

		// 超出总量限制则停止收集，但不会截断单条消息
		if ((int)(entry.size() + result.size()) > MAX_LEN)
			break;

		result = entry + result;
	}

	return result;
}

void CChatTask_InputHint::Start()
{
	if (_apiName.empty())
	{
		_Fail("No API name");
		return;
	}

	if (_originalInputContent.plainContent.empty())
	{
		_Fail("Empty input");
		return;
	}

	// 在 task 内收集上下文
	_chatContext = _CollectChatContextFromOps();

	LlmSessionSetting setting;
	if (!g_llmLib.LoadLlmSetting(setting, _apiName, false, "chatrules_inputhint"))
	{
		_Fail("Failed to load LLM setting");
		return;
	}

	setting.api.tools.clear();

	LlmSessionRequest request;

	std::string userMsg;
	if (!_chatContext.empty())
	{
		userMsg += "Recent chat context:\n";
		userMsg += _chatContext;
		userMsg += "\n\n";
	}

	// 在光标位置插入光标符号(仅用于提示 LLM 补全位置, 不属于实际输入内容)
	std::wstring inputWithCaret = _originalInputContent.plainContent;
	if (_caretPlainPos >= 0 && _caretPlainPos <= (int)inputWithCaret.size())
	{
		inputWithCaret.insert((size_t)_caretPlainPos, L"\x2038");
	}
	_inputWithCaret = inputWithCaret;

	userMsg += "User's partial input:\n";
	userMsg += widechar_to_utf8(inputWithCaret.c_str());
// 	userMsg += "\n\n";
// 	userMsg += "Completion:";

	request.AddUserMessage(userMsg.c_str());
	request.isStreaming = true;

	if (!_llmChats[0]->Request(request, setting))
	{
		_Fail("Failed to send LLM request");
		return;
	}

	_hasStartedRequest = true;

	// 同时(无先后)发送一个独立的 checkcomplete 请求, 判断输入是否语法完整
	// 使用 _llmChats[1]
	if (_llmChats.size() >= 2)
	{
		LlmSessionSetting ccSetting;
		if (g_llmLib.LoadLlmSetting(ccSetting, _apiName, false, "chatrules_checkcomplete"))
		{
			ccSetting.api.tools.clear();

			LlmSessionRequest ccRequest;
			// checkcomplete 只判断用户当前输入的完整性, 用原始纯文本(不含光标标记)
			std::string ccMsg = widechar_to_utf8(_originalInputContent.plainContent.c_str());
			ccRequest.AddUserMessage(ccMsg.c_str());
			ccRequest.isStreaming = true;

			if (_llmChats[1]->Request(ccRequest, ccSetting))
				_checkCompleteStarted = true;
		}
	}

	// 若 checkcomplete 请求未能启动, 视为已完成(不阻塞最终决定)
	if (!_checkCompleteStarted)
	{
		_checkCompleteFinished = true;
		_isInputComplete = false;
	}

	_status = TaskStatus::Running;
}


void CChatTask_InputHint::Update()
{
	if (_status != TaskStatus::Running)
		return;

	// 两个请求并行处理, 无先后
	_ProcessInputHintSession();
	_ProcessCheckCompleteSession();

	// 两个请求都完成后再统一决定显示/隐藏
	_TryFinalize();
}

void CChatTask_InputHint::_ProcessInputHintSession()
{
	if (_inputHintFinished)
		return;

	if (_llmChats.empty())
	{
		_inputHintFinished = true;
		return;
	}

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
		_Fail(output.errorMessage);
		return;
	}

	_resultText = output.fullContent;

	size_t start = _resultText.find_first_not_of(" \t\r\n");
	size_t end = _resultText.find_last_not_of(" \t\r\n");
	if (start != std::string::npos && end != std::string::npos)
		_resultText = _resultText.substr(start, end - start + 1);
	else
		_resultText.clear();

	if (!_resultText.empty())
	{
		// 解析 LLM 返回: old~~||~~new
		const std::string separator = "~~||~~";
		size_t sepPos = _resultText.find(separator);
		if (sepPos != std::string::npos)
		{
			std::string oldUtf8 = _resultText.substr(0, sepPos);
			std::string newUtf8 = _resultText.substr(sepPos + separator.size());

			std::wstring oldW = utf8_to_widechar(oldUtf8);
			std::wstring newW = utf8_to_widechar(newUtf8);

			// 校验补全结果的合理性: InputHint 只做简短续写,
			// 拒绝把输入当成"问题"去长篇回答的异常结果
			if (Utils::IsValidCompletion(oldW, newW))
			{
				// 修复 LLM 未纳入光标后内容导致的拼接重复
				Utils::FixDuplicationAtJoin(_originalInputContent.plainContent, oldW, newW);

				// 基于原始 InputContent 拷贝后用 ReplaceInputContent 应用替换
				_newInputContent = _originalInputContent;
				bool replaced = Utils::ReplaceInputContent(_newInputContent, oldW, newW);

				// 如果 oldW 以 "<<Old Content>>" 开头，去掉后再尝试一次
				if (!replaced)
				{
					const std::wstring oldPrefix = L"<<Old Content>>";
					if (oldW.size() > oldPrefix.size() &&
						oldW.compare(0, oldPrefix.size(), oldPrefix) == 0)
					{
						std::wstring strippedOld = oldW.substr(oldPrefix.size());
						_newInputContent = _originalInputContent;
						Utils::FixDuplicationAtJoin(_originalInputContent.plainContent, strippedOld, newW);
						replaced = Utils::ReplaceInputContent(_newInputContent, strippedOld, newW);
					}
				}

				// 替换失败(可能在 tag 内部), 退化为纯文本构建
				if (!replaced)
				{
					_newInputContent.plainContent = newW;
					_newInputContent.tagSegments.clear();
				}

				// 计算 diff 并暂存, 等 checkcomplete 也完成后再决定是否显示
				Utils::DiffInputContent(_originalInputContent, _newInputContent, _pendingOldDiff, _pendingNewDiff);
				_hintValid = true;
			}
		}

	}

	_inputHintFinished = true;
}

void CChatTask_InputHint::_ProcessCheckCompleteSession()
{
	if (_checkCompleteFinished)
		return;

	if (!_checkCompleteStarted)
	{
		_checkCompleteFinished = true;
		return;
	}

	if (_llmChats.size() < 2 || !_llmChats[1]->HasActiveSession())
	{
		// 会话意外结束, 视为未知(按不完整处理, 不阻塞显示)
		_checkCompleteFinished = true;
		_isInputComplete = false;
		return;
	}

	LlmSessionOutput output;
	if (!_llmChats[1]->Process(output, _requestInterrupt))
		return;

	if (!output.isCompleted)
		return;

	if (!_requestInterrupt && !output.hasError)
	{
		// 解析结果: 包含 [complete] 视为完整; 否则(含 [incomplete] 或其它)视为不完整
		std::string result = output.fullContent;
		std::transform(result.begin(), result.end(), result.begin(),
			[](unsigned char c) { return (char)std::tolower(c); });
		_isInputComplete = (result.find("[complete]") != std::string::npos);
	}

	_checkCompleteFinished = true;
}

void CChatTask_InputHint::_TryFinalize()
{
	// task 已在处理中失败, 无需再决定
	if (_status != TaskStatus::Running)
		return;

	// 两个请求都完成后才决定
	if (!_inputHintFinished || !_checkCompleteFinished)
		return;

	if (_context && _context->chatDialogA)
	{
		// 输入语法完整时不显示补全; 否则若有有效补全则显示
		if (!_isInputComplete && _hintValid)
		{
			// 计算补全后光标应定位的 token 位置
			int applyCaretTokenPos = Utils::CalcApplyCaretPos(
				_originalInputContent.plainContent,
				_newInputContent.plainContent,
				_newInputContent,
				_caretPlainPos);
			_context->chatDialogA->ShowHint(_anchorRect, _pendingNewDiff, _pendingOldDiff, _newInputContent, applyCaretTokenPos);
		}
		else
			_context->chatDialogA->HideHint();
	}

	// 保存请求与结果到 recent.json（此时 inputhint 和 checkcomplete 结果均已就绪）
	if (!_requestInterrupt && !_resultText.empty())
	{
		const char* dbPath = GetOpenedDBFolderPath_utf8();
		std::string rawDir = std::string(dbPath) + "\\_log\\InputHint\\raw";
		Utils::EnsureFolder(rawDir.c_str());

		std::wstring wDir = utf8_to_widechar(rawDir);

		// 滚动保留最近 5 条记录: recent4.json → 删除, recent3.json → recent4.json, ...
		DeleteFileW((wDir + L"\\recent4.json").c_str());
		MoveFileW((wDir + L"\\recent3.json").c_str(), (wDir + L"\\recent4.json").c_str());
		MoveFileW((wDir + L"\\recent2.json").c_str(), (wDir + L"\\recent3.json").c_str());
		MoveFileW((wDir + L"\\recent1.json").c_str(), (wDir + L"\\recent2.json").c_str());
		MoveFileW((wDir + L"\\recent.json").c_str(), (wDir + L"\\recent1.json").c_str());

		std::wstring filename = wDir + L"\\recent.json";

		// 确定 checkComplete 状态
		const char* checkCompleteState = "incomplete";
		if (!_checkCompleteStarted)
			checkCompleteState = "blocked";
		else if (_isInputComplete)
			checkCompleteState = "complete";

		nlohmann::ordered_json j;
		j["context"] = _chatContext;
		j["originalContent"] = widechar_to_utf8(_inputWithCaret.c_str());
		j["originalRaw"] = _resultText;
		j["originalResult"] = widechar_to_utf8(_newInputContent.plainContent.c_str());
		j["checkComplete"] = checkCompleteState;

		std::ofstream ofs(filename);
		if (ofs.is_open())
		{
			ofs << j.dump(2);
			ofs.close();
		}
	}

	_status = TaskStatus::Success;
}

void CChatTask_InputHint::Interrupt()
{
	_requestInterrupt = true;
	Update();
}

