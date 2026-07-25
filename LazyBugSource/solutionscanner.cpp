#include "stdh.h"

#include <string>
#include <unordered_set>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <set>

#include "stringparser/stringparser.h"
#include "SolutionScanner.h"
#include "SolutionDB.h"
#include "CppSymbol.h"
#include "SolutionIndexer.h"
#include "Utils.h"
#include "ProjSetting.h"
#include "DirWatchEntryConfig.h"

// ========== CSolutionScanner 实现 ==========

void CSolutionScanner::Init(CSolutionDB& db, CppSymbol::CSymbolDB& symbolDB, TreeSitterSymbol::CSymbolDB& symbolDB2, CSolutionIndexer& indexer
#ifdef USE_EMBEDDING_DB
	, CEmbeddingDB& embeddingDB
#endif
)
{
	_db = &db;
	_symbolDB = &symbolDB;
	_symbolDB2 = &symbolDB2;
#ifdef USE_EMBEDDING_DB
	_embeddingDB = &embeddingDB;
#endif
	_solutionIndexer = &indexer;

	_dbFolder = db.GetDBFolderPath();
	_slnDumpTime = Utils::GetZeroFileTime();
	_dirWatchConfigTime = Utils::GetZeroFileTime();
	_nextFreeSourceBit = SOURCE_SLNDUMP << 1;
	_dirWatchEntries.clear();

	if (_db)
		_db->GetProjSettingLib().Load();
}

void CSolutionScanner::Clear()
{
	_foldersWatcher.Stop();

	_db = nullptr;
	_symbolDB = nullptr;
	_symbolDB2 = nullptr;
#ifdef USE_EMBEDDING_DB
	_embeddingDB = nullptr;
#endif
	_slnDumpTime = Utils::GetZeroFileTime();
	_dirWatchConfigTime = Utils::GetZeroFileTime();
	_dirWatchEntries.clear();
	_nextFreeSourceBit = SOURCE_SLNDUMP << 1;
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

				// 收集目录监视的文件增减
				std::vector<SolutionFile*> dirNewFiles;
				std::vector<std::string> dirRemovedFiles;

				for (int i = 0;i < nInfo;i++)
				{
					const ChangedFileInformation& info = infos[i];

					// 处理目录监视的文件增减（一个文件可能匹配多个条目）
					if (info.action == FA_ADDED || info.action == FA_REMOVED)
					{
						path = info.name;
						StringLower(path);

						std::vector<DirWatchEntry*> entries;
						_FindDirWatchEntriesForFile(path.c_str(), entries);

						for (DirWatchEntry* pEntry : entries)
						{
							if (info.action == FA_ADDED)
							{
								bool isNew = false;
								SolutionFile* pFile = _db->UpdateFileSource(pEntry->sourceBit, path, true, &isNew);
								if (pFile)
								{
									pEntry->files.insert(path);
									if (isNew)
										dirNewFiles.push_back(pFile);
								}
							}
							else // FA_REMOVED
							{
								bool isRemoved = false;
								_db->UpdateFileSource(pEntry->sourceBit, path, false, &isRemoved);
								pEntry->files.erase(path);
								if (isRemoved)
									dirRemovedFiles.push_back(path);
							}
						}
					}

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

				// 通知目录监视的文件增减
				if (!dirNewFiles.empty() || !dirRemovedFiles.empty())
				{
					std::vector<SolutionFile*> emptyUpdated;
					_symbolDB->SetDeltaContent(dirNewFiles, emptyUpdated, dirRemovedFiles);
					_symbolDB2->SetDeltaContent(dirNewFiles, emptyUpdated, dirRemovedFiles);
					_solutionIndexer->SetDeltaContent(dirNewFiles, emptyUpdated, dirRemovedFiles);
#ifdef USE_EMBEDDING_DB
					if (_embeddingDB)
						_embeddingDB->SetDeltaContent(dirNewFiles, emptyUpdated, dirRemovedFiles);
#endif
				}
			}
		}
	}
}

