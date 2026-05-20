#include "stdh.h"
#include "BackupDepot.h"
#include "Utils_File.h"
#include <windows.h>
#include <io.h>
#include <algorithm>

void CBackupDepot::Init(const char* dbFolder)
{
	m_folderPath = dbFolder;
	m_folderPath += "\\_backup";
	Utils::EnsureFolder(m_folderPath.c_str());
	ScanExistingFiles();
}

void CBackupDepot::Add(const char* filePath)
{
	if (!filePath || *filePath == '\0')
		return;
	if (!Utils::IsFileExist(filePath))
		return;

	// 从原始路径中提取文件名(不含目录)
	std::string fullPath = Utils::GetActualFilePath(filePath);
	const char* nameStart = strrchr(fullPath.c_str(), '\\');
	if (!nameStart)
		nameStart = strrchr(fullPath.c_str(), '/');
	if (nameStart)
		nameStart++;
	else
		nameStart = fullPath.c_str();

	// 提取文件名和扩展名
	std::string baseName = nameStart;
	size_t dotPos = baseName.rfind('.');
	std::string namePart = (dotPos != std::string::npos) ? baseName.substr(0, dotPos) : baseName;
	std::string extPart = (dotPos != std::string::npos) ? baseName.substr(dotPos) : "";

	// 生成备份文件名: 原文件名_YYYYMMDD_HHMMSS.扩展名
	SYSTEMTIME st;
	GetLocalTime(&st);

	char timeSuffix[64];
	sprintf_s(timeSuffix, "_%04d%02d%02d_%02d%02d%02d",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);

	std::string backupFileName = namePart + timeSuffix + extPart;
	std::string backupFullPath = m_folderPath + "\\" + backupFileName;

	// 复制文件到备份目录
	Utils::CopyFile(fullPath.c_str(), backupFullPath.c_str());

	// 将新备份加入列表
	AbsTick tick = Utils::GetFileTick(backupFullPath.c_str());
	BackupFileInfo info;
	info.fileName = backupFileName;
	info.fileTick = tick;

	// 按时间从旧到新插入, 保持有序
	auto it = std::upper_bound(m_files.begin(), m_files.end(), info,
		[](const BackupFileInfo& a, const BackupFileInfo& b) { return a.fileTick < b.fileTick; });
	m_files.insert(it, info);

	// 超过上限时删除最旧的
	if ((int)m_files.size() > MAX_BACKUP_COUNT)
		RemoveOldest();
}

void CBackupDepot::Clear()
{
	m_files.clear();
	m_folderPath = "";
}

void CBackupDepot::ScanExistingFiles()
{
	m_files.clear();

	std::string searchPath = m_folderPath + "\\*";
	_finddata_t findData;
	intptr_t hFind = _findfirst(searchPath.c_str(), &findData);
	if (hFind == -1)
		return;

	do
	{
		// 跳过目录(. 和 .. 以及子目录)
		if (findData.attrib & _A_SUBDIR)
			continue;

		BackupFileInfo info;
		info.fileName = findData.name;
		std::string fullPath = m_folderPath + "\\" + info.fileName;
		info.fileTick = Utils::GetFileTick(fullPath.c_str());
		m_files.push_back(info);
	} while (_findnext(hFind, &findData) == 0);

	_findclose(hFind);

	// 按时间从旧到新排序
	std::sort(m_files.begin(), m_files.end(),
		[](const BackupFileInfo& a, const BackupFileInfo& b) { return a.fileTick < b.fileTick; });
}

void CBackupDepot::RemoveOldest()
{
	if (m_files.empty())
		return;

	const BackupFileInfo& oldest = m_files.front();
	std::string fullPath = m_folderPath + "\\" + oldest.fileName;
	Utils::RemoveFile(fullPath.c_str());
	m_files.erase(m_files.begin());
}
