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
		_forceRefresh = false;
		_briefingChatFileName.clear();
		_llmChat.Clear();
	}
	// 强制重新生成title brief（刷新按钮触发）
	void Refresh();

	// 是否正在生成title brief（用于刷新按钮loading状态）
	bool IsRefreshing() const
	{
		return _forceRefresh || !_briefingChatFileName.empty();
	}

	void Update(CChatDialogA& chatDlg);

	// 从第一个user message截断取得title
	static std::wstring GetSimpleTitle(const std::vector<ChatOp>& ops);

	int _availableTries;
	
	// AI聊天相关
	CLlmChat _llmChat;
	std::string _briefingChatFileName;

	// 强制刷新标志
	bool _forceRefresh = false;
};



