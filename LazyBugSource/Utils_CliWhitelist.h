#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include "timer/timer.h"

#include "Utils_File.h"
#include "Utils_FindInFile.h"
#include "Utils_SolutionDump.h"

struct FindInFileResults;
struct SolutionDump;

class CLlmSkills;

namespace Utils
{
	void LoadCliWhitelists(std::vector<std::string>& list);
	void EnsureCliWhitelists();


	// 获取 global_rules.md 文件路径
	std::string GetGlobalRulesFilePath();
	// 获取 project_rules.md 文件路径
	std::string GetProjectRulesFilePath();

	void EnsureGlobalRulesFile();
	void EnsureProjectRulesFile();


}
