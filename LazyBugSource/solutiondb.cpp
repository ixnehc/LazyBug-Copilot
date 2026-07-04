#include "stdh.h"

#include <string>
#include <unordered_set>
#include <regex>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <direct.h>
#include <io.h>
#include <stdio.h>

#include "SolutionDB.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"

#include "Utils.h"

// 静态辅助函数：确保指定目录存在，并在该目录下创建 .org 文件（如果不存在）
static void EnsureOrgFile(const char* folderPath, const char* content)
{
	Utils::EnsureFolder(folderPath);
	std::string orgFilePath = std::string(folderPath) + "\\.org";
	
	// 检查文件是否存在
	std::ifstream checkFile;
	Utils::OpenIFStream(checkFile, orgFilePath.c_str());
	bool fileExists = checkFile.is_open();
	if (fileExists)
	{
		checkFile.close();
		return;
	}
	
	// 文件不存在，创建并写入内容
	std::ofstream orgFile;
	Utils::OpenOFStream(orgFile, orgFilePath.c_str());
	if (orgFile.is_open())
	{
		orgFile << content;
		orgFile.close();
	}
}

//////////////////////////////////////////////////////////////////////////
//CDatabase


//清空指定目录，并在该目录下保存一个.db文件，其中包含数据库设置信息
bool CSolutionDB::New(const char* pathDBFolder, const SolutionDBSetting& setting)
{
    if (!pathDBFolder)
    {
        return false;
    }
    
	//确保一些目录存在
	if (true)
	{
		Utils::EnsureFolder(pathDBFolder);
		
		std::string path;
		path = std::string(pathDBFolder) + "\\_pch";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_defines";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_defines2";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_index";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_log";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_strlib";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_strlib2";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_chats";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_checkpoints";
		Utils::EnsureFolder(path.c_str());
		path = std::string(pathDBFolder) + "\\_embedding";
		Utils::EnsureFolder(path.c_str());
	}

    // 创建.db文件
	std::string dbFileName = pathDBFolder;
	dbFileName += "\\.db";
	
	try
	{
        std::vector<BYTE> buffer;
        
        // 使用DP_BeginSave/DP_EndSave宏来处理DataPacket
        DP_BeginSave(dp, buffer)
        {
            _SaveSetting(dp, setting);
        }
        DP_EndSave()
        
        // 使用STL的ofstream写入文件
        std::ofstream file;
		Utils::OpenOFStream(file, dbFileName.c_str());

		if (!file.is_open())
        {
            return false;
        }
            
        if (!buffer.empty())
        {
            file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        }
        file.close();
        
        return true;
    }
    catch (...)
    {
        return false;
    }
}

void CSolutionDB::_SaveSetting(CDataPacket& dp, const SolutionDBSetting& setting)
{
    const DWORD DB_VERSION = 1;
    dp.Data_WriteSimple<DWORD>(DB_VERSION);
 
    dp.Data_WriteString(setting.pathSln);
}

void CSolutionDB::_LoadSetting(CDataPacket& dp, SolutionDBSetting& setting)
{
    DWORD dbVersion = dp.Data_ReadSimple<DWORD>();
    
    dp.Data_ReadString(setting.pathSln);
}

void CSolutionDB::Open(const char* pathDBFolder)
{
    // 清空现有数据
    {
        CSolutionFiles::WriteLock lock(_files._filesMutex);
        _files._lowerCasedFiles.clear();
    }
    
    // 保存数据库路径
    _pathDBFolder = pathDBFolder;
    
    // 打开数据库文件
    std::string dbFileName = _pathDBFolder + "\\.db";
    
    // 使用STL的ifstream检查文件是否存在
    std::ifstream file;
	Utils::OpenIFStream(file, dbFileName.c_str());

    if (!file.is_open())
        return;
            
    // 读取文件大小
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
        
    if (fileSize == 0)
    {
        file.close();
        return;
    }
        
    // 读取文件内容到缓冲区
    std::vector<BYTE> buffer(fileSize);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();
        
    // 创建DataPacket并设置数据缓冲区
    CDataPacket dp;
    dp.SetDataBufferPointer(buffer.data());
        
    // 从DataPacket加载设置
    _LoadSetting(dp, _setting);

	if (true)
	{
		std::string path = GetDBFolderPath();
		path += "\\.projsettings";
		_projSettingLib.Init(path.c_str());
	}

	_symbolDB.Init(_pathDBFolder.c_str(), _projSettingLib);
	_symbolDB2.Init(_pathDBFolder.c_str(), _projSettingLib);
#ifdef USE_EMBEDDING_DB
	_embeddingDB.Init(_pathDBFolder.c_str(), _symbolDB, _symbolDB2);
#endif

	if (true)
	{
		std::string path = GetDBFolderPath();
		path += "\\_index";
		_solutionIndexer.Open(path.c_str());
	}

#ifdef USE_EMBEDDING_DB
	_scanner.Init(*this, _symbolDB,_symbolDB2,_solutionIndexer,_embeddingDB);
#else
	_scanner.Init(*this, _symbolDB,_symbolDB2,_solutionIndexer);
#endif

	// 确保 _checkpoints 目录下存在 .org 文件
	{
		std::string checkpointsPath = GetDBFolderPath();
		checkpointsPath += "\\_checkpoints";
		EnsureOrgFile(checkpointsPath.c_str(), _setting.pathSln.c_str());
	}

	// 确保 _chats 目录下存在 .org 文件
	{
		std::string chatsPath = GetDBFolderPath();
		chatsPath += "\\_chats";
		EnsureOrgFile(chatsPath.c_str(), _setting.pathSln.c_str());
	}

}

