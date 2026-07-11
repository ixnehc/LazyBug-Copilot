#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include "../Common/codediff/CodeDiff.h"
#include <string>

#include "LlmLib.h"

class CChatTask_VerifyLlmApiProvider : public CChatTask
{
public:
	CChatTask_VerifyLlmApiProvider(const LlmApiProviderTypeName& providerTypeName);
	
	const char* GetType() override { return "VerifyLlmApiProvider"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 1; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();

	bool _hasStartedRequest;
	bool _requestInterrupt;
	LlmApiProviderTypeName _providerTypeName;
	friend class CChatTask_VerifyLlmApiProvider;
};