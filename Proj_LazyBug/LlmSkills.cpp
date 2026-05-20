#include "stdh.h"
#include "LlmSkills.h"
#include "stringparser/stringparser.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <windows.h>

#include "Utils_File.h"

// ============================================================================
// 内部辅助函数
// ============================================================================

CLlmSkills g_llmSkills;

// 去除字符串首尾空白
static std::string _Trim(const std::string& s)
{
	size_t start = 0;
	while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r' || s[start] == '\n'))
		start++;
	size_t end = s.size();
	while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r' || s[end - 1] == '\n'))
		end--;
	return s.substr(start, end - start);
}

// 解析 SKILL.md 文件中的 YAML frontmatter
// 返回 true 表示成功解析 name 和 description
// 移除字符串开头的UTF-8 BOM和不可读字符
static std::string _RemoveBOMAndUnreadable(const std::string& str)
{
	const char* data = str.data();
	size_t len = str.size();
	size_t start = 0;

	// 跳过UTF-8 BOM (\xEF\xBB\xBF)
	if (len >= 3 && (unsigned char)data[0] == 0xEF && (unsigned char)data[1] == 0xBB && (unsigned char)data[2] == 0xBF)
	{
		start = 3;
	}

	// 跳过开头的不可读字符（控制字符，但保留换行符和制表符）
	while (start < len && (unsigned char)data[start] < 0x20 && data[start] != '\t')
	{
		start++;
	}

	return str.substr(start);
}

bool ParseSkillMd(const std::string& filePath, std::string& outName, std::string& outDescription)
{
	std::ifstream file;
	Utils::OpenIFStream(file, filePath.c_str());

	if (!file.is_open())
		return false;

	std::string line;
	bool firstLine = true;

	// 跳过起始的空行，找到第一个 "---"
	while (std::getline(file, line))
	{
		// 第一行需要处理BOM和不可读字符
		if (firstLine)
		{
			line = _RemoveBOMAndUnreadable(line);
			firstLine = false;
		}

		line = _Trim(line);
		if (line.empty())
			continue;  // 跳过空行

		if (line == "---")
			break;  // 找到第一个 ---

		// 非空行且不是 ---，格式错误
		file.close();
		return false;
	}

	// 如果没找到第一个 "---"，返回失败
	if (line != "---")
	{
		file.close();
		return false;
	}

	// 读取 frontmatter 内容直到第二个 "---"
	std::string yamlContent;
	while (std::getline(file, line))
	{
		line = _Trim(line);
		if (line == "---")
			break;
		yamlContent += line + "\n";
	}
	file.close();

	// 解析 name 和 description 字段
	outName.clear();
	outDescription.clear();

	// 简单的逐行解析：匹配 "key: value" 格式
	// 对于多行 description，用缩进检测
	std::istringstream yamlStream(yamlContent);
	std::string yamlLine;
	bool inDescription = false;

	while (std::getline(yamlStream, yamlLine))
	{
		if (yamlLine.empty() && inDescription)
		{
			// 空行在 description 中，可能是多行描述的一部分
			outDescription += "\n";
			continue;
		}

		if (yamlLine.empty())
			continue;

		// 检查是否以空格开头（后续行可能属于上一个多行字段）
		if (yamlLine[0] == ' ' || yamlLine[0] == '\t')
		{
			if (inDescription)
			{
				// 这是 description 的续行
				std::string trimmedLine = _Trim(yamlLine);
				if (!outDescription.empty())
					outDescription += "\n";
				outDescription += trimmedLine;
			}
			continue;
		}

		inDescription = false;

		// 解析 "key: value"
		size_t colonPos = yamlLine.find(':');
		if (colonPos == std::string::npos)
			continue;

		std::string key = _Trim(yamlLine.substr(0, colonPos));
		std::string value = _Trim(yamlLine.substr(colonPos + 1));

		if (key == "name")
		{
			outName = value;
		}
		else if (key == "description")
		{
			outDescription = value;
			inDescription = true;
		}
	}

	return !outName.empty() && !outDescription.empty();
}

