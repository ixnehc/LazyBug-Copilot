#pragma once

class CLlmSkills
{
public:
	struct Skill
	{
		enum class Type
		{
			None,
			BuiltIn,
			Global,
			Project,
		};
		Type tp;
		std::string name;
		std::string description;
		std::string folderPath;
		bool enable;
	};

	//先清除所有指定tp的skill, 再在指定目录下搜集所有的skill信息,作为 tp 载入
	bool ReLoad(const char* rootPath, Skill::Type tp);

	void Clear();

	//将所有的skill 的信息汇总,输出为一个字符串
	void Dump(std::string& str);

	std::vector<Skill> _skills;

};

extern CLlmSkills g_llmSkills;

// 解析 SKILL.md 文件中的 YAML frontmatter
// 返回 true 表示成功解析 name 和 description
// outContent: 可选，如果提供，将填充第二个"---"之后的内容
bool ParseSkillMd(const std::string& filePath, std::string& outName, std::string& outDescription, std::string* outContent = nullptr);
