#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>

class CChatTask_HistoryEmbeddingQuery : public CChatTask
{
public:
	CChatTask_HistoryEmbeddingQuery(const std::string& apiName);

	const char* GetType() override { return "HistoryEmbeddingQuery"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 1; }

private:
	enum class Phase
	{
		PlanQueries,   // 等待 LLM 返回检索 query 计划
		Done
	};

	void _Fail(const std::string& reason = "");

	std::string _apiName;
	Phase      _phase = Phase::PlanQueries;
	bool       _hasStartedRequest = false;
	bool       _requestInterrupt = false;
};
