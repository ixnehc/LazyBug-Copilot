#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include "timer/timer.h"

#include "Utils_File.h"
#include "Utils_SolutionDump.h"

struct FindInFileResults;
struct SolutionDump;

class CLlmSkills;

namespace Utils
{


extern const char* FileEditResultErrorPrefix;

// 统一的代码文件扩展名列表（用于文件搜索过滤）
extern const char* CODE_FILE_EXTENSIONS;


// 将.分割符替换为::的函数（NormalizeSymbolName的反操作）
extern std::string RestoreSymbolName(const std::string& symbolName);
extern std::string NormalizeSymbolName(const std::string& symbolName);

extern const char* GetDBRootFolder_utf8();
extern const wchar_t* GetWebViewUserFolder();

extern bool CheckFileBinary(const char* path);

bool IsCppFile(std::string& lowerCasedSuffix);

// 判断文件是否为图片文件（根据扩展名）
bool IsImageFile(const char* filePath);



}
