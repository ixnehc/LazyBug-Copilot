#include "stdh.h"

#include "CheckpointsFileChange.h"

#include "stringparser/stringparser.h"

#include "Utils.h"

void CCheckpointsFileChange::SetCheckpoints(CCheckpoints* checkpoints)
{
	_checkpoints = checkpoints;
}

void CCheckpointsFileChange::Activate(FilesCheckpointUID oldCp, FilesCheckpointUID newCp, const char* filePath)
{
	if (!_checkpoints)
		return;

	_oldCheckpointId = oldCp;
	_newCheckpointId = newCp;
	_filePath = filePath ? filePath : "";
	_hasNewChange = false;
	_lastChangeTime = 0;
	
	// 缓存oldCheckpoint中的文件内容（只需要读取一次）
	_oldFileExists = false;
	_oldFileContent.clear();
	
	if (_checkpoints && _oldCheckpointId != FilesCheckpointUID_Invalid && !_filePath.empty())
	{
		_oldFileExists = _checkpoints->GetCheckpointFileContent(_oldCheckpointId, _filePath.c_str(), _oldFileContent);
	}
	
	// 初始化newCheckpoint的缓存
	_newCheckpointFileTime = 0;
	_newFileContent.clear();
	_newFileExists = false;
	
	// 立即检查一次变化并生成FileChange
	Update();
}

void CCheckpointsFileChange::Deactivate()
{
	_oldCheckpointId = FilesCheckpointUID_Invalid;
	_newCheckpointId = FilesCheckpointUID_Invalid;
	_filePath.clear();
	_hasNewChange = false;
	_lastChangeTime = 0;
	_currentChange.Clear();
	
	// 清理缓存的oldCheckpoint内容
	_oldFileContent.clear();
	_oldFileExists = false;
	
	// 清理缓存的newCheckpoint内容
	_newCheckpointFileTime = 0;
	_newFileContent.clear();
	_newFileExists = false;
}

void CCheckpointsFileChange::Update()
{
	if (!_checkpoints || 
		_oldCheckpointId == FilesCheckpointUID_Invalid || 
		_newCheckpointId == FilesCheckpointUID_Invalid ||
		_filePath.empty())
	{
		return;
	}
	
	// 首先检查newCheckpoint文件的修改时间是否发生了变化
	AbsTick currentNewCheckpointTime = _checkpoints->GetCheckpointTime(_newCheckpointId);
	
	// 如果时间没有变化且之前已经读取过，直接返回
	if (currentNewCheckpointTime == _newCheckpointFileTime && _newCheckpointFileTime != 0)
	{
		return;
	}
	
	// 时间发生了变化，需要重新读取newCheckpoint的文件内容
	_newCheckpointFileTime = currentNewCheckpointTime;
	_newFileContent.clear();
	_newFileExists = _checkpoints->GetCheckpointFileContent(_newCheckpointId, _filePath.c_str(), _newFileContent);
	
	// 判断文件变化类型
	FileChangeType changeType = FileChange_None;
	if (!_oldFileExists && _newFileExists)
	{
		changeType = FileChange_Added;
	}
	else if (_oldFileExists && !_newFileExists)
	{
		changeType = FileChange_Deleted;
	}
	else if (_oldFileExists && _newFileExists)
	{
		// 比较文件内容
		if (_oldFileContent != _newFileContent)
		{
			changeType = FileChange_Modified;
		}
	}
	
	// 如果有变化，更新FileChange
	if (changeType != FileChange_None)
	{
		_currentChange.Clear();
		_currentChange.type = changeType;
		
		_currentChange.lowerCaseFullPath = _filePath;
		StringLower(_currentChange.lowerCaseFullPath);
		Utils::FileContentCodingFormat codingFmt;
		Utils::ConvertFileContentIntoUTF8(_oldFileContent, _currentChange.oldContent,codingFmt);
		Utils::ConvertFileContentIntoUTF8(_newFileContent, _currentChange.newContent,codingFmt);
		
		// 转换为FILETIME (这里简化处理，实际可能需要更精确的转换)
		memset(&_currentChange.oldTime, 0, sizeof(FILETIME));
		memset(&_currentChange.newTime, 0, sizeof(FILETIME));
		
		_hasNewChange = true;
		_lastChangeTime = GetAbsTick();
	}
	else
	{
		// 没有变化，清空当前的FileChange
		_currentChange.Clear();
		_hasNewChange = false;
	}
}

AbsTick CCheckpointsFileChange::FetchNewFileChange()
{
	if (_hasNewChange)
	{
		AbsTick result = _lastChangeTime;
		_hasNewChange = false; // 取走后重置标志
		return result;
	}
	return 0;
}

FileChange* CCheckpointsFileChange::GetFileChange()
{
	if (_currentChange.IsEmpty())
		return nullptr;
	return &_currentChange;
}
