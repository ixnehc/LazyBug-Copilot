#pragma once

//#include "Changelist.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <fstream>

#include "SolutionScanner.h"
#include "ProjSetting.h"
#include "CppSymbol.h"
#include "TreeSitterSymbol.h"
#include "SolutionIndexer.h"
#include "SolutionDump.h"
#include "EmbeddingDB.h"
#include <shared_mutex>


class CDataPacket;

struct SolutionFile :public ProjFile
{
	SolutionFile()
	{
		needDiscard = false;
		setting = ProjSettingHandle_Null;
	}
	
	// 获取项目设置句柄
	ProjSettingHandle GetSettingHandle() const { return setting; }
	
	// 获取项目设置
	const ProjSetting* GetSetting(CProjSettingLib& projSettingLib) const
	{
		return projSettingLib.Get(setting);
	}
	
	// 检查是否有有效的项目设置
	bool HasValidSetting() const 
	{
		return setting != ProjSettingHandle_Null;
	}
	
	ProjSettingHandle setting;
	bool needDiscard;
};

class CSolutionFiles
{
public:

public:
	typedef std::shared_lock<std::shared_mutex> ReadLock;
	typedef std::unique_lock<std::shared_mutex> WriteLock;

	std::unordered_map<std::string, SolutionFile> _lowerCasedFiles;//这里面的key路径全部为小写,为fullPath
	mutable std::shared_mutex _filesMutex; // 读写锁保护_lowerCasedFiles
	
	// 检查路径是否存在
	bool Exists(const std::string& path) const
	{
		ReadLock lock(_filesMutex);
		return _lowerCasedFiles.find(path) != _lowerCasedFiles.end();
	}
	
	// 获取条目数量
	size_t Size() const
	{
		ReadLock lock(_filesMutex);
		return _lowerCasedFiles.size();
	}
	
};

struct SolutionDBSetting
{
	void Clear()
	{
		pathSln = "";
	}
	std::string pathSln;//绝对路径
};

class CSolutionDB
{
public:

	CSolutionDB()
	{
	}

	bool IsEmpty()
	{
		return _setting.pathSln.empty();
	}

	bool IsContent()
	{
		CSolutionFiles::ReadLock lock(_files._filesMutex);
		return !_files._lowerCasedFiles.empty();
	}

	static bool New(const char* pathDB, const SolutionDBSetting& setting);//在指定目录下新建一个database(清空这个目录,并在这个目录下保存一个.db文件,里面为setting的内容

	// 修改New函数声明，移除文件系统参数
	void Open(const char* pathDB);//打开pathDB目录,并读取里面的.db文件
	void Close();

	void Update();

	// 更新函数声明以匹配实现
	void RefreshSolutionFiles(const SolutionDump& slnDump, std::vector<SolutionFile*>* newFiles, std::vector<SolutionFile*>* updatedFiles, std::vector<std::string>* removedFiles);

	const char* GetSlnPath()
	{
		return _setting.pathSln.c_str();
	}

	const char* GetDBFolderPath()
	{
		return _pathDBFolder.c_str();
	}

	const CSolutionFiles& GetFiles()	{		return _files;	}

	CProjSettingLib& GetProjSettingLib()	{		return _projSettingLib;	}

	CSolutionScanner& GetScanner()	{		return _scanner;	}
	CSolutionIndexer& GetSolutionIndexer()	{		return _solutionIndexer;	}
	
public:

	std::string _pathDBFolder;//Database所在的目录
	SolutionDBSetting _setting;

	CProjSettingLib _projSettingLib;
	CSolutionScanner _scanner;
	CppSymbol::CSymbolDB _symbolDB;
	TreeSitterSymbol::CSymbolDB _symbolDB2;
	CEmbeddingDB _embeddingDB;
	CSolutionIndexer _solutionIndexer;
	CSolutionFiles _files;


	static void _SaveSetting(CDataPacket& dp, const SolutionDBSetting& setting);
	static void _LoadSetting(CDataPacket& dp, SolutionDBSetting& setting);
	void _LoadEntries();
	void _LoadFiles();//
};