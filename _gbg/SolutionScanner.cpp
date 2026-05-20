#include "stdh.h"

#include <string>
#include <unordered_set>
#include <regex>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>  // 添加ifstream/ofstream支持

#include "xml/tinyxml.h"
#include "stringparser/stringparser.h"
#include "SolutionScanner.h"
#include "SolutionDB.h"
#include "CppSymbol.h"
#include "SolutionIndexer.h"
#include "Utils.h"
#include "ProjSetting.h"



// ========== CSolutionScanner 实现 ==========

void CSolutionScanner::Init(CSolutionDB& db, CppSymbol::CSymbolDB& symbolDB, CSolutionIndexer& indexer)
{
	_db = &db;
	_symbolDB = &symbolDB;
	_solutionIndexer = &indexer;

    _slnFullPath = db.GetSlnPath();
//     _workspacePath = db.GetWorkspacePath();
	_dbFolder = db.GetDBFolderPath();
    _cachePath = db.GetDBFolderPath();
	_cachePath += "\\.dbcache";
    _scanTime = 0;
    _isScanning = false;
    _isDirty = false;

	// 初始化文件监控器的后缀过滤列表
// 	std::vector<std::string> suffixes = {"c", "cpp", "h", "hpp", "inl"};
// 	_foldersWatcher.SetSuffixFilter(suffixes);

	if (_db)
		_db->GetProjSettingLib().Load();
    
    _LoadCache();

	_CommitScanned();

}

void CSolutionScanner::Clear()
{
	_WaitTillIdle();

//	_fileWatcher.Stop();
	_foldersWatcher.Stop();

	_db = nullptr;
	_symbolDB = nullptr;
    _projs.clear();
    _scanTime = 0;
    _slnFullPath.clear();
//     _workspacePath.clear();
    _cachePath.clear();
    _isScanning = false;
    _isDirty = false;

}

void CSolutionScanner::Update()
{
	_Refresh();

	if (_db)
	{
		if (_db->IsContent())
		{
			if (!_foldersWatcher.IsStarted())
				_foldersWatcher.Start();

			const ChangedFileInformation* infos;
			int nInfo = _foldersWatcher.FetchChangedFiles(infos);

			if (_solutionIndexer && _symbolDB)
			{
				std::string path;
				std::string suffix;
				bool symbolDBNotified = false;
				for (int i = 0;i < nInfo;i++)
				{
					const ChangedFileInformation& info = infos[i];
					if (info.action == FA_MODIFIED || info.action == FA_ADDED || FA_RENAMED_OLD_NAME || FA_RENAMED_NEW_NAME)
					{
						path = info.name;
						StringLower(path);
						_solutionIndexer->UpdateIfExists(path.c_str());

						if (!symbolDBNotified)
						{
							suffix = GetFileSuffix(path);
							if ((suffix == "c") || (suffix == "cpp") || (suffix == "h") || (suffix == "hpp") || (suffix == "inl"))
							{
								_symbolDB->NotifyFilesChanged();
								symbolDBNotified = true;
							}
						}
					}
				}
			}
		}
	}
}


void CSolutionScanner::_WaitTillIdle()
{
	while (_isScanning.load())
		Sleep(5);
}

bool CSolutionScanner::_IsScanning()
{
	return _isScanning.load();
}

void CSolutionScanner::_CommitScanned()
{
	if (!_db)
		return;
	if (!_symbolDB)
		return;

	bool isInitialCommit = !_db->IsContent();

	if (isInitialCommit)
	{
		_db->RefreshSolutionFiles(_projs,nullptr,nullptr,nullptr);
		_symbolDB->SetContent(_db->GetFiles());
		_solutionIndexer->SetContent(_db->GetFiles());
		
		// 将所有文件路径添加到文件夹监控器
		const CSolutionFiles& files = _db->GetFiles();
		CSolutionFiles::ReadLock lock(files._filesMutex);
		for (const auto& filePair : files._lowerCasedFiles)
		{
			const SolutionFile& file = filePair.second;
			_foldersWatcher.AddFilePath(file.lowerCasedFilePath.c_str());
		}
	}
	else
	{
		std::vector<SolutionFile*> newFiles,updatedFiles;
		std::vector<std::string> removedFiles;
		_db->RefreshSolutionFiles(_projs, &newFiles, &updatedFiles,&removedFiles);
		_symbolDB->SetDeltaContent(newFiles,updatedFiles,removedFiles);
		_solutionIndexer->SetDeltaContent(newFiles, updatedFiles, removedFiles);
		
		// 将新增和更新的文件路径添加到文件夹监控器
		for (SolutionFile* pFile : newFiles)
		{
			_foldersWatcher.AddFilePath(pFile->lowerCasedFilePath.c_str());
		}
	}
}

