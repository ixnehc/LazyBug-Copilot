#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>

class CChatTask_CompressSummarize : public CChatTask
{
public:
	// workingOpIndex: CChatOpsCompress::_workingOps 的索引
	CChatTask_CompressSummarize(int workingOpIndex);

	const char* GetType() override { return "CompressSummarize"; }
	void Start() override;
	void Update() override;
	void Interrupt() override { _requestInterrupt = true; }
	bool NeedLlmSession() override { return true; }

private:
	void _Fail();
	void _Succeed(const std::string& result);

	bool _hasStartedRequest;
	bool _requestInterrupt;
	int _workingOpIndex;
};