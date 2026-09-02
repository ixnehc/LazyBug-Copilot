#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>

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

	std::string _projectFilePath;
	std::string _fileFullPath;
};
