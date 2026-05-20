#include "stdh.h"
#include "ChatDialog.h"

#include "ChatBrief.h"


#include "LlmSession.h"

#include "LlmLib.h"

void CChatBrief::Update(CChatDialog& chatDlg)
{
	if (_availableTries <= 0)
		return;

	if (chatDlg.GetChatCtrl().HasTitle())
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
				if (_briefingChatFileName==chatFileName)
				{
					std::string content = output.fullContent;
					size_t pos = content.find('\n');
					if (pos != std::string::npos) 
						content = content.substr(0, pos);
					std::wstring title = utf8_to_widechar(content.c_str());
					if (!title.empty())
						chatDlg.GetChatCtrl().SetTitle(title);
				}
				_briefingChatFileName = "";
			}
		}
		return;
	}

	const std::vector<ChatCtrlOp>& ops = chatDlg.GetChatCtrl().GetOps();

	//得到第一个session 的所有内容包括user message和ai message
	std::wstring userMessage;
	std::wstring aiMessage;
	for (const auto& op : ops)
	{
		if (op.type == ChatCtrlOp::Op_AddUserMessage)
		{
			userMessage = op.content;
		}
		if (op.type == ChatCtrlOp::Op_AddStreamingAIMessage)
		{
			aiMessage = op.content;
			break;
		}
	}

	if (userMessage.empty() || aiMessage.empty())
		return;

	LlmSessionRequest request;
	std::string question = u8"Please create a very concise title for the following dialogue.\n";
	question += u8"Question: " + widechar_to_utf8(userMessage.c_str()) + "\n";
	question += u8"Answer: " + widechar_to_utf8(aiMessage.c_str()) + "\n";
	request.AddUserMessage(question.c_str());

	LlmSessionSetting settings;
	if (!g_llmLib.LoadLlmSetting(settings, LlmApiPurpose::MinorChat, ""))
		g_llmLib.LoadLlmSetting(settings, LlmApiPurpose::MajorChat, "");

	settings.api.tools.clear();
	settings.api.rule = "";

	_llmChat.Request(request, settings);
	_briefingChatFileName=chatFileName;
	_availableTries--;
}

