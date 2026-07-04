#include "stdh.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cctype>
#include <mutex>

#include "Utils.h"
#include "CppSymbol.h"
#include "SolutionDB.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <sys/stat.h>
#include <string>

// Windows 进程相关头文件
#include <windows.h>
#include <process.h>
#include <io.h>


CppSymbol_Begin


//////////////////////////////////////////////////////////////////////////
//FileLocation

void SaveFileLocation(const FileLocation& loc, CDataPacket& dp)
{
	// 保存文件路径索引
	dp.Data_WriteSimple(loc.filePath);

	// 保存行列信息
	dp.Data_WriteSimpleR(loc.lineLoc);
}

void LoadFileLocation(FileLocation& loc, CDataPacket& dp, CStringPool& strPool)
{
	// 加载文件路径索引
	dp.Data_ReadSimple(loc.filePath);
	loc.strFilePath = strPool.GetTempStr(loc.filePath);

	// 加载行列信息
	dp.Data_ReadSimple(loc.lineLoc);
}


CSymbolParser::CSymbolParser()
	: _running(false),
	  _threadPriority(ThreadPriority::NORMAL),
	  _discardId(ParseRequestId_Invalid)       // 初始丢弃序号为0，表示不丢弃任何请求
{
}

CSymbolParser::~CSymbolParser()
{
	Close();
}

bool CSymbolParser::SetThreadPriority(ThreadPriority priority)
{
	int nativePriority;
	
	switch (priority)
	{
	case ThreadPriority::LOWEST:
		nativePriority = THREAD_PRIORITY_LOWEST;
		break;
	case ThreadPriority::BELOW_NORMAL:
		nativePriority = THREAD_PRIORITY_BELOW_NORMAL;
		break;
	case ThreadPriority::NORMAL:
		nativePriority = THREAD_PRIORITY_NORMAL;
		break;
	case ThreadPriority::ABOVE_NORMAL:
		nativePriority = THREAD_PRIORITY_ABOVE_NORMAL;
		break;
	case ThreadPriority::HIGHEST:
		nativePriority = THREAD_PRIORITY_HIGHEST;
		break;
	case ThreadPriority::TIME_CRITICAL:
		nativePriority = THREAD_PRIORITY_TIME_CRITICAL;
		break;
	default:
		nativePriority = THREAD_PRIORITY_NORMAL;
		break;
	}
	
	// 使用完整的命名空间避免与类成员函数名冲突
	return ::SetThreadPriority(GetCurrentThread(), nativePriority) != 0;
}

void CSymbolParser::Init(int numThreads, ThreadPriority priority)
{
	if (_running)
	{
		return;
	}

	_running = true;
	_threadPriority = priority;

	_activeCount = 0;

	// 创建工作线程
	for (int i = 0; i < numThreads; ++i)
	{
		_threads.push_back(std::thread(&CSymbolParser::WorkerThread, this));
	}
}

void CSymbolParser::Close()
{
	if (!_running)
	{
		return;
	}

	// 停止所有线程
	_running = false;
	_requestCV.notify_all();

	// 等待所有线程结束
	for (std::thread& thread : _threads)
	{
		if (thread.joinable())
		{
			thread.join();
		}
	}

	_threads.clear();

	// 清空队列
	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_requestQueue.clear();
	}
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_resultQueue.clear();
	}
}

bool CSymbolParser::Request(ParseRequest& request)
{
	if (!_running)
	{
		return false;
	}

	_activeCount++;
	{
		std::lock_guard<std::mutex> lockRequest(_requestMutex);
		std::lock_guard<std::mutex> lockResult(_resultMutex);

		// 如果都不在，添加新请求
		_requestQueue.resize(_requestQueue.size() + 1);
		_requestQueue[_requestQueue.size() - 1].MoveFrom(request);
	}

	_requestCV.notify_one();
	return true;
}

bool CSymbolParser::FetchResult(ParseResult& result)
{
	if (true)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);

		// 如果结果队列为空，返回false
		if (_resultQueue.empty())
		{
			return false;
		}

		result = std::move(_resultQueue.front());
		_resultQueue.pop_front();
		result.discarded = _discardId > result.requestId;
	}

	if (_activeCount>0)
		_activeCount--;
	return true;
}

void CSymbolParser::WorkerThread()
{
	// 设置当前线程的优先级
	SetThreadPriority(_threadPriority);
	
	while (_running)
	{
		ParseRequest request;

		// 获取请求
		{
			std::unique_lock<std::mutex> lock(_requestMutex);
			_requestCV.wait(lock, [this] {
				return !_running || !_requestQueue.empty();
			});

			if (!_running)
			{
				break;
			}

			request = std::move(_requestQueue.front());
			_requestQueue.pop_front();
		}

		// 处理请求
	ParseResult result;
#ifdef ENABLE_PARSER_DEBUG
	result.debugBaseTime = request.debugStartTime;
#endif
		ProcessRequestWithSubprocess(request, result);
		
		// 设置结果的请求序号和丢弃标志
		result.requestId = request.requestId;
		result.discarded = false;

		// 保存结果
		{
			std::lock_guard<std::mutex> lock(_resultMutex);
			_resultQueue.push_back(std::move(result));
		}
	}
}

