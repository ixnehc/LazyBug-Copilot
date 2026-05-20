#include "stdh.h"

#include <string>
#include <unordered_set>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>  // 添加ifstream/ofstream支持

#include "stringparser/stringparser.h"
#include "SolutionScanner.h"
#include "SolutionDB.h"
#include "CppSymbol.h"
#include "SolutionIndexer.h"
#include "Utils.h"
#include "ProjSetting.h"



// ========== CSolutionScanner 实现 ==========

void CSolutionScanner::Init(CSolutionDB& db, CppSymbol::CSymbolDB& symbolDB, TreeSitterSymbol::CSymbolDB& symbolDB2, CSolutionIndexer& indexer)
{
	_db = &db;
	_symbolDB = &symbolDB;
	_symbolDB2 = &symbolDB2;
	_solutionIndexer = &indexer;

	_dbFolder = db.GetDBFolderPath();
	_slnDumpTime = Utils::GetZeroFileTime();

	// 初始化文件监控器的后缀过滤列表
// 	std::vector<std::string> suffixes = {"c", "cpp", "h", "hpp", "inl"};
// 	_foldersWatcher.SetSuffixFilter(suffixes);

	if (_db)
		_db->GetProjSettingLib().Load();
}

void CSolutionScanner::Clear()
{
	_foldersWatcher.Stop();

	_db = nullptr;
	_symbolDB = nullptr;
	_symbolDB2 = nullptr;
	_slnDumpTime = Utils::GetZeroFileTime();

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

			if (_solutionIndexer && (_symbolDB||_symbolDB2))
			{
				std::string path;
				std::string suffix; 
				bool symbolDBNotified = false;
				bool symbolDB2Notified = false;
				for (int i = 0;i < nInfo;i++)
				{
					const ChangedFileInformation& info = infos[i];
					if (info.action == FA_MODIFIED || info.action == FA_ADDED || FA_RENAMED_OLD_NAME || FA_RENAMED_NEW_NAME)
					{
						path = info.name;
						StringLower(path);
						_solutionIndexer->UpdateIfExists(path.c_str());

						if ((!symbolDBNotified)|| (!symbolDB2Notified))
							suffix = GetFileSuffix(path);

						if (!symbolDBNotified)
						{
							if (Utils::IsCppFile(suffix))
							{
								_symbolDB->NotifyFilesChanged();
								symbolDBNotified = true;
							}
						} 

						if (!symbolDB2Notified)
						{
							if(TreeSitterSymbol::GetLanguageFromExtension(suffix)!=Language::Unknown)
							{
								_symbolDB2->NotifyFilesChanged();
								symbolDB2Notified = true;
							}
						}

					}
				}
			}
		}
	}
}

void CSolutionScanner::_CommitScanned(const SolutionDump& slnDump)
{
	if (!_db)
		return;
	if (!_symbolDB)
		return;

	bool isInitialCommit = !_db->IsContent();

	if (isInitialCommit)
	{
		_db->RefreshSolutionFiles(slnDump,nullptr,nullptr,nullptr);
		_symbolDB->SetContent(_db->GetFiles());
		_symbolDB2->SetContent(_db->GetFiles());
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
		_db->RefreshSolutionFiles(slnDump, &newFiles, &updatedFiles,&removedFiles);
		_symbolDB->SetDeltaContent(newFiles,updatedFiles,removedFiles);
		_symbolDB2->SetDeltaContent(newFiles, updatedFiles, removedFiles);
		_solutionIndexer->SetDeltaContent(newFiles, updatedFiles, removedFiles);
		
		// 将新增和更新的文件路径添加到文件夹监控器
		for (SolutionFile* pFile : newFiles)
		{
			_foldersWatcher.AddFilePath(pFile->lowerCasedFilePath.c_str());
		}
	}

	if (_db->GetProjSettingLib().IsDirty())
		_db->GetProjSettingLib().Save();
}


void CSolutionScanner::_Refresh()
{
	if (_dbFolder.empty())
		return;

	std::string slnDumpPath = _dbFolder + "\\.slndmp";

	FILETIME t = Utils::GetFileTime(slnDumpPath.c_str());
	if (Utils::EqualFileTime(t, _slnDumpTime))
		return;

	SolutionDump slnDump;
	extern bool LoadSolutionDump(const char* fullPath, SolutionDump & slnDump);
	if (LoadSolutionDump(slnDumpPath.c_str(), slnDump))
	{
		_slnDumpTime = t;

		_CommitScanned(slnDump);
	}
}

