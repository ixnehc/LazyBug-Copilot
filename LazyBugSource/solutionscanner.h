#pragma once

#include <unordered_map>
#include <set>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

#include "ProjSetting.h"
#include "SolutionDump.h"
// #include "filewatcher/FileWatcher.h"
#include "filewatcher/FoldersWatcher.h"

#include "Utils.h"

#include "DirWatchEntryConfig.h"



class CProjFiles
{
public:
	// 获取项目设置句柄
	ProjSettingHandle GetSettingHandle() const { return _setting; }
	
	// 获取项目设置
	const ProjSetting* GetSetting(CProjSettingLib &projSettingLib) const 
	{
		return projSettingLib.Get(_setting);
	}

protected:
	std::vector< ProjFile> _files;
	AbsTick _scanTime;//上一次scan proj文件时,proj文件的修改时间
	ProjSettingHandle _setting;//项目设置句柄

	friend class CSolutionScanner;
	friend class CSolutionDB;
};

class CSolutionDB;
namespace CppSymbol
{
	class CSymbolDB;
}

namespace TreeSitterSymbol
{
	class CSymbolDB;
}

class CSolutionIndexer;
#ifdef USE_EMBEDDING_DB
class CEmbeddingDB;
#endif
class CSolutionScanner
{
public:
	CSolutionScanner():_foldersWatcher(8)
	{
		_db = nullptr;
		_symbolDB = nullptr;
		_symbolDB2 = nullptr;
#ifdef USE_EMBEDDING_DB
		_embeddingDB = nullptr;
#endif
		_solutionIndexer = nullptr;
		_slnDumpTime = Utils::GetZeroFileTime();
		_dirWatchConfigTime = Utils::GetZeroFileTime();
		_nextFreeSourceBit = SOURCE_SLNDUMP << 1;  // bit 0 保留给 slndmp
	}
	void Init(CSolutionDB &db,CppSymbol::CSymbolDB &symbolDB, TreeSitterSymbol::CSymbolDB& symbolDB2,CSolutionIndexer &indexer
#ifdef USE_EMBEDDING_DB
		, CEmbeddingDB& embeddingDB
#endif
	);
	void Clear();

	void Update();

	void GetWatcherFolderPathes(std::vector<std::string>& pathes)	{		return _foldersWatcher.GetFolderPathes(pathes);	}

protected:
	void _WaitTillIdle();

	void _Refresh();

	// 目录监视相关
	void _RefreshDirWatch(SourceUpdateResult& outMergedResult);
	void _ScanDirEntry(DirWatchEntry& entry);
	FileSourceMask _AllocSourceBit();
	void _FreeSourceBit(FileSourceMask bit);
	void _FindDirWatchEntriesForFile(const char* lowerCasedPath, std::vector<DirWatchEntry*>& outEntries);

	FILETIME _slnDumpTime;
	FILETIME _dirWatchConfigTime;

	CSolutionDB* _db;
	CppSymbol::CSymbolDB* _symbolDB;
	TreeSitterSymbol::CSymbolDB* _symbolDB2;
#ifdef USE_EMBEDDING_DB
	CEmbeddingDB* _embeddingDB;
#endif
	CSolutionIndexer* _solutionIndexer;

	std::string _dbFolder;

	CFoldersWatcher _foldersWatcher;

	// 目录监视相关
	std::vector<DirWatchEntry> _dirWatchEntries;
	FileSourceMask _nextFreeSourceBit;
	std::vector<FileSourceMask> _freeSourceBits;  // 回收的空闲位

	// 线程同步相关
	std::mutex _scanMutex;

};
