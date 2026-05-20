#include "stdh.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cctype>
#include <mutex>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <direct.h>
#include <tlhelp32.h>
#include <process.h>
#include "Utils.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <sys/stat.h>
#include <string>

#include <regex>
#include "xml/tinyxml.h"

#include "SolutionDump.h"


// 处理路径的辅助函数
std::string ProcessPath(
	const std::string& originalPath,
	const std::string& vcxprojDir
)
{
	if (vcxprojDir.empty())
		return originalPath;
	std::string result = vcxprojDir + "\\" + originalPath;
	return ResolveRelativePathWithDots(result);
}

extern std::vector<std::string> ExtractFilePathsFromProj(const std::string& projContent, const std::string& pathProj);

std::vector<std::string> ExtractAddionalIncludePathesFromVcxproj(
	const std::string& vcxprojContent,
	const std::string& pathVcxProj)
{
	// 获取vcxproj文件所在的目录
	std::string vcxprojDir = GetFileFolderPath(pathVcxProj);

	std::vector<std::string> includePaths;
	std::vector<std::string> lastValidPaths; // 存储最后一个section中存在的路径
	std::sregex_iterator end;

	// 匹配AdditionalIncludeDirectories标签
	std::regex additionalIncludePattern("<AdditionalIncludeDirectories>([^<]+)</AdditionalIncludeDirectories>");
	std::sregex_iterator additionalIncludeIter(vcxprojContent.begin(), vcxprojContent.end(), additionalIncludePattern);

	// 遍历所有找到的AdditionalIncludeDirectories标签
	while (additionalIncludeIter != end)
	{
		std::string directories = (*additionalIncludeIter)[1].str();
		std::vector<std::string> currentIncludePaths;
		bool allPathsExist = true;

		// 分割路径（以分号分隔）
		size_t pos = 0;
		std::string delimiter = ";";
		std::string remainingDirectories = directories;

		while ((pos = remainingDirectories.find(delimiter)) != std::string::npos)
		{
			std::string dir = remainingDirectories.substr(0, pos);
			if (!dir.empty())
			{
				std::string absolutePath;
				// 检查是否已经是绝对路径
				if (dir[0] != '$')
				{
					if ((dir.length() >= 2 && dir[1] == ':') || (dir.length() >= 1 && dir[0] == '\\'))
					{
						// 已经是绝对路径，直接使用
						absolutePath = ResolveRelativePathWithDots(dir);
					}
					else
					{
						// 相对路径，与vcxprojDir结合创建绝对路径
						absolutePath = vcxprojDir + "\\" + dir;
						absolutePath = ResolveRelativePathWithDots(absolutePath);
					}

					// 检查路径是否存在
					if (Utils::IsFileExist(absolutePath.c_str()))
					{
						currentIncludePaths.push_back(absolutePath);
					}
					else
					{
						allPathsExist = false;
						// 不立即break，继续检查其他路径
					}
				}
			}
			remainingDirectories.erase(0, pos + delimiter.length());
		}

		// 处理最后一个路径
		if (!remainingDirectories.empty() && remainingDirectories != "%(AdditionalIncludeDirectories)")
		{
			std::string absolutePath;
			// 检查是否已经是绝对路径
			if ((remainingDirectories.length() >= 2 && remainingDirectories[1] == ':') || (remainingDirectories.length() >= 1 && remainingDirectories[0] == '\\'))
			{
				// 已经是绝对路径，直接使用
				absolutePath = ResolveRelativePathWithDots(remainingDirectories);
			}
			else
			{
				// 相对路径，与vcxprojDir结合创建绝对路径
				absolutePath = vcxprojDir + "\\" + remainingDirectories;
				absolutePath = ResolveRelativePathWithDots(absolutePath);
			}

			// 检查路径是否存在
			if (Utils::IsFileExist(absolutePath.c_str()))
			{
				currentIncludePaths.push_back(absolutePath);
			}
			else
			{
				allPathsExist = false;
			}
		}

		// 如果当前section的所有路径都存在，使用这个section
		if (allPathsExist && !currentIncludePaths.empty())
		{
			includePaths = std::move(currentIncludePaths);
			break;
		}

		// 保存当前section中存在的路径作为最后一个有效的路径集合
		if (!currentIncludePaths.empty())
		{
			lastValidPaths = std::move(currentIncludePaths);
		}

		// 继续查找下一个AdditionalIncludeDirectories标签
		++additionalIncludeIter;
	}

	// 如果找到了所有路径都存在的section，返回它
	if (!includePaths.empty())
	{
		return includePaths;
	}

	// 否则返回最后一个section中存在的路径列表
	return lastValidPaths;
}

