#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include "LlmMcps.h"
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

	// 确保 mcp 已完全启用(否则调用 EnableMcpFullyAndReload)，设置 _uid 后由 Update 等待连接
	bool _HandleExistingMcp(WUID existingUid, const CLlmMcps::Mcp& existingMcp);

	WUID _uid = 0;           // 动态添加的 mcp uid
	std::string _name;       // mcp 名称 (请求的名称)
	std::string _existingName; // 已存在 mcp 的实际名称 (当配置相同但名称不同时)
	bool _isNewMcp = false;  // 是否为本次新增的 mcp (非已存在的)
};