#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>

class CChatTask_QueryFinish : public CChatTask
{
public:
	CChatTask_QueryFinish();
	~CChatTask_QueryFinish();

	const char* GetType() override { return "QueryFinish"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return false; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();
};
