#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <mutex>
#include <atomic>

#include "ProjSetting.h"
// #include "filewatcher/FileWatcher.h"
#include "filewatcher/FoldersWatcher.h"


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

class CSolutionIndexer;
class CSolutionScanner
{
public:
	CSolutionScanner():_foldersWatcher(8)
	{
		_db = nullptr;
		_symbolDB = nullptr;
		_solutionIndexer = nullptr;
		_isScanning = false;
		_isDirty = false;
	}
	void Init(CSolutionDB &db,CppSymbol::CSymbolDB &symbolDB,CSolutionIndexer &indexer);
	void Clear();

	void Update();

	const std::unordered_map<std::string, CProjFiles>& GetProjs()
	{
		return _projs;
	}

	void GetWatcherFolderPathes(std::vector<std::string>& pathes)	{		return _foldersWatcher.GetFolderPathes(pathes);	}

protected:
	void _WaitTillIdle();

	//检测solution文件的修改时间是否更新,如果是的话,则启动一个线程来_ScanSolution()
	//检测所有_projs,是否有proj文件修改时间更新,如果是的话,则启动一个线程来_ScanProj()
	void _Refresh();
	bool _IsScanning();

	void _CommitScanned();

	void _ScanSolution();//读取_slnPath文件的内容,解析出里面所有的proj,更新_projs(删除不存在的,添加新的)
	void _ScanProj(const char* projFullPath, CProjFiles& projFiles);

	void _LoadCache();//从cache文件内读入_projs和_scanTime
	void _SaveCache();//保存_projs和_scanTime

	std::unordered_map<std::string, CProjFiles> _projs;
	AbsTick _scanTime;//上一次scan solution时,sln文件的修改时间

	CSolutionDB* _db;
	CppSymbol::CSymbolDB* _symbolDB;
	CSolutionIndexer* _solutionIndexer;

	std::string _dbFolder;
	std::string _slnFullPath;
// 	std::string _workspacePath;
	std::string _cachePath;

// 	CFileWatcher _fileWatcher;
	CFoldersWatcher _foldersWatcher;

	// 线程同步相关
	std::mutex _scanMutex;
	std::atomic<bool> _isScanning;
	std::atomic<bool> _isDirty;  // 标记数据是否需要保存

};