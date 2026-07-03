#include "stdh.h"

#include "Checkpoints.h"
#include "Utils.h"
#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"

void CCheckpoints::Init(const char *dbFolderPath)
{
	_folderPath = dbFolderPath;
	_folderPath += "\\_checkpoints";
	
	// 确保checkpoint文件夹存在
	Utils::EnsureFolder(_folderPath.c_str());
}

void CCheckpoints::Clear()
{
	_folderPath.clear();
}

FilesCheckpointUID CCheckpoints::CreateEmptyCheckpoint()
{
	// 创建新的checkpoint
	FilesCheckpoint checkpoint;
	checkpoint.uid = GenWUID();
	checkpoint.isUndo = false;

	// 保存checkpoint到硬盘
	if (SaveCheckpoint(checkpoint))
	{
		return checkpoint.uid;
	}

	return FilesCheckpointUID_Invalid;
}


FilesCheckpointUID CCheckpoints::AddCheckpoint(const char *filePath)
{
	if (!filePath)
		return FilesCheckpointUID_Invalid;
	
	// 创建新的checkpoint
	FilesCheckpoint checkpoint;
	checkpoint.uid = GenWUID();
	checkpoint.isUndo = false;
	
	// 添加文件信息
	FilesCheckpoint::Entry entry = CreateFileEntry(filePath);
	checkpoint.entries.push_back(entry);
	
	// 保存checkpoint到硬盘
	if (SaveCheckpoint(checkpoint))
	{
		return checkpoint.uid;
	}
	
	return FilesCheckpointUID_Invalid;
}

void CCheckpoints::AddFileToCheckpoint(FilesCheckpointUID checkpointId, const char* filePath)
{
	if (!filePath || checkpointId == FilesCheckpointUID_Invalid)
		return;
	
	// 加载现有checkpoint
	FilesCheckpoint checkpoint;
	if (!LoadCheckpoint(checkpointId, checkpoint))
		return;
	
	// 检查文件是否已存在
	for (auto& entry : checkpoint.entries)
	{
		if (entry.filePath == filePath)
		{
			return;
		}
	}
	
	// 添加新的entry
	FilesCheckpoint::Entry newEntry = CreateFileEntry(filePath);
	checkpoint.entries.push_back(newEntry);
	
	// 保存更新后的checkpoint
	SaveCheckpoint(checkpoint);
}

void CCheckpoints::RemoveFileFromCheckpoint(FilesCheckpointUID checkpointId, const char* filePath)
{
	if (!filePath || checkpointId == FilesCheckpointUID_Invalid)
		return;
	
	// 加载现有checkpoint
	FilesCheckpoint checkpoint;
	if (!LoadCheckpoint(checkpointId, checkpoint))
		return;
	
	// 查找并删除指定的文件entry
	for (auto it = checkpoint.entries.begin(); it != checkpoint.entries.end(); ++it)
	{
		if (it->filePath == filePath)
		{
			checkpoint.entries.erase(it);
			// 保存更新后的checkpoint
			SaveCheckpoint(checkpoint);
			return;
		}
	}
}

FilesCheckpointUID CCheckpoints::CreateCheckpointFromFilelist(std::vector<const char*>& fileList)
{
	if (fileList.empty())
		return FilesCheckpointUID_Invalid;
	
	// 创建新的checkpoint
	FilesCheckpoint checkpoint;
	checkpoint.uid = GenWUID();
	checkpoint.isUndo = false;
	
	// 为每个文件创建entry
	for (const char* filePath : fileList)
	{
		if (filePath && filePath[0] != '\0')
		{
			FilesCheckpoint::Entry entry = CreateFileEntry(filePath);
			checkpoint.entries.push_back(entry);
		}
	}
	
	// 保存checkpoint到硬盘
	if (SaveCheckpoint(checkpoint))
	{
		return checkpoint.uid;
	}
	
	return FilesCheckpointUID_Invalid;
}