void CSymbolParser::DiscardAll(ParseRequestId requestId)
{
	// 设置新的丢弃序号为当前最大请求序号 - 简化为直接访问
	_discardId = requestId;
	
	// 清除请求队列中尚未开始处理的请求
	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_activeCount -= _requestQueue.size();
		if (_activeCount < 0)
			_activeCount = 0;
		_requestQueue.clear();
	}
}

//////////////////////////////////////////////////////////////////////////
//使用子进程进行解析的函数

bool CSymbolParser::CreateRequestDataTempFile(BYTE *data,DWORD szData,std::string &tempFilePath)
{

	// 创建临时文件用于传递请求数据
	char tempPath[MAX_PATH];
	char tempFile[MAX_PATH];

	if (GetTempPathA(MAX_PATH, tempPath) == 0)
		return false;

	if (GetTempFileNameA(tempPath, "LBP", 0, tempFile) == 0)
		return false;

	// 将请求数据序列化到临时文件
	{
		HANDLE hFile = CreateFileA(tempFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
		if (hFile == INVALID_HANDLE_VALUE)
		{
			DeleteFileA(tempFile);
			return false;
		}

		DWORD bytesWritten;
		if (!WriteFile(hFile, data, szData, &bytesWritten, NULL) || bytesWritten != szData)
		{
			CloseHandle(hFile);
			DeleteFileA(tempFile);
			return false;
		}

		CloseHandle(hFile);
	}

	tempFilePath = tempFile;

	return true;
}


void CSymbolParser::ProcessRequestWithSubprocess(const ParseRequest& request, ParseResult& result)
{
	result.parseFilePath = request.lowerCasedParseFilePath;
	result.success = false;
	result.requestId = request.requestId;

	result.AddDebugTime("B-A");

	// 使用256k栈数组避免动态内存分配
	const int BUFFER_SIZE = 512*1024;  
	char workBuffer[BUFFER_SIZE];

	// 计算序列化后的数据大小（不实际写入）
	int requestDataSize;
	{
		CDataPacket dp;
		request.Save(dp);
		requestDataSize = dp.GetDataSize();
	}
	
	// 检查大小限制：命令行前缀 + 十六进制字符串 + 后缀
	const char* cmdPrefix = LAZYBUG_CPP_PARSER_EXENAME" \"d";
	int prefixLen = strlen(cmdPrefix);
	int hexSize = requestDataSize * 2;
	int totalCommandSize = prefixLen + hexSize + 2;  // +2 for "\"\0"

	const int maxCommandLineSize = 30000;
	char* commandLine = nullptr;
	std::string tempFilePath;
	if (totalCommandSize > maxCommandLineSize)
	{
		CDataPacket dp;
		dp.SetDataBufferPointer((BYTE*)workBuffer);
		request.Save(dp);

		if (!CreateRequestDataTempFile((BYTE*)workBuffer, dp.GetDataSize(), tempFilePath))
			return;

		const char* cmdPrefixF = LAZYBUG_CPP_PARSER_EXENAME" \"f";
		strcpy(workBuffer, cmdPrefixF);
		int prefixLen = strlen(cmdPrefixF);
		int pathLength = tempFilePath.length();
		strcpy(workBuffer + prefixLen, tempFilePath.c_str());
		commandLine = workBuffer;
		commandLine[prefixLen + pathLength] = '"';
		commandLine[prefixLen + pathLength + 1] = '\0';
	}
	else
	{
		// 在buffer开头写入命令行前缀
		strcpy(workBuffer, cmdPrefix);

		// 在"LazyBugCppParser.exe \"d"后面保存二进制数据
		char* binaryStart = workBuffer + prefixLen;
		{
			CDataPacket dp;
			dp.SetDataBufferPointer((BYTE*)binaryStart);
			request.Save(dp);
		}

		// 从后往前把二进制转换为十六进制字符串
		// 从二进制数据的末尾开始，向后写入hex字符，这样hex字符串会直接跟在前缀后面
		char* binaryEnd = binaryStart + requestDataSize - 1;  // 指向最后一个字节
		char* hexEnd = binaryStart + hexSize - 1;            // 指向hex字符串的最后位置

		// 从后往前转换
		for (int i = requestDataSize - 1; i >= 0; i--)
		{
			unsigned char byte = (unsigned char)binaryStart[i];
			hexEnd[0] = "0123456789ABCDEF"[byte & 0xF];     // 低4位
			hexEnd[-1] = "0123456789ABCDEF"[byte >> 4];       // 高4位
			hexEnd -= 2;
		}

		// 添加结束引号和字符串结束符
		commandLine = workBuffer;
		commandLine[prefixLen + hexSize] = '"';
		commandLine[prefixLen + hexSize + 1] = '\0';
	}

	result.AddDebugTime("B-B");  
	 
	// 创建进程
	STARTUPINFOA si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// 创建管道接收标准输出
	HANDLE hStdoutRead, hStdoutWrite;
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;

	if (!CreatePipe(&hStdoutRead, &hStdoutWrite, &sa, 0))
	{
		return;
	}

	// 确保读取端不被继承
	SetHandleInformation(hStdoutRead, HANDLE_FLAG_INHERIT, 0);

	si.hStdOutput = hStdoutWrite;
	si.hStdError = hStdoutWrite;

	// 启动进程
	if (!CreateProcessA(
		NULL,           // 应用程序名称
		commandLine,    // 命令行
		NULL,           // 进程安全属性
		NULL,           // 线程安全属性
		TRUE,           // 继承句柄
		CREATE_NO_WINDOW | DETACHED_PROCESS | IDLE_PRIORITY_CLASS,  // 创建标志：不创建窗口、独立进程且低优先级
		NULL,           // 环境
		NULL,           // 当前目录
		&si,            // 启动信息
		&pi))           // 进程信息
	{
		CloseHandle(hStdoutRead);
		CloseHandle(hStdoutWrite);
		return;
	}

	result.AddDebugTime("B-C");

	// 关闭写入端，以便子进程能够检测到EOF
	CloseHandle(hStdoutWrite);

	// 读取子进程的输出 - 先尝试使用栈缓冲区，如果过大则切换到动态内存
	char* outputStart = workBuffer;  // 初始使用栈缓冲区
	int outputSize = 0;
	char readBuffer[4096];
	DWORD bytesRead;
	std::vector<char> dynamicBuffer;  // 动态缓冲区，仅在需要时使用
	bool useDynamicBuffer = false;
	
	while (ReadFile(hStdoutRead, readBuffer, sizeof(readBuffer), &bytesRead, NULL) && bytesRead > 0)
	{
		if (!useDynamicBuffer && outputSize + bytesRead > BUFFER_SIZE)
		{
			// 栈缓冲区不够大，切换到动态内存
			useDynamicBuffer = true;
			dynamicBuffer.reserve(outputSize + bytesRead + 4096);  // 预留一些额外空间
			dynamicBuffer.resize(outputSize);
			memcpy(dynamicBuffer.data(), workBuffer, outputSize);
			outputStart = dynamicBuffer.data();
		}
		
		if (useDynamicBuffer)
		{
			// 使用动态缓冲区
			dynamicBuffer.resize(outputSize + bytesRead);
			memcpy(dynamicBuffer.data() + outputSize, readBuffer, bytesRead);
			outputStart = dynamicBuffer.data();
		}
		else
		{
			// 使用栈缓冲区
			memcpy(outputStart + outputSize, readBuffer, bytesRead);
		}
		
		outputSize += bytesRead;
	}

	// 等待进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 直接使用读取到的二进制数据进行反序列化
	CDataPacket dp;
	dp.SetDataBufferPointer((BYTE*)outputStart);
	result.Load(dp, true);

	result.AddDebugTime("B-D");

	// 删除临时文件
	if (!tempFilePath.empty())
		DeleteFileA(tempFilePath.c_str());

	// 获取退出代码
	DWORD exitCode;
	GetExitCodeProcess(pi.hProcess, &exitCode);

	// 清理句柄
	CloseHandle(hStdoutRead);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	result.AddDebugTime("B-E");

#ifdef ENABLE_PARSER_DEBUG
	result.debugBaseTime = request.debugStartTime;
#endif

	// 检查是否成功
	if (exitCode != 0 || outputSize == 0)
	{
		return;
	}

}

/*
使用示例：

// 要使用子进程解析功能，只需要调用 ProcessRequestWithSubprocess 而不是 ProcessRequest：

ParseRequest request;
request.parseFilePath = "path/to/your/file.cpp";
request.setting = yourProjSetting;
request.requestId = someId;

ParseResult result;

// 使用子进程方式（新功能）
CSymbolParser::ProcessRequestWithSubprocess(request, result);

// 或者使用原来的方式（保留）
// CSymbolParser::ProcessRequest(request, result);

if (result.success) {
    // 处理解析结果
    for (const auto& filePair : result.definesByFile) {
        const std::string& filePath = filePair.first;
        const std::vector<RawSymbolDefine>& defines = filePair.second;
        // 处理符号定义...
    }
}

注意：
1. 确保 Proj_LazyBugCppParser.exe 在PATH中或在当前目录下
2. 子进程方式提供了进程隔离，避免了主进程崩溃的风险
3. 可以通过配置文件或环境变量来选择使用哪种方式
*/

CppSymbol_End