void CSolutionScanner::_Refresh()
{
	if (_dbFolder.empty())
		return;

	bool isInitialCommit = _db && !_db->IsContent();

	std::string slnDumpPath = _dbFolder + "\\.slndmp";
	std::string dirWatchPath = _dbFolder + "\\.dirwatch";

	FILETIME tSln = Utils::GetFileTime(slnDumpPath.c_str());
	FILETIME tDirWatch = Utils::GetFileTime(dirWatchPath.c_str());

	bool slnChanged = !Utils::EqualFileTime(tSln, _slnDumpTime);
	bool dirWatchChanged = !Utils::EqualFileTime(tDirWatch, _dirWatchConfigTime);

	if (!slnChanged && !dirWatchChanged)
		return;

	if (isInitialCommit)
	{
		_slnDumpTime = tSln;
		_dirWatchConfigTime = tDirWatch;

		// 加载 slnDump
		SolutionDump slnDump;
		extern bool LoadSolutionDump(const char* fullPath, SolutionDump & slnDump);
		LoadSolutionDump(slnDumpPath.c_str(), slnDump);

		// 加载 .dirwatch 并扫描
		std::vector<DirWatchEntry> dirWatchEntries;
		LoadDirWatchConfig(dirWatchPath.c_str(), dirWatchEntries);
		for (auto& entry : dirWatchEntries)
		{
			entry.sourceBit = _AllocSourceBit();
			_ScanDirEntry(entry);
		}
		_dirWatchEntries = std::move(dirWatchEntries);

		// 一次性初始化 DB
		_db->InitSources(slnDump, _dirWatchEntries);

		// 全量通知
		_symbolDB->SetContent(_db->GetFiles());
		_symbolDB2->SetContent(_db->GetFiles());
		_solutionIndexer->SetContent(_db->GetFiles());
#ifdef USE_EMBEDDING_DB
		if (_embeddingDB)
			_embeddingDB->SetContent(_db->GetFiles());
#endif

		// 注册文件监视（仅 slnDump 文件）和目录监视
		const CSolutionFiles& files = _db->GetFiles();
		{
			CSolutionFiles::ReadLock lock(files._filesMutex);
			for (const auto& fp : files._lowerCasedFiles)
			{
				if (fp.second.sourceMask & SOURCE_SLNDUMP)
					_foldersWatcher.AddFilePath(fp.second.lowerCasedFilePath.c_str());
			}
		}
		for (const auto& entry : _dirWatchEntries)
			_foldersWatcher.AddFolder(entry.directoryPath.c_str());

		if (_db->GetProjSettingLib().IsDirty())
			_db->GetProjSettingLib().Save();

		return;
	}

	// ---- 增量更新路径 ----
	SourceUpdateResult mergedResult;

	if (slnChanged)
	{
		_slnDumpTime = tSln;

		SolutionDump slnDump;
		extern bool LoadSolutionDump(const char* fullPath, SolutionDump & slnDump);
		if (LoadSolutionDump(slnDumpPath.c_str(), slnDump))
		{
			std::unordered_map<std::string, ProjSettingHandle> fileSettings;
			for (const auto& projPair : slnDump.projs)
			{
				const SolutionDump::ProjDump& projDump = projPair.second;
				ProjSettingHandle h = _db->GetProjSettingLib().Add(projDump.setting);
				for (const std::string& f : projDump.files)
					fileSettings[f] = h;
			}

			SourceUpdateResult slnResult;
			_db->UpdateSource_Sln(fileSettings, slnResult);
			mergedResult.newFiles.insert(mergedResult.newFiles.end(), slnResult.newFiles.begin(), slnResult.newFiles.end());
			mergedResult.updatedFiles.insert(mergedResult.updatedFiles.end(), slnResult.updatedFiles.begin(), slnResult.updatedFiles.end());
			mergedResult.removedFiles.insert(mergedResult.removedFiles.end(), slnResult.removedFiles.begin(), slnResult.removedFiles.end());
		}
	}

	if (dirWatchChanged)
	{
		_dirWatchConfigTime = tDirWatch;
		_RefreshDirWatch(mergedResult);
	}

	// 确保目录监视条目都已注册
	for (const auto& entry : _dirWatchEntries)
		_foldersWatcher.AddFolder(entry.directoryPath.c_str());

	// 增量通知 indexers
	if (!mergedResult.newFiles.empty() || !mergedResult.updatedFiles.empty() || !mergedResult.removedFiles.empty())
	{
		_symbolDB->SetDeltaContent(mergedResult.newFiles, mergedResult.updatedFiles, mergedResult.removedFiles);
		_symbolDB2->SetDeltaContent(mergedResult.newFiles, mergedResult.updatedFiles, mergedResult.removedFiles);
		_solutionIndexer->SetDeltaContent(mergedResult.newFiles, mergedResult.updatedFiles, mergedResult.removedFiles);
#ifdef USE_EMBEDDING_DB
		if (_embeddingDB)
			_embeddingDB->SetDeltaContent(mergedResult.newFiles, mergedResult.updatedFiles, mergedResult.removedFiles);
#endif
		for (SolutionFile* f : mergedResult.newFiles)
		{
			if (f->sourceMask & SOURCE_SLNDUMP)
				_foldersWatcher.AddFilePath(f->lowerCasedFilePath.c_str());
		}
	}

	if (_db->GetProjSettingLib().IsDirty())
		_db->GetProjSettingLib().Save();
}

