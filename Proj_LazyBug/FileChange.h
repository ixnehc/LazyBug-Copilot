#pragma once

#include <vector>

#include "FileSystem/IFileSystem.h"  // 添加对IFileSystem.h的包含

#include "timer/wuid.h"
#include "timer/timer.h"


enum FileChangeType
{
	FileChange_None,
	FileChange_Added,   // 数据库中有，基准文件夹中没有（新增）
	FileChange_Deleted, // 基准文件夹中有，数据库中没有（删除）
	FileChange_Modified // 两者都有，但内容不同（修改）
};

// 文件差异信息结构
struct FileChange
{
	FileChange()
	{
		Zero();
	}
	void Zero()
	{
		type = FileChange_None;
		memset(&newTime, 0, sizeof(newTime));
		memset(&oldTime, 0, sizeof(oldTime));
	}
	void Clear()
	{
		lowerCaseFullPath.clear();
		newContent.clear();
		oldContent.clear();

		Zero();
	}
	bool IsEmpty() { return type == FileChange_None; }

	bool Equals(const FileChange& other) const
	{
		if (type != other.type)
			return false;
		if (lowerCaseFullPath != other.lowerCaseFullPath)
			return false;
		if (memcmp(&newTime, &other.newTime, sizeof(FILETIME)) != 0)
			return false;
		if (memcmp(&oldTime, &other.oldTime, sizeof(FILETIME)) != 0)
			return false;
		if (newContent != other.newContent)
			return false;
		if (oldContent != other.oldContent)
			return false;
		return true;
	}

	void LoadBrief(IFile* fl);
	void SaveBrief(IFile* fl) const;
	void LoadContent(IFile* fl);
	void SaveContent(IFile* fl) const;
	void Load(IFile* fl);
	void Save(IFile* fl) const;

	void DiscardContent();

	//Brief部分
	FileChangeType type;       // 差异类型
	std::string lowerCaseFullPath; // 全小写
	FILETIME newTime;          // 数据库中的时间戳,目前没有使用
	FILETIME oldTime;        // 基准文件夹中的时间戳,目前没有使用

	//内容部分
	std::vector<BYTE> newContent;
	std::vector<BYTE> oldContent;
};
