#include "stdh.h"
#include "ChatTask_AddFileToProject.h"
#include "LlmChat.h"
#include "ChatAgent.h"
#include "Utils.h"
#include "Checkpoints.h"

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
	if (!_fileFullPath.empty())
		result += "\nFile: " + _fileFullPath;
	if (!_projectFilePath.empty())
		result += "\nProject: " + _projectFilePath;
	_SendToolCallResult(result.c_str());
	_SendToolCallMessage_Execution(result.c_str());
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

	// 构建受影响的文件列表：.vcxproj 恒加入，.vcxproj.filters 存在才加入
	std::vector<std::string> checkpointFilePaths;
	checkpointFilePaths.push_back(_projectFilePath);
	std::string filtersPath = _projectFilePath + ".filters";
	if (Utils::IsFileExist(filtersPath.c_str()))
		checkpointFilePaths.push_back(filtersPath);

	// 目标项目文件（含 .filters）只读时无法写入，直接失败返回
	for (const std::string& f : checkpointFilePaths)
	{
		if (Utils::IsFileReadOnly(f.c_str()))
		{
			_Fail(("Target project file is read-only: " + f).c_str());
			return;
		}
	}

	std::wstring wProjectPath = utf8_to_widechar(_projectFilePath.c_str());
	std::wstring wFilePath = utf8_to_widechar(_fileFullPath.c_str());

	// 记录 ProjectEdit 所属的 AI 消息 ID
	std::wstring aiMessageId;
	if (_context && _context->chatAgent)
		aiMessageId = _context->chatAgent->GetCurrentAIMessageId();

	std::string errorMsg;

	// 本次 ProjectEdit op 即将插入 _ops 的位置，before-checkpoint 需从此位置向前查找。
	// 使用 disable 边界（最后一个未 disable 的 op 之后），避免从被 disable 的旧 op 之后查找。
	int projEditIndex = 0;
	if (_context && _context->chatOpsCtrl)
		projEditIndex = _context->chatOpsCtrl->GetDisableAfterIndex();

	if (!_BeginProjectEditCheckpoint(projEditIndex, checkpointFilePaths, errorMsg))
	{
		_Fail(errorMsg.c_str());
		return;
	}

	char bridgeErrorMsg[512] = { 0 };
	bool ok = AddFileToProjectInVS(
		(const unsigned short*)wProjectPath.c_str(),
		(const unsigned short*)wFilePath.c_str(),
		bridgeErrorMsg,
		(int)sizeof(bridgeErrorMsg));

	if (!ok)
	{
		_Fail(bridgeErrorMsg[0] ? bridgeErrorMsg : "Failed to add file to project");
		return;
	}

	// bridge 已成功，立即写入 after-edit checkpoint
	std::string description = "Added file \"" + _fileFullPath + "\" to project \"" + _projectFilePath + "\"";

	if (!_EndProjectEditCheckpoint(aiMessageId, _projectFilePath, checkpointFilePaths, description, errorMsg))
	{
		std::string result = "File added to project, but failed to record checkpoint: " + errorMsg;
		if (!_fileFullPath.empty())
			result += "\nFile: " + _fileFullPath;
		if (!_projectFilePath.empty())
			result += "\nProject: " + _projectFilePath;
		_SendToolCallResult(result.c_str());
		_SendToolCallMessage_Execution(result.c_str());
		_status = TaskStatus::Failure;
		return;
	}

	std::string result = "Successfully added file to project.";
	if (!_fileFullPath.empty())
		result += "\nFile: " + _fileFullPath;
	if (!_projectFilePath.empty())
		result += "\nProject: " + _projectFilePath;
	_SendToolCallResult(result.c_str());
	_SendToolCallMessage_Execution(result.c_str());
	_status = TaskStatus::Success;
}

void CChatTask_AddFileToProject::Interrupt()
{
	_status = TaskStatus::Failure;
}
