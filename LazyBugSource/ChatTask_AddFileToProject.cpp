#include "stdh.h"
#include "ChatTask_AddFileToProject.h"
#include "LlmChat.h"
#include "ChatAgent.h"
#include "Utils.h"
#include "Checkpoints.h"

extern std::wstring utf8_to_widechar(const char* utf8_str);

// 由宿主(VSIX 或独立 LazyBug.exe)提供的桥接实现
extern bool AddFileToProjectInVS(const unsigned short* projectFilePath, const unsigned short* fileFullPath, char* errorMsg, int errorMsgSize);

// 项目文件落盘到磁盘的等待时间（VS 的 SaveSolutionElement 可能是异步 flush）
static const unsigned long PROJECT_EDIT_CHECKPOINT_DELAY_MS = 2000;

CChatTask_AddFileToProject::CChatTask_AddFileToProject()
	: _delayStartTick(0)
	, _bridgeDone(false)
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

	std::wstring wProjectPath = utf8_to_widechar(_projectFilePath.c_str());
	std::wstring wFilePath = utf8_to_widechar(_fileFullPath.c_str());

	// 记录 ProjectEdit 所属的 AI 消息 ID
	std::wstring aiMessageId;
	if (_context && _context->chatAgent)
		aiMessageId = _context->chatAgent->GetCurrentAIMessageId();

	// 构建受影响的文件列表：.vcxproj 恒加入，.vcxproj.filters 存在才加入
	_checkpointFilePaths.clear();
	_checkpointFilePaths.push_back(_projectFilePath);
	std::string filtersPath = _projectFilePath + ".filters";
	if (Utils::IsFileExist(filtersPath.c_str()))
		_checkpointFilePaths.push_back(filtersPath);

	std::string errorMsg;

	// 本次 ProjectEdit op 即将插入 _ops 的位置，before-checkpoint 需从此位置向前查找。
	// 使用 disable 边界（最后一个未 disable 的 op 之后），避免从被 disable 的旧 op 之后查找。
	int projEditIndex = 0;
	if (_context && _context->chatOpsCtrl)
		projEditIndex = _context->chatOpsCtrl->GetDisableAfterIndex();

	if (!_BeginProjectEditCheckpoint(projEditIndex, _checkpointFilePaths, errorMsg))
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

	// bridge 已成功，after-edit checkpoint 延迟到 Update 中写入，
	// 以便 VS 将项目文件（SaveSolutionElement）异步落盘完成后再读取。
	_aiMessageId = aiMessageId;
	_description = "Added file \"" + _fileFullPath + "\" to project \"" + _projectFilePath + "\"";
	_bridgeDone = true;
	_delayStartTick = GetTickCount();
}

void CChatTask_AddFileToProject::Update()
{
	if (!_bridgeDone)
		return;

	if (GetTickCount() - _delayStartTick < PROJECT_EDIT_CHECKPOINT_DELAY_MS)
		return;

	_bridgeDone = false;
	_FinishCheckpoint();
}

void CChatTask_AddFileToProject::_FinishCheckpoint()
{
	std::string errorMsg;
	if (!_EndProjectEditCheckpoint(_aiMessageId, _projectFilePath, _checkpointFilePaths, _description, errorMsg))
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
