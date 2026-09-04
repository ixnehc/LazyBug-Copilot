#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <vector>

class CChatTask_AddFileToProject : public CChatTask
{
public:
	CChatTask_AddFileToProject();

	const char* GetType() override { return "AddFileToProject"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 0; }

private:
	void _Fail(const char* reason);
	void _FinishCheckpoint();

	std::string _projectFilePath;
	std::string _fileFullPath;

	// 延迟到 Update 中再写入 after-edit checkpoint 所需的状态
	std::wstring _aiMessageId;
	std::string _description;
	std::vector<std::string> _checkpointFilePaths;
	unsigned long _delayStartTick;
	bool _bridgeDone;
};