FilesCheckpointUID CCheckpoints::Restore(const std::vector<FilesCheckpointUID>& checkpointChain, FilesCheckpointUID undoCheckpoint)
{
	if (checkpointChain.empty())
		return FilesCheckpointUID_Invalid;
	
	// 收集所有需要恢复的文件路径
	std::unordered_set<std::string> allFiles;
	for (FilesCheckpointUID uid : checkpointChain)
	{
		FilesCheckpoint cp;
		if (LoadCheckpoint(uid, cp))
		{
			for (const auto& entry : cp.entries)
			{
				allFiles.insert(entry.filePath);
			}
		}
	}
	
	FilesCheckpointUID resultUID = undoCheckpoint;
	
	if (undoCheckpoint == FilesCheckpointUID_Invalid)
	{
		// 创建新的undo checkpoint
		FilesCheckpoint currentState;
		currentState.uid = GenWUID();
		currentState.isUndo = true;
		
		// 记录当前状态
		for (const std::string& filePath : allFiles)
		{
			FilesCheckpoint::Entry entry = CreateFileEntry(filePath.c_str());
			currentState.entries.push_back(entry);
		}
		
		// 保存当前状态作为undo checkpoint
		if (SaveCheckpoint(currentState))
		{
			resultUID = currentState.uid;
		}
		else
		{
			return FilesCheckpointUID_Invalid;
		}
	}
	else
	{
		// 使用传入的undoCheckpoint，为其添加文件信息
		for (const std::string& filePath : allFiles)
		{
			AddFileToCheckpoint(undoCheckpoint, filePath.c_str());
		}
		resultUID = undoCheckpoint;
	}
	
	// 按顺序应用checkpoint链
	for (FilesCheckpointUID uid : checkpointChain)
	{
		FilesCheckpoint cp;
		if (LoadCheckpoint(uid, cp))
		{
			ApplyCheckpoint(cp,"");
		}
	}
	
	return resultUID;
}

bool CCheckpoints::CheckCheckpointsFilesModified(const std::vector<FilesCheckpointUID>& checkpointChain, std::vector<std::string>* modifiedFiles)
{
	if (checkpointChain.empty())
		return false;
	
	// 清空输出参数
	if (modifiedFiles)
		modifiedFiles->clear();
	
	// 收集所有文件及其在checkpoints中的最新修改时间
	std::unordered_map<std::string, AbsTick> fileLatestTimes;
	
	// 遍历checkpoint链，收集每个文件的最新修改时间
	for (FilesCheckpointUID uid : checkpointChain)
	{
		FilesCheckpoint checkpoint;
		if (!LoadCheckpoint(uid, checkpoint, false)) // 只加载元数据
			continue;
			
		for (const auto& entry : checkpoint.entries)
		{
			// 更新文件的最新修改时间
			auto it = fileLatestTimes.find(entry.filePath);
			if (it == fileLatestTimes.end() || entry.fileTime > it->second)
			{
				fileLatestTimes[entry.filePath] = entry.fileTime;
			}
		}
	}
	
	bool hasModification = false;
	
	// 检查每个文件的当前修改时间是否比checkpoint中记录的更新
	for (const auto& pair : fileLatestTimes)
	{
		const std::string& filePath = pair.first;
		AbsTick checkpointTime = pair.second;
		
		if (Utils::IsFileExist(filePath.c_str()))
		{
			AbsTick currentTime = Utils::GetFileTick(filePath.c_str());
			if (currentTime > checkpointTime)
			{
				hasModification = true;
				if (modifiedFiles)
					modifiedFiles->push_back(filePath);
			}
		}
		else
		{
			// 文件不存在，但checkpoint中有记录（且内容不为空）
			// 需要检查checkpoint中该文件是否有内容
			for (FilesCheckpointUID uid : checkpointChain)
			{
				std::vector<BYTE> content;
				if (GetCheckpointFileContent(uid, filePath.c_str(), content))
				{
					if (!content.empty())
					{
						hasModification = true;
						if (modifiedFiles)
							modifiedFiles->push_back(filePath);
					}
					break; // 找到了记录就退出
				}
			}
		}
	}
	
	return hasModification;
}

bool CCheckpoints::RestoreNoUndo(FilesCheckpointUID checkpointId,const char *filePath)
{
	FilesCheckpoint cp;
	if (LoadCheckpoint(checkpointId, cp))
	{
		return ApplyCheckpoint(cp,filePath);
	}
	return false;
}


