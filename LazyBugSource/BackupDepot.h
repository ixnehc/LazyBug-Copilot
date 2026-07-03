#pragma once

#include <vector>
#include <string>

class CBackupDepot
{
public:
	void Init(const char* folderPath);
	void Add(const char* filePath);
	void Clear();

private:
	static const int MAX_BACKUP_COUNT = 100;

	struct BackupFileInfo
	{
		std::string fileName;
		AbsTick fileTick; // 文件修改时间, 用于排序
	};

	std::string m_folderPath;
	std::vector<BackupFileInfo> m_files; // 按时间从旧到新排序

	void ScanExistingFiles();
	std::string GenerateBackupFileName(const char* originalFileName);
	void RemoveOldest();
};

