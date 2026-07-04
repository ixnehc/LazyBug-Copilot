#include "stdh.h"

#include "FoldersWatcher.h"
#include "../stringparser/stringparser.h"
#include <algorithm>

CFoldersWatcher::CFoldersWatcher(int maxWatchFolder)
	: _maxWatchFolder(maxWatchFolder)
	, _dwNotifyFilter(0)
	, _bStarted(FALSE)
{
}

CFoldersWatcher::~CFoldersWatcher()
{
	Stop();
	
	// 使用独占锁（写锁）保护对 _watchedFolders 的访问
	std::unique_lock<std::shared_mutex> lock(_foldersMutex);
	
	// 清理所有监控目录
	for (auto& pair : _watchedFolders)
	{
		delete pair.second;
	}
	_watchedFolders.clear();
}

BOOL CFoldersWatcher::IsStarted()
{
	return _bStarted;
}

BOOL CFoldersWatcher::Start(DWORD dwNotifyFilter)
{
	if (_bStarted)
		return FALSE;
	
	_dwNotifyFilter = dwNotifyFilter;
	
	// 使用共享锁（读锁）保护对 _watchedFolders 的访问
	std::shared_lock<std::shared_mutex> lock(_foldersMutex);
	
	// 启动所有监控器
	for (auto& pair : _watchedFolders)
	{
		WatchedFolder* pFolder = pair.second;
		if (pFolder->pWatcher == nullptr)
		{
			pFolder->pWatcher = new CFileWatcher();
		}
		
		// 设置后缀过滤
		if (!_suffixFilters.empty())
		{
			pFolder->pWatcher->SetSuffixFilter(_suffixFilters);
		}
		
		pFolder->pWatcher->Start(pFolder->path.c_str(), _dwNotifyFilter);
	}
	
	_bStarted = TRUE;
	return TRUE;
}

void CFoldersWatcher::Stop()
{
	if (!_bStarted)
		return;

	// 使用共享锁（读锁）保护对 _watchedFolders 的访问
	std::shared_lock<std::shared_mutex> lock(_foldersMutex);

	// 在停止前收集所有待处理的变化
	_CollectPendingChanges();
	
	
	// 停止所有监控器
	for (auto& pair : _watchedFolders)
	{
		WatchedFolder* pFolder = pair.second;
		if (pFolder->pWatcher)
		{
			pFolder->pWatcher->Stop();
		}
	}
	
	_bStarted = FALSE;
}

std::string CFoldersWatcher::_ExtractFolderPath(const char* filePath)
{
	return GetFileFolderPath(filePath);
}

WatchedFolder* CFoldersWatcher::_FindContainingFolder(const char* filePath)
{
	std::string fileFolder = _ExtractFolderPath(filePath);
	
	// 首先检查是否有完全匹配的目录
	std::string lowerFolder = fileFolder;
	StringLower(lowerFolder);
	
	auto it = _watchedFolders.find(lowerFolder);
	if (it != _watchedFolders.end())
	{
		return it->second;
	}
	
	// 检查是否有父目录包含此文件
	for (auto& pair : _watchedFolders)
	{
		if (CheckPathContaining(pair.second->path.c_str(), filePath))
		{
			return pair.second;
		}
	}
	
	return nullptr;
}

BOOL CFoldersWatcher::_AddWatchFolder(const std::string& folderPath)
{
	std::string lowerPath = folderPath;
	StringLower(lowerPath);
	
	// 检查是否已存在
	if (_watchedFolders.find(lowerPath) != _watchedFolders.end())
	{
		return TRUE;
	}
	
	WatchedFolder* pNewFolder = new WatchedFolder();
	pNewFolder->path = folderPath;
	
	// 如果已经启动，立即创建并启动监控器
	if (_bStarted)
	{
		pNewFolder->pWatcher = new CFileWatcher();
		
		// 设置后缀过滤
		if (!_suffixFilters.empty())
		{
			pNewFolder->pWatcher->SetSuffixFilter(_suffixFilters);
		}
		
		if (!pNewFolder->pWatcher->Start(folderPath.c_str(), _dwNotifyFilter))
		{
			delete pNewFolder;
			return FALSE;
		}
	}
	
	_watchedFolders[lowerPath] = pNewFolder;
	return TRUE;
}