void CCheckpoints::UndoRestore(FilesCheckpointUID checkpointId)
{
	if (checkpointId == FilesCheckpointUID_Invalid)
		return;
	
	// 加载undo checkpoint
	FilesCheckpoint undoCheckpoint;
	if (LoadCheckpoint(checkpointId, undoCheckpoint) && undoCheckpoint.isUndo)
	{
		// 恢复文件状态
		ApplyCheckpoint(undoCheckpoint,"");
		
		// 删除undo checkpoint文件
		std::string checkpointPath = GetCheckpointFilePath(checkpointId);
		Utils::RemoveFile(checkpointPath.c_str());
	}
}

bool CCheckpoints::LoadCheckpoint(FilesCheckpointUID uid, FilesCheckpoint& checkpoint, bool loadContent) const
{
	std::string checkpointPath = GetCheckpointFilePath(uid);
	
	std::vector<BYTE> fileData;
	if (!Utils::LoadFileContent(checkpointPath.c_str(), fileData))
		return false;
	
	if (fileData.empty())
		return false;
	
	// 使用CDataPacket读取数据
	CDataPacket dp;
	dp.SetDataBufferPointer(fileData.data());
	
	// 读取checkpoint数据
	checkpoint.uid = dp.Data_ReadSimple<FilesCheckpointUID>();
	checkpoint.isUndo = dp.Data_ReadSimple<bool>();
	
	DWORD entryCount = dp.Data_ReadSimple<DWORD>();
	
	checkpoint.entries.clear();
	checkpoint.entries.resize(entryCount);
	
	// Read metadata
	for (DWORD i = 0; i < entryCount; i++)
	{
		dp.Data_ReadString(checkpoint.entries[i].filePath);
		checkpoint.entries[i].fileTime = dp.Data_ReadSimple<AbsTick>();
	}

	if (loadContent)
	{
		// Read content
		for (DWORD i = 0; i < entryCount; i++)
		{
			DP_ReadVector(dp, checkpoint.entries[i].content);
		}
	}
	
	return true;
}

bool CCheckpoints::ApplyCheckpoint(const FilesCheckpoint& checkpoint, const char* filePath)
{
	for (const auto& entry : checkpoint.entries)
	{
		bool needSkip = false;
		if (filePath && filePath[0] != 0)
		{
			if (entry.filePath != filePath)
				needSkip = true;
		}
		
		if (needSkip)
			continue;
			
		if (entry.content.empty())
		{
			// 内容为空表示删除文件
			if (Utils::IsFileExist(entry.filePath.c_str()))
			{
				if (!Utils::RemoveFile(entry.filePath.c_str()))
					return false;
			}
		}
		else
		{
			// 恢复文件内容
			// 确保目录存在
			size_t lastSlash = entry.filePath.find_last_of("\\");
			if (lastSlash != std::string::npos)
			{
				std::string dirPath = entry.filePath.substr(0, lastSlash);
				Utils::EnsureFolder(dirPath.c_str());
			}
			
			if (!Utils::SaveFileContent(entry.filePath.c_str(), entry.content))
				return false;
				
			// 恢复文件的修改时间
			if (entry.fileTime != 0)
			{
				Utils::SetFileTick(entry.filePath.c_str(), entry.fileTime);
			}
		}
	}
	return true;
}

bool CCheckpoints::SaveCheckpoint(const FilesCheckpoint& checkpoint)
{
	if (_folderPath.empty())
		return false;
		
	std::string checkpointPath = GetCheckpointFilePath(checkpoint.uid);
	
	// 使用DP_BeginSave/DP_EndSave宏简化数据序列化
	std::vector<BYTE> buffer;
	
	DP_BeginSave(dp, buffer)
	{
		// 写入文件格式：
		// [UID][isUndo][entry_count]
		// [entry1_meta][entry2_meta]...
		// [entry1_content][entry2_content]...
		
		dp.Data_WriteSimple(checkpoint.uid);
		dp.Data_WriteSimple(checkpoint.isUndo);
		
		DWORD entryCount = (DWORD)checkpoint.entries.size();
		dp.Data_WriteSimple(entryCount);
		
		// 先写入所有元数据
		for (const auto& entry : checkpoint.entries)
		{
			dp.Data_WriteString(entry.filePath);
			dp.Data_WriteSimple(entry.fileTime);
		}

		// 再写入所有内容
		for (const auto& entry : checkpoint.entries)
		{
			DP_WriteVector(dp, entry.content);
		}
	}
	DP_EndSave()
	
	// 保存到文件
	return Utils::SaveFileContent(checkpointPath.c_str(), buffer);
}

