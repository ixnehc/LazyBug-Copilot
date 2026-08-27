#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include "SolutionDBMsgs.h"
#include <string>
#include <vector>

class CChatTask_HistoryEmbeddingQuery : public CChatTask
{
public:
	CChatTask_HistoryEmbeddingQuery(const std::string& apiName, const std::string& embeddingApiName);

	const char* GetType() override { return "HistoryEmbeddingQuery"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 1; }

private:
	enum class Phase
	{
		PlanQueries,      // LLM 从聊天历史生成检索 query 计划
		RequestEmbedding,  // 对当前 query 请求 embedding
		QuerySimilar,     // 向 SolutionDB 查询相似 chunk
		MergeAndFormat,   // 合并去重、按行数限制拼接为文本
		Done
	};

	void _Fail(const std::string& reason = "");

	// Phase 1: 使用 LLM 从聊天历史生成检索 query
	void _StartPlanQueries();
	bool _ProcessPlanQueriesResponse(const std::string& llmOutput);

	// Phase 2: 对当前 query 请求 embedding
	void _StartEmbeddingRequest();

	// Phase 3: 向 SolutionDB 查询相似 chunk
	void _ExecuteSimilarityQuery();

	// Phase 4: 合并去重、格式化、写入 InputHintContext
	void _MergeAndFormatChunks();

	std::string _apiName;
	std::string _embeddingApiName;
	std::string _embeddingModelName;
	std::vector<std::string> _queries;
	int        _currentQueryIndex = 0;
	std::vector<float> _currentEmbedding;
	std::vector<SolutionDBMsg_SimilarChunks::Chunk> _allChunks;
	uint64_t _embeddingDBVersion = 0;  // 第一次执行 QuerySimilar 前记录的 embedding db 版本号
	Phase      _phase = Phase::PlanQueries;
	bool       _hasStartedRequest = false;
	bool       _requestInterrupt = false;
};
