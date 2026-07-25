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

// 文件来源位掩码，支持多来源追踪
using FileSourceMask = uint32_t;
constexpr FileSourceMask SOURCE_SLNDUMP = 1 << 0;

// 来源更新结果
struct SourceUpdateResult
{
	std::vector<struct SolutionFile*> newFiles;
	std::vector<struct SolutionFile*> updatedFiles;
	std::vector<std::string> removedFiles;
};

// 单文件来源更新结果
enum class FileDelta : uint8_t
{
	None,       // 无变化（文件本来就不在该来源中，或本来就在该来源中）
	New,        // 文件 sourceMask 从 0 变为非 0
	Updated,    // sourceMask 变化但未穿越 0
	Removed     // sourceMask 变为 0（文件完全移除追踪）
};

struct ProjFile
{
	std::string lowerCasedFilePath;//(完整路径)
	std::string filePath;//原始路径名(包含大小写)(完整路径)
	std::string fileName;
};