std::string CCheckpoints::GetCheckpointFilePath(FilesCheckpointUID uid) const
{
	if (_folderPath.empty())
		return "";
		
	char hexStr[32];
	sprintf_s(hexStr, "%llx.cp", uid);
	return _folderPath + "\\" + hexStr;
}

void CCheckpoints::DiscardCheckpoint(FilesCheckpointUID checkpointId)
{
	if (checkpointId == FilesCheckpointUID_Invalid)
		return;
	
	// 获取checkpoint文件路径
	std::string checkpointPath = GetCheckpointFilePath(checkpointId);
	
	// 删除checkpoint文件
	if (Utils::IsFileExist(checkpointPath.c_str()))
	{
		Utils::RemoveFile(checkpointPath.c_str());
	}
}

FilesCheckpoint::Entry CCheckpoints::CreateFileEntry(const char* filePath)
{
	FilesCheckpoint::Entry entry;
	entry.filePath = filePath;
	entry.fileTime = 0;
	
	if (Utils::IsFileExist(filePath))
	{
		// 读取文件内容
		if (Utils::LoadFileContent(filePath, entry.content))
		{
			entry.fileTime = Utils::GetFileTick(filePath);
		}
	}
	// 如果文件不存在，content保持为空
	
	return entry;
}

AbsTick CCheckpoints::GetCheckpointTime(FilesCheckpointUID checkpointId)
{
	if (checkpointId == FilesCheckpointUID_Invalid)
		return 0;
		
	std::string checkpointPath = GetCheckpointFilePath(checkpointId);
	
	if (!Utils::IsFileExist(checkpointPath.c_str()))
		return 0;
		
	// 获取文件的修改时间
	return Utils::GetFileTick(checkpointPath.c_str());
}

bool CCheckpoints::IsCheckpointContainingFile(FilesCheckpointUID checkpointId, const char* filePath) const
{
	if (checkpointId == FilesCheckpointUID_Invalid || !filePath)
		return false;

	// 只加载元数据进行检查
	FilesCheckpoint checkpoint;
	if (!LoadCheckpoint(checkpointId, checkpoint, false))
		return false;

	// 查找指定文件
	for (const auto& entry : checkpoint.entries)
	{
		if (StringEqualNoCase(entry.filePath.c_str(), filePath))
			return true;
	}
	return false;
}

bool CCheckpoints::GetCheckpointFileList(FilesCheckpointUID checkpointId, std::vector<const char*>& fileList)
{
	if (checkpointId == FilesCheckpointUID_Invalid)
		return false;
		
	// 只加载元数据，不需要加载文件内容
	FilesCheckpoint checkpoint;
	if (!LoadCheckpoint(checkpointId, checkpoint, false))
		return false;
		
	// 清空输出列表
	fileList.clear();
	
	// 添加所有文件路径
	for (const auto& entry : checkpoint.entries)
	{
		fileList.push_back(entry.filePath.c_str());
	}
	
	return true;
}

bool CCheckpoints::GetCheckpointFileContent(FilesCheckpointUID checkpointId, const char *filePath, std::vector<BYTE>& fileContent)
{
	if (checkpointId == FilesCheckpointUID_Invalid || !filePath)
		return false;
		
	// 加载checkpoint
	FilesCheckpoint checkpoint;
	if (!LoadCheckpoint(checkpointId, checkpoint, true))
		return false;
		
	// 查找指定文件
	for (const auto& entry : checkpoint.entries)
	{
		if (StringEqualNoCase(entry.filePath.c_str(), filePath))
		{
			fileContent = entry.content;
			return true; // 即使content为空也返回true，表示找到了文件记录
		}
	}
	
	return false; // 没有找到文件记录
}

bool CCheckpoints::GetCheckpointFileTick(FilesCheckpointUID checkpointId, const char* filePath, AbsTick& t)
{
	if (checkpointId == FilesCheckpointUID_Invalid || !filePath)
		return false;

	// 只加载元数据
	FilesCheckpoint checkpoint;
	if (!LoadCheckpoint(checkpointId, checkpoint, false))
		return false;

	// 查找指定文件
	for (const auto& entry : checkpoint.entries)
	{
		if (entry.filePath == filePath)
		{
			t = entry.fileTime;
			return true; 
		}
	}

	return false; // 没有找到文件记录
}
