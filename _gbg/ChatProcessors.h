#pragma once

#include <deque>

#include "LlmProcessors.h"


struct ChatCmd_EditFile
{
	ChatCmd_EditFile()
	{
		isValid = false;
	}
	bool isValid;
	std::string filePath;
	std::string instruction;
	std::string content;
};


struct ChatProcessorsResult: public LlmProcessorsResult
{
};

class CChatDialog;
class CChatTaskMgr;
struct ChatProcessorsContext :public LlmProcessorsContext
{
	ChatProcessorsContext()
	{
		chatDialog = nullptr;
		chatTaskMgr = nullptr;
	}
	CChatDialog* chatDialog;
	CChatTaskMgr* chatTaskMgr;
};

class CChatProcessor_PatchFile : public CLlmProcessor
{
public:
	const char* GetName()	{		return "PatchFile";	}

	int GetProcessStart(const std::string& workingStr, const LlmProcessorsContext& context) const override;
	bool Process(std::string& workingStr, const LlmProcessorsContext& context, LlmProcessorsResult& result) override;
	void Stop(const LlmProcessorsContext& context, LlmProcessorsResult& result) override;

protected:
	std::vector<UnifiedDiffChunk> _chunks;
};


class CChatProcessor_ReplaceInFile : public CLlmProcessor
{
public:
	const char* GetName() override { return "ReplaceInFile"; }

	void Start() override
	{
		_filePath = "";
		_fileEditId=L"";
	}


	int GetProcessStart(const std::string& workingStr, const LlmProcessorsContext& context) const override;
	bool Process(std::string& workingStr, const LlmProcessorsContext& context, LlmProcessorsResult& result) override;
	void Stop(const LlmProcessorsContext& context, LlmProcessorsResult& result) override;

protected:
	std::wstring _fileEditId;
	std::string _filePath;
};


class CChatProcessor_ReplaceInFileUseJson :public CChatProcessor_ReplaceInFile
{
public:
	const char* GetName() override { return "ReplaceInFileUseJson"; }

	bool Process(std::string& workingStr, const LlmProcessorsContext& context, LlmProcessorsResult& result) override;

protected:
	// 从部分JSON内容中提取文件路径的辅助函数
	bool ExtractFilePathFromPartialJson(const std::string& content, std::string& filePath);

};


class CChatProcessor_CollectFileEdit : public CLlmProcessor
{
public:
	CChatProcessor_CollectFileEdit()
	{
	}
	const char* GetName() { return "CollectFileEdit"; }
	int GetProcessStart(const std::string& workingStr, const LlmProcessorsContext& context) const;
	bool Process(std::string& workingStr, const LlmProcessorsContext& context, LlmProcessorsResult& result) override;

protected:
	std::wstring _fileEditId;
};


//这个类用来处理LLM的返回字符串
class CChatProcessors: public CLlmProcessors
{
public:

	void Init(CChatDialog* chatDialog);

	void Interrupt() override;

private:
	ChatProcessorsResult _result;
	ChatProcessorsContext _context;

	const LlmProcessorsContext& _GetContext() override	{		return (LlmProcessorsContext & )_context;	}
	LlmProcessorsResult& _GetResult() override	{		return (LlmProcessorsResult&)_result;	}
	void _GetProcessors(std::vector<CLlmProcessor*>& processors) override;
	CChatProcessor_CollectFileEdit _collectFileEdit;
	CChatProcessor_ReplaceInFile _replaceInFile;
	CChatProcessor_ReplaceInFileUseJson _replaceInFileUseJson;

	CChatProcessor_PatchFile _patchFile;
};