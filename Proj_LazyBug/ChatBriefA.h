#pragma once


#include "resource.h"

#include "LlmChat.h"

class CChatDialogA;

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

	int _availableTries;
	
	// AI聊天相关
	CLlmChat _llmChat;
	std::string _briefingChatFileName;
};



