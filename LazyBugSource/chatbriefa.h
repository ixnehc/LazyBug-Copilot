#pragma once


//#include "resource.h"

#include "LlmChat.h"

class CChatDialogA;
struct ChatOp;
class CChatBriefA
{
public:
	CChatBriefA()
	{
		Activate();
	}
	void Activate()
	{
		_availableTries = 3;
	}
	void Update(CChatDialogA& chatDlg);

	// 从第一个user message截断取得title
	static std::wstring GetSimpleTitle(const std::vector<ChatOp>& ops);

	int _availableTries;
	
	// AI聊天相关
	CLlmChat _llmChat;
	std::string _briefingChatFileName;
};



