#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>

class CChatTask_CompressSummarize : public CChatTask
{
public:
	// workingOpIndex: CChatOpsCompress::_workingOps 的索引
	// summarizeApiName: 用于 summarize 的 API 名称
	CChatTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName);

	const char* GetType() override { return "CompressSummarize"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return true; }

private:
	void _Fail();
	void _Succeed(const std::string& result);

	int _originalTokenCount;

	bool _hasStartedRequest;
	bool _requestInterrupt;
	int _workingOpIndex;
	std::string _summarizeApiName;
};