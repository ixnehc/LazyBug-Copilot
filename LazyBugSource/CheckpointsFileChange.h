#pragma once

#include "Checkpoints.h"
#include "FileChange.h"
#include "timer/timer.h"

//检测并持续监控两个checkpoint里某个文件的区别,产生新的FileChange
class CCheckpointsFileChange
{
public:
	CCheckpointsFileChange()
	{
		_checkpoints = nullptr;
		_oldCheckpointId = FilesCheckpointUID_Invalid;
		_newCheckpointId = FilesCheckpointUID_Invalid;
		_hasNewChange = false;
		_lastChangeTime = 0;
		_oldFileExists = false;
		_newCheckpointFileTime = 0;
		_newFileExists = false;
	}

	void SetCheckpoints(CCheckpoints* checkpoints);

	bool IsActivated()	{		return _oldCheckpointId != FilesCheckpointUID_Invalid || _newCheckpointId != FilesCheckpointUID_Invalid;	}

	//开始工作,在两个checkpoint之间比较,检查指定文件的变化,生成一个file change
	void Activate(FilesCheckpointUID oldCp, FilesCheckpointUID newCp, const char* filePath);

	//停止工作
	void Deactivate();

	//监控新的checkpoint,如果新的checkpoint里的文件内容发生了变化,就产生一个新的file change
	void Update();

	//取走最近一次新的FileChange产生的时间,返回0表示没有新的FileChange产生
	AbsTick FetchNewFileChange();

	//得到当前的FileChange
	FileChange* GetFileChange();
protected:

	CCheckpoints* _checkpoints;

	FilesCheckpointUID _oldCheckpointId;
	FilesCheckpointUID _newCheckpointId;
	std::string _filePath;
	
	FileChange _currentChange;
	bool _hasNewChange;
	AbsTick _lastChangeTime;
	
	// 缓存oldCheckpoint中的文件内容，避免重复读取
	std::vector<BYTE> _oldFileContent;
	bool _oldFileExists;
	
	// 缓存newCheckpoint的文件修改时间和内容，用于检测变化
	AbsTick _newCheckpointFileTime;
	std::vector<BYTE> _newFileContent;
	bool _newFileExists;
};