std::string CFoldersWatcher::_FindCommonParent(const std::string& path1, const std::string& path2)
{
	std::vector<std::string> parts1, parts2;
	SplitStringBy("\\", path1, &parts1);
	SplitStringBy("\\", path2, &parts2);
	
	std::string commonPath;
	size_t minSize = min(parts1.size(), parts2.size());
	
	for (size_t i = 0; i < minSize; ++i)
	{
		if (StringEqualNoCase(parts1[i].c_str(), parts2[i].c_str()))
		{
			if (!commonPath.empty())
				commonPath += "\\";
			commonPath += parts1[i];
		}
		else
		{
			break;
		}
	}
	
	return commonPath;
}

void CFoldersWatcher::_MergeFolders()
{
	if (_watchedFolders.size() <= _maxWatchFolder)
		return;
	
	// 收集当前所有待处理的变化
	_CollectPendingChanges();
	
	// 找到最适合合并的两个目录（公共父目录最深的）
	std::string bestPath1, bestPath2, bestCommonParent;
	int maxCommonDepth = -1;
	
	for (auto it1 = _watchedFolders.begin(); it1 != _watchedFolders.end(); ++it1)
	{
		for (auto it2 = std::next(it1); it2 != _watchedFolders.end(); ++it2)
		{
			std::string commonParent = _FindCommonParent(it1->second->path, it2->second->path);
			if (!commonParent.empty())
			{
				std::vector<std::string> parts;
				SplitStringBy("\\", commonParent, &parts);
				int depth = static_cast<int>(parts.size());
				
				if (depth > maxCommonDepth)
				{
					maxCommonDepth = depth;
					bestPath1 = it1->first;
					bestPath2 = it2->first;
					bestCommonParent = commonParent;
				}
			}
		}
	}
	
	if (maxCommonDepth > 0)
	{
		// 收集所有被新父目录包含的监控目录
		std::vector<std::string> foldersToRemove;
		foldersToRemove.push_back(bestPath1);
		foldersToRemove.push_back(bestPath2);
		
		for (auto& pair : _watchedFolders)
		{
			// 跳过已经在删除列表中的
			if (pair.first == bestPath1 || pair.first == bestPath2)
				continue;
			
			// 检查是否被新父目录包含
			if (CheckPathContaining(bestCommonParent.c_str(), pair.second->path.c_str()))
			{
				foldersToRemove.push_back(pair.first);
			}
		}
		
		// 合并所有被删除目录的待处理变化
		for (const std::string& pathToRemove : foldersToRemove)
		{
			WatchedFolder* pFolder = _watchedFolders[pathToRemove];
			for (int i = 0; i < pFolder->pendingChanges.GetCount(); ++i)
			{
				_allChanges.Add(pFolder->pendingChanges[i]);
			}
		}
		
		// 删除所有被包含的监控目录
		for (const std::string& pathToRemove : foldersToRemove)
		{
			WatchedFolder* pFolder = _watchedFolders[pathToRemove];
			_watchedFolders.erase(pathToRemove);
			delete pFolder;
		}
		
		// 添加新的父目录
		_AddWatchFolder(bestCommonParent);
	}
}

void CFoldersWatcher::_CollectPendingChanges()
{
	if (!_bStarted)
		return;
	
	for (auto& pair : _watchedFolders)
	{
		WatchedFolder* pFolder = pair.second;
		if (pFolder->pWatcher && pFolder->pWatcher->IsStarted())
		{
			const ChangedFileInformation* files = nullptr;
			int count = pFolder->pWatcher->FetchChangedFiles(files);
			
			for (int i = 0; i < count; ++i)
			{
				ChangedFileInformation2 cfi2;
				cfi2.action = files[i].action;
				
				// 构建完整路径
				std::string fullPath = pFolder->path + "\\" + files[i].name;
				cfi2.name = fullPath;
				
				pFolder->pendingChanges.Add(cfi2);
			}
		}
	}
}