FileSourceMask CSolutionScanner::_AllocSourceBit()
{
	if (!_freeSourceBits.empty())
	{
		FileSourceMask bit = _freeSourceBits.back();
		_freeSourceBits.pop_back();
		return bit;
	}
	FileSourceMask bit = _nextFreeSourceBit;
	_nextFreeSourceBit <<= 1;
	if (_nextFreeSourceBit == 0)
		_nextFreeSourceBit = SOURCE_SLNDUMP << 1;
	return bit;
}

void CSolutionScanner::_FreeSourceBit(FileSourceMask bit)
{
	if (bit != 0 && bit != SOURCE_SLNDUMP)
		_freeSourceBits.push_back(bit);
}

void CSolutionScanner::_FindDirWatchEntriesForFile(const char* lowerCasedPath, std::vector<DirWatchEntry*>& outEntries)
{
	outEntries.clear();

	std::string suffix = GetFileSuffix(lowerCasedPath);
	StringLower(suffix);

	for (auto& entry : _dirWatchEntries)
	{
		if (!CheckPathContaining(entry.directoryPath.c_str(), lowerCasedPath))
			continue;

		for (const auto& ext : entry.extensions)
		{
			if (suffix == ext)
			{
				outEntries.push_back(&entry);
				break;
			}
		}
	}
}

// 递归扫描辅助
static void _ScanDirRecursive(const std::string& dirPath, const std::vector<std::string>& extensions, std::set<std::string>& outFiles)
{
	std::string searchPath = dirPath + "\\*";

	WIN32_FIND_DATAA fd;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 跳过 . 和 ..
			if (strcmp(fd.cFileName, ".") != 0 && strcmp(fd.cFileName, "..") != 0)
			{
				std::string subDir = dirPath + "\\" + fd.cFileName;
				_ScanDirRecursive(subDir, extensions, outFiles);
			}
			continue;
		}

		std::string fileName = fd.cFileName;
		std::string suffix = GetFileSuffix(fileName);
		StringLower(suffix);

		for (const auto& ext : extensions)
		{
			if (suffix == ext)
			{
				std::string fullPath = dirPath + "\\" + fileName;
				StringLower(fullPath);
				outFiles.insert(fullPath);
				break;
			}
		}
	}
	while (FindNextFileA(hFind, &fd));

	FindClose(hFind);
}

