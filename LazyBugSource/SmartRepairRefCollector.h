#pragma once

#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

// 从 SolutionDB 中得到的引用区间
#include "SolutionDBMsgs.h"

#include "SmartRepairDefines.h"

class CSmartRepairRefCollector
{
public:
	// 表示一次收集请求
	struct Request
	{
		Request()
		{
			line = 0;
			sessionId = SmartRepairSessionID_Invalid;
		}
		std::string filePath;     // 文件路径（在硬盘上的路径）
		std::string fileContent;  // 编辑器中的完整内容
		int line;             // 当前行（编辑器中的行号）
		SmartRepairSessionID sessionId;
	};

	// 收集结果
	struct Result
	{
		Result()
		{
			sessionId = SmartRepairSessionID_Invalid;
		}
		void Clear()
		{
			snippetsStr.clear();
			sessionId = SmartRepairSessionID_Invalid;
		}
		bool IsValid()
		{
			return sessionId != SmartRepairSessionID_Invalid;
		}
		const char* GetSnippetsStr()
		{
			if (!IsValid())
				return "";
			return snippetsStr.c_str();
		}
		SmartRepairSessionID sessionId;
		std::string snippetsStr;
	};

public:
	CSmartRepairRefCollector();
	~CSmartRepairRefCollector();

	void Init()	{	}

	void Clear();

	// 发起一次新的收集, 会中断当前正在运行的收集线程
	void RequestCollect(Request& req);

	// 获取最新一次收集到的结果（线程安全, 若有则返回true）
	bool FetchLatestResult(Result& outResult);

	void Interrupt();
	void Update();

private:
	// 线程信息结构
	struct ThreadInfo
	{
		std::thread thread;
		std::atomic<bool> cancelFlag;
		std::atomic<bool> finishedFlag;
		
		ThreadInfo() : cancelFlag(false), finishedFlag(false) {}
		
		// 禁用拷贝构造和拷贝赋值
		ThreadInfo(const ThreadInfo&) = delete;
		ThreadInfo& operator=(const ThreadInfo&) = delete;
		
		// 禁用移动构造和移动赋值（因为std::thread不支持移动后再使用）
		ThreadInfo(ThreadInfo&&) = delete;
		ThreadInfo& operator=(ThreadInfo&&) = delete;
	};

	// 后台收集线程
	void _CollectWorker(Request req, ThreadInfo* threadInfo);

	// 清理丢弃的线程（非阻塞）
	void _CleanupObsoleteThreads();


private:
	std::unique_ptr<ThreadInfo> _currentThreadInfo;		// 当前线程信息
	std::vector<std::unique_ptr<ThreadInfo>> _discardedThreads;	// 丢弃线程队列

	std::mutex _mutex;					// 保护数据
	Result _latestResult;				// 最新结果
};
