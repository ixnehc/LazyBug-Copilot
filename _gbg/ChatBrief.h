#pragma once


#include "resource.h"

#include "LlmChat.h"

class CChatDialog;

class CChatBrief
{
public:
	CChatBrief()
	{
		Activate();
	}
	void Activate()
	{
		_availableTries = 3;
	}
	void Update(CChatDialog& chatDlg);

	int _availableTries;
	
	// AI聊天相关
	CLlmChat _llmChat;
	std::string _briefingChatFileName;
};