void CSolutionScanner::_Refresh()
{
	if (_slnFullPath.empty())
		return;
    // 如果已经有扫描线程在运行，直接返回
    if (_isScanning.load())
    {
        return;
    }
    
    // 检查solution文件是否存在
    if (!Utils::IsFileExist(_slnFullPath.c_str()))
    {
        return;
    }
    
    bool needScanSln = false;
    std::vector<std::string> projsToScan;
    
    // 检查solution文件的修改时间
    AbsTick currentSlnTime = Utils::GetFileTick(_slnFullPath.c_str());
    if (currentSlnTime != _scanTime)
    {
        needScanSln = true;
    }
    
    // 检查所有项目文件的修改时间，收集需要更新的项目
    for (auto& projPair : _projs)
    {
        const std::string& projFullPath = projPair.first;
        CProjFiles& projFiles = projPair.second;
        
        if (Utils::IsFileExist(projFullPath.c_str()))
        {
            AbsTick currentProjTime = Utils::GetFileTick(projFullPath.c_str());
            if (currentProjTime != projFiles._scanTime)
            {
                projsToScan.push_back(projFullPath);
            }
        }
    }
    
    // 如果需要扫描，启动扫描线程
    if (needScanSln || !projsToScan.empty())
    {
        // 标记数据为dirty
        _isDirty = true;
		_isScanning = true;

        std::thread scanThread
		(
			[this, needScanSln, projsToScan]() 
			{
				std::lock_guard<std::mutex> lock(_scanMutex);

				if (true)
				{
					// 只有在需要时才扫描solution文件
					if (needScanSln)
					{
						_ScanSolution();
					}
                
					// 只扫描需要更新的项目文件
					for (const std::string& projFullPath : projsToScan)
					{
						auto it = _projs.find(projFullPath);
						if (it != _projs.end())
						{
							if (Utils::IsFileExist(projFullPath.c_str()))
							{
								_ScanProj(projFullPath.c_str(), it->second);
							}
						}
					}
				}
            
				_isScanning = false;
			}
		);
        scanThread.detach();
    }
    else
    {
        // 没有需要扫描的文件，但如果数据是dirty的，则保存缓存

		if (_db)
		{
			if (_db->GetProjSettingLib().IsDirty())
				_db->GetProjSettingLib().Save();
		}

        if (_isDirty.load())
        {
            _SaveCache();

			_CommitScanned();
            _isDirty = false;
        }
    }
}

void CSolutionScanner::_ScanSolution()
{
    // 读取solution文件内容
    std::string slnContent;
	Utils::FileContentCodingFormat codingFmt;
    if (!Utils::GetFileContentIntoUTF8(_slnFullPath.c_str(), slnContent, codingFmt))
    {
        return;
    }
    
    // 提取所有vcxproj文件路径
    std::vector<std::string> vcxprojPaths = ExtractProjPathFromSln(slnContent, _slnFullPath);
    
    // 创建新的项目集合
    std::unordered_map<std::string, CProjFiles> newProjs;
    
    // 处理每个项目文件
    for (const std::string& projPath : vcxprojPaths)
    {
        // 检查是否已存在
        auto it = _projs.find(projPath);
        if (it != _projs.end())
        {
            // 已存在，移动到新集合
            newProjs[projPath] = std::move(it->second);
        }
        else
        {
            // 新项目，创建空的CProjFiles，初始化设置句柄
            CProjFiles projFiles;
            projFiles._setting = ProjSettingHandle_Null;
            newProjs[projPath] = projFiles;
        }
    }
    
    // 更新项目集合
    _projs = std::move(newProjs);
    
    // 更新扫描时间
    _scanTime = Utils::GetFileTick(_slnFullPath.c_str());
}

void CSolutionScanner::_ScanProj(const char* projFullPath, CProjFiles& projFiles)
{
	bool isVcxProj = false;
	{
		std::string extension = GetFileSuffix(std::string(projFullPath));
		StringLower(extension);
		if (extension == "vcxproj")
			isVcxProj = true;
	}

    // 读取vcxproj文件内容
    std::string projContent;
	Utils::FileContentCodingFormat codingFmt;
	if (!Utils::GetFileContentIntoUTF8(projFullPath, projContent,codingFmt))
    {
        return;
    }
    
    // 提取文件路径列表
	extern std::vector<std::string> ExtractFilePathsFromProj(const std::string & projContent, const std::string & pathProj);
	std::vector<std::string> filePaths = ExtractFilePathsFromProj(projContent, projFullPath);

    // 提取额外的包含路径
	std::vector<std::string> includePaths;
	if (isVcxProj)
		includePaths = ExtractAddionalIncludePathesFromVcxproj(projContent, projFullPath);

    // 提取预编译头文件路径
	std::string pchFullPath;
	if (isVcxProj)
		pchFullPath = ExtractPCHPathFromVcxproj_XML(projContent, projFullPath);
    
    // 创建ProjSetting并添加到库中
    ProjSetting setting;
    setting.additionalIncludeFullPathes = includePaths;
	if (!pchFullPath.empty())
	{
		setting.lowerCasedPchFullPath = pchFullPath;
		StringLower(setting.lowerCasedPchFullPath);
		if (true)
		{
			setting.lowerCasedPchOutputFullPath = _dbFolder + "\\_pch\\";
			std::string s = pchFullPath;
			RemoveFileSuffix(s);
			std::string name;
			ConvertFullPathToName(s.c_str(), name);
			setting.lowerCasedPchOutputFullPath += name;
			setting.lowerCasedPchOutputFullPath += ".pch";
			StringLower(setting.lowerCasedPchOutputFullPath);
		}
	}

    projFiles._setting = _db->GetProjSettingLib().Add(setting);
    
    // 清空现有文件列表
    projFiles._files.clear();
    
    // 转换为ProjFile结构
    for (const std::string& realPath : filePaths)
    {
       
        ProjFile projFile;
        projFile.filePath = realPath;
        projFile.lowerCasedFilePath = realPath;
        StringLower(projFile.lowerCasedFilePath);
        projFile.fileName = GetFileName(realPath);
        
        projFiles._files.push_back(projFile);
    }
    
    // 更新扫描时间
    projFiles._scanTime = Utils::GetFileTick(projFullPath);
}

