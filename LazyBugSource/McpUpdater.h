#pragma once
#include "filewatcher/filewatcher.h"
#include <string>

class CMcpUpdater
{
public:
	CMcpUpdater();
	~CMcpUpdater();

	// 重新计算路径并重启监控
	void Reset();

	// 每帧调用，检测变化并更新
	// isDynOnly=true 时仅同步 Dynamic 类型 MCP，跳过目录监控和 .setting 文件检测
	bool Update(bool isDynOnly = false);

private:
	// 启动/停止监控
	void _StartWatchers();
	void _StopWatchers();

private:
	std::string _globalFolderPath;
	std::string _projectFolderPath;
	std::string _settingPath;

	CFileWatcher _globalWatcher;    // 监控Global _mcps目录
	CFileWatcher _projectWatcher;   // 监控Project _mcps目录

	FILETIME _settingFileTime;      // .setting文件的修改时间
};
