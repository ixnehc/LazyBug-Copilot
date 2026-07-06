#include "stdh.h"
#include "ChatTask_InputHint.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCtrl.h"
#include "ChatDialogA.h"
#include <unordered_set>

CChatTask_InputHint::CChatTask_InputHint(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect)
{
	_originalInputContent = Utils::BuildInputContent(content);
	_apiName = apiName;
	_hasStartedRequest = false;
	_requestInterrupt = false;
	_anchorRect = anchorRect;

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
	if (!g_llmLib.LoadLlmSetting(setting, _apiName, false, ""))
	{
		_Fail("Failed to load LLM setting");
		return;
	}

	setting.api.tools.clear();
	setting.rulesFiles.clear();

	LlmSessionRequest request;

	std::string prompt;
	prompt = "You are an input auto-completion assistant. Based on the recent chat context and the user's partial input,\n"
		"suggest a plausible completion of what the user might be typing next.\n"
		"You may also correct typos or improve the existing partial input when appropriate.\n"
		"The completion should be concise and in the same language as the partial input.\n"
		"A caret marker \xE2\x80\xB8 (U+2038) indicates the current cursor position in the partial input. "
		"You should complete the text AT the caret position. The caret marker itself is NOT part of the "
		"user's actual input, so you MUST NOT include it in your output.\n"
		"You MUST strictly follow this output format and output NOTHING else:\n"
		"<<Old Content>>~~||~~<<New Content>>\n"
		"Where <Old Content> is the user's partial input (WITHOUT the caret marker), and <New Content> is the corrected and/or completed text.\n"
		"Old and new content are separated by ~~||~~.\n\n"
		"Examples:\n"
		"Partial input \"Hello\xE2\x80\xB8\" -> output: Hello~~||~~Hello world\n"
		"Partial input \"How are\xE2\x80\xB8\" -> output: How are~~||~~How are you\n"
		"Partial input \"请帮我\xE2\x80\xB8\" -> output: 请帮我~~||~~请帮我写一个函数\n"
		"Partial input \"hwo to\xE2\x80\xB8\" (typo) -> output: hwo to~~||~~how to write\n"
		"Partial input \"请写\xE2\x80\xB8一个函数\" (caret in middle) -> output: 请写一个函数~~||~~请写一个高效的函数\n\n";

	if (!_chatContext.empty())
	{
		prompt += "Recent chat context:\n";
		prompt += _chatContext;
		prompt += "\n\n";
	}

	// 在光标位置插入光标符号(仅用于提示 LLM 补全位置, 不属于实际输入内容)
	std::wstring inputWithCaret = _originalInputContent.plainContent;
	if (_caretPlainPos >= 0 && _caretPlainPos <= (int)inputWithCaret.size())
	{
		inputWithCaret.insert((size_t)_caretPlainPos, L"\x2038");
	}

	prompt += "User's partial input:\n";
	prompt += widechar_to_utf8(inputWithCaret.c_str());
	prompt += "\n\n";
	prompt += "Completion:";


	request.AddUserMessage(prompt.c_str());
	request.isStreaming = true;

	if (!_llmChat->Request(request, setting))
	{
		_Fail("Failed to send LLM request");
		return;
	}

	_hasStartedRequest = true;
	_status = TaskStatus::Running;
}

void CChatTask_InputHint::Update()
{
	if (_status != TaskStatus::Running)
		return;

	if (!_llmChat)
		return;

	if (_llmChat->HasActiveSession())
	{
		LlmSessionOutput output;
		if (_llmChat->Process(output, _requestInterrupt))
		{
			if (output.isCompleted)
			{
				if (_requestInterrupt)
				{
					_Fail("Interrupted");
				}
				else if (output.hasError)
				{
					_Fail(output.errorMessage);
				}
				else
				{
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

							// 基于原始 InputContent 拷贝后用 ReplaceInputContent 应用替换
							_newInputContent = _originalInputContent;
							if (!Utils::ReplaceInputContent(_newInputContent, oldW, newW))
							{
								// 替换失败(可能在 tag 内部), 退化为纯文本构建
								_newInputContent.plainContent = newW;
								_newInputContent.tagSegments.clear();
							}

							Utils::DiffedInputContent oldDiff, newDiff;
							Utils::DiffInputContent(_originalInputContent, _newInputContent, oldDiff, newDiff);

							if (_context && _context->chatDialogA)
								_context->chatDialogA->ShowHint(_anchorRect, newDiff, oldDiff);
						}
					}

					_status = TaskStatus::Success;
				}
			}
		}
	}
	else if (_hasStartedRequest)
	{
		_Fail("LLM session ended unexpectedly");
	}
}

void CChatTask_InputHint::Interrupt()
{
	_requestInterrupt = true;
	Update();
}