void CSolutionScanner::_ScanDirEntry(DirWatchEntry& entry)
{
	entry.files.clear();

	if (entry.recursive)
		_ScanDirRecursive(entry.directoryPath, entry.extensions, entry.files);
	else
	{
		std::string searchPath = entry.directoryPath + "\\*";

		WIN32_FIND_DATAA fd;
		HANDLE hFind = FindFirstFileA(searchPath.c_str(), &fd);
		if (hFind == INVALID_HANDLE_VALUE)
			return;

		do
		{
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				continue;

			std::string fileName = fd.cFileName;
			std::string suffix = GetFileSuffix(fileName);
			StringLower(suffix);

			for (const auto& ext : entry.extensions)
			{
				if (suffix == ext)
				{
					std::string fullPath = entry.directoryPath + "\\" + fileName;
					StringLower(fullPath);
					entry.files.insert(fullPath);
					break;
				}
			}
		}
		while (FindNextFileA(hFind, &fd));

		FindClose(hFind);
	}
}

void CSolutionScanner::_RefreshDirWatch(SourceUpdateResult& outMergedResult)
{
	// 加载新的配置
	std::vector<DirWatchEntry> newEntries;
	std::string dirWatchPath = _dbFolder + "\\.dirwatch";
	LoadDirWatchConfig(dirWatchPath.c_str(), newEntries);

	// 建立旧 entry 的目录路径索引
	std::unordered_map<std::string, int> oldIndexByPath;
	for (int i = 0; i < (int)_dirWatchEntries.size(); i++)
		oldIndexByPath[_dirWatchEntries[i].directoryPath] = i;

	// 建立新 entry 的目录路径索引
	std::set<std::string> newEntryPaths;
	for (auto& e : newEntries)
		newEntryPaths.insert(e.directoryPath);

	// 找出删除的条目
	for (auto& oldPair : oldIndexByPath)
	{
		if (newEntryPaths.find(oldPair.first) == newEntryPaths.end())
		{
			// 该目录被删除
			DirWatchEntry& oldEntry = _dirWatchEntries[oldPair.second];
			SourceUpdateResult result;
			_db->UpdateSource_Folder(oldEntry.sourceBit, {}, result);

			outMergedResult.newFiles.insert(outMergedResult.newFiles.end(), result.newFiles.begin(), result.newFiles.end());
			outMergedResult.updatedFiles.insert(outMergedResult.updatedFiles.end(), result.updatedFiles.begin(), result.updatedFiles.end());
			outMergedResult.removedFiles.insert(outMergedResult.removedFiles.end(), result.removedFiles.begin(), result.removedFiles.end());

			_FreeSourceBit(oldEntry.sourceBit);
		}
	}

	// 处理新增和修改的条目
	std::vector<DirWatchEntry> updatedEntries;
	for (auto& newEntry : newEntries)
	{
		auto oldIt = oldIndexByPath.find(newEntry.directoryPath);
		if (oldIt != oldIndexByPath.end())
		{
			// 已有条目，复用旧 sourceBit
			newEntry.sourceBit = _dirWatchEntries[oldIt->second].sourceBit;
		}
		else
		{
			// 全新条目，分配新位
			newEntry.sourceBit = _AllocSourceBit();
		}
		_ScanDirEntry(newEntry);

		SourceUpdateResult result;
		_db->UpdateSource_Folder(newEntry.sourceBit, newEntry.files, result);

		outMergedResult.newFiles.insert(outMergedResult.newFiles.end(), result.newFiles.begin(), result.newFiles.end());
		outMergedResult.updatedFiles.insert(outMergedResult.updatedFiles.end(), result.updatedFiles.begin(), result.updatedFiles.end());
		outMergedResult.removedFiles.insert(outMergedResult.removedFiles.end(), result.removedFiles.begin(), result.removedFiles.end());

		updatedEntries.push_back(newEntry);
	}

	// 替换旧列表
	_dirWatchEntries = std::move(updatedEntries);
}

