#include "stdh.h"
#include "ChatTask_InputHint.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCtrl.h"
#include "ChatDialogA.h"
#include "utils_file.h"
#include "InputHintContext.h"
#include <fstream>
#include <algorithm>
#include <cctype>


extern const char* GetOpenedDBFolderPath_utf8();

const CChatTask_InputHint::InputHintFormat CChatTask_InputHint::kInputHintFormat =
	CChatTask_InputHint::InputHintFormat::Json;

CChatTask_InputHint::CChatTask_InputHint(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect, int contentVersion)
{
	_originalInputContent = Utils::BuildInputContent(content);
	_apiName = apiName;
	_hasStartedRequest = false;
	_requestInterrupt = false;
	_anchorRect = anchorRect;
	_contentVersion = contentVersion;

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

	// 从 InputHintContext 获取已维护好的聊天上下文与输入快照
	InputHintContext* ctx = (_context ? _context->inputHintCtx : nullptr);
	if (ctx)
	{
		_caretPlainPos = ctx->GetCaretPlainPos();
		_caretLine = ctx->GetCaretLine();
		_beforeCaretLines = ctx->GetBeforeCaretLines();
		_afterCaretLines = ctx->GetAfterCaretLines();
	}
	else
	{
		// 兜底: context 未接线时退回用原始输入生成光标行(含光标标记)
		_caretLine = _originalInputContent.plainContent;
		if (_caretPlainPos >= 0 && _caretPlainPos <= (int)_caretLine.size())
			_caretLine.insert((size_t)_caretPlainPos, L"\x2038");
	}

	// 拼接完整带光标标记的输入文本(用于日志/调试)
	_inputWithCaret.clear();
	if (!_beforeCaretLines.empty())
		_inputWithCaret += _beforeCaretLines + L"\n";
	_inputWithCaret += _caretLine;
	if (!_afterCaretLines.empty())
		_inputWithCaret += L"\n" + _afterCaretLines;

	// 同时(无先后)启动两个独立请求
	bool hintStarted = _StartInputHintSession();
	bool ccStarted   = _StartCheckCompleteSession();

	if (!hintStarted)
	{
		_Fail("Failed to send LLM request");
		return;
	}

	_hasStartedRequest = true;

	// 若 checkcomplete 请求未能启动, 视为已完成(不阻塞最终决定)
	if (!ccStarted)
	{
		_checkCompleteFinished = true;
		_isInputComplete = false;
	}

	_status = TaskStatus::Running;
}

bool CChatTask_InputHint::_StartInputHintSession()
{
	LlmSessionSetting setting;
	const char* ruleName = (kInputHintFormat == InputHintFormat::Json)
		? "chatrules_inputhint_json" : "chatrules_inputhint";
	if (!g_llmLib.LoadLlmSetting(setting, _apiName, false, ruleName))
		return false;

	setting.api.tools.clear();
	setting.api.thinkingMode = LlmThinkingMode::Disable;

	LlmSessionRequest request;

	std::string userMsg;
	const std::string* chatOpsContent = nullptr;
	if (_context && _context->inputHintCtx)
		chatOpsContent = &_context->inputHintCtx->GetChatOpsContent();

	if (chatOpsContent && !chatOpsContent->empty())
	{
		userMsg += "Recent chat context:\n";
		userMsg += *chatOpsContent;
		userMsg += "\n\n";
	}

	// 从 InputHintContext 读取合并后的相似代码上下文(输入 embedding + 历史 embedding)
	std::string similarChunksText;
	if (_context && _context->inputHintCtx)
		similarChunksText = _context->inputHintCtx->GetMergedSimilarChunksText();

	if (!similarChunksText.empty())
	{
		userMsg += "Relevant code context:\n";
		userMsg += similarChunksText;
		userMsg += "\n";
	}

	// 使用 InputHintContext 维护好的三部分(光标行 + 光标前行 + 光标后行)
	if (!_beforeCaretLines.empty())
	{
		userMsg += "Lines before current line (for context only, do NOT modify):\n";
		userMsg += widechar_to_utf8(_beforeCaretLines.c_str()) + "\n";
		userMsg += "\n";
	}

	if (!_afterCaretLines.empty())
	{
		userMsg += "Lines after current line (for context only, do NOT modify):\n";
		userMsg += widechar_to_utf8(_afterCaretLines.c_str()) + "\n";
		userMsg += "\n";
	}

	userMsg += "User's partial input:\n";
	userMsg += widechar_to_utf8(_caretLine.c_str());

	request.AddUserMessage(userMsg.c_str());
	request.isStreaming = true;
	request.allowMcpTools = false;

	return _llmChats[0]->Request(request, setting);
}

