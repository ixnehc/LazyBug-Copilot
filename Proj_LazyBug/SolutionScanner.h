#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

#include "ProjSetting.h"
#include "SolutionDump.h"
// #include "filewatcher/FileWatcher.h"
#include "filewatcher/FoldersWatcher.h"

#include "Utils.h"



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
class CSolutionScanner
{
public:
	CSolutionScanner():_foldersWatcher(8)
	{
		_db = nullptr;
		_symbolDB = nullptr;
		_symbolDB2 = nullptr;
		_solutionIndexer = nullptr;
		_slnDumpTime = Utils::GetZeroFileTime();
	}
	void Init(CSolutionDB &db,CppSymbol::CSymbolDB &symbolDB, TreeSitterSymbol::CSymbolDB& symbolDB2,CSolutionIndexer &indexer);
	void Clear();

	void Update();

	void GetWatcherFolderPathes(std::vector<std::string>& pathes)	{		return _foldersWatcher.GetFolderPathes(pathes);	}

protected:
	void _WaitTillIdle();

	//检测solution文件的修改时间是否更新,如果是的话,则启动一个线程来_ScanSolution()
	//检测所有_projs,是否有proj文件修改时间更新,如果是的话,则启动一个线程来_ScanProj()
	void _Refresh();

	void _CommitScanned(const SolutionDump& slnDump);


	FILETIME _slnDumpTime;

	CSolutionDB* _db;
	CppSymbol::CSymbolDB* _symbolDB;
	TreeSitterSymbol::CSymbolDB* _symbolDB2;
	CSolutionIndexer* _solutionIndexer;

	std::string _dbFolder;

	CFoldersWatcher _foldersWatcher;

	// 线程同步相关
	std::mutex _scanMutex;

};
