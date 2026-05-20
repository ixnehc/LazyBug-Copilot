#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <vector>

class CChatTask_Question : public CChatTask
{
public:
	CChatTask_Question();
	~CChatTask_Question();

	const char* GetType() override { return "Question"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return false; }

	bool DependsOn(CChatTask* task) override;

	// 设置用户的回答（由外部UI调用）
	void SetUserAnswer(const std::string& answer);

private:
	void _Fail();
	void _Succeed();
	void _ClearQuestionInUi();

	std::string _question;
	std::vector<std::string> _options;
	std::string _userAnswer;
	bool _waitingForUser;
	__int64 _questionId;
};