void CFoldersWatcher::SetSuffixFilter(const std::vector<std::string>& suffixes)
{
	_suffixFilters = suffixes;
	
	// 如果已经启动，更新所有监控器的过滤器
	if (_bStarted)
	{
		// 使用共享锁（读锁）保护对 _watchedFolders 的访问
		std::shared_lock<std::shared_mutex> lock(_foldersMutex);
		
		for (auto& pair : _watchedFolders)
		{
			WatchedFolder* pFolder = pair.second;
			if (pFolder->pWatcher)
			{
				pFolder->pWatcher->SetSuffixFilter(suffixes);
			}
		}
	}
}

void CFoldersWatcher::ClearSuffixFilter()
{
	_suffixFilters.clear();
	
	// 如果已经启动，清除所有监控器的过滤器
	if (_bStarted)
	{
		// 使用共享锁（读锁）保护对 _watchedFolders 的访问
		std::shared_lock<std::shared_mutex> lock(_foldersMutex);
		
		for (auto& pair : _watchedFolders)
		{
			WatchedFolder* pFolder = pair.second;
			if (pFolder->pWatcher)
			{
				pFolder->pWatcher->ClearSuffixFilter();
			}
		}
	}
}

void CFoldersWatcher::AddFilePath(const char* path)
{
	if (!path || !*path)
		return;

	std::unique_lock<std::shared_mutex> lock(_foldersMutex);

	// 检查是否已经被某个监控目录包含
	WatchedFolder* pContaining = _FindContainingFolder(path);
	if (pContaining)
	{
		// 已经被监控，无需添加
		return;
	}
	
	// 提取文件所在目录
	std::string folderPath = _ExtractFolderPath(path);
	if (folderPath.empty())
		return;
	
	// 尝试添加新的监控目录
	if (_AddWatchFolder(folderPath))
	{
		// 检查是否超过最大限制
		if (_watchedFolders.size() > _maxWatchFolder)
		{
			_MergeFolders();
		}
	}
}

int CFoldersWatcher::FetchChangedFiles(const ChangedFileInformation*& files)
{
	thread_local static std::vector<ChangedFileInformation> sChangedFileVector;

	// 使用共享锁（读锁）保护对 _watchedFolders 的访问
	std::shared_lock<std::shared_mutex> lock(_foldersMutex);
	
	// 清空之前的结果
	sChangedFileVector.clear();
	_allChanges.Clear();
	
	if (!_bStarted)
	{
		files = nullptr;
		return 0;
	}
	
	// 收集所有监控器的变化
	_CollectPendingChanges();
	
	
	// 将待处理的变化添加到总列表
	for (auto& pair : _watchedFolders)
	{
		WatchedFolder* pFolder = pair.second;
		for (int i = 0; i < pFolder->pendingChanges.GetCount(); ++i)
		{
			_allChanges.Add(pFolder->pendingChanges[i]);
		}
		pFolder->pendingChanges.Clear();
	}
	
	// 转换为返回格式
	int count = _allChanges.GetCount();
	for (int i = 0; i < count; ++i)
	{
		ChangedFileInformation cfi;
		cfi.action = _allChanges[i].action;
		cfi.name = _allChanges[i].name.c_str();
		sChangedFileVector.push_back(cfi);
	}
	
	if (count > 0)
	{
		files = sChangedFileVector.data();
	}
	else
	{
		files = nullptr;
	}
	
	return count;
}

void CFoldersWatcher::GetFolderPathes(std::vector<std::string>& pathes)
{
	// 使用共享锁（读锁）保护对 _watchedFolders 的访问
	std::shared_lock<std::shared_mutex> lock(_foldersMutex);
	
	pathes.clear();
	pathes.reserve(_watchedFolders.size());
	
	for (const auto& pair : _watchedFolders)
	{
		pathes.push_back(pair.second->path);
	}
}