/**
 * @brief 关闭数据库
 * 
 * 关闭数据库，清理资源
 */
void CSolutionDB::Close()
{
	_symbolDB.Clear();
	_symbolDB2.Clear();
#ifdef USE_EMBEDDING_DB
	_embeddingDB.Clear();
#endif
	_scanner.Clear();
	_solutionIndexer.Close();

	 // 清空数据库条目
    {
        CSolutionFiles::WriteLock lock(_files._filesMutex);
        _files._lowerCasedFiles.clear();
    }
    
    // 清空路径信息
    _pathDBFolder.clear();
    _setting.Clear();

	_projSettingLib.Clear();
}

void CSolutionDB::Update()
{
	_scanner.Update(); 
}

void CSolutionDB::RefreshSolutionFiles(const SolutionDump& slnDump,
	std::vector<SolutionFile*>* newFiles, std::vector<SolutionFile*>* updatedFiles, std::vector<std::string>* removedFiles)
{
	CSolutionFiles::WriteLock lock(_files._filesMutex);

	if (newFiles)
		newFiles->clear();
	if (removedFiles)
		removedFiles->clear();

	// 第一步：标记所有现有文件为需要丢弃
	for (auto& filePair : _files._lowerCasedFiles)
	{
		filePair.second.needDiscard = true;
	}

	// 第二步：遍历所有项目，将ProjSetting添加到_projSettingLib，并更新或添加文件条目
	std::set<std::string> processedFiles;
	for (const auto& projPair : slnDump.projs)
	{
		const SolutionDump::ProjDump& projDump = projPair.second;

		// 将项目的ProjSetting添加到_projSettingLib
		ProjSettingHandle projSetting = _projSettingLib.Add(projDump.setting);

		for (const std::string& lowerCasedFilePath : projDump.files)
		{
			if (processedFiles.find(lowerCasedFilePath) != processedFiles.end())
				continue;//已经处理过
			processedFiles.insert(lowerCasedFilePath);

			// 检查文件是否已存在
			auto it = _files._lowerCasedFiles.find(lowerCasedFilePath);
			if (it != _files._lowerCasedFiles.end())
			{
				// 文件已存在，更新标记为不需要丢弃并更新设置
				it->second.needDiscard = false;
				if (projSetting != it->second.setting)
				{
					if (updatedFiles)
						updatedFiles->push_back(&(*it).second);
				}
				it->second.setting = projSetting;
			}
			else
			{
				// 文件不存在，添加新条目
				SolutionFile newFile;
				newFile.lowerCasedFilePath = lowerCasedFilePath;
				newFile.filePath = lowerCasedFilePath; // 仅有 lowerCased 路径，filePath 同值
				newFile.fileName = GetFileName(lowerCasedFilePath);
				newFile.needDiscard = false;
				newFile.setting = projSetting;  // 设置项目设置句柄
				auto insertResult = _files._lowerCasedFiles.insert({ lowerCasedFilePath, newFile });
				if (newFiles)
					newFiles->push_back(&insertResult.first->second);
			}
		}
	}

	// 第三步：删除所有仍然标记为需要丢弃的文件
	for (auto it = _files._lowerCasedFiles.begin(); it != _files._lowerCasedFiles.end(); )
	{
		if (it->second.needDiscard)
		{
			if (removedFiles)
				removedFiles->push_back(it->first);
			it = _files._lowerCasedFiles.erase(it);
		}
		else
		{
			++it;
		}
	}
}

