#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>

#include "timer/wuid.h"
#include "timer/timer.h"

typedef WUID FilesCheckpointUID;
#define FilesCheckpointUID_Invalid (0)

//代表一组文件的状态(内容,存在与否)
struct FilesCheckpoint
{
	struct Entry
	{
		std::string filePath;//full path
		std::vector<BYTE> content;//为空表示这个文件不存在
		AbsTick fileTime;
	};
	FilesCheckpointUID uid;
	bool isUndo;//是否用来undo restore的check point
	std::vector<Entry> entries;
};

class CCheckpoints
{
public:
	CCheckpoints()
	{
		Zero();
	}
	void Zero()
	{
	}
	void Init(const char *folderPath);
	void Clear();

	FilesCheckpointUID CreateEmptyCheckpoint();

	//根据一个文件名,根据它的内容,存在与否,生成一个checkpoint
	//将这个checkpoint保存到硬盘上,并返回一个id, checkpoint文件的名称为16进制的id,后缀为.cp
	FilesCheckpointUID AddCheckpoint(const char *filePath);

	//为check point添加一个文件的信息
	void AddFileToCheckpoint(FilesCheckpointUID checkpointId,const char* filePath);

	void RemoveFileFromCheckpoint(FilesCheckpointUID checkpointId, const char* filePath);

	//根据一组文件,创建一个checkpoint记录它们当前的状态
	FilesCheckpointUID CreateCheckpointFromFilelist(std::vector<const char*>& fileList);

	//沿着一系列checkpoint恢复到最后一个checkpoint
	//返回一个临时的checkpoint用来标记当前文件的状态(undo checkpoint)
	//如果传入了指定的undoCheckpoint,则会在这个undoCheckpoint添加文件信息
	FilesCheckpointUID Restore(const std::vector< FilesCheckpointUID>& checkpointChain, FilesCheckpointUID undoCheckpoint);

	bool CheckCheckpointsFilesModified(const std::vector< FilesCheckpointUID>& checkpointChain, std::vector<std::string>* modifiedFiles = nullptr);

	//恢复到一个checkpoint,不考虑undo
	bool RestoreNoUndo(FilesCheckpointUID checkpointId,const char *filePath);

	//用一个undo checkpoint Undo 之前的Restore(),并删除这个undo checkpoint
	void UndoRestore(FilesCheckpointUID checkpointId);

	//删除checkpoint文件
	void DiscardCheckpoint(FilesCheckpointUID checkpointId);

	//得到checkpoint的文件(.cp文件)的修改时间
	AbsTick GetCheckpointTime(FilesCheckpointUID checkpointId);

	//得到checkpoint里的文件列表
	bool GetCheckpointFileList(FilesCheckpointUID checkpointId, std::vector<const char*>& fileList);

	//得到checkpoint里某个文件的内容
	bool GetCheckpointFileContent(FilesCheckpointUID checkpointId, const char *filePath, std::vector<BYTE>& fileContent);
	bool GetCheckpointFileTick(FilesCheckpointUID checkpointId, const char* filePath, AbsTick &t);

	bool IsCheckpointContainingFile(FilesCheckpointUID checkpointId, const char* filePath) const;

protected:

	std::string _folderPath;

	// 辅助方法
	bool LoadCheckpoint(FilesCheckpointUID uid, FilesCheckpoint& checkpoint, bool loadContent = true) const;
	bool ApplyCheckpoint(const FilesCheckpoint& checkpoint,const char *filePath);
	bool SaveCheckpoint(const FilesCheckpoint& checkpoint);
	std::string GetCheckpointFilePath(FilesCheckpointUID uid) const;
	
	// 创建文件entry，读取指定文件的内容
	FilesCheckpoint::Entry CreateFileEntry(const char* filePath);

};