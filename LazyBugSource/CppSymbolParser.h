#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include <clang-c/Index.h>
#include <clang-c/CXString.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXFile.h>

#include "concurrent/lock.h"

#include "ProjSetting.h"

#include "StringPool.h"

#include "CppSymbolDefines.h"

class IFile;  // 保留IFile的前置声明（以防IFileSystem.h未定义）
class CSolutionFiles;
struct SolutionFile;
class CDataPacket;

#define CppSymbol_Begin namespace CppSymbol{
#define CppSymbol_End }


namespace CppSymbol
{

	class CSymbolParser
	{
	public:

		CSymbolParser();
		~CSymbolParser();

		// 初始化解析池,指定工作线程数量和优先级
		void Init(int numThreads = 4, ThreadPriority priority = ThreadPriority::NORMAL);
		// 关闭解析池
		void Close();
		// 提交解析请求
		bool Request(ParseRequest& request);
		// 获取解析结果,如果有结果立即返回true,否则返回false
		bool FetchResult(ParseResult& result);
		// 丢弃所有已完成和未完成的结果
		void DiscardAll(ParseRequestId requestId);

		bool IsFlushed() const		{			return _activeCount <= 0;		}
		int GetActiveCount()		{			return _activeCount;		}

		// 新增：使用子进程进行解析的函数
		static void ProcessRequestWithSubprocess(const ParseRequest& request, ParseResult &result);
		static bool CreateRequestDataTempFile(BYTE* data, DWORD szData, std::string& tempFilePath);

	private:
		// 工作线程函数
		void WorkerThread();
		// 设置当前线程的优先级
		static bool SetThreadPriority(ThreadPriority priority);

	private:
		bool _running;                // 线程池是否在运行
		std::vector<std::thread> _threads;  // 工作线程池
		ThreadPriority _threadPriority; // 线程优先级
		
		std::mutex _requestMutex;     // 请求队列互斥锁
		std::condition_variable _requestCV;  // 请求队列条件变量
		std::deque<ParseRequest> _requestQueue;  // 请求队列
		
		std::mutex _resultMutex;      // 结果队列互斥锁
		std::deque<ParseResult> _resultQueue;    // 结果队列
		
		ParseRequestId _discardId;      // 丢弃序号（主线程访问）

		int _activeCount;
	};

}

