#include "stdh.h"
#include "McpUpdater.h"
#include "LlmMcps.h"
#include "LlmMcpServers.h"
#include "utils.h"
#include "Utils_Mcp.h"
#include "stringparser/stringparser.h"

CMcpUpdater::CMcpUpdater()
	: _settingFileTime{0}
{
}

CMcpUpdater::~CMcpUpdater()
{
	_StopWatchers();
}

void CMcpUpdater::Reset()
{
	_globalFolderPath = Utils::GetGlobalMcpsFolder();
	_projectFolderPath = Utils::GetProjectMcpsFolder();
	_settingPath = Utils::GetMcpSettingFilePath();
	_settingFileTime = {0};
	_StartWatchers();
}

//返回有没有发生重新载入
bool CMcpUpdater::Update()
{
	bool needReloadSetting = false;
	bool needReloadOutdated = false;
	bool needReloadTool = false;

	// 检查Global目录变化
	if (_globalWatcher.IsStarted())
	{
		const ChangedFileInformation* files = nullptr;
		int count = _globalWatcher.FetchChangedFiles(files);
		if (count > 0)
		{
			if (!_globalFolderPath.empty())
			{
				g_llmMcps.ReLoad(_globalFolderPath.c_str(), CLlmMcps::Mcp::Type::Global);
				needReloadSetting = true;
				needReloadOutdated = true;
				needReloadTool = true;
			}
		}
	}

	// 检查Project目录变化
	if (_projectWatcher.IsStarted())
	{
		const ChangedFileInformation* files = nullptr;
		int count = _projectWatcher.FetchChangedFiles(files);
		if (count > 0)
		{
			if (!_projectFolderPath.empty())
			{
				g_llmMcps.ReLoad(_projectFolderPath.c_str(), CLlmMcps::Mcp::Type::Project);
				needReloadSetting = true;
				needReloadOutdated = true;
				needReloadTool = true;
			}
		}
	}

	// 检查.setting文件变化
	if (!_settingPath.empty())
	{
		FILETIME fileTime = Utils::GetFileTime(_settingPath.c_str());
		if (CompareFileTime(&fileTime, &_settingFileTime) != 0)
		{
			needReloadSetting = true;
			needReloadTool = true;
		}
	}

	if (needReloadSetting)
	{
		FILETIME fileTime = Utils::GetFileTime(_settingPath.c_str());
		g_llmMcps.ReLoadSettings(_settingPath.c_str());
		_settingFileTime = fileTime;
	}

	// 目录下文件发生变化时，检查文件是否过期并更新
	if (needReloadOutdated)
	{
		g_llmMcps.UpdateReLoadOutdated();
	}

	// 同步更新g_llmMcpServers
	if (g_llmMcpServers.UpdateSync())
		needReloadTool = true;

	if (needReloadTool)
		g_llmMcpServers.LoadToolsToMcps();

	return needReloadSetting||needReloadOutdated||needReloadTool;
}

void CMcpUpdater::_StartWatchers()
{
	_StopWatchers();

	// 监控Global _mcps目录
	if (!_globalFolderPath.empty())
	{
		Utils::EnsureFolder(_globalFolderPath.c_str());
		_globalWatcher.Start(_globalFolderPath.c_str(), WNF_CHANGE_LAST_WRITE | WNF_CHANGE_FILE_NAME | WNF_CHANGE_DIR_NAME);
		g_llmMcps.ReLoad(_globalFolderPath.c_str(), CLlmMcps::Mcp::Type::Global);
	}

	// 监控Project _mcps目录
	if (!_projectFolderPath.empty())
	{
		Utils::EnsureFolder(_projectFolderPath.c_str());
		_projectWatcher.Start(_projectFolderPath.c_str(), WNF_CHANGE_LAST_WRITE | WNF_CHANGE_FILE_NAME | WNF_CHANGE_DIR_NAME);
		g_llmMcps.ReLoad(_projectFolderPath.c_str(), CLlmMcps::Mcp::Type::Project);
	}

	// 初始化.setting文件时间
	if (!_settingPath.empty())
	{
		_settingFileTime = Utils::GetFileTime(_settingPath.c_str());
		g_llmMcps.ReLoadSettings(_settingPath.c_str());
	}
}

void CMcpUpdater::_StopWatchers()
{
	_globalWatcher.Stop();
	_projectWatcher.Stop();
	_settingFileTime = {0};
}


