#include "stdh.h"

#include "SymbolRefsCache.h"

#include "SmartRepairRefCollector.h"

#include "Utils.h"
#include "SolutionDBApi.h"
#include "stringparser/stringparser.h"
#include <algorithm>

CSmartRepairRefCollector::CSmartRepairRefCollector()
{
}

CSmartRepairRefCollector::~CSmartRepairRefCollector()
{
	Clear();
}

void CSmartRepairRefCollector::Clear()
{
	// 取消当前任务
	if (_currentThreadInfo)
	{
		_currentThreadInfo->cancelFlag = true;
	}

	// 等待当前线程结束
	if (_currentThreadInfo && _currentThreadInfo->thread.joinable())
	{
		_currentThreadInfo->thread.join();
	}

	// 取消所有丢弃的线程
	for (auto& threadInfo : _discardedThreads)
	{
		threadInfo->cancelFlag = true;
	}

	// 等待所有丢弃的线程结束
	for (auto& threadInfo : _discardedThreads)
	{
		if (threadInfo->thread.joinable())
		{
			threadInfo->thread.join();
		}
	}
}

void CSmartRepairRefCollector::Interrupt()
{
	// 如果有当前线程，取消它并移到丢弃队列
	if (_currentThreadInfo)
	{
		// 设置取消标志
		_currentThreadInfo->cancelFlag = true;

		// 移到丢弃队列
		_discardedThreads.push_back(std::move(_currentThreadInfo));
		_currentThreadInfo.reset();
	}

	// 清理已完成的丢弃线程
	_CleanupObsoleteThreads();
}

void CSmartRepairRefCollector::RequestCollect(Request& req)
{
	Interrupt();
	
	// 创建新的线程信息
	_currentThreadInfo = std::make_unique<ThreadInfo>();
	
	// 启动新的收集线程
	_currentThreadInfo->thread = std::thread(
		&CSmartRepairRefCollector::_CollectWorker, this, std::move(req), _currentThreadInfo.get()
	);
}

bool CSmartRepairRefCollector::FetchLatestResult(Result& outResult)
{
	std::lock_guard<std::mutex> lock(_mutex);
	outResult = std::move(_latestResult);
	return outResult.IsValid();
}

void CSmartRepairRefCollector::Update()
{
	_CleanupObsoleteThreads();
}

//转换为字符串
static bool MakeSnippetsString(CSymbolRefsCache::RefSnippets &snippets, std::string &resultSnippetsStr, std::atomic<bool>& cancelFlag)
{
	resultSnippetsStr.clear();

	std::string snippetsStr;

	// 遍历所有有行范围的文件
	for (const auto& kv : snippets.fileRanges)
	{
		if (cancelFlag) return false;

		WORD fileIndex = kv.first;
		if (fileIndex >= snippets.files.size())
			continue; // 不合法

		const auto& fileInfo = snippets.files[fileIndex];
		const std::string& filePath = fileInfo.path;

		const auto& ranges = kv.second;
		if (ranges.empty())
			continue;


		// 读取文件内容
		std::string fileContent;
		Utils::FileContentCodingFormat codingFmt;
		if (!Utils::GetFileContentIntoUTF8(filePath.c_str(), fileContent,codingFmt))
			continue; // 文件读取失败，跳过

		std::vector<std::string> lines;
		SplitLines(fileContent, lines);

		// 输出文件头
		snippetsStr += "---" + filePath + "---\n";

		// 遍历该文件的所有range，提取代码
		for (const LineRange& range : ranges)
		{
			if (cancelFlag) return false;

			int startIdx = max(0, (int)range.start); // 行号转0基索引
			int endIdx   = min((int)lines.size(), (int)range.end);
			
			for (int i = startIdx; i < endIdx; ++i)
			{
				snippetsStr += lines[i];
				snippetsStr += "\n";
			}
			snippetsStr += "\n...\n\n"; // 各snippet之间加空行
		}

		snippetsStr += "\n";
	}

	resultSnippetsStr = std::move(snippetsStr);
	return true;
}


void CSmartRepairRefCollector::_CollectWorker(Request req, ThreadInfo* threadInfo)
{
	// 检查是否被取消
	if (threadInfo->cancelFlag)
	{
		threadInfo->finishedFlag = true;
		return;
	}

	extern CSymbolRefsCache g_symbolRefsCache;

	std::vector<std::string> lines;
	SplitLines(req.fileContent, lines);

	if (threadInfo->cancelFlag)
	{
		threadInfo->finishedFlag = true;
		return;
	}

	CSymbolRefsCache::RefSnippets snippets;
	g_symbolRefsCache.FindNearbyRefs(req.filePath, lines, req.line, 10, snippets, threadInfo->cancelFlag);

	if (threadInfo->cancelFlag)
	{
		threadInfo->finishedFlag = true;
		return;
	}
		
	std::string snippetsStr;
	MakeSnippetsString(snippets, snippetsStr, threadInfo->cancelFlag);
	if (threadInfo->cancelFlag)
	{
		threadInfo->finishedFlag = true;
		return;
	}

	// 保存结果
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_latestResult.sessionId = req.sessionId;
		_latestResult.snippetsStr = std::move(snippetsStr);
	}
	
	// 标记线程完成
	threadInfo->finishedFlag = true;
}

void CSmartRepairRefCollector::_CleanupObsoleteThreads()
{
	auto it = _discardedThreads.begin();
	while (it != _discardedThreads.end())
	{
		// 检查线程是否已经完成
		if ((*it)->finishedFlag)
		{
			// 线程已完成，可以安全地join并移除
			if ((*it)->thread.joinable())
			{
				(*it)->thread.join();
			}
			it = _discardedThreads.erase(it);
		}
		else
		{
			// 线程还在运行，跳过这次清理
			++it;
		}
	}
}

