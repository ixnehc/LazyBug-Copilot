#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <vector>

class CChatTask_InputEmbeddingQuery : public CChatTask
{
public:
	CChatTask_InputEmbeddingQuery(const std::string& embeddingApiName);

	const char* GetType() override { return "InputEmbeddingQuery"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 1; }

private:
	enum class Phase
	{
		Embedding,  // 等待 embedding API 返回向量
		Query,      // 向 SolutionDB 查询相似 chunk
		Done
	};

	void _Fail(const std::string& reason = "");

	// 从 InputHintContext 构造 embedding 查询文本（输入纯文本 + 最近一条用户消息）
	std::string _BuildQueryText();

	std::string             _embeddingApiName;
	std::string             _modelName;
	std::string             _queryText;
	std::vector<float>      _embedding;
	Phase                   _phase = Phase::Embedding;
	bool                    _hasStartedRequest = false;
	bool                    _requestInterrupt = false;
};
