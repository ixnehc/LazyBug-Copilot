#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <thread>
#include <atomic>

class CChatTask_Mcp : public CChatTask
{
public:
	CChatTask_Mcp();
	~CChatTask_Mcp();

	const char* GetType() override { return "Mcp"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return false; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();

	static std::string _ExtractArgsSummary(const std::string& rawArguments);

	std::thread _thread;
	std::atomic<bool> _done{ false };
	std::atomic<bool> _success{ false };
	HANDLE _hCancelEvent = nullptr;  // 取消事件
	std::string _result;
	std::wstring _mcpId;  // MCP 显示 ID，用于轮询停止状态
};