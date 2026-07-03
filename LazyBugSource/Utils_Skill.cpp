#include "stdh.h"
#include <fstream>
#include <sstream>
#include "Utils.h"
#include "Utils_Skill.h"
#include "LlmSkills.h"
#include "stringparser/stringparser.h"
#include <string>



// 外部函数声明
extern const char* GetCurModuleFolderPath_utf8();
extern const char* GetOpenedDBFolderPath_utf8();
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern std::string widechar_to_utf8(const wchar_t* str);

namespace Utils
{

// 辅助函数：检查目录是否包含 skill.md 文件
static bool IsSkillFolder(const std::wstring& folderPath)
{
	std::wstring skillMdPath = folderPath + L"\\skill.md";
	
	DWORD attrib = GetFileAttributesW(skillMdPath.c_str());
	return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

// 辅助函数：递归复制目录
static bool CopyFolderRecursive(const std::wstring& srcPath, const std::wstring& targetPath)
{
	// 创建目标目录
	if (!CreateDirectoryW(targetPath.c_str(), NULL))
	{
		DWORD error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS)
		{
			return false;
		}
	}

	// 遍历源目录
	std::wstring searchPath = srcPath + L"\\*.*";
	WIN32_FIND_DATAW fd;
	HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return true; // 目录为空

	bool success = true;
	do
	{
		// 跳过 . 和 ..
		if (fd.cFileName[0] == L'.')
		{
			if (fd.cFileName[1] == L'\0' || (fd.cFileName[1] == L'.' && fd.cFileName[2] == L'\0'))
				continue;
		}

		std::wstring srcFullPath = srcPath + L"\\" + fd.cFileName;
		std::wstring targetFullPath = targetPath + L"\\" + fd.cFileName;

		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 递归复制子目录
			if (!CopyFolderRecursive(srcFullPath, targetFullPath))
			{
				success = false;
				break;
			}
		}
		else
		{
			// 复制文件
			if (!CopyFileW(srcFullPath.c_str(), targetFullPath.c_str(), FALSE))
			{
				success = false;
				break;
			}
		}
	} while (FindNextFileW(hFind, &fd));

	FindClose(hFind);
	return success;
}

void SyncBuiltInSkills()
{
	// 获取 source folder（模块所在目录下的 skills）
	std::string srcPathUtf8 = GetCurModuleFolderPath_utf8();
	srcPathUtf8 += "\\skills";
	std::wstring srcPath = utf8_to_widechar(srcPathUtf8);

	// 获取 target folder（DB 根目录下的 skills\builtin）
	std::string targetPathUtf8 = GetDBRootFolder_utf8();
	targetPathUtf8 += "\\_skills\\builtin";
	std::wstring targetPath = utf8_to_widechar(targetPathUtf8);

	// 确保 target folder 存在
	Utils::EnsureFolder(targetPathUtf8.c_str());

	// 遍历 source folder 下的所有子目录
	std::wstring searchPath = srcPath + L"\\*.*";
	WIN32_FIND_DATAW fd;
	HANDLE hFind = FindFirstFileW(searchPath.c_str(), &fd);

	if (hFind == INVALID_HANDLE_VALUE)
		return; // source folder 不存在或为空

	do
	{
		// 只处理目录
		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		// 跳过 . 和 ..
		if (fd.cFileName[0] == L'.')
		{
			if (fd.cFileName[1] == L'\0' || (fd.cFileName[1] == L'.' && fd.cFileName[2] == L'\0'))
				continue;
		}

		std::wstring skillSrcPath = srcPath + L"\\" + fd.cFileName;

		// 检查是否是 skill 目录（包含 skill.md）
		if (!IsSkillFolder(skillSrcPath))
			continue;

		// 检查 target folder 下是否存在对应目录
		std::wstring skillTargetPath = targetPath + L"\\" + fd.cFileName;

		DWORD attrib = GetFileAttributesW(skillTargetPath.c_str());
		if (attrib != INVALID_FILE_ATTRIBUTES)
		{
			// 目录已存在，跳过
			continue;
		}

		// 复制整个 skill 目录到 target folder
		CopyFolderRecursive(skillSrcPath, skillTargetPath);

	} while (FindNextFileW(hFind, &fd));

	FindClose(hFind);
}


