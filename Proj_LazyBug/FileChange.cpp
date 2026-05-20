#include "stdh.h"

#include "FileChange.h"
#include "Utils.h"
#include "FileSystem/IFileSystem.h"
#include "stringparser/stringparser.h"


//////////////////////////////////////////////////////////////////////////
//FileChange

void FileChange::LoadBrief(IFile* fl)
{
	if (!fl)
		return;

	// 读取变更类型
	IFile_ReadVar(fl, type);

	// 读取文件路径
	IFile_ReadString(fl, lowerCaseFullPath);

	// 读取时间戳
	IFile_ReadVar(fl, newTime);
	IFile_ReadVar(fl, oldTime);
}

void FileChange::LoadContent(IFile* fl)
{
	// 读取文件内容
	if (type == FileChange_Added || type == FileChange_Modified)
	{
		IFile_ReadVector(fl, newContent);
	}

	if (type == FileChange_Deleted || type == FileChange_Modified)
	{
		IFile_ReadVector(fl, oldContent);
	}

}


void FileChange::Load(IFile* fl)
{
	if (!fl)
		return;

	LoadBrief(fl);
	LoadContent(fl);
}

void FileChange::SaveBrief(IFile* fl) const
{
	if (!fl)
		return;

	// 读取变更类型
	IFile_WriteVar(fl, type);

	// 读取文件路径
	IFile_WriteString(fl, lowerCaseFullPath);

	// 读取时间戳
	IFile_WriteVar(fl, newTime);
	IFile_WriteVar(fl, oldTime);
}

void FileChange::SaveContent(IFile* fl) const
{
	// 写入文件内容
	if (type == FileChange_Added || type == FileChange_Modified)
	{
		IFile_WriteVector(fl, newContent);
	}

	if (type == FileChange_Deleted || type == FileChange_Modified)
	{
		IFile_WriteVector(fl, oldContent);
	}

}

void FileChange::Save(IFile* fl) const
{
	if (!fl)
		return;

	SaveBrief(fl);
	SaveContent(fl);

}

void FileChange::DiscardContent()
{
	// 清空内容数据，保留元数据
	newContent.clear();
	oldContent.clear();
}