void CSolutionScanner::_LoadCache()
{
    if (_cachePath.empty()) 
        return;

    std::ifstream file(_cachePath, std::ios::binary);
    if (!file.is_open()) 
        return;

    try 
	{
        // 读取扫描时间
        file.read(reinterpret_cast<char*>(&_scanTime), sizeof(_scanTime));

        // 读取项目数量
        size_t projCount;
        file.read(reinterpret_cast<char*>(&projCount), sizeof(projCount));

        for (size_t i = 0; i < projCount; ++i) {
            // 读取项目路径
            size_t pathLength;
            file.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
            
            std::string projPath(pathLength, '\0');
            file.read(&projPath[0], pathLength);

            // 读取项目扫描时间
            AbsTick projScanTime;
            file.read(reinterpret_cast<char*>(&projScanTime), sizeof(projScanTime));

            // 读取设置句柄
            ProjSettingHandle settingHandle;
            file.read(reinterpret_cast<char*>(&settingHandle), sizeof(settingHandle));

            // 读取文件数量
            size_t fileCount;
            file.read(reinterpret_cast<char*>(&fileCount), sizeof(fileCount));

            CProjFiles projFiles;
            projFiles._scanTime = projScanTime;
            projFiles._setting = settingHandle;

            for (size_t j = 0; j < fileCount; ++j) 
			{
                ProjFile projFile;

                // 读取文件路径
                file.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
                projFile.filePath.resize(pathLength);
                file.read(&projFile.filePath[0], pathLength);

                // 读取小写文件路径
                file.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
                projFile.lowerCasedFilePath.resize(pathLength);
                file.read(&projFile.lowerCasedFilePath[0], pathLength);

                // 读取文件名
                file.read(reinterpret_cast<char*>(&pathLength), sizeof(pathLength));
                projFile.fileName.resize(pathLength);
                file.read(&projFile.fileName[0], pathLength);

                projFiles._files.push_back(projFile);
            }

            _projs[projPath] = projFiles;
        }
    }
    catch (...) 
	{
        // 发生异常时清空缓存数据
        _projs.clear();
        _scanTime = 0;
    }
}

void CSolutionScanner::_SaveCache()
{
    if (_cachePath.empty()) 
        return;

    std::ofstream file(_cachePath, std::ios::binary);
    if (!file.is_open()) 
        return;

    try 
	{
        // 写入扫描时间
        file.write(reinterpret_cast<const char*>(&_scanTime), sizeof(_scanTime));

        // 写入项目数量
        size_t projCount = _projs.size();
        file.write(reinterpret_cast<const char*>(&projCount), sizeof(projCount));

        for (const auto& projPair : _projs) 
		{
            const std::string& projPath = projPair.first;
            const CProjFiles& projFiles = projPair.second;

            // 写入项目路径
            size_t pathLength = projPath.size();
            file.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
            file.write(projPath.c_str(), pathLength);

            // 写入项目扫描时间
            file.write(reinterpret_cast<const char*>(&projFiles._scanTime), sizeof(projFiles._scanTime));

            // 写入设置句柄
            file.write(reinterpret_cast<const char*>(&projFiles._setting), sizeof(projFiles._setting));

            // 写入文件数量
            size_t fileCount = projFiles._files.size();
            file.write(reinterpret_cast<const char*>(&fileCount), sizeof(fileCount));

            for (const auto& projFile : projFiles._files) 
			{
                // 写入文件路径
                pathLength = projFile.filePath.size();
                file.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
                file.write(projFile.filePath.c_str(), pathLength);

                // 写入小写文件路径
                pathLength = projFile.lowerCasedFilePath.size();
                file.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
                file.write(projFile.lowerCasedFilePath.c_str(), pathLength);

                // 写入文件名
                pathLength = projFile.fileName.size();
                file.write(reinterpret_cast<const char*>(&pathLength), sizeof(pathLength));
                file.write(projFile.fileName.c_str(), pathLength);
            }
        }
    }
    catch (...) {        }
}