// 从vcxproj文件中提取预编译头文件路径
std::string ExtractPCHPathFromVcxproj(
	const std::string& vcxprojContent,
	const std::string& workspacePath,
	const std::string& pathVcxProj)
{
	// 获取vcxproj文件所在的目录
	std::string vcxprojDir = GetFileFolderPath(pathVcxProj);

	std::sregex_iterator end;

	// 方法1: 查找设置了PrecompiledHeader为Create的ClCompile项目
	std::regex createPchPattern("<ClCompile\\s+Include=\"([^\"]+)\"[^/>]*>(?:(?!</ClCompile>)[\\s\\S])*?<PrecompiledHeader[^>]*>Create</PrecompiledHeader>[\\s\\S]*?</ClCompile>", std::regex::icase);
	std::sregex_iterator createPchIter(vcxprojContent.begin(), vcxprojContent.end(), createPchPattern);

	if (createPchIter != end)
	{
		std::string cppPath = (*createPchIter)[1].str();
		if (!cppPath.empty())
		{
			// 从.cpp文件路径推断.h文件路径
			std::string pchPath = cppPath;
			size_t dotPos = pchPath.find_last_of('.');
			if (dotPos != std::string::npos &&
				(pchPath.substr(dotPos) == ".cpp" || pchPath.substr(dotPos) == ".CPP"))
			{
				pchPath = pchPath.substr(0, dotPos) + ".h";

				std::string realPath = ProcessPath(pchPath, vcxprojDir);

				// 转换为相对于workspace的路径
				if (realPath.length() > workspacePath.length() &&
					realPath.substr(0, workspacePath.length()) == workspacePath)
				{
					return realPath.substr(workspacePath.length() + 1);
				}
				return realPath;
			}
		}
	}

	// 方法2: 查找PrecompiledHeaderFile标签（原有方法）
	std::regex pchPattern("<PrecompiledHeaderFile>([^<]+)</PrecompiledHeaderFile>");
	std::sregex_iterator pchIter(vcxprojContent.begin(), vcxprojContent.end(), pchPattern);

	if (pchIter != end)
	{
		std::string pchPath = (*pchIter)[1].str();
		if (!pchPath.empty() && pchPath != "%(PrecompiledHeaderFile)")
		{
			std::string realPath = ProcessPath(pchPath, vcxprojDir);

			// 转换为相对于workspace的路径
			if (realPath.length() > workspacePath.length() &&
				realPath.substr(0, workspacePath.length()) == workspacePath)
			{
				return realPath.substr(workspacePath.length() + 1);
			}
			return realPath;
		}
	}

	return "";
}


