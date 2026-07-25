#include "stdh.h"
#include <fstream>

#include "DirWatchEntryConfig.h"

#include "Utils_File.h"

#include "nlohmann/json.hpp"
using json = nlohmann::ordered_json;


// ========== DirWatchEntry 序列化 ==========
static void to_json(json& j, const DirWatchEntry& e)
{
	j = json{
		{ "path", e.directoryPath },
		{ "extensions", e.extensions },
		{ "recursive", e.recursive }
	};
}

static void from_json(const json& j, DirWatchEntry& e)
{
	j.at("path").get_to(e.directoryPath);
	j.at("extensions").get_to(e.extensions);
	j.at("recursive").get_to(e.recursive);
	e.sourceBit = 0;
}

// ========== 加载/保存 .dirwatch 配置文件 ==========
bool LoadDirWatchConfig(const char* fullPath, std::vector<DirWatchEntry>& entries)
{
	entries.clear();

	std::ifstream file;
	Utils::OpenIFStream(file, fullPath);
	if (!file.is_open())
		return false;

	try
	{
		json root = json::parse(file);
		if (root.contains("entries"))
			root["entries"].get_to(entries);
	}
	catch (const json::exception&)
	{
		return false;
	}

	return true;
}

bool SaveDirWatchConfig(const char* fullPath, const std::vector<DirWatchEntry>& entries)
{
	json root;
	root["entries"] = entries;

	std::ofstream file;
	Utils::OpenOFStream(file, fullPath);
	if (!file.is_open())
		return false;

	file << root.dump(2);
	return true;
}