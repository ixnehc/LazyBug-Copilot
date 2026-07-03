#include "stdh.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cctype>
#include <mutex>

#include "Changelists.h"
#include "VcxprojDatabase.h"
#include "Utils.h"
#include "FileSystem/IFileSystem.h"
#include "stringparser/stringparser.h"
#include <sys/stat.h>
#include <string>


//////////////////////////////////////////////////////////////////////////
//FileChange

void FileChange::LoadBrief(IFile* fl)
{
	if (!fl)
		return;

	// 读取变更类型
	IFile_ReadVar(fl, type);

	// 读取文件路径
	IFile_ReadString(fl, relativePath);

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
	IFile_WriteString(fl, relativePath);

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

//////////////////////////////////////////////////////////////////////////
//FileChangelist

void FileChangelist::LoadBrief(IFile* fl)
{
	if (!fl)
		return;

	// 读取UID和父UID
	IFile_ReadVar(fl, uid);
	IFile_ReadVar(fl, parent);

	IFile_ReadVar(fl, isCommit);

	// 读取描述
	IFile_ReadString(fl, desc);

	// 读取变更数量
	int changeCount;
	IFile_ReadVar(fl, changeCount);

	// 调整 vector 大小并逐个加载摘要信息
	changes.resize(changeCount);
	for (int i = 0; i < changeCount; ++i)
	{
		changes[i].LoadBrief(fl);
	}
}

void FileChangelist::Load(IFile* fl)
{
	if (!fl)
		return;

	// 读取UID和父UID
	IFile_ReadVar(fl, uid);
	IFile_ReadVar(fl, parent);

	IFile_ReadVar(fl, isCommit);

	// 读取描述
	IFile_ReadString(fl, desc);

	// 读取变更数量
	int changeCount;
	IFile_ReadVar(fl, changeCount);

	// 调整 vector 大小并逐个加载完整信息
	changes.resize(changeCount);
	for (int i = 0; i < changeCount; ++i)
	{
		changes[i].LoadBrief(fl);
	}

	for (int i = 0; i < changeCount; ++i)
	{
		changes[i].LoadContent(fl);
	}
	isFullLoaded = true;
}

void FileChangelist::Save(IFile* fl) const
{
	if (!fl)
		return;

	// 写入UID和父UID
	IFile_WriteVar(fl, uid);
	IFile_WriteVar(fl, parent);

	IFile_WriteVar(fl, isCommit);

	// 写入描述
	IFile_WriteString(fl, desc);

	// 写入变更数量
	int changeCount = static_cast<int>(changes.size());
	IFile_WriteVar(fl, changeCount);

	// 逐个写入变更项
	for (const FileChange& change : changes)
	{
		change.SaveBrief(fl);
	}

	for (const FileChange& change : changes)
	{
		change.SaveContent(fl);
	}

}

void FileChangelist::DiscardContent()
{
	// 对每个变更项，丢弃其内容
	for (FileChange& change : changes)
	{
		change.DiscardContent();
	}
	isFullLoaded = false;
}

//////////////////////////////////////////////////////////////////////////
//

void CChangelists::Init(CSolutionDB* vcxprojDB)
{
	_vcxprojDB = vcxprojDB;

	std::string folder = _vcxprojDB->_pathDBFolder;
	folder += "\\_changelists";

	IFileSystem_EnsureFolderAbs(_vcxprojDB->_pFS, folder.c_str());

	_Load(folder.c_str());

}

void CChangelists::Clear()
{
	_briefs.clear();
	Zero();
}


void CChangelists::_Load(const char* folder)
{
	if (!_vcxprojDB || !_vcxprojDB->_pFS)
	{
		return;
	}

	IFileSystem* pFS = _vcxprojDB->_pFS;

	//读取folder目录下的所有文件，每一个文件是一个FileChangelist,文件名是FileChangelist的uid,以16进制表示
	//读取的内容存放在_briefs中
	if(true)	
	{
		// 获取文件夹中的所有文件
		std::vector<std::string> files;
		IFileSystem_EnumFiles(pFS, folder, files);
		
		// 遍历所有文件
		for (const std::string& filePath : files)
		{
			// 获取文件名（不包含路径）
			std::string fileName = GetFileTitle(filePath);

			if (fileName == "cur")
				continue;
			
			// 尝试将文件名解析为十六进制的UID
			FileChangeListUID uid = FileChangeListUID_Invalid;
			try
			{
				// 将十六进制字符串转换为整数
				uid = std::stoull(fileName, nullptr, 16);
			}
			catch (const std::exception&)
			{
				// 如果转换失败，跳过此文件
				continue;
			}
			
			// 如果UID有效，则读取文件内容
			if (uid != FileChangeListUID_Invalid)
			{
				// 打开文件
				std::string fullPath = folder;
				fullPath = fullPath + "\\" + filePath;
				IFile* pFile = pFS->OpenFileAbs(fullPath.c_str(), FileAccessMode_Read);
				if (pFile)
				{
					// 创建变更列表对象
					FileChangelist changelist;
					
					// 只加载摘要信息
					changelist.LoadBrief(pFile);
					
					// 关闭文件
					pFS->CloseFile(pFile);
					
					// 将变更列表添加到_briefs映射中
					_briefs[uid] = changelist;
				}
			}
		}
	}

	_LoadCur();

	_ver++;
}

bool CChangelists::_LoadCur()
{
	if (!_vcxprojDB || !_vcxprojDB->_pFS)
		return false;

	//以"cur.d"为文件名，读取_curUID
	std::string filePath = _vcxprojDB->_pathDBFolder + "\\_changelists\\cur.d";
	IFile* pFile = _vcxprojDB->_pFS->OpenFileAbs(filePath.c_str(), FileAccessMode_Read);
	if (pFile)
	{
		DWORD ver;
		IFile_ReadVar(pFile, ver);
		IFile_ReadVar(pFile, _curUID);
		_vcxprojDB->_pFS->CloseFile(pFile);
		return true;
	}
	return false;
}


void CChangelists::_SaveCur()
{
	if (!_vcxprojDB || !_vcxprojDB->_pFS)
		return;

	//以"cur.d"为文件名，保存_curUID到文件中
	std::string filePath = _vcxprojDB->_pathDBFolder + "\\_changelists\\cur.d";
	IFile* pFile = _vcxprojDB->_pFS->OpenFileAbs(filePath.c_str(), FileAccessMode_Write);
	if (pFile)
	{
		DWORD ver=1;
		IFile_WriteVar(pFile, ver);
		IFile_WriteVar(pFile, _curUID);
		_vcxprojDB->_pFS->CloseFile(pFile);
	}
}

void CChangelists::_BuildCurChangelist(IFileSystem *pFS,std::vector<FileChange>& collectedChanges)
{
	// 定义排序比较函数
	auto compareFileChanges = [](const FileChange& a, const FileChange& b) -> bool
	{
		// 定义类型的优先级：Modified=0, Added=1, Deleted=2
		int typePriorityA, typePriorityB;
		switch (a.type)
		{
		case FileChange_Modified: typePriorityA = 0; break;
		case FileChange_Added:    typePriorityA = 1; break;
		case FileChange_Deleted:  typePriorityA = 2; break;
		default:                  typePriorityA = 3; break; // FileChange_None 或其他
		}
		switch (b.type)
		{
		case FileChange_Modified: typePriorityB = 0; break;
		case FileChange_Added:    typePriorityB = 1; break;
		case FileChange_Deleted:  typePriorityB = 2; break;
		default:                  typePriorityB = 3; break; // FileChange_None 或其他
		}

		if (typePriorityA != typePriorityB)
		{
			return typePriorityA < typePriorityB; // 类型优先级高的在前
		}
		else
		{
			// 类型相同，按 relativePath 字符串比较（已小写）
			return a.relativePath < b.relativePath;
		}
	};

	// 对收集到的变更进行排序
	std::sort(collectedChanges.begin(), collectedChanges.end(), compareFileChanges);

	if (_curUID == FileChangeListUID_Invalid)
	{
		if (collectedChanges.empty())
			return;

		FileChangelist newCl;
		newCl.uid = GenWUID();
		newCl.parent = FileChangeListUID_Invalid;

		_curUID = newCl.uid;

		_SaveChangelist(pFS, newCl);
		_SaveCur();

		_briefs[newCl.uid] = std::move(newCl);


		_ver++;
	}

	std::unordered_map< FileChangeListUID, FileChangelist>::iterator it = _briefs.find(_curUID);
	if (it != _briefs.end())
	{
		FileChangelist& cur = (*it).second;
		if (!cur.isCommit)
		{
			cur.changes = std::move(collectedChanges);
			_SaveChangelist(pFS, cur);
			cur.DiscardContent();
		}
		else
		{
			if (collectedChanges.empty())
				return;

			FileChangelist newCl;
			newCl.uid = GenWUID();
			newCl.parent = _curUID;
			newCl.changes = std::move(collectedChanges);

			_curUID = newCl.uid;

			_SaveChangelist(pFS, newCl);
			_SaveCur();

 			newCl.DiscardContent();
			_briefs[newCl.uid] = std::move(newCl);
		}

		_ver++;
	}
}

bool CChangelists::RefreshCur(const std::unordered_set<std::string>& candidateFiles0)
{
	if (!_vcxprojDB || !_vcxprojDB->_pFS)
	{
		return false;
	}

	std::unordered_set<std::string> effectiveCandidateFiles;
	for (const std::string& loweredRelativePath : candidateFiles0)
	{
		if (_vcxprojDB->_files.Exists(loweredRelativePath))
			effectiveCandidateFiles.insert(loweredRelativePath);
	}
	if (effectiveCandidateFiles.size() <= 0)
		return false;

	// Merge paths from the current uncommitted changelist
	if (_curUID != FileChangeListUID_Invalid)
	{
		FileChangelist* curCl = _FindChangelist(_curUID);
		if (curCl && !curCl->isCommit)
		{
			for (const auto& change : curCl->changes)
			{
				std::string path = change.relativePath;
				StringLower(path);
				effectiveCandidateFiles.insert(path);
			}
		}
	}

	std::vector<FileChange> collectedChanges;
	IFileSystem* pFS = _vcxprojDB->_pFS;
	std::string baseFolder = _vcxprojDB->_pathDBFolder + "\\_base";

	if (!pFS->ExistFolderAbs(baseFolder.c_str()))
	{
		// If base folder doesn't exist, all existing workspace files in candidates are 'Added'.
        // However, the original RefreshCur() returns. Let's stick to that for consistency.
        // Alternatively, handle this by treating all workspace files as 'Added'.
		return false; 
	}

	for (const std::string& loweredRelativePath : effectiveCandidateFiles)
	{
		std::string newFilePath = _vcxprojDB->_setting.pathWorkspace + "\\" + loweredRelativePath;
		std::string baseFilePath = baseFolder + "\\" + loweredRelativePath;

		
		bool workspaceExists = _vcxprojDB->_files.Exists(loweredRelativePath)&&pFS->ExistFileAbs(newFilePath.c_str());
		bool baseExists = pFS->ExistFileAbs(baseFilePath.c_str());

		FileChange change;
		change.relativePath = loweredRelativePath;

		if (workspaceExists)
		{
			if (baseExists)
			{
				// File exists in both workspace and base, check for modification
				FILETIME newTime = Utils::GetFileTime(newFilePath.c_str());
				FILETIME oldTime = Utils::GetFileTime(baseFilePath.c_str());

				if (!Utils::EqualFileTime(newTime, oldTime))
				{
					change.type = FileChange_Modified;
					change.newTime = newTime;
					change.oldTime = oldTime;
					Utils::GetFileContentIntoUTF8(newFilePath.c_str(), change.newContent);
					Utils::GetFileContentIntoUTF8(baseFilePath.c_str(), change.oldContent);

					if (change.newContent != change.oldContent) // Double check content if times differ or if times are unreliable
					{
						collectedChanges.push_back(change);
					}
				}
			}
			else
			{
				// File exists in workspace, not in base -> Added
				change.type = FileChange_Added;
				change.newTime = Utils::GetFileTime(newFilePath.c_str());
				change.oldTime = Utils::GetZeroFileTime();
				Utils::GetFileContentIntoUTF8(newFilePath.c_str(), change.newContent);
				collectedChanges.push_back(change);
			}
		}
		else
		{
			if (baseExists)
			{
				// File does not exist in workspace, but exists in base -> Deleted
				change.type = FileChange_Deleted;
				change.newTime = Utils::GetZeroFileTime();
				change.oldTime = Utils::GetFileTime(baseFilePath.c_str());
				Utils::GetFileContentIntoUTF8(baseFilePath.c_str(), change.oldContent);
				collectedChanges.push_back(change);
			}
			// Else (neither workspace nor base exists): This candidate file doesn't represent a change. Do nothing.
		}
	}

	_BuildCurChangelist(pFS, collectedChanges);

	return true;
}

void CChangelists::RefreshCur()
{
	//比较当前workspace下文件和base目录下的文件,生成一个change list记录在_cur里
	if (!_vcxprojDB || !_vcxprojDB->_pFS)
	{
		return;
	}

	// 使用 vector 收集变更，以便排序
	std::vector<FileChange> collectedChanges;

	// 获取文件系统接口
	IFileSystem* pFS = _vcxprojDB->_pFS;
	
	// 构建基准目录路径
	std::string baseFolder = _vcxprojDB->_pathDBFolder + "\\_base";
	
	// 检查基准目录是否存在
	if (!pFS->ExistFolderAbs(baseFolder.c_str()))
	{
		return; // 基准目录不存在，无法比较
	}
	
	// 获取基准目录中的所有文件
	std::vector<std::string> baseFiles;//workspace下的相对路径
	IFileSystem_EnumFilesR(pFS, baseFolder.c_str(), baseFiles);
	
	// 创建基准文件映射表（相对路径 -> 完整路径）
	std::unordered_set<std::string> loweredBaseFileMap;
	
	// 处理基准文件夹中的所有文件，提取相对路径
	for (const std::string& path : baseFiles)
	{
		std::string lowered = path;
		StringLower(lowered);
		loweredBaseFileMap.insert(lowered);
	}
	
	// 遍历数据库中的文件
	{
		std::shared_lock< std::shared_mutex> lock(_files._filesMutex);
		for (const auto& fileEntry : _vcxprojDB->_files._lowerCasedFiles)
		{
			const std::string& loweredRelativePath = fileEntry.first;
			std::string newFilePath = _vcxprojDB->_setting.pathWorkspace + "\\\\" + loweredRelativePath;
			std::string baseFilePath = baseFolder + "\\\\" + loweredRelativePath;

			if (!pFS->ExistFileAbs(newFilePath.c_str()))
				continue;

			auto baseIt = loweredBaseFileMap.find(loweredRelativePath);

			if (baseIt == loweredBaseFileMap.end())
			{
				// 数据库中有，但基准文件夹中没有 -> 新增
				FileChange change;
				change.type = FileChange_Added;
				change.relativePath = loweredRelativePath;
				change.newTime = Utils::GetFileTime(newFilePath.c_str());
				change.oldTime = Utils::GetZeroFileTime();

				// 读取新文件内容
				Utils::GetFileContentIntoUTF8(newFilePath.c_str(), change.newContent);

				// 添加到收集列表
				collectedChanges.push_back(std::move(change));
			}
			else
			{
				// 两者都有，检查是否修改
				FILETIME newTime = Utils::GetFileTime(newFilePath.c_str());
				FILETIME oldTime = Utils::GetFileTime(baseFilePath.c_str());

				if (!Utils::EqualFileTime(newTime, oldTime))
				{
					// 时间戳不同，认为文件已修改
					FileChange change;
					change.type = FileChange_Modified;
					change.relativePath = loweredRelativePath;
					change.newTime = newTime;
					change.oldTime = oldTime;

					// 读取新文件内容
					Utils::GetFileContentIntoUTF8(newFilePath.c_str(), change.newContent);
					Utils::GetFileContentIntoUTF8(baseFilePath.c_str(), change.oldContent);

					if (!(change.newContent == change.oldContent))
						collectedChanges.push_back(std::move(change));// 添加到收集列表
				}
				// 从基准文件列表中移除已处理的文件
				loweredBaseFileMap.erase(baseIt);
			}
		}
	}
	
	// 剩余在基准文件夹中但不在数据库中的文件 -> 删除的文件
	for (const auto& baseEntry : loweredBaseFileMap)
	{
		std::string baseFilePath = baseFolder + "\\\\" + baseEntry;

		FileChange change;
		change.type = FileChange_Deleted;
		change.relativePath = baseEntry;
		change.newTime = Utils::GetZeroFileTime();
		change.oldTime = Utils::GetFileTime(baseFilePath.c_str());
		
		// 读取旧文件内容
		Utils::GetFileContentIntoUTF8(baseFilePath.c_str(), change.oldContent);

		// 添加到收集列表
		collectedChanges.push_back(std::move(change));
	}

	_BuildCurChangelist(pFS,collectedChanges);
}

void CChangelists::_SaveChangelist(IFileSystem* pFS, const FileChangelist& cl)
{
	// 以UID的十六进制值作为文件名保存变更列表
	std::stringstream ss;
	ss << std::hex << cl.uid;
	std::string fileName = ss.str() + ".d";

	// 构建完整路径
	std::string changesFolder = _vcxprojDB->_pathDBFolder + "\\_changelists";
	std::string filePath = changesFolder + "\\" + fileName;

	// 保存文件
	IFile* pFile = pFS->OpenFileAbs(filePath.c_str(), FileAccessMode_Write);
	if (pFile)
	{
		cl.Save(pFile);
		pFS->CloseFile(pFile);
	}
}

void CChangelists::_LoadChangelist(IFileSystem* pFS, FileChangeListUID uid, FileChangelist& cl)
{
	// 以UID的十六进制值作为文件名保存变更列表
	std::stringstream ss;
	ss << std::hex << uid;
	std::string fileName = ss.str() + ".d";

	// 构建完整路径
	std::string changesFolder = _vcxprojDB->_pathDBFolder + "\\_changelists";
	std::string filePath = changesFolder + "\\" + fileName;

	// 保存文件
	IFile* pFile = pFS->OpenFileAbs(filePath.c_str(), FileAccessMode_Read);
	if (pFile)
	{
		cl.Load(pFile);
		pFS->CloseFile(pFile);
	}
}


FileChangelist* CChangelists::_FindChangelist(FileChangeListUID uid)
{
	std::unordered_map< FileChangeListUID, FileChangelist>::iterator it= _briefs.find(uid);
	if (it == _briefs.end())
		return nullptr;
	return &(*it).second;
}

void CChangelists::EnsureFullLoaded(FileChangeListUID uid)
{
	//比较当前workspace下文件和base目录下的文件,生成一个change list记录在_cur里
	if (!_vcxprojDB || !_vcxprojDB->_pFS)
		return;

	// 获取文件系统接口
	IFileSystem* pFS = _vcxprojDB->_pFS;

	FileChangelist* changelist = _FindChangelist(uid);
	if (changelist)
	{
		if (!changelist->IsFullLoaded())
			_LoadChangelist(_vcxprojDB->_pFS, uid, *changelist);
	}
}


// 将_cur的change commit到base目录
// 将_cur保存到_briefs中去,并清空_cur
void CChangelists::CommitCur()
{
	if (!_vcxprojDB || !_vcxprojDB->_pFS || _curUID == FileChangeListUID_Invalid)
	{
		return;
	}


	// 获取文件系统接口
	IFileSystem* pFS = _vcxprojDB->_pFS;
	
	// 构建基准目录路径
	std::string baseFolder = _vcxprojDB->_pathDBFolder + "\\_base";

	FileChangelist* curCl = _FindChangelist(_curUID);
	if (!curCl)
		return;
	if (curCl->isCommit)
		return;

	_LoadChangelist(pFS, curCl->uid, *curCl);
	
	// 应用变更到base目录
	for (FileChange& change : curCl->changes)
	{
		const std::string& relativePath = change.relativePath;
		
		// 构建目标文件路径
		std::string baseFilePath = baseFolder + "\\" + relativePath;
		
		switch (change.type)
		{
		case FileChange_Added:
		case FileChange_Modified:
			{
				// 保存新内容到文件
				IFile* pFile = pFS->OpenFileAbs(baseFilePath.c_str(), FileAccessMode_Write);
				if (pFile)
				{
					pFile->Write(&change.newContent[0], (DWORD)change.newContent.size());
					pFS->CloseFile(pFile);

					pFS->SetFileTimeAbs(baseFilePath.c_str(), change.newTime);
				}
			}
			break;
			
		case FileChange_Deleted:
			// 删除文件
			if (pFS->ExistFileAbs(baseFilePath.c_str()))
			{
				pFS->RemoveFileAbs(baseFilePath.c_str());
			}
			break;
		}
	}

	curCl->isCommit = true;

	_SaveChangelist(pFS, *curCl);

	curCl->DiscardContent();

	_ver++;
}

// 应用 changelist 的变更到基准目录
void CChangelists::_ApplyChangelist(IFileSystem* pFS, const FileChangelist& cl, const std::string& baseFolder)
{
    // 加载 changelist 的完整内容
    _LoadChangelist(pFS, cl.uid, const_cast<FileChangelist&>(cl));
    
    // 应用变更
    for (const FileChange& change : cl.changes)
    {
        std::string baseFilePath = baseFolder + "\\" + change.relativePath;
        
        switch (change.type)
        {
        case FileChange_Added:
        case FileChange_Modified:
            // 写入新内容
            {
                IFile* pFile = pFS->OpenFileAbs(baseFilePath.c_str(), FileAccessMode_Write);
                if (pFile)
                {
					pFile->Write(&change.newContent[0], (DWORD)change.newContent.size());
                    pFS->CloseFile(pFile);
                    pFS->SetFileTimeAbs(baseFilePath.c_str(), change.newTime);
                }
            }
            break;
            
        case FileChange_Deleted:
            // 删除文件
            if (pFS->ExistFileAbs(baseFilePath.c_str()))
            {
                pFS->RemoveFileAbs(baseFilePath.c_str());
            }
            break;
        }
    }
    
    // 释放内容
    const_cast<FileChangelist&>(cl).DiscardContent();
}

// 撤销 changelist 的变更到基准目录
void CChangelists::_UnapplyChangelist(IFileSystem* pFS, const FileChangelist& cl, const std::string& baseFolder)
{
    // 加载 changelist 的完整内容
    _LoadChangelist(pFS, cl.uid, const_cast<FileChangelist&>(cl));
    
    // 反向应用变更
    for (auto it = cl.changes.rbegin(); it != cl.changes.rend(); ++it)
    {
        const FileChange& change = *it;
        std::string baseFilePath = baseFolder + "\\" + change.relativePath;
        
        switch (change.type)
        {
        case FileChange_Added:
            // 删除文件
            if (pFS->ExistFileAbs(baseFilePath.c_str()))
            {
                pFS->RemoveFileAbs(baseFilePath.c_str());
            }
            break;
            
        case FileChange_Deleted:
            // 恢复文件
            {
                IFile* pFile = pFS->OpenFileAbs(baseFilePath.c_str(), FileAccessMode_Write);
                if (pFile)
                {
					pFile->Write(&change.oldContent[0], (DWORD)change.oldContent.size());
                    pFS->CloseFile(pFile);
                    pFS->SetFileTimeAbs(baseFilePath.c_str(), change.oldTime);
                }
            }
            break;
            
        case FileChange_Modified:
            // 恢复旧版本
            {
                IFile* pFile = pFS->OpenFileAbs(baseFilePath.c_str(), FileAccessMode_Write);
                if (pFile)
                {
					pFile->Write(&change.oldContent[0], (DWORD)change.oldContent.size());
					pFS->CloseFile(pFile);
                    pFS->SetFileTimeAbs(baseFilePath.c_str(), change.oldTime);
                }
            }
            break;
        }
    }
    
    // 释放内容
    const_cast<FileChangelist&>(cl).DiscardContent();
}

bool CChangelists::SwitchTo(FileChangeListUID uid)
{
	RefreshCur();

    if (!_vcxprojDB || !_vcxprojDB->_pFS || uid == FileChangeListUID_Invalid)
    {
        return false;
    }

    // 检查目标 changelist 是否存在
    auto targetCl = _FindChangelist(uid);
    if (!targetCl)
    {
        return false;
    }

    // 如果目标就是当前 changelist,直接返回
    if (uid == _curUID)
    {
        return true;
    }

    // 获取文件系统接口
    IFileSystem* pFS = _vcxprojDB->_pFS;
    
    // 构建基准目录路径
    std::string baseFolder = _vcxprojDB->_pathDBFolder + "\\_base";
	std::string workspaceFolder = _vcxprojDB->_setting.pathWorkspace;

    // 获取当前 changelist
    FileChangelist* curCl = _FindChangelist(_curUID);
    if (!curCl)
    {
        return false;
    }

    // 加载当前 changelist 的完整内容
    _LoadChangelist(pFS, curCl->uid, *curCl);

    // 找到当前 changelist 和目标 changelist 的共同祖先
    std::vector<FileChangeListUID> curPath;
    std::vector<FileChangeListUID> targetPath;
    
    // 构建当前 changelist 的路径
    FileChangeListUID current = curCl->uid;
    while (current != FileChangeListUID_Invalid)
    {
        curPath.push_back(current);
        auto cl = _FindChangelist(current);
        if (!cl)
            break;
        current = cl->parent;
    }
    
    // 构建目标 changelist 的路径
    current = uid;
    while (current != FileChangeListUID_Invalid)
    {
        targetPath.push_back(current);
        auto cl = _FindChangelist(current);
        if (!cl)
            break;
        current = cl->parent;
    }
    
    // 找到共同祖先
    FileChangeListUID commonAncestor = FileChangeListUID_Invalid;
    size_t curAncestorIndex = 0;
    size_t targetAncestorIndex = 0;
    
    for (size_t i = 0; i < curPath.size(); ++i)
    {
        for (size_t j = 0; j < targetPath.size(); ++j)
        {
            if (curPath[i] == targetPath[j])
            {
                commonAncestor = curPath[i];
                curAncestorIndex = i;
                targetAncestorIndex = j;
                break;
            }
        }
        if (commonAncestor != FileChangeListUID_Invalid)
            break;
    }
    
    // 如果共同祖先是当前 changelist,只需要从共同祖先 apply 到目标
    if (commonAncestor == curCl->uid)
    {
        // 从共同祖先开始,依次 apply 直到目标 changelist
        for (size_t i = targetAncestorIndex; i > 0; --i)
        {
            auto cl = _FindChangelist(targetPath[i-1]);
            if (!cl)
                continue;
			_ApplyChangelist(pFS, *cl, workspaceFolder);
			if (cl->isCommit)
	            _ApplyChangelist(pFS, *cl, baseFolder);
        }
    }
    // 如果共同祖先是目标 changelist,只需要从当前 unapply 到共同祖先
    else if (commonAncestor == uid)
    {
        // 从当前 changelist 开始,依次 unapply 直到共同祖先
        for (size_t i = 0; i < curAncestorIndex; ++i)
        {
            auto cl = _FindChangelist(curPath[i]);
            if (!cl)
                continue;
			_UnapplyChangelist(pFS, *cl, workspaceFolder);
			if (cl->isCommit)
				_UnapplyChangelist(pFS, *cl, baseFolder);
        }
    }
    // 一般情况:需要先 unapply 到共同祖先,再 apply 到目标
    else
    {
        // 从当前 changelist 开始,依次 unapply 直到共同祖先
        for (size_t i = 0; i < curAncestorIndex; ++i)
        {
            auto cl = _FindChangelist(curPath[i]);
            if (!cl)
                continue;
			_UnapplyChangelist(pFS, *cl, workspaceFolder);
			if (cl->isCommit)
				_UnapplyChangelist(pFS, *cl, baseFolder);
		}
        
        // 从共同祖先开始,依次 apply 直到目标 changelist
        for (size_t i = targetAncestorIndex; i > 0; --i)
        {
            auto cl = _FindChangelist(targetPath[i-1]);
            if (!cl)
                continue;
			_ApplyChangelist(pFS, *cl, workspaceFolder);
			if (cl->isCommit)
				_ApplyChangelist(pFS, *cl, baseFolder);
		}
    }
    
    // 更新当前 UID
    _curUID = uid;
    _SaveCur();
    
    _ver++;
    return true;
}

bool CChangelists::HasChild(FileChangeListUID uid) const
{
	// 检查是否有任何changelist以此uid为parent
	for (const auto& cl : _briefs)
	{
		if (cl.second.parent == uid)
		{
			return true;
		}
	}
	
	// 没有找到以此uid为parent的changelist
	return false;
}

bool CChangelists::Remove(FileChangeListUID uid)
{
	if (HasChild(uid))
		return false;

	auto targetCl = _FindChangelist(uid);
	if (!targetCl)
	{
		return false;
	}

	if (targetCl->parent == FileChangeListUID_Invalid)
		return false;

	if (_curUID == uid)
		SwitchTo(targetCl->parent);

	_briefs.erase(uid);

	// 以UID的十六进制值作为文件名保存变更列表
	std::stringstream ss;
	ss << std::hex << uid;
	std::string fileName = ss.str() + ".d";

	// 构建完整路径
	std::string changesFolder = _vcxprojDB->_pathDBFolder + "\\_changelists";
	std::string filePath = changesFolder + "\\" + fileName;

	IFileSystem* pFS = _vcxprojDB->_pFS;
	pFS->RemoveFileAbs(filePath.c_str());

	_ver++;

	return true;
}
