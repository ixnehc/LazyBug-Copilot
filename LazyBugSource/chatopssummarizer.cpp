#include "stdh.h"

#include "ChatOpsSummarizer.h"
#include "ChatOpsCompress.h"
#include "ChatOpsCtrl.h"
#include "ChatAgent.h"
#include "LlmLib.h"
#include "LlmLibDefines.h"
#include "Utils_Context.h"

extern CLlmLib g_llmLib;


CChatOpsSummarizer::CChatOpsSummarizer()
{
}

CChatOpsSummarizer::~CChatOpsSummarizer()
{
}

void CChatOpsSummarizer::Init(CChatOpsCtrl* opsCtrl, CChatAgent* agent)
{
	_opsCtrl = opsCtrl;
	_agent = agent;

	ChatTaskContext ctx;
	ctx.chatOpsCtrl = opsCtrl;
	_taskMgr.Init(ctx);
}

void CChatOpsSummarizer::Clear()
{
	_taskMgr.Shutdown();
	_state = State_Idle;
	_opsCtrl = nullptr;
	_agent = nullptr;
}

void CChatOpsSummarizer::Update()
{
	_taskMgr.Update();

	if (_taskMgr.IsRunning())
	{
		_state = State_Summarizing;
		return;
	}

	_state = State_Idle;

	// 空闲时检查是否有未总结的 session
	_CheckAndStartSummarize();
}

void CChatOpsSummarizer::_CheckAndStartSummarize()
{
	if (!_opsCtrl)
		return;
	if (_state != State_Idle)
		return;

	// agent 正在工作时不启动总结
	if (_agent && _agent->IsWorking())
		return;

	// 解析 summarize API
	std::string summarizeApiName = g_llmLib.GetSummarizeApi();
	if (summarizeApiName == SUMMARIZE_API_AUTO)
	{
		summarizeApiName = Utils::ResolveAutoSummarizeApi();
	}
	else if (summarizeApiName == SUMMARIZE_API_DISABLE)
	{
		summarizeApiName.clear();
	}
	if (summarizeApiName.empty())
		return;

	const std::vector<ChatOp>& ops = _opsCtrl->GetOps();

	// 查找最近 N 个未 disable 的 session
	std::vector<int> sessionEnds = _opsCtrl->FindLastNNotDisabledSessionEnds(20);
	if (sessionEnds.empty())
		return;

	// 从旧到新遍历（跳过最近 2 个 session，即索引 0 和 1）
	for (int i = (int)sessionEnds.size() - 1; i >= 2; i--)
	{
		int endIndex = sessionEnds[i];
		const ChatOp& op = ops[endIndex];

		// 检查是否已有有效的总结结果
		auto it = op.compressedContents.find(static_cast<int>(CChatOpsCompress::CompressLevel::Level_Partial));
		if (it != op.compressedContents.end() && !it->second.empty())
			continue;

// 		// 估算 token 数
// 		int nTokens = _opsCtrl->EstimateUncompressedSessionAIContentToken(endIndex, CChatOpsCompress::GetSessionSummarizeToolTypes());
// 		if (nTokens < 300)
// 			continue;

		// 提交任务（Immediate 模式，直接写回 ChatOp）
		_taskMgr.AddTask_CompressSummarize(endIndex, summarizeApiName, CompressSummarizeMode::Immediate);

		_state = State_Summarizing;
		return;  // 一次只提交一个，下一帧继续
	}
}

