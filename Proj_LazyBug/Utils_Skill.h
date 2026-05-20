#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include <functional>
#include "timer/timer.h"

#include "Utils_File.h"
#include "Utils_FindInFile.h"
#include "Utils_SolutionDump.h"

#include "LlmSkills.h"


struct FindInFileResults;
struct SolutionDump;

class CLlmSkills;

namespace Utils
{
// Filter 返回值类型
enum class EnumFilesInSkillFolderFilter
{
	Reject,		// 跳过此文件/目录
	Accept,		// 接受此文件/目录
	Stop		// 停止遍历
};

// 获取所有skills目录及其对应的类型
// 返回: vector<pair<目录路径, Skill类型>>
std::vector<std::pair<std::string, CLlmSkills::Skill::Type>> GetSkillsFolder(const char* openedDBFolder);

void SyncBuiltInSkills();

// 枚举所有 skills 目录下的文件和目录
// filter: 过滤回调函数，返回 Reject-跳过, Accept-接受, Stop-停止遍历
// 参数: filePath-文件路径, type-Skill类型
// 返回: 是否正常完成（false 表示被 stop 中断）
bool EnumFilesInSkillFolder(const char* openedDBFolder, 
	std::function<EnumFilesInSkillFolderFilter(const char* filePath, CLlmSkills::Skill::Type type)> filter);

void LoadLlmSkills(CLlmSkills &skills, const char* openedDBFolder);

// 根据完整文件路径生成 skill tag 字符串
// 如果文件在 skill 或 _skill 子目录下且文件名为 skill.md
// 则返回 true 并将 tagName 设为：文件所在目录的目录名 + "(skill)"
// 否则返回 false
bool MakeSkillTagName(const char* filePath, std::string& tagName);

}
