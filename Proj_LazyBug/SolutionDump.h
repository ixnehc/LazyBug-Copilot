#pragma once

#include <set>
#include <map>
#include "SolutionDBDefines.h"

struct SolutionDumpTimeStamps
{
	bool IsValid()
	{
		return !lowerCasedPath.empty();
	}
	std::string lowerCasedPath;
	FILETIME t;
	struct Proj
	{
		std::string lowerCasedPath;
		FILETIME t;
	};
	std::vector<Proj> projs;
};

struct SolutionDump
{
	std::string lowerCasedPath;

	struct ProjDump
	{
		ProjSetting setting;
		std::set<std::string> files; // lowerCased 文件路径
	};

	std::map<std::string, ProjDump> projs; // key 是 lowerCased 的项目文件路径

};