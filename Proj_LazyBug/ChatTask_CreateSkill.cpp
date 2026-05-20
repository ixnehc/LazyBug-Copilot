#include "stdh.h"
#include "ChatTask_CreateSkill.h"
#include "LlmSkills.h"
#include "Utils.h"
#include "LlmChat.h"
#include "ChatAgent.h"
#include "ChatOpsCtrl.h"
#include <fstream>
#include <windows.h>

extern const char* GetOpenedDBFolderPath_utf8();
extern std::wstring utf8_to_widechar(const char* utf8_str);

// 从路径中提取文件名部分（最后一个反斜杠之后的内容）
static std::string _ExtractSkillName(const std::string& path)
{
	size_t pos = path.find_last_of("\\/");
	if (pos != std::string::npos)
		return path.substr(pos + 1);
	return path;
}

CChatTask_CreateSkill::CChatTask_CreateSkill()
{
}

CChatTask_CreateSkill::~CChatTask_CreateSkill()
{
}

void CChatTask_CreateSkill::_Fail(const char* reason)
{
	std::string result = "Error: ";
	result += reason ? reason : "Unknown error";
	_SendToolCallResult(result.c_str());
	_status = TaskStatus::Failure;
}

void CChatTask_CreateSkill::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_CreateSkill::Start()
{
	_status = TaskStatus::Running;

	// 获取 name 参数
	if (!_toolCall.GetStringParam("name", _name))
	{
		_Fail("Missing 'name' parameter");
		return;
	}

	// 获取 type 参数
	if (!_toolCall.GetStringParam("type", _type))
	{
		_Fail("Missing 'type' parameter");
		return;
	}

	// 验证 type 参数
	if (_type != "Global" && _type != "Project")
	{
		_Fail("Invalid 'type' parameter. Must be 'Global' or 'Project'");
		return;
	}

	// 获取 description 参数
	if (!_toolCall.GetStringParam("description", _description))
	{
		_Fail("Missing 'description' parameter");
		return;
	}

	// 获取 content 参数（可选）
	_toolCall.GetStringParam("content", _content);

	// 确定 skill 的存储路径
	std::string skillRootPath;
	CLlmSkills::Skill::Type skillType;

	if (_type == "Global")
	{
		// Global skill 存储在 DB root 目录下的 _skills 目录
		skillRootPath = Utils::GetDBRootFolder_utf8();
		skillRootPath += "\\_skills";
		skillType = CLlmSkills::Skill::Type::Global;
	}
	else // Project
	{
		// Project skill 存储在项目目录下的 _skills 目录
		const char* dbFolderPath = GetOpenedDBFolderPath_utf8();
		if (!dbFolderPath || dbFolderPath[0] == '\0')
		{
			_Fail("Project folder not available. Please open a project first.");
			return;
		}
		skillRootPath = dbFolderPath;
		skillRootPath += "\\_skills";
		skillType = CLlmSkills::Skill::Type::Project;
	}

	// 创建 skill 目录路径（支持子目录，如 folder\skillA）
	std::string skillFolderPath = skillRootPath + "\\" + _name;

	// 提取纯 skill 名称（去除路径部分）
	std::string pureSkillName = _ExtractSkillName(_name);
	if (pureSkillName.empty())
	{
		_Fail("Invalid skill name");
		return;
	}

	// 检查 skill 是否已存在
	std::wstring wSkillFolderPath = utf8_to_widechar(skillFolderPath.c_str());
	DWORD attrs = GetFileAttributesW(wSkillFolderPath.c_str());
	if (attrs != INVALID_FILE_ATTRIBUTES)
	{
		_Fail(("Skill '" + _name + "' already exists").c_str());
		return;
	}

	Utils::EnsureFolder(skillFolderPath.c_str());

	// 创建 SKILL.md 文件
	std::string skillMdPath = skillFolderPath + "\\SKILL.md";
	std::ofstream skillMdFile;
	Utils::OpenOFStream(skillMdFile, skillMdPath.c_str());
	if (!skillMdFile.is_open())
	{
		_Fail("Failed to create SKILL.md file");
		return;
	}

	// 写入 YAML frontmatter 和内容（使用纯 skill 名称）
	skillMdFile << "---\n";
	skillMdFile << "name: " << pureSkillName << "\n";
	skillMdFile << "description: " << _description << "\n";
	skillMdFile << "---\n";
	
	if (!_content.empty())
	{
		skillMdFile << "\n" << _content << "\n";
	}
	
	skillMdFile.close();

	// 创建 .enable 文件
	std::string enableFilePath = skillFolderPath + "\\.enable";
	std::ofstream enableFile;
	Utils::OpenOFStream(enableFile, enableFilePath.c_str());
	enableFile.close();

	// 重新加载 skills
	g_llmSkills.ReLoad(skillRootPath.c_str(), skillType);

	// 返回成功结果
	std::string result = "Skill '" + pureSkillName + "' created successfully at: " + skillFolderPath;
	_SendToolCallResult(result.c_str());
	_SendToolCallMessage(result.c_str());
	_Succeed();
}

void CChatTask_CreateSkill::Update()
{
	// 此任务在 Start 中完成，不需要 Update 处理
}

void CChatTask_CreateSkill::Interrupt()
{
	_status = TaskStatus::Failure;
}

