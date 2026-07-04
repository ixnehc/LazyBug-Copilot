#include "stdh.h"
#include <fstream>

#include "SolutionDump.h"

#include "Utils_File.h"

#include "stringparser/stringparser.h"

static const int SOLUTION_DUMP_VERSION = 2;

static void to_json(json& j, const ProjSetting& ps)
{
	j = json{
		{ "additionalIncludeFullPathes", ps.additionalIncludeFullPathes },
		{ "lowerCasedPchFullPath",       ps.lowerCasedPchFullPath },
		{ "lowerCasedPchOutputFullPath", ps.lowerCasedPchOutputFullPath }
	};
}

static void from_json(const json& j, ProjSetting& ps)
{
	j.at("additionalIncludeFullPathes").get_to(ps.additionalIncludeFullPathes);
	j.at("lowerCasedPchFullPath").get_to(ps.lowerCasedPchFullPath);
	j.at("lowerCasedPchOutputFullPath").get_to(ps.lowerCasedPchOutputFullPath);
}

static void to_json(json& j, const SolutionDump::ProjDump& pd)
{
	j = json{
		{ "setting", pd.setting },
		{ "files",   pd.files }
	};
}

static void from_json(const json& j, SolutionDump::ProjDump& pd)
{
	j.at("setting").get_to(pd.setting);
	j.at("files").get_to(pd.files);
}

static void to_json(json& j, const SolutionDump& sd)
{
	j = json{
		{ "lowerCasedPath", sd.lowerCasedPath },
		{ "projs",          sd.projs }
	};
}

static void from_json(const json& j, SolutionDump& sd)
{
	j.at("lowerCasedPath").get_to(sd.lowerCasedPath);
	j.at("projs").get_to(sd.projs);
}

static void to_json(json& j, const FILETIME& ft)
{
	uint64_t value = (uint64_t(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
	j = value;
}

static void from_json(const json& j, FILETIME& ft)
{
	uint64_t value = j.get<uint64_t>();
	ft.dwLowDateTime  = DWORD(value & 0xFFFFFFFF);
	ft.dwHighDateTime = DWORD(value >> 32);
}

static void to_json(json& j, const SolutionDumpTimeStamps::Proj& proj)
{
	j = json{
		{ "lowerCasedPath", proj.lowerCasedPath },
		{ "t",              proj.t }
	};
}

static void from_json(const json& j, SolutionDumpTimeStamps::Proj& proj)
{
	j.at("lowerCasedPath").get_to(proj.lowerCasedPath);
	j.at("t").get_to(proj.t);
}

static void to_json(json& j, const SolutionDumpTimeStamps& ts)
{
	j = json{
		{ "lowerCasedPath", ts.lowerCasedPath },
		{ "t",              ts.t },
		{ "projs",          ts.projs }
	};
}

static void from_json(const json& j, SolutionDumpTimeStamps& ts)
{
	j.at("lowerCasedPath").get_to(ts.lowerCasedPath);
	j.at("t").get_to(ts.t);
	j.at("projs").get_to(ts.projs);
}

extern bool is_pure_ascii(const char* str);

void SaveSolutionDump(const char* fullPath, SolutionDump& slnDump)
{
	json root;
	root["version"] = SOLUTION_DUMP_VERSION;
	root["solution"] = slnDump;

	// 先写入临时文件
	std::string tempPath = std::string(fullPath) + ".tmp";
	bool tempPathIsAscii = is_pure_ascii(tempPath.c_str());
	{
		std::ofstream ofs;
		if (tempPathIsAscii)
			ofs.open(tempPath.c_str(), std::ios::binary | std::ios::out);
		else
			ofs.open(utf8_to_widechar(tempPath).c_str(), std::ios::binary | std::ios::out);

		if (!ofs.is_open())
			return;
		
		ofs << root.dump(4);
		ofs.close(); // 确保文件完全写入并关闭
	}

	// 原子替换：临时文件重命名为目标文件
	// MoveFileEx 是原子操作，可以安全替换现有文件
	if (tempPathIsAscii)
		MoveFileExA(tempPath.c_str(), fullPath, MOVEFILE_REPLACE_EXISTING);
	else
		MoveFileExW(utf8_to_widechar(tempPath).c_str(), utf8_to_widechar(fullPath).c_str(), MOVEFILE_REPLACE_EXISTING);
}

bool LoadSolutionDump(const char* fullPath, SolutionDump& slnDump)
{
	try {
		std::ifstream ifs;

		if (is_pure_ascii(fullPath))
			ifs.open(fullPath, std::ios::binary | std::ios::in);
		else
			ifs.open(utf8_to_widechar(fullPath).c_str(), std::ios::binary | std::ios::in);

		if (!ifs.is_open())
			return false;

		json root = json::parse(ifs);

// 		int version = root.value("version", 0);
// 		if (version != SOLUTION_DUMP_VERSION)
// 			return;

		root.at("solution").get_to(slnDump);
		return true;
	}
	catch (const std::exception&) {
		// JSON 解析错误，可能是文件损坏或正在写入
		return false;
	}
}


void SaveSolutionDumpTimeStamp(const char* fullPath, SolutionDumpTimeStamps& timeStamp)
{
	json root;
	root["version"] = SOLUTION_DUMP_VERSION;
	root["timeStamps"] = timeStamp;

	// 先写入临时文件
	std::string tempPath = std::string(fullPath) + ".tmp";
	bool tempPathIsAscii = is_pure_ascii(tempPath.c_str());
	{
		std::ofstream ofs;
		if (tempPathIsAscii)
			ofs.open(tempPath.c_str(), std::ios::binary | std::ios::out);
		else
			ofs.open(utf8_to_widechar(tempPath).c_str(), std::ios::binary | std::ios::out);

		if (!ofs.is_open())
			return;
		
		ofs << root.dump(4);
		ofs.close(); // 确保文件完全写入并关闭
	}

	// 原子替换：临时文件重命名为目标文件
	if (tempPathIsAscii)
		MoveFileExA(tempPath.c_str(), fullPath, MOVEFILE_REPLACE_EXISTING);
	else
		MoveFileExW(utf8_to_widechar(tempPath).c_str(), utf8_to_widechar(fullPath).c_str(), MOVEFILE_REPLACE_EXISTING);
}

void LoadSolutionDumpTimeStamp(const char* fullPath, SolutionDumpTimeStamps& timeStamp)
{
	try {
		std::ifstream ifs;
		if (is_pure_ascii(fullPath))
			ifs.open(fullPath, std::ios::binary | std::ios::in);
		else
			ifs.open(utf8_to_widechar(fullPath).c_str(), std::ios::binary | std::ios::in);

		if (!ifs.is_open())
			return;

		json root = json::parse(ifs);

		int version = root.value("version", 0);
		if (version != SOLUTION_DUMP_VERSION)
			return;

		root.at("timeStamps").get_to(timeStamp);
	}
	catch (const std::exception&) {
		// JSON 解析错误，可能是文件损坏或正在写入
		return;
	}
}
