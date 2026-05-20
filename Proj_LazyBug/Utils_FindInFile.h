#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include "timer/timer.h"

#include "Utils_File.h"
#include "nlohmann/json.hpp"

struct FindInFileResults;

namespace Utils
{


extern void FindInFile(const char* key, std::vector<std::string>& folderPathes, FindInFileResults& results, int maxResults = 0, FindInFileFilter filterCallback = nullptr);
extern bool SearchWithRipGrep(const char* key, std::vector<std::string>& folderPathes, FindInFileResults& results, int maxResults = 0, FindInFileFilter filterCallback = nullptr);
extern void DumpFindInFileResult(const char *key, const FindInFileResults& results,std::string &resultString, int maxResult);

int FindMatchingLines(const std::string& filePath, const std::string& key, const std::string& content, FindInFileResults& results, int maxLines);

// Build JSON from find-in-files results map
extern void BuildFindInFilesResultJson(nlohmann::json& json, const std::unordered_map<std::string, FindInFileResults>& resultsList, int maxResult);

// Build error JSON
extern void BuildFindInFilesErrorJson(nlohmann::json& json, const char* errorMessage);

// Dump formatted text from JSON
extern void DumpFindInFileResultsFromJson(nlohmann::json& json, std::string& outText);


}
