#include "stdh.h"

#include <regex>

#include "stringparser/stringparser.h"


extern std::string ProcessPath(
	const std::string& originalPath,
	const std::string& vcxprojDir
);


// ========== C++ 项目文件解析 (.vcxproj) ==========

// 从vcxproj文件中提取文件路径
std::vector<std::string> ExtractFilePathsFromVcxproj(
	const std::string& vcxprojContent,
	const std::string& pathVcxProj)
{
	std::vector<std::string> filePaths;

	// 获取vcxproj文件所在的目录
	std::string vcxprojDir = GetFileFolderPath(pathVcxProj);

	std::sregex_iterator end;

	// 定义正则表达式来匹配文件引用模式
	std::regex includePattern("<(?:ClInclude|ClCompile|None|Content|ResourceCompile|Library|ProjectReference|VSCTCompile|CustomBuild)\\s+Include=\"([^\"]+)\"");

	// 使用正则表达式搜索文件内容
	std::sregex_iterator includeIter(vcxprojContent.begin(), vcxprojContent.end(), includePattern);

	// 处理 Include 属性中的文件路径
	for (; includeIter != end; ++includeIter)
	{
		std::string realPath = ProcessPath((*includeIter)[1].str(), vcxprojDir);
		filePaths.push_back(realPath);
	}

	return filePaths;
}


// ========== C# 项目文件解析 (.csproj) ==========

// 从csproj文件中提取文件路径
std::vector<std::string> ExtractFilePathsFromCsproj(
	const std::string& csprojContent,
	const std::string& pathCsProj)
{
	std::vector<std::string> filePaths;

	// 获取csproj文件所在的目录
	std::string csprojDir = GetFileFolderPath(pathCsProj);

	std::sregex_iterator end;

	// 定义正则表达式来匹配C#项目中的文件引用模式
	// C#项目通常包含 Compile、EmbeddedResource、None、Content 等类型的文件
	std::regex includePattern("<(Compile|EmbeddedResource|None|Content|Resource|ApplicationDefinition|Page|ResourceDictionary)\\s+Include=\"([^\"]+)\"");

	// 使用正则表达式搜索文件内容
	std::sregex_iterator includeIter(csprojContent.begin(), csprojContent.end(), includePattern);

	// 处理 Include 属性中的文件路径
	for (; includeIter != end; ++includeIter)
	{
		std::string realPath = ProcessPath((*includeIter)[2].str(), csprojDir);
		filePaths.push_back(realPath);
	}

	// 处理通配符路径（如 **\\*.cs 等）
	std::regex wildcardPattern("<(Compile|EmbeddedResource|None|Content)\\s+Include=\"([^\"]*\\*[^\"]*)\"");
	std::sregex_iterator wildcardIter(csprojContent.begin(), csprojContent.end(), wildcardPattern);

	for (; wildcardIter != wildcardIter; ++wildcardIter)
	{
		std::string wildcardPath = (*wildcardIter)[2].str();
		if (!wildcardPath.empty())
		{
			// 将通配符路径转换为绝对路径
			std::string absoluteWildcardPath = ProcessPath(wildcardPath, csprojDir);

			// 这里简化处理：直接添加通配符路径
			// 实际应用中可能需要展开通配符匹配具体文件
			filePaths.push_back(absoluteWildcardPath);
		}
	}

	return filePaths;
}


// ========== Visual Basic 项目文件解析 (.vbproj) ==========

// 从vbproj文件中提取文件路径
std::vector<std::string> ExtractFilePathsFromVbproj(
	const std::string& vbprojContent,
	const std::string& pathVbProj)
{
	std::vector<std::string> filePaths;

	// 获取vbproj文件所在的目录
	std::string vbprojDir = GetFileFolderPath(pathVbProj);

	std::sregex_iterator end;

	// 定义正则表达式来匹配VB项目中的文件引用模式
	std::regex includePattern("<(Compile|EmbeddedResource|None|Content|Import)\\s+Include=\"([^\"]+)\"");

	// 使用正则表达式搜索文件内容
	std::sregex_iterator includeIter(vbprojContent.begin(), vbprojContent.end(), includePattern);

	// 处理 Include 属性中的文件路径
	for (; includeIter != end; ++includeIter)
	{
		std::string relativePath = (*includeIter)[2].str();
		// 跳过通配符路径
		if (relativePath.find('*') == std::string::npos)
		{
			std::string realPath = ProcessPath(relativePath, vbprojDir);
			filePaths.push_back(realPath);
		}
	}

	return filePaths;
}


