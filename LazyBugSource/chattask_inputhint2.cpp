#include "stdh.h"
#include "ChatTask_InputHint2.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCtrl.h"
#include "ChatDialogA.h"
#include <unordered_set>

CChatTask_InputHint2::CChatTask_InputHint2(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect, int contentVersion)
{
	_originalInputContent = Utils::BuildInputContent(content);
	_apiName = apiName;
	_hasStartedRequest = false;
	_requestInterrupt = false;
	_anchorRect = anchorRect;
	_contentVersion = contentVersion;

	// InputHint2 中光标位置仅保留记录，不作为 prompt 中的补全标记
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

			if (segIdx < segs.size() && pos == segs[segIdx].startPos)
			{
				pos = segs[segIdx].endPos;
				segIdx++;
			}
			else
			{
				pos++;
			}
			tokenIdx++;
		}
	}
}


void CChatTask_InputHint2::_Fail(const std::string& reason)
{
	if (_context && _context->chatDialogA)
		_context->chatDialogA->HideHint();
	_status = TaskStatus::Failure;
}

std::string CChatTask_InputHint2::_CollectChatContextFromOps()
{
	if (!_context || !_context->chatOpsCtrl)
		return "";

	const std::vector<ChatOp>& ops = _context->chatOpsCtrl->GetOps();

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

		if ((int)(entry.size() + result.size()) > MAX_LEN)
			break;

		result = entry + result;
	}

	return result;
}

void CChatTask_InputHint2::Start()
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
	prompt = "You are an input expansion assistant. The user is typing some fragmented keywords or short notes,\n"
		"and you need to expand them into a complete, natural, well-formed message that the user likely intends to say.\n"
		"Based on the recent chat context and the user's input fragments, generate the full message.\n"
		"The output should be in the same language as the input fragments, concise and to the point.\n"
		"You MUST strictly follow this output format and output NOTHING else:\n"
		"<<Old Content>>~~||~~<<New Content>>\n"
		"Where <Old Content> is the user's original fragment input, and <New Content> is the expanded complete message.\n"
		"Old and new content are separated by ~~||~~.\n\n"
		"Examples:\n"
		"Fragments: \"bug 修复 崩溃\" -> output: bug 修复 崩溃~~||~~请帮我修复这个崩溃的bug\n"
		"Fragments: \"add logging error\" -> output: add logging error~~||~~Please add logging to track down the error\n"
		"Fragments: \"重构 性能 优化\" -> output: 重构 性能 优化~~||~~请对这部分代码进行重构以提升性能\n"
		"Fragments: \"review PR\" -> output: review PR~~||~~Could you review my pull request?\n\n";

	if (!_chatContext.empty())
	{
		prompt += "Recent chat context:\n";
		prompt += _chatContext;
		prompt += "\n\n";
	}

	prompt += "User's input fragments:\n";
	prompt += widechar_to_utf8(_originalInputContent.plainContent.c_str());
	prompt += "\n\n";
	prompt += "Expansion:";

	request.AddUserMessage(prompt.c_str());
	request.isStreaming = true;

	if (!_llmChats[0]->Request(request, setting))
	{
		_Fail("Failed to send LLM request");
		return;
	}

	_hasStartedRequest = true;
	_status = TaskStatus::Running;
}

void CChatTask_InputHint2::Update()
{
	if (_status != TaskStatus::Running)
		return;

	if (_llmChats.empty())
		return;

	if (_llmChats[0]->HasActiveSession())
	{
		LlmSessionOutput output;
		if (_llmChats[0]->Process(output, _requestInterrupt))
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
						const std::string separator = "~~||~~";
						size_t sepPos = _resultText.find(separator);
						if (sepPos != std::string::npos)
						{
							std::string oldUtf8 = _resultText.substr(0, sepPos);
							std::string newUtf8 = _resultText.substr(sepPos + separator.size());

							std::wstring oldW = utf8_to_widechar(oldUtf8);
							std::wstring newW = utf8_to_widechar(newUtf8);

							_newInputContent = _originalInputContent;
							if (!Utils::ReplaceInputContent(_newInputContent, oldW, newW))
							{
								_newInputContent.plainContent = newW;
								_newInputContent.tagSegments.clear();
							}

							Utils::DiffedInputContent oldDiff, newDiff;
							Utils::GhostContent ghostContent;
							Utils::DiffInputContent(_originalInputContent, _newInputContent, oldDiff, newDiff, ghostContent);

							// 计算补全后光标应定位的 token 位置
							int applyCaretTokenPos = Utils::CalcApplyCaretPos(
								_originalInputContent.plainContent,
								_newInputContent.plainContent,
								_newInputContent,
								_caretPlainPos);

							if (_context && _context->chatDialogA)
								_context->chatDialogA->ShowHint(_anchorRect, newDiff, oldDiff, _newInputContent, applyCaretTokenPos, ghostContent, _contentVersion);
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

void CChatTask_InputHint2::Interrupt()
{
	_requestInterrupt = true;
	Update();
}
