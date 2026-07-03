#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include "../Common/codediff/CodeDiff.h"
#include <string>
#include "Utils.h"

class CChatTask_FastApply : public CChatTask
{
public:
	CChatTask_FastApply(const std::string& filePath, const std::string& updateContent, const std::wstring& fileEditId);
	
	const char* GetType() override { return "FastApply"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return true; }

	bool DependsOn(CChatTask* task) override;

private:
	void _SaveResult();

	std::wstring _fileEditId;
	std::string _filePath;
	std::string _updateContent;
	std::string _originalFileContent;
	Utils::FileContentCodingFormat _originalFileCodingFmt;
	std::string _collectedResult;
	bool _hasStartedRequest;
	bool _requestInterrupt;
	friend class CChatTask_FastApply;
};