#pragma once
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <ctime>
#include <unordered_set>

struct FindInFileResults;
class CSolutionFiles;
struct SolutionFile;


// 前向声明
struct SolutionFile;
class CSolutionFiles;
struct FindInFileResults;

// 任务结构体
struct IndexingTask
{
	enum TaskType { Add, Remove, SetContent, UpdateIfExists };
	TaskType type;
	std::string lowerCasedFilePath;
	time_t mtime;
	std::string content;
	std::shared_ptr<std::vector<SolutionFile>> filesSnapshot;  // 用于SetContent任务
};

// 基类接口
class CSolutionIndexerImplBase
{
public:
	CSolutionIndexerImplBase();
	virtual ~CSolutionIndexerImplBase();

	virtual bool Open(const char* indexPath) = 0;
	virtual void Close() = 0;
	virtual bool Find(const char* key, int maxResult, FindInFileResults& results) = 0;

	void QueueTask(const IndexingTask& task);
	void StartWorkerThread();
	void StopWorkerThread();

	bool HasPendingWork() const
	{
		return _taskProcessing || !_taskQueue.empty();
	}

	int GetPendingOpCount() const { return _pendingOpCount.load(); }

protected:
	void _OnFileProcessed() { _pendingOpCount--; }

	virtual void WorkerLoop();

	// 子类必须实现：检查索引引擎是否已准备好
	virtual bool IsReady() const = 0;

	// 子类必须实现：具体的增删改操作
	virtual void AddDocument(const std::string& filePath, time_t mtime, const std::string& content) = 0;
	virtual void RemoveDocument(const std::string& filePath) = 0;

	// 子类必须实现：检查文件修改时间并添加文档（如果已改变）
	virtual void AddDocumentIfChanged(const std::string& filePath, time_t mtime, const std::string& content) = 0;
	virtual void ProcessSetContent(std::shared_ptr<std::vector<SolutionFile>> filesSnapshot) = 0;
	virtual void ProcessUpdateIfExists(const std::string& lowerCasedFilePath) = 0;

	// 模板方法：统一的任务分发逻辑（子类不需要重写）
	virtual void ProcessTask(const IndexingTask& task);

	// 带缓存的二进制文件检查，内部调用Utils::CheckFileBinary
	// 同时维护文本后缀白名单和二进制后缀缓存，避免重复I/O
	bool _CheckFileBinary(const char* path);

	// 工作线程相关
	std::thread _workerThread;
	std::atomic<bool> _workerRunning;
	std::atomic<bool> _taskProcessing{false};
	std::atomic<int> _pendingOpCount{0};
	std::queue<IndexingTask> _taskQueue;
	mutable std::mutex _queueMutex;
	std::condition_variable _queueCondition;

	// 文件后缀缓存：文本白名单 + 二进制黑名单，避免重复调用CheckFileBinary的I/O开销
	std::unordered_set<std::string> _textExtensionCache;
	std::unordered_set<std::string> _binaryExtensionCache;
	std::mutex _extCacheMutex;
};


class CSolutionIndexer
{
public:
	CSolutionIndexer();
	~CSolutionIndexer();

	bool Open(const char* indexPath);//打开索引库,如果为空则新建一个
	void Close();//关闭索引库

	//将一组文件更新到索引库中,如果某文件已经存在于索引库中,则要比较日期,只更新日期发生变化的文件
	//如果索引库中的某文件不在files中,则从索引库中删除这个文件
	//files可能包含非常多的文件,要使用单独的线程来索引这些文件,SetContent(..)要能马上返回
	void SetContent(const CSolutionFiles& files);

	//处理文件的增删和更新
	void SetDeltaContent(const std::vector<SolutionFile*>& newFiles, const std::vector<SolutionFile*>& updatedFiles, std::vector<std::string>& removedFiles);

	//检查指定文件是否已在索引中，如果在索引中且文件时间不同，则更新索引
	void UpdateIfExists(const char* lowerCasedFilePath);

	bool Find(const char* key, int maxResult, FindInFileResults &results);

	bool IsIndexing() const	{ return _impl->HasPendingWork();	}
	int GetPendingOpCount() const { return _impl->GetPendingOpCount(); }

private:
	std::unique_ptr<CSolutionIndexerImplBase> _impl;
};