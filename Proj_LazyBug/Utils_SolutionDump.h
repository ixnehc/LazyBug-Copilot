#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include "timer/timer.h"


struct SolutionDump;

namespace Utils
{

	extern bool GenerateSolutionDump(const char* dbFolder, const char* slnPath, SolutionDump& dmp);
};