// 递归扫描目录，收集所有包含 SKILL.md 的子目录
static void _ScanSkills(const std::wstring& wRootPath, std::vector<CLlmSkills::Skill>& outSkills, CLlmSkills::Skill::Type tp, int maxDepth = 3)
{
	if (maxDepth <= 0)
		return;

	std::wstring wSearchPattern = wRootPath + L"\\*";

	WIN32_FIND_DATAW findData;
	HANDLE hFind = FindFirstFileW(wSearchPattern.c_str(), &findData);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do
	{
		// 跳过 "." 和 ".."
		if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
			continue;

		// 跳过以 "." 开头的隐藏目录
		if (findData.cFileName[0] == L'.')
			continue;

		std::wstring wFullPath = wRootPath + L"\\" + findData.cFileName;
		std::string fullPath = widechar_to_utf8(wFullPath.c_str());

		if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 检查该目录下是否存在 SKILL.md
			std::wstring wSkillMdPath = wFullPath + L"\\SKILL.md";
			DWORD skillMdAttrs = GetFileAttributesW(wSkillMdPath.c_str());
			if (skillMdAttrs != INVALID_FILE_ATTRIBUTES && !(skillMdAttrs & FILE_ATTRIBUTE_DIRECTORY))
			{
				// SKILL.md 存在，解析它
				std::string skillMdPath = widechar_to_utf8(wSkillMdPath.c_str());
				std::string name, description;
				if (ParseSkillMd(skillMdPath, name, description))
				{
					CLlmSkills::Skill skill;
					skill.tp = tp;
					skill.name = name;
					skill.description = description;
					skill.folderPath = fullPath;

					// 检查同一目录下是否存在 .enable 文件
					std::wstring wEnableFilePath = wFullPath + L"\\.enable";
					DWORD enableAttrs = GetFileAttributesW(wEnableFilePath.c_str());
					skill.enable = (enableAttrs != INVALID_FILE_ATTRIBUTES && !(enableAttrs & FILE_ATTRIBUTE_DIRECTORY));

					outSkills.push_back(skill);
				}
			}
			else
			{
				// 记录当前 outSkills 的大小，用于判断递归扫描是否找到 skill
				size_t prevSize = outSkills.size();
				
				// 继续递归扫描子目录
				_ScanSkills(wFullPath, outSkills, tp, maxDepth - 1);
				
				// 如果递归扫描后没有找到任何 skill，则该目录为空目录，添加一个 name 为空的 Skill
				if (outSkills.size() == prevSize)
				{
					CLlmSkills::Skill skill;
					skill.tp = tp;
					skill.name = "";  // name 为空
					skill.description = "";
					skill.folderPath = fullPath;
					skill.enable = false;
					outSkills.push_back(skill);
				}
			}
		}
	} while (FindNextFileW(hFind, &findData));

	FindClose(hFind);
}

// ============================================================================
// CLlmSkills 公共方法
// ============================================================================

bool CLlmSkills::ReLoad(const char* rootPath, Skill::Type tp)
{
	if (!rootPath || rootPath[0] == '\0')
		return false;

	// 转换为宽字符
	std::wstring wRootPath = utf8_to_widechar(rootPath);

	// 1. 清除所有指定 tp 的 skill
	_skills.erase(
		std::remove_if(_skills.begin(), _skills.end(),
			[tp](const Skill& s) { return s.tp == tp; }),
		_skills.end()
	);

	// 2. 检查 rootPath 是否存在
	DWORD attrs = GetFileAttributesW(wRootPath.c_str());
	if (attrs == INVALID_FILE_ATTRIBUTES || !(attrs & FILE_ATTRIBUTE_DIRECTORY))
		return false;

	// 3. 扫描目录，搜集所有 skill 信息
	std::vector<Skill> newSkills;
	_ScanSkills(wRootPath, newSkills, tp);

	// 4. 将搜集到的 skill 加入 _skills
	_skills.insert(_skills.end(), newSkills.begin(), newSkills.end());

	return true;
}

void CLlmSkills::Clear()
{
	_skills.clear();
}

void CLlmSkills::Dump(std::string& str)
{
	str.clear();

	if (_skills.empty())
	{
		str = "";
		return;
	}

	// 优先级: Project > Global > BuiltIn
	// 同名skill按优先级覆盖，只保留最高优先级的
	auto priority = [](Skill::Type tp) -> int {
		switch (tp)
		{
		case Skill::Type::Project:  return 3;
		case Skill::Type::Global:   return 2;
		case Skill::Type::BuiltIn:  return 1;
		default:                    return 0;
		}
	};

	std::vector<const Skill*> deduped; // 用于保持插入顺序
	std::unordered_map<std::string, size_t> nameToIndex; // name -> deduped 中的索引

	for (const auto& skill : _skills)
	{
		// 跳过未启用的skill
		if (!skill.enable)
			continue;

		auto it = nameToIndex.find(skill.name);
		if (it == nameToIndex.end())
		{
			// 第一次遇到该名字，直接加入
			nameToIndex[skill.name] = deduped.size();
			deduped.push_back(&skill);
		}
		else
		{
			// 同名已存在，比较优先级
			const Skill* existing = deduped[it->second];
			if (priority(skill.tp) > priority(existing->tp))
			{
				deduped[it->second] = &skill; // 高优先级覆盖低优先级
			}
		}
	}

	// 按类型分组输出（保持类型输出顺序: BuiltIn -> Global -> Project）
	auto dumpType = [&str, &deduped](Skill::Type tp, const char* tpName) {
		for (const auto* skill : deduped)
		{
			if (skill->tp == tp)
			{
				str += "- name: " + skill->name + "\n";
				str += "  description: " + skill->description + "\n";
				str += "  path: " + skill->folderPath + "\n\n";
			}
		}
	};

	dumpType(Skill::Type::BuiltIn, "BuiltIn");
	dumpType(Skill::Type::Global, "Global");
	dumpType(Skill::Type::Project, "Project");

	// 去掉末尾多余的换行
	while (!str.empty() && str.back() == '\n')
		str.pop_back();
}
