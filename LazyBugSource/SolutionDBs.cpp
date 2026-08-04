#include "stdh.h"

#include "SolutionDBs.h"

#include "stringparser/stringparser.h"
#include "Utils.h"

CSolutionDB* CSolutionDBs::Obtain(const char* dbFolder, const char* slnPath)
{
	if (!dbFolder)
		return nullptr;
	if (!dbFolder[0])
		return nullptr;
	std::string path;
	path = dbFolder;
	StringLower(path);

	// 读锁快速路径：已存在则直接返回
	{
		std::shared_lock<std::shared_mutex> lock(_mutex);
		auto it = _entries.find(path);
		if (it != _entries.end())
			return &it->second;
	}

	// 写锁：创建 DB，避免重复 New()/Open()
	std::unique_lock<std::shared_mutex> lock(_mutex);

	// double-check：其他线程可能已在等待锁期间完成创建
	auto it = _entries.find(path);
	if (it != _entries.end())
		return &it->second;

	// DB不存在时，尝试创建
	std::string pathDB = std::string(dbFolder) + "\\.db";
	if (!Utils::IsFileExist(pathDB.c_str()) && slnPath && slnPath[0])
	{
		SolutionDBSetting setting;
		setting.pathSln = slnPath;
		if (!CSolutionDB::New(dbFolder, setting))
			return nullptr;
	}

	CSolutionDB& db = _entries[path];
	db.Open(dbFolder);
	return &db;
}

void CSolutionDBs::CloseAll()
{
	std::unique_lock< std::shared_mutex> lock(_mutex);

	for (auto& entry : _entries)
	{
		entry.second.Close();
	}
	_entries.clear();
}

void CSolutionDBs::ClearDB(const char* dbFolder)
{
	if (!dbFolder || !dbFolder[0])
		return;

	std::string path = dbFolder;
	StringLower(path);

	std::unique_lock< std::shared_mutex> lock(_mutex);
	auto it = _entries.find(path);
	if (it != _entries.end())
		it->second.ClearDB();
}
 
 
void CSolutionDBs::Update()
{
	std::shared_lock< std::shared_mutex> lock(_mutex);

	for (auto& entry : _entries)
	{
		entry.second.Update();
	}
}
