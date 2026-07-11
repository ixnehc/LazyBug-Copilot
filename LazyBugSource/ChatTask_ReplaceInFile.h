#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include "../Common/codediff/CodeDiff.h"
#include <string>

class CChatTask_ReplaceInFile : public CChatTask
{
public:
	CChatTask_ReplaceInFile(const std::wstring &fileEditId);
	CChatTask_ReplaceInFile(const std::string&filePath, const std::string& oldContent, const std::string& newContent);

	const char* GetType() override { return "ReplaceInFile"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 0; }

	bool DependsOn(CChatTask* task) override;

private:

	bool _wasReadOnly; // 记录上一次检测的只读状态，避免重复更新

	std::string _oldLines;
	std::string _newLines;

	std::wstring _fileEditId;
	std::string _filePath;

	friend class CChatTask_ReplaceInFile;
};