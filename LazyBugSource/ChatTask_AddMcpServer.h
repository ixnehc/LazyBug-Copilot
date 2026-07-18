#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include "timer/wuid.h"

class CChatTask_AddMcpServer : public CChatTask
{
public:
	CChatTask_AddMcpServer();
	~CChatTask_AddMcpServer();

	const char* GetType() override { return "AddMcpServer"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 0; }

private:
	void _Fail(const char* reason);
	void _Succeed();

	WUID _uid = 0;           // 动态添加的 mcp uid
	DWORD _startTick = 0;    // 开始时间, 用于超时检测
	std::string _name;       // mcp 名称
};