std::string ExtractPCHPathFromVcxproj_XML(
	const std::string& vcxprojContent,
	const std::string& pathVcxProj)
{
	std::string vcxprojDir = GetFileFolderPath(pathVcxProj);

	TiXmlDocument doc;
	doc.Parse(vcxprojContent.c_str());

	if (doc.Error()) {
		return ""; // XML解析失败
	}

	TiXmlElement* root = doc.FirstChildElement();
	if (!root) {
		return "";
	}

	// 方法1: 查找设置了 PrecompiledHeader 为 "Create" 的 ClCompile 项
	for (TiXmlElement* itemGroup = root->FirstChildElement("ItemGroup"); itemGroup; itemGroup = itemGroup->NextSiblingElement("ItemGroup"))
	{
		for (TiXmlElement* clCompile = itemGroup->FirstChildElement("ClCompile"); clCompile; clCompile = clCompile->NextSiblingElement("ClCompile"))
		{
			// 查找子节点 PrecompiledHeader
			TiXmlElement* precompiledHeader = clCompile->FirstChildElement("PrecompiledHeader");
			if (precompiledHeader && precompiledHeader->FirstChild() && precompiledHeader->FirstChild()->ToText())
			{
				const char* text = precompiledHeader->FirstChild()->Value().c_str();
				if (text && _stricmp(text, "Create") == 0)
				{
					const std::string* includePath = clCompile->Attribute("Include");
					if (includePath)
					{
						std::string pchPath = *includePath;
						std::string realPath = ProcessPath(pchPath, vcxprojDir);
						return realPath;

						//                         size_t dotPos = pchPath.find_last_of('.');
						//                         if (dotPos != std::string::npos) {
						//                             pchPath = pchPath.substr(0, dotPos) + ".h";
						//                             std::string realPath = ProcessPath(pchPath, vcxprojDir);
						//                             return realPath;
						//                         }
					}
				}
			}
		}
	}

	// 方法2: 查找 PrecompiledHeaderFile 标签 (作为备用)
	std::vector<TiXmlElement*> stack;
	stack.push_back(root);
	while (!stack.empty())
	{
		TiXmlElement* current = stack.back();
		stack.pop_back();

		TiXmlElement* pchFileElement = current->FirstChildElement("PrecompiledHeaderFile");
		if (pchFileElement && pchFileElement->FirstChild() && pchFileElement->FirstChild()->ToText())
		{
			const char* text = pchFileElement->FirstChild()->Value().c_str();
			if (text)
			{
				std::string pchPath = text;
				if (!pchPath.empty() && pchPath.find("PrecompiledHeaderFile") == std::string::npos)
				{
					std::string realPath = ProcessPath(pchPath, vcxprojDir);
					return realPath;
				}
			}
		}

		for (TiXmlElement* child = current->FirstChildElement(); child; child = child->NextSiblingElement())
		{
			stack.push_back(child);
		}
	}

	return "";
}

//从.sln文件里提取出所有项目文件的绝对路径(支持.vcxproj, .csproj, .vbproj, .fsproj等)
std::vector<std::string> ExtractProjPathFromSln(const std::string& slnContent, const std::string& pathSln)
{
	std::vector<std::string> projectPaths;

	// 获取.sln文件所在的目录
	std::string slnDir = GetFileFolderPath(pathSln);

	// 使用正则表达式查找项目文件引用
	// 典型格式: Project("{...}") = "ProjectName", "Path\To\ProjectName.xxx", "{...}"
	std::regex projectPattern("Project\\(\"[^\"]*\"\\)\\s*=\\s*\"[^\"]*\"\\s*,\\s*\"([^\"]*)\"");
	std::sregex_iterator projectIter(slnContent.begin(), slnContent.end(), projectPattern);
	std::sregex_iterator end;

	// 支持的项目文件扩展名列表
	const std::vector<std::string> supportedExtensions = {
		".vcxproj",   // C++ 项目
		".csproj",    // C# 项目
		".vbproj",    // Visual Basic 项目
		".fsproj",    // F# 项目
		".vcproj",    // 旧版 C++ 项目
		".pyproj",    // Python 项目
		".njsproj",   // Node.js 项目
		".jsproj",    // JavaScript 项目
		".sqlproj",   // SQL Server 项目
		".wixproj",   // WiX 安装项目
		".shproj"     // 共享项目
	};

	for (; projectIter != end; ++projectIter)
	{
		std::string projectPath = (*projectIter)[1].str();

		// 检查是否是支持的项目文件类型
		bool isSupported = false;
		for (const auto& ext : supportedExtensions)
		{
			if (projectPath.length() > ext.length())
			{
				std::string pathExt = projectPath.substr(projectPath.length() - ext.length());
				// 不区分大小写比较
				if (_stricmp(pathExt.c_str(), ext.c_str()) == 0)
				{
					isSupported = true;
					break;
				}
			}
		}

		if (isSupported)
		{
			// 将路径转换为绝对路径
			std::string absolutePath = ProcessPath(projectPath, slnDir);
			projectPaths.push_back(absolutePath);
		}
	}

	return projectPaths;
}