bool CChatTask_InputHint::_StartCheckCompleteSession()
{
	if (_llmChats.size() < 2)
		return false;

	LlmSessionSetting ccSetting;
	if (!g_llmLib.LoadLlmSetting(ccSetting, _apiName, false, "chatrules_checkcomplete"))
		return false;

	ccSetting.api.tools.clear();

	LlmSessionRequest ccRequest;
	// checkcomplete 只判断用户当前输入的完整性, 用原始纯文本(不含光标标记)
	std::string ccMsg = widechar_to_utf8(_originalInputContent.plainContent.c_str());
	ccRequest.AddUserMessage(ccMsg.c_str());
	ccRequest.isStreaming = true;
	ccRequest.allowMcpTools = false;

	if (!_llmChats[1]->Request(ccRequest, ccSetting))
		return false;

	_checkCompleteStarted = true;
	return true;
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
		std::wstring oldW, newW;
		if (_ExtractOldNew(_resultText, oldW, newW))
		{
			// 删除 LLM 可能残留的光标标记
			static const wchar_t caretMarker = L'\x2038';
			auto removeCaret = [](std::wstring& s) {
				size_t p = s.find(caretMarker);
				if (p != std::wstring::npos)
					s.erase(p, 1);
				};
			removeCaret(oldW);
			removeCaret(newW);

			// 校验补全结果的合理性: InputHint 只做简短续写,
			// 拒绝把输入当成"问题"去长篇回答的异常结果
			if (Utils::IsValidCompletion(oldW, newW))
			{
				// 修复 LLM 未纳入光标后内容导致的拼接重复
				Utils::FixDuplicationAtJoin(_originalInputContent.plainContent, oldW, newW);

				// 基于原始 InputContent 拷贝后用 ReplaceInputContent 应用替换
				_newInputContent = _originalInputContent;
				bool replaced = Utils::ReplaceInputContent(_newInputContent, oldW, newW, _caretPlainPos);

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
						replaced = Utils::ReplaceInputContent(_newInputContent, strippedOld, newW, _caretPlainPos);
					}
				}

				// 替换失败(可能在 tag 内部), 退化为纯文本构建
				if (!replaced)
				{
					_Fail("Fail to replace");
					return;
				}

				// 计算 diff 并暂存, 等 checkcomplete 也完成后再决定是否显示
				Utils::DiffInputContent(_originalInputContent, _newInputContent, _pendingOldDiff, _pendingNewDiff, _pendingGhost);
				_hintValid = true;
			}
		}

	}

	_inputHintFinished = true;
}

bool CChatTask_InputHint::_ExtractOldNew(const std::string& result, std::wstring& oldW, std::wstring& newW)
{
	if (kInputHintFormat == InputHintFormat::Separator)
	{
		// 现有解析: old~~||~~new
		const std::string separator = "~~||~~";
		size_t sepPos = result.find(separator);
		if (sepPos == std::string::npos)
			return false;
		oldW = utf8_to_widechar(result.substr(0, sepPos));
		newW = utf8_to_widechar(result.substr(sepPos + separator.size()));
		return true;
	}

	// JSON 解析: {"old":"...", "new":"..."}
	std::string text = result;
	// 去掉可能包裹的 markdown 代码围栏(```json ... ```)
	auto stripCodeFence = [](std::string s) -> std::string {
		size_t b = s.find_first_not_of(" \t\r\n");
		if (b == std::string::npos)
			return "";
		size_t e = s.find_last_not_of(" \t\r\n");
		s = s.substr(b, e - b + 1);
		if (s.rfind("```", 0) == 0)
		{
			size_t nl = s.find('\n');
			if (nl != std::string::npos)
				s = s.substr(nl + 1);
			size_t lastFence = s.rfind("```");
			if (lastFence != std::string::npos)
				s = s.substr(0, lastFence);
		}
		b = s.find_first_not_of(" \t\r\n");
		e = s.find_last_not_of(" \t\r\n");
		if (b == std::string::npos)
			return "";
		return s.substr(b, e - b + 1);
		};
	text = stripCodeFence(text);

	try
	{
		json j = json::parse(text);
		if (!j.is_object())
			return false;
		auto itOld = j.find("old");
		auto itNew = j.find("new");
		if (itOld == j.end() || itNew == j.end())
			return false;

		std::string oldUtf8 = itOld->is_string() ? itOld->get<std::string>() : itOld->dump();
		std::string newUtf8 = itNew->is_string() ? itNew->get<std::string>() : itNew->dump();

		oldW = utf8_to_widechar(oldUtf8);
		newW = utf8_to_widechar(newUtf8);
		return true;
	}
	catch (...)
	{
		return false;
	}
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
			_context->chatDialogA->ShowHint(_anchorRect, _pendingNewDiff, _pendingOldDiff, _newInputContent, applyCaretTokenPos, _pendingGhost, _contentVersion);
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

