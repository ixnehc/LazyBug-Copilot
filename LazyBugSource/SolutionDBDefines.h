#pragma once

struct ProjSetting
{
	std::vector<std::string> additionalIncludeFullPathes;//额外的include路径
	std::string lowerCasedPchFullPath;//预编译头文件的路径
	std::string lowerCasedPchOutputFullPath;//预编译头文件输出文件的路径

	// 判断两个ProjSetting是否相等
	bool Equals(const ProjSetting& other) const
	{
		return additionalIncludeFullPathes == other.additionalIncludeFullPathes &&
			lowerCasedPchFullPath == other.lowerCasedPchFullPath &&
			lowerCasedPchOutputFullPath == other.lowerCasedPchOutputFullPath;
	}
};

typedef unsigned __int64 ProjSettingHandle;//WUID
#define ProjSettingHandle_Null (0ULL)

struct ProjFile
{
	std::string lowerCasedFilePath;//(完整路径)
	std::string filePath;//原始路径名(包含大小写)(完整路径)
	std::string fileName;
};
