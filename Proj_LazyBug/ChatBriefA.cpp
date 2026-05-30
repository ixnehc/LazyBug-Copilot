#include "stdh.h"
#include "ChatDialogA.h"

#include "ChatBriefA.h"


#include "LlmSession.h"

#include "LlmLib.h"

void CChatBriefA::Update(CChatDialogA& chatDlg)
{
	if (_availableTries <= 0)
	{
		if (!chatDlg.HasTitle())
		{
			std::wstring userMessage;
			const std::vector<ChatOp>& ops = chatDlg.GetOps();
			for (const auto& op : ops)
			{
				if (op.type == ChatOp::Op_AddUserMessage)
				{
					userMessage = utf8_to_widechar(op.contentUtf8);
					break;
				}
			}
			if (!userMessage.empty())
			{
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
				// 设置标题
				chatDlg.SetTitle(s);
			}

		}

		return;
	}

	if (chatDlg.HasTitle())
		return;

	std::string chatFileName = chatDlg.GetChatFileName();
	if (chatFileName.empty())
		return;

	if (!_briefingChatFileName.empty())
	{
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
					std::wstring title = utf8_to_widechar(content.c_str());
					if (!title.empty())
						chatDlg.SetTitle(title);
				}
				_briefingChatFileName = "";
			}
		}
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
		return;

	LlmSessionRequest request;
	std::string question = u8"Please create a very concise title for the following dialogue.\n";
	question += u8"Question: " + userMessage + "\n";
	question += u8"Answer: " + aiMessage + "\n";
	request.AddUserMessage(question.c_str());

	LlmSessionSetting settings;
	if (!g_llmLib.LoadLlmSetting(settings, LlmApiRole::Auxiliary, ""))
		g_llmLib.LoadLlmSetting(settings, LlmApiRole::Agent, "");

	settings.api.tools.clear();
	settings.api.thinkingMode = LlmThinkingMode::Disable;
	settings.api.rule = "";

	_llmChat.Request(request, settings);
	_briefingChatFileName = chatFileName;
	_availableTries--;
}

