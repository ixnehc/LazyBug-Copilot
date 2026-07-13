#include "stdh.h"
#include "ChatTask_InputHint3.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatOpsCtrl.h"
#include "ChatDialogA.h"
#include "utils_file.h"
#include <fstream>
#include <unordered_set>


extern const char* GetOpenedDBFolderPath_utf8();

CChatTask_InputHint3::CChatTask_InputHint3(const std::wstring& content, const std::string& apiName, int caretTokenPos, const CRect& anchorRect)
{
	_originalInputContent = Utils::BuildInputContent(content);
	_apiName = apiName;
	_hasStartedRequest = false;
	_requestInterrupt = false;
	_anchorRect = anchorRect;

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


void CChatTask_InputHint3::_Fail(const std::string& reason)
{
	if (_context && _context->chatDialogA)
		_context->chatDialogA->HideHint();
	_status = TaskStatus::Failure;
}

std::string CChatTask_InputHint3::_CollectChatContextFromOps()
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

void CChatTask_InputHint3::Start()
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

	if (_caretPlainPos < 0)
	{
		_Fail("Invalid caret position");
		return;
	}

	// 根据光标位置切分前缀/后缀
	const std::wstring& full = _originalInputContent.plainContent;
	std::wstring prefix = full.substr(0, (size_t)_caretPlainPos);
	std::wstring suffix = full.substr((size_t)_caretPlainPos);

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
	prompt = "You are a text infill assistant. The user has provided a prefix (text before the cursor) and a suffix (text after the cursor).\n"
		"Generate ONLY the middle text that should go between them, so that the whole text reads naturally.\n"
		"The generated middle text MUST flow naturally with BOTH the prefix and the suffix.\n"
		"IMPORTANT: You are ONLY filling text between prefix and suffix. You are NOT a chatbot and you MUST NOT answer, solve or explain the prefix.\n"
		"Even if the prefix looks like a complete question or a request (e.g. ends with \"?\" or \"请给出方案\"), you MUST NOT provide the answer or solution. Only fill the gap between prefix and suffix.\n"
		"IMPORTANT: If the prefix and suffix already form a complete and natural sentence without any gap, output an EMPTY string. Do NOT add filler words just for the sake of filling. Only generate text when there is a genuine semantic gap that needs to be filled.\n"
		"Output ONLY the middle text itself, with NO extra explanation, NO markers, NO formatting. Output the generated text and nothing else.\n\n"
		"Examples:\n"
		"Prefix: \"请帮我\"  Suffix: \"修复这个bug\"  -> 写一段代码来\n"
		"Prefix: \"I need to \"  Suffix: \" the error\"  -> debug\n"
		"Prefix: \"今天天气\"  Suffix: \"\"  -> 真不错\n"
		"Prefix: \"今天天气很好\"  Suffix: \"\"  -> \n"
		"Prefix: \"帮我\"  Suffix: \"这段代码\"  -> 优化\n"
		"Prefix: \"如何实现快速排序？\"  Suffix: \"\"  -> \n"
		"Prefix: \"请给出方案\"  Suffix: \"\"  -> \n\n";

	if (!_chatContext.empty())
	{
		prompt += "Recent chat context:\n";
		prompt += _chatContext;
		prompt += "\n\n";
	}

	prompt += "Prefix:\n";
	prompt += widechar_to_utf8(prefix.c_str());
	prompt += "\n\n";
	prompt += "Suffix:\n";
	prompt += widechar_to_utf8(suffix.c_str());
	prompt += "\n\n";
	prompt += "Middle text:";

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

void CChatTask_InputHint3::Update()
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

					// trim
					size_t start = _resultText.find_first_not_of(" \t\r\n");
					size_t end = _resultText.find_last_not_of(" \t\r\n");
					if (start != std::string::npos && end != std::string::npos)
						_resultText = _resultText.substr(start, end - start + 1);
					else
						_resultText.clear();

					const std::wstring& full = _originalInputContent.plainContent;
					std::wstring prefix = full.substr(0, (size_t)_caretPlainPos);
					std::wstring suffix = full.substr((size_t)_caretPlainPos);

					if (!_resultText.empty())
					{
						std::wstring generated = utf8_to_widechar(_resultText);
						std::wstring newW = prefix + generated + suffix;

						_newInputContent = _originalInputContent;
						if (!Utils::ReplaceInputContent(_newInputContent, full, newW))
						{
							_newInputContent.plainContent = newW;
							_newInputContent.tagSegments.clear();
						}

						Utils::DiffedInputContent oldDiff, newDiff;
						Utils::GhostContent ghostContent;
						Utils::DiffInputContent(_originalInputContent, _newInputContent, oldDiff, newDiff, ghostContent);

						int applyCaretTokenPos = Utils::CalcApplyCaretPos(
							_originalInputContent.plainContent,
							_newInputContent.plainContent,
							_newInputContent,
							_caretPlainPos);

						if (_context && _context->chatDialogA)
							_context->chatDialogA->ShowHint(_anchorRect, newDiff, oldDiff, _newInputContent, applyCaretTokenPos, ghostContent);
					}

					// 保存请求与结果到 recent.json
					if (!_requestInterrupt)
					{
						const char* dbPath = GetOpenedDBFolderPath_utf8();
						std::string rawDir = std::string(dbPath) + "\\_log\\InputHint3\\raw";
						Utils::EnsureFolder(rawDir.c_str());

						std::wstring wDir = utf8_to_widechar(rawDir);
						DeleteFileW((wDir + L"\\recent4.json").c_str());
						MoveFileW((wDir + L"\\recent3.json").c_str(), (wDir + L"\\recent4.json").c_str());
						MoveFileW((wDir + L"\\recent2.json").c_str(), (wDir + L"\\recent3.json").c_str());
						MoveFileW((wDir + L"\\recent1.json").c_str(), (wDir + L"\\recent2.json").c_str());
						MoveFileW((wDir + L"\\recent.json").c_str(), (wDir + L"\\recent1.json").c_str());

						nlohmann::ordered_json j;
						j["context"] = _chatContext;
						j["prefix"] = widechar_to_utf8(prefix.c_str());
						j["suffix"] = widechar_to_utf8(suffix.c_str());
						j["raw"] = _resultText;
						j["generatedEmpty"] = _resultText.empty();

						std::ofstream ofs(wDir + L"\\recent.json");
						if (ofs.is_open())
						{
							ofs << j.dump(2);
							ofs.close();
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

void CChatTask_InputHint3::Interrupt()
{
	_requestInterrupt = true;
	Update();
}
