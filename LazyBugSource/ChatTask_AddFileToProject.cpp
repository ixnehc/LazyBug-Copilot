#include "stdh.h"
#include "ChatTask_AddFileToProject.h"
#include "LlmChat.h"

extern std::wstring utf8_to_widechar(const char* utf8_str);

// 由宿主(VSIX 或独立 LazyBug.exe)提供的桥接实现
extern bool AddFileToProjectInVS(const unsigned short* projectFilePath, const unsigned short* fileFullPath, char* errorMsg, int errorMsgSize);

CChatTask_AddFileToProject::CChatTask_AddFileToProject()
{
}

void CChatTask_AddFileToProject::_Fail(const char* reason)
{
	std::string result = "Error: ";
	result += reason ? reason : "Unknown error";
	_SendToolCallResult(result.c_str());
	_status = TaskStatus::Failure;
}

void CChatTask_AddFileToProject::Start()
{
	_status = TaskStatus::Running;

	if (!_toolCall.GetStringParam("projectFilePath", _projectFilePath) || _projectFilePath.empty())
	{
		_Fail("Missing 'projectFilePath' parameter");
		return;
	}

	if (!_toolCall.GetStringParam("fileFullPath", _fileFullPath) || _fileFullPath.empty())
	{
		_Fail("Missing 'fileFullPath' parameter");
		return;
	}

	std::wstring wProjectPath = utf8_to_widechar(_projectFilePath.c_str());
	std::wstring wFilePath = utf8_to_widechar(_fileFullPath.c_str());

	char errorMsg[512] = { 0 };
	bool ok = AddFileToProjectInVS(
		(const unsigned short*)wProjectPath.c_str(),
		(const unsigned short*)wFilePath.c_str(),
		errorMsg,
		(int)sizeof(errorMsg));

	if (!ok)
	{
		_Fail(errorMsg[0] ? errorMsg : "Failed to add file to project");
		return;
	}

	std::string result = "File added to project successfully: " + _fileFullPath;
	_SendToolCallResult(result.c_str());
	_status = TaskStatus::Success;
}

void CChatTask_AddFileToProject::Update()
{
}

void CChatTask_AddFileToProject::Interrupt()
{
	_status = TaskStatus::Failure;
}
