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

	std::unordered_map<std::string, CSolutionDB>::iterator it = _entries.find(path);
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

	std::unique_lock< std::shared_mutex> lock(_mutex);
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
 
void CSolutionDBs::Update()
{
	std::shared_lock< std::shared_mutex> lock(_mutex);

	for (auto& entry : _entries)
	{
		entry.second.Update();
	}
}
