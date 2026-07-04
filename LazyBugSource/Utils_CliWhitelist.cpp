#include "stdh.h"
#include <fstream>
#include <sstream>
#include "Utils.h"
#include "Utils_CliWhitelist.h"
#include "stringparser/stringparser.h"
#include <string>



// 外部函数声明
extern const char* GetCurModuleFolderPath_utf8();
extern const char* GetOpenedDBFolderPath_utf8();

std::vector<std::string> g_cliWhitelist;

namespace Utils
{

	void LoadCliWhitelists(std::vector<std::string>& list)
	{
		// 从两个目录读取白名单文件
		const char* searchFolders[] = {
			GetDBRootFolder_utf8(),
			GetCurModuleFolderPath_utf8()
		};

		for (int i = 0; i < sizeof(searchFolders) / sizeof(searchFolders[0]); i++)
		{
			std::string filePath = std::string(searchFolders[i]) + "\\" + LAZYBUG_CLI_WHITELIST_FILENAME;

			std::string content;
			FileContentCodingFormat codingFmt;
			if (!GetFileContentIntoUTF8(filePath.c_str(), content, codingFmt))
			{
				continue; // 文件不存在或读取失败，跳过
			}

			// 按行解析
			std::istringstream stream(content);
			std::string line;
			while (std::getline(stream, line))
			{
				// 去掉行首空白字符后判断
				size_t firstNonSpace = 0;
				while (firstNonSpace < line.size() && (line[firstNonSpace] == ' ' || line[firstNonSpace] == '\t' || line[firstNonSpace] == '\r' || line[firstNonSpace] == '\n'))
				{
					firstNonSpace++;
				}

				// 如果这一行全是空白字符，略过
				if (firstNonSpace >= line.size())
					continue;

				// 如果以 // 开始，略过
				if (line[firstNonSpace] == '/' && firstNonSpace + 1 < line.size() && line[firstNonSpace + 1] == '/')
					continue;

				// 去掉行尾空白字符（特别是 \r）
				size_t lastNonSpace = line.size() - 1;
				while (lastNonSpace >= firstNonSpace && (line[lastNonSpace] == ' ' || line[lastNonSpace] == '\t' || line[lastNonSpace] == '\r' || line[lastNonSpace] == '\n'))
				{
					lastNonSpace--;
				}

				list.push_back(line.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1));
			}
		}
	}

	void EnsureCliWhitelists()
	{
		std::string filePath = std::string(GetDBRootFolder_utf8()) + "\\" + LAZYBUG_CLI_WHITELIST_FILENAME;

		// 如果文件已存在，不用创建
		if (IsFileExist(filePath.c_str()))
			return;

		// 确保 DB 目录存在
		EnsureFolder(GetDBRootFolder_utf8());

		// 创建新文件，写入注释
		const char* comment =
			"// This file lists which CLI (command-line) commands are trusted.\n"
			"// Trusted CLI commands will be executed directly without user confirmation.\n"
			"// Each line uses a regular expression to match command-line commands.\n"
			"// Hint: If you are not familiar with regular expressions, you can ask AI to write them for you.\n"
			"// Some examples:\n"
			"//^dir(\\s+\\S+)?\\s*$\n"
			"//^(cd|chdir)(\\s+\\S+)?\\s*$\n"
			"//^findstr(\\s+.*)?\\s*$\n"
			"\n";

		SaveFileContent(filePath.c_str(), comment);
	}


	// 获取 global_rules.md 文件路径
	std::string GetGlobalRulesFilePath()
	{
		const char* dbRootFolder = GetDBRootFolder_utf8();
		if (!dbRootFolder || *dbRootFolder == '\0')
			return "";

		return std::string(dbRootFolder) + "\\" + LAZYBUG_GLOBAL_CONTEXT_FILENAME;
	}

	// 获取 project_rules.md 文件路径
	std::string GetProjectRulesFilePath()
	{
		const char* openedDBFolder = GetOpenedDBFolderPath_utf8();
		if (!openedDBFolder || *openedDBFolder == '\0')
			return "";

		return std::string(openedDBFolder) + "\\" + LAZYBUG_PROJECT_CONTEXT_FILENAME;
	}

	void EnsureGlobalRulesFile()
	{
		std::string filePath = GetGlobalRulesFilePath();
		if (filePath.empty())
			return;

		EnsureFileFolder(filePath.c_str());

		// 如果文件不存在，则创建空文件
		if (!IsFileExist(filePath.c_str()))
		{
			SaveFileContent(filePath.c_str(), "");
		}
	}

	void EnsureProjectRulesFile()
	{
		std::string filePath = GetProjectRulesFilePath();
		if (filePath.empty())
			return;

		EnsureFileFolder(filePath.c_str());

		// 如果文件不存在，则创建空文件
		if (!IsFileExist(filePath.c_str()))
		{
			SaveFileContent(filePath.c_str(), "");
		}
	}



}
