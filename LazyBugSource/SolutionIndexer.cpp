#include "stdh.h"

#include "SolutionIndexer.h"
// #include "SolutionIndexerImpl_Xapian.h"
#include "SolutionIndexerImpl_Lucene.h"
#include "Utils.h"
#include "SolutionDB.h"


// ==================== CSolutionIndexerImplBase 实现 ====================

CSolutionIndexerImplBase::CSolutionIndexerImplBase()
	: _workerRunning(false)
{
}

CSolutionIndexerImplBase::~CSolutionIndexerImplBase()
{
	StopWorkerThread();
}

void CSolutionIndexerImplBase::QueueTask(const IndexingTask& task)
{
	{
		std::lock_guard<std::mutex> lock(_queueMutex);
		_taskQueue.push(task);
	}
	_queueCondition.notify_one();
}

void CSolutionIndexerImplBase::StartWorkerThread()
{
	if (!_workerRunning)
	{
		_workerRunning = true;
		_workerThread = std::thread(&CSolutionIndexerImplBase::WorkerLoop, this);
	}
}

void CSolutionIndexerImplBase::StopWorkerThread()
{
	if (_workerRunning)
	{
		{
			std::lock_guard<std::mutex> lock(_queueMutex);
			_workerRunning = false;
		}
		_queueCondition.notify_all();

		if (_workerThread.joinable())
		{
			_workerThread.join();
		}
	}
}

void CSolutionIndexerImplBase::ProcessTask(const IndexingTask& task)
{
	if (!IsReady())
		return;

	try
	{
		if (task.type == IndexingTask::Remove)
		{
			RemoveDocument(task.lowerCasedFilePath);
		}
		else if (task.type == IndexingTask::Add)
		{
			if (Utils::CheckFileBinary(task.lowerCasedFilePath.c_str()))
				return;

			time_t currentMTime = Utils::GetFileTimeT(task.lowerCasedFilePath.c_str());

			Utils::FileContentCodingFormat codingFmt;
			std::string content;
			if (Utils::GetFileContentIntoUTF8(task.lowerCasedFilePath.c_str(), content, codingFmt))
			{
				AddDocumentIfChanged(task.lowerCasedFilePath, currentMTime, content);
			}
		}
		else if (task.type == IndexingTask::SetContent)
		{
			ProcessSetContent(task.filesSnapshot);
		}
		else if (task.type == IndexingTask::UpdateIfExists)
		{
			ProcessUpdateIfExists(task.lowerCasedFilePath);
		}
	}
	catch (...)
	{
		// 兜底确保工作线程不会因异常退出
	}
}

void CSolutionIndexerImplBase::WorkerLoop()
{
	while (_workerRunning)
	{
		IndexingTask task;
		bool hasTask = false;

		{
			std::unique_lock<std::mutex> lock(_queueMutex);
			_queueCondition.wait(lock, [this] { return !_taskQueue.empty() || !_workerRunning; });

			if (!_workerRunning && _taskQueue.empty())
				break;

			if (!_taskQueue.empty())
			{
				task = _taskQueue.front();
				_taskQueue.pop();
				hasTask = true;
			}
		}

		if (hasTask)
		{
			ProcessTask(task);
		}
	}
}


// CSolutionIndexer 实现

CSolutionIndexer::CSolutionIndexer() : 
//	_impl(std::make_unique<CSolutionIndexerImpl_Xapian>())
	_impl(std::make_unique<CSolutionIndexerImpl_Lucene>())
{
}

CSolutionIndexer::~CSolutionIndexer() = default;

bool CSolutionIndexer::Open(const char* indexPath)
{
	return _impl->Open(indexPath);
}

void CSolutionIndexer::Close()
{
	_impl->Close();
}

void CSolutionIndexer::SetContent(const CSolutionFiles& files)
{
	// 快速复制文件列表快照
	auto filesSnapshot = std::make_shared<std::vector<SolutionFile>>();
	{
		CSolutionFiles::ReadLock lock(files._filesMutex);
		filesSnapshot->reserve(files._lowerCasedFiles.size());
		for (const auto& pair : files._lowerCasedFiles)
		{
			filesSnapshot->push_back(pair.second);
		}
	}

	// 启动工作线程
	_impl->StartWorkerThread();

	// 创建SetContent任务
	IndexingTask task;
	task.type = IndexingTask::SetContent;
	task.filesSnapshot = filesSnapshot;
	_impl->QueueTask(task);
}

void CSolutionIndexer::SetDeltaContent(const std::vector<SolutionFile*>& newFiles,
	const std::vector<SolutionFile*>& updatedFiles,
	std::vector<std::string>& removedFiles)
{
	// 处理新增文件
	for (const auto* file : newFiles)
	{
		if (!file)
			continue;

		IndexingTask task;
		task.type = IndexingTask::Add;
		task.lowerCasedFilePath = file->lowerCasedFilePath;
		_impl->QueueTask(task);
	}

	// 处理更新文件
	for (const auto* file : updatedFiles)
	{
		if (!file)
			continue;

		IndexingTask task;
		task.type = IndexingTask::Add;
		task.lowerCasedFilePath = file->lowerCasedFilePath;
		_impl->QueueTask(task);
	}

	// 处理删除文件
	for (const auto& lowerCasedFilePath : removedFiles)
	{
		IndexingTask task;
		task.type = IndexingTask::Remove;
		task.lowerCasedFilePath = lowerCasedFilePath;
		_impl->QueueTask(task);
	}
}

void CSolutionIndexer::UpdateIfExists(const char* lowerCasedFilePath)
{
	if (!lowerCasedFilePath || lowerCasedFilePath[0] == '\0')
		return;

	IndexingTask task;
	task.type = IndexingTask::UpdateIfExists;
	task.lowerCasedFilePath = lowerCasedFilePath;
	_impl->QueueTask(task);
}

bool CSolutionIndexer::Find(const char* key, int maxResult, FindInFileResults& results)
{
	return _impl->Find(key, maxResult, results);
}