std::vector<std::pair<std::string, CLlmSkills::Skill::Type>> GetSkillsFolder(const char* openedDBFolder)
{
	std::vector<std::pair<std::string, CLlmSkills::Skill::Type>> folders;

	// 1. BuiltIn: 从 GetCurModuleFolderPath_utf8() 下的 skills 目录
	{
		std::string builtInPath = GetDBRootFolder_utf8();
		builtInPath += "\\_skills\\builtin";
		folders.push_back({ builtInPath, CLlmSkills::Skill::Type::BuiltIn });
	}

	// 2. Global: 从 GetDBRootFolder_utf8() 下的 _skills 目录
	{
		std::string globalPath = GetDBRootFolder_utf8();
		globalPath += "\\_skills\\global";
		folders.push_back({ globalPath, CLlmSkills::Skill::Type::Global });
	}

	// 3. Project: 从 openedDBFolder 下的 _skills 目录
	{
		if (openedDBFolder && openedDBFolder[0] != '\0')
		{
			std::string projectPath = openedDBFolder;
			projectPath += "\\_skills";
			folders.push_back({ projectPath, CLlmSkills::Skill::Type::Project });
		}
	}

	return folders;
}

// 递归枚举目录下的文件
static bool EnumFilesRecursive(const std::wstring& folderPath, CLlmSkills::Skill::Type type,
	std::function<EnumFilesInSkillFolderFilter(const char* filePath, CLlmSkills::Skill::Type type)> filter)
{
	std::wstring wSearchPath = folderPath + L"\\*.*";
	WIN32_FIND_DATAW fd;
	HANDLE hFind = FindFirstFileW(wSearchPath.c_str(), &fd);
	
	if (hFind == INVALID_HANDLE_VALUE)
		return true; // 目录不存在或为空，继续
	
	bool shouldContinue = true;
	
	do
	{
		// 跳过 . 和 ..
		if (fd.cFileName[0] == L'.')
		{
			if (fd.cFileName[1] == L'\0' || (fd.cFileName[1] == L'.' && fd.cFileName[2] == L'\0'))
				continue;
		}
		
		// 跳过隐藏文件/目录
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
			continue;
		
		std::wstring wFullPath = folderPath + L"\\" + fd.cFileName;
		
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// 递归子目录
			if (!EnumFilesRecursive(wFullPath, type, filter))
			{
				shouldContinue = false;
				break;
			}
		}
		else
		{
			// 文件：调用 filter（转为 UTF-8）
			if (filter)
			{
				std::string fullPathUtf8 = widechar_to_utf8(wFullPath.c_str());
				EnumFilesInSkillFolderFilter result = filter(fullPathUtf8.c_str(), type);
				if (result == EnumFilesInSkillFolderFilter::Stop)
				{
					shouldContinue = false;
					break;
				}
				// Reject 或 Accept 都继续遍历
			}
		}
	} while (FindNextFileW(hFind, &fd));
	
	FindClose(hFind);
	return shouldContinue;
}

bool EnumFilesInSkillFolder(const char* openedDBFolder, 
	std::function<EnumFilesInSkillFolderFilter(const char* filePath, CLlmSkills::Skill::Type type)> filter)
{
	auto folders = GetSkillsFolder(openedDBFolder);
	
	for (const auto& folder : folders)
	{
		std::wstring wFolderPath = utf8_to_widechar(folder.first);
		if (!EnumFilesRecursive(wFolderPath, folder.second, filter))
			return false; // 被中断
	}
	
	return true; // 正常完成
}

void LoadLlmSkills(CLlmSkills &skills,const char *openedDBFolder)
{
	auto folders = GetSkillsFolder(openedDBFolder);
	for (const auto& folder : folders)
	{
		skills.ReLoad(folder.first.c_str(), folder.second);
	}
}

bool MakeSkillTagName(const char* filePath, std::string& tagName)
{
	tagName.clear();

	std::string fileName = GetFileName(filePath);
	StringLower(fileName);
	if (fileName != "skill.md")
		return false;

	// 调用 _ParseSkillMd 解析 skill.md 文件，获取 skill 名称
	std::string skillName;
	std::string skillDescription;
	if (!ParseSkillMd(filePath, skillName, skillDescription))
		return false;

	tagName = skillName + "(skill)";
	return true;
}


}
