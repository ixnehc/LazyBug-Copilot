#pragma once

#include "FileWatcher.h"
#include <unordered_map>
#include <set>
#include <shared_mutex>

struct WatchedFolder
{
	std::string path;
	CFileWatcher* pWatcher;
	CChangedFileList pendingChanges;  // 存储待处理的文件变化
	
	WatchedFolder() : pWatcher(nullptr) {}
	~WatchedFolder()
	{
		if (pWatcher)
		{
			delete pWatcher;
			pWatcher = nullptr;
		}
	}
};

class CFoldersWatcher
{
public:
	CFoldersWatcher(int maxWatchFolder);
	~CFoldersWatcher();

	void AddFilePath(const char* path);

	// 设置文件后缀过滤列表（例如：".cpp", ".h"）
	void SetSuffixFilter(const std::vector<std::string>& suffixes);
	void ClearSuffixFilter();

	BOOL IsStarted();
	BOOL Start(DWORD dwNotifyFilter = WNF_CHANGE_LAST_WRITE | WNF_CHANGE_FILE_NAME);
	void Stop();
	int FetchChangedFiles(const ChangedFileInformation*& files);

	void GetFolderPathes(std::vector<std::string>& pathes);

protected:
	// 从文件路径提取目录路径
	std::string _ExtractFolderPath(const char* filePath);
	
	// 查找包含指定文件路径的监控目录
	WatchedFolder* _FindContainingFolder(const char* filePath);
	
	// 添加新的监控目录
	BOOL _AddWatchFolder(const std::string& folderPath);
	
	// 合并监控目录以减少数量
	void _MergeFolders();
	
	// 查找两个路径的公共父目录
	std::string _FindCommonParent(const std::string& path1, const std::string& path2);
	
	// 收集所有待处理的文件变化
	void _CollectPendingChanges();
	
protected:
	int _maxWatchFolder;
	DWORD _dwNotifyFilter;
	BOOL _bStarted;
	
	// 使用unordered_map存储监控的目录，key为目录路径（小写），value为监控信息
	std::unordered_map<std::string, WatchedFolder*> _watchedFolders;
	
	// 用于保护 _watchedFolders 的读写访问
	mutable std::shared_mutex _foldersMutex;
	
	// 临时存储所有收集到的文件变化
	CChangedFileList _allChanges;
	
	// 后缀过滤列表
	std::vector<std::string> _suffixFilters;
};
