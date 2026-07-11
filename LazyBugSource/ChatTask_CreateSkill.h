#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>

class CChatTask_CreateSkill : public CChatTask
{
public:
	CChatTask_CreateSkill();
	~CChatTask_CreateSkill();

	const char* GetType() override { return "CreateSkill"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 0; }

private:
	void _Fail(const char* reason);
	void _Succeed();

	std::string _name;
	std::string _type;      // "Global" or "Project"
	std::string _description;
	std::string _content;
};
