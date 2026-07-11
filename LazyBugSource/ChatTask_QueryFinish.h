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
	int GetLlmSessionCount() override { return 0; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();
};