namespace Utils
{

bool GenerateSolutionDump(const char *dbFolder,const char *slnPath, SolutionDump& dmp)
{
	if (!slnPath || !*slnPath)
		return false;

	// 检查solution文件是否存在
	if (!IsFileExist(slnPath))
		return false;

	// 读取solution文件内容
	std::string slnContent;
	FileContentCodingFormat codingFmt;
	if (!GetFileContentIntoUTF8(slnPath, slnContent, codingFmt))
		return false;

	// 设置solution路径（小写）
	dmp.lowerCasedPath = slnPath;
	StringLower(dmp.lowerCasedPath);

	// 提取所有项目文件路径
	std::vector<std::string> projPaths = ExtractProjPathFromSln(slnContent, slnPath);

	// 清空现有的项目数据
	dmp.projs.clear();

	// 处理每个项目文件
	for (const std::string& projPath : projPaths)
	{
		// 检查项目文件是否存在
		if (!IsFileExist(projPath.c_str()))
			continue;

		// 判断是否是 vcxproj 文件
		bool isVcxProj = false;
		std::string extension = GetFileSuffix(projPath);
		StringLower(extension);
		if (extension == "vcxproj")
			isVcxProj = true;

		// 读取项目文件内容
		std::string projContent;
		if (!GetFileContentIntoUTF8(projPath.c_str(), projContent, codingFmt))
			continue;

		// 创建项目转储对象
		SolutionDump::ProjDump projDump;
		std::string lowerCasedProjPath = projPath;
		StringLower(lowerCasedProjPath);

		// 提取文件路径列表
		std::vector<std::string> filePaths = ExtractFilePathsFromProj(projContent, projPath);

		// 转换为小写路径存入 set
		for (const std::string& realPath : filePaths)
		{
			std::string lowerPath = realPath;
			StringLower(lowerPath);
			projDump.files.insert(lowerPath);
		}

		// 如果是vcxproj文件，提取额外的设置信息
		if (isVcxProj)
		{
			// 提取额外的包含路径
			projDump.setting.additionalIncludeFullPathes = ExtractAddionalIncludePathesFromVcxproj(projContent, projPath);

			// 提取预编译头文件路径
			std::string pchFullPath = ExtractPCHPathFromVcxproj_XML(projContent, projPath);
			if (!pchFullPath.empty())
			{
				projDump.setting.lowerCasedPchFullPath = pchFullPath;
				StringLower(projDump.setting.lowerCasedPchFullPath);

				projDump.setting.lowerCasedPchOutputFullPath = dbFolder;
				projDump.setting.lowerCasedPchOutputFullPath += "\\_pch\\";
				std::string s = projDump.setting.lowerCasedPchFullPath;
				RemoveFileSuffix(s);
				std::string name;
				ConvertFullPathToName(s.c_str(), name);
				projDump.setting.lowerCasedPchOutputFullPath += name;
				projDump.setting.lowerCasedPchOutputFullPath += ".pch";
				StringLower(projDump.setting.lowerCasedPchOutputFullPath);
			}
		}

		// 添加到solution dump
		dmp.projs[lowerCasedProjPath] = std::move(projDump);
	}

	return true;
}

}
