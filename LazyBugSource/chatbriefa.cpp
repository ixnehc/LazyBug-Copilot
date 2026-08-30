#include "stdh.h"
#include "ChatDialogA.h"

#include "ChatBriefA.h"


#include "LlmSession.h"

#include "LlmLib.h"

std::wstring CChatBriefA::GetSimpleTitle(const std::vector<ChatOp>& ops)
{
	std::wstring userMessage;
	for (const auto& op : ops)
	{
		if (op.type == ChatOp::Op_AddUserMessage)
		{
			userMessage = utf8_to_widechar(op.contentUtf8);
			break;
		}
	}
	if (userMessage.empty())
		return L"";

	std::wstring s = ExtractPlainText(userMessage);
	// 将换行符替换为空格
	std::replace(s.begin(), s.end(), '\n', ' ');
	std::replace(s.begin(), s.end(), '\r', ' ');
	// 截取最多50个字符
	if (s.length() > 50)
	{
		s = s.substr(0, 50);
	}
	// 末尾加上"..."
	s += L"...";
	return s;
}

void CChatBriefA::Update(CChatDialogA& chatDlg)
{
	// 处理进行中的 brief 会话（包括强制刷新产生的会话）
	if (!_briefingChatFileName.empty())
	{
		std::string chatFileName = chatDlg.GetChatFileName();
		LlmSessionOutput output;
		if (_llmChat.Process(output))
		{
			if (output.isCompleted)
			{
				if (_briefingChatFileName == chatFileName)
				{
					std::string content = output.fullContent;
					size_t pos = content.find('\n');
					if (pos != std::string::npos)
						content = content.substr(0, pos);
					// 去除两侧的引号
					if (!content.empty() && content.front() == '"')
						content.erase(0, 1);
					if (!content.empty() && content.back() == '"')
						content.pop_back();
					std::wstring title = utf8_to_widechar(content.c_str());
					if (!title.empty())
					{
						chatDlg.SetTitle(title);
						_availableTries = 0;
					}
				}
				_briefingChatFileName = "";
				_forceRefresh = false;
			}
		}
		return;
	}

	// 非强制刷新时，才应用次数限制和“已有AI标题不再生成”的规则
	if (!_forceRefresh)
	{
		if (_availableTries <= 0)
		{
			if (!chatDlg.HasTitle())
			{
				std::wstring title = GetSimpleTitle(chatDlg.GetOps());
				if (!title.empty())
					chatDlg.SetTitle(title);
			}
			return;
		}

		if (chatDlg.HasTitle())
		{
			const wchar_t* currentTitle = chatDlg.GetTitle();
			// 如果当前title不是simple title，说明已经是AI生成的，不需要重新生成
			if (wcscmp(currentTitle, GetSimpleTitle(chatDlg.GetOps()).c_str()) != 0)
			{
				_availableTries = 0;
				return;
			}
		}
	}

	std::string chatFileName = chatDlg.GetChatFileName();
	if (chatFileName.empty())
	{
		_forceRefresh = false;
		return;
	}

	const std::vector<ChatOp>& ops = chatDlg.GetOps();

	//得到第一个session 的所有内容包括user message和ai message
	std::string userMessage;
	std::string aiMessage;
	for (const auto& op : ops)
	{
		if (op.type == ChatOp::Op_AddUserMessage)
		{
			userMessage = op.contentUtf8;
		}
		if (op.type == ChatOp::Op_AddStreamingAIMessage)
		{
			aiMessage = op.contentUtf8;
			break;
		}
	}

	if (userMessage.empty() || aiMessage.empty())
	{
		_forceRefresh = false;
		return;
	}

	LlmSessionRequest request;
	std::string question = u8"Please create a very concise title for the following dialogue.\n";
	question += u8"Question: " + userMessage + "\n";
	question += u8"Answer: " + aiMessage + "\n";
	request.AddUserMessage(question.c_str());

	LlmSessionSetting settings;
	if (!g_llmLib.LoadLlmSetting(settings, g_llmLib.GetBriefApi(), false, ""))
	{
		_availableTries = 0;
		_forceRefresh = false;
		return;
	}

	settings.api.tools.clear();
//	settings.api.thinkingMode = LlmThinkingMode::Disable;
	settings.api.rule = "";

	_llmChat.Request(request, settings);
	_briefingChatFileName = chatFileName;
	_availableTries--;
}

void CChatBriefA::Refresh()
{
	// 清理进行中的 brief 会话，强制重新生成
	_llmChat.Clear();
	_briefingChatFileName.clear();
	_forceRefresh = true;
	_availableTries = 1;
}

