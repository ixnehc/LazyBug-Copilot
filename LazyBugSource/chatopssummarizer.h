#pragma once

#include "ChatOpsCtrl.h"
#include "ChatTaskMgr.h"

class CChatOpsCtrl;
class CChatAgent;
struct ChatOp;

class CChatOpsSummarizer
{
public:
	enum State
	{
		State_Idle,
		State_Summarizing,
	};

	CChatOpsSummarizer();
	~CChatOpsSummarizer();

	void Init(CChatOpsCtrl* opsCtrl, CChatAgent* agent);
	void Clear();
	void Update();

	State GetState() const { return _state; }
	bool IsSummarizing() const { return _state == State_Summarizing; }

private:
	// 检查是否有未总结的 session 并启动总结任务
	void _CheckAndStartSummarize();

	CChatOpsCtrl*  _opsCtrl = nullptr;
	CChatAgent*    _agent = nullptr;
	State          _state = State_Idle;
	CChatTaskMgr   _taskMgr;
};