// ========== F# 项目文件解析 (.fsproj) ==========

// 从fsproj文件中提取文件路径
std::vector<std::string> ExtractFilePathsFromFsproj(
	const std::string& fsprojContent,
	const std::string& pathFsProj)
{
	std::vector<std::string> filePaths;

	// 获取fsproj文件所在的目录
	std::string fsprojDir = GetFileFolderPath(pathFsProj);

	std::sregex_iterator end;

	// 定义正则表达式来匹配F#项目中的文件引用模式
	std::regex includePattern("<(Compile|EmbeddedResource|None|Content|Resource)\\s+Include=\"([^\"]+)\"");

	// 使用正则表达式搜索文件内容
	std::sregex_iterator includeIter(fsprojContent.begin(), fsprojContent.end(), includePattern);

	// 处理 Include 属性中的文件路径
	for (; includeIter != end; ++includeIter)
	{
		std::string relativePath = (*includeIter)[2].str();
		// 跳过通配符路径
		if (relativePath.find('*') == std::string::npos)
		{
			std::string realPath = ProcessPath(relativePath, fsprojDir);
			filePaths.push_back(realPath);
		}
	}

	return filePaths;
}


// ========== Python 项目文件解析 (.pyproj) ==========

// 从pyproj文件中提取文件路径
std::vector<std::string> ExtractFilePathsFromPyproj(
	const std::string& pyprojContent,
	const std::string& pathPyProj)
{
	std::vector<std::string> filePaths;

	// 获取pyproj文件所在的目录
	std::string pyprojDir = GetFileFolderPath(pathPyProj);

	std::sregex_iterator end;

	// 定义正则表达式来匹配Python项目中的文件引用模式
	std::regex includePattern("<(Compile|Content|None|Folder|Interpreter)\\s+Include=\"([^\"]+)\"");

	// 使用正则表达式搜索文件内容
	std::sregex_iterator includeIter(pyprojContent.begin(), pyprojContent.end(), includePattern);

	// 处理 Include 属性中的文件路径
	for (; includeIter != end; ++includeIter)
	{
		std::string relativePath = (*includeIter)[2].str();
		// 跳过通配符路径和文件夹
		if (relativePath.find('*') == std::string::npos)
		{
			std::string realPath = ProcessPath(relativePath, pyprojDir);
			filePaths.push_back(realPath);
		}
	}

	return filePaths;
}


// ========== 通用项目文件解析 ==========

// 通用的项目文件解析函数，根据文件扩展名调用相应的解析函数
std::vector<std::string> ExtractFilePathsFromProj(const std::string& projContent, const std::string& pathProj)
{
	// 获取文件扩展名
	std::string extension = GetFileSuffix(pathProj);
	StringLower(extension);

	// 根据扩展名调用相应的解析函数
	if (extension == "vcxproj" || extension == "vcproj")
	{
		return ExtractFilePathsFromVcxproj(projContent, pathProj);
	}
	else if (extension == "csproj")
	{
		return ExtractFilePathsFromCsproj(projContent, pathProj);
	}
	else if (extension == "vbproj")
	{
		return ExtractFilePathsFromVbproj(projContent, pathProj);
	}
	else if (extension == "fsproj")
	{
		return ExtractFilePathsFromFsproj(projContent, pathProj);
	}
	else if (extension == "pyproj")
	{
		return ExtractFilePathsFromPyproj(projContent, pathProj);
	}
	else
	{
		// 对于其他未知类型，尝试使用通用的Include模式
		std::vector<std::string> filePaths;
		std::string projDir = GetFileFolderPath(pathProj);
		std::sregex_iterator end;

		std::regex includePattern("<[^>]+\\s+Include=\"([^\"]+)\"");
		std::sregex_iterator includeIter(projContent.begin(), projContent.end(), includePattern);

		for (; includeIter != end; ++includeIter)
		{
			std::string relativePath = (*includeIter)[1].str();
			if (relativePath.find('*') == std::string::npos)
			{
				std::string realPath = ProcessPath(relativePath, projDir);
				filePaths.push_back(realPath);
			}
		}

		return filePaths;
	}
}
