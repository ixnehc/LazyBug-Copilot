#pragma once
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include "LlmMcps.h"

class CLlmMcpServers
{
public:
	struct Server
	{
		WUID mcpUid = 0;  // MCP的唯一标识
		std::string command;
		std::vector<std::string> args;
		std::unordered_map<std::string, std::string> env;  // 自定义环境变量
		FILETIME fileTime = { 0 };  // MCP.json 文件时间

		// 运行时数据
		std::vector<CLlmMcps::Mcp::Tool> tools;  // 从server获取的tools
		bool toolsFetched = false;                // tools是否已获取
		std::string output;                       // 输出信息

		void AppendOutput(const std::string& out) {
			if (!output.empty()) output += '\n';
			output += out;
		}

		// 进程和管道句柄
		HANDLE hProcess = nullptr;
		HANDLE hThread = nullptr;
		HANDLE hPipeRead = nullptr;   // 读取子进程输出
		HANDLE hPipeWrite = nullptr;  // 写入子进程输入
	};

	// 每帧调用，检测版本变化并同步
	bool UpdateSync();//返回server的状态有没有发生变化

	// 将tools回填到g_llmMcps
	void LoadToolsToMcps();

	// 析构时清理所有server
	~CLlmMcpServers();

private:
	std::vector<Server> _servers;
	int _syncedVer = -1;            // 上次完成同步的版本
	int _pendingVer = -1;           // 正在同步的目标版本

	// 异步启动相关
	std::thread _workerThread;
	std::mutex _mutex;
	std::atomic<bool> _workerRunning{false};
	std::vector<Server> _pendingServers;  // 正在创建中的servers
	std::unordered_set<WUID> _pendingRemoveUids;  // 需要移除的server uid集合
	bool _workerDone = false;             // worker是否已完成

	// 检查是否需要同步
	bool _NeedSync() const;

	// 启动后台同步线程
	void _StartWorker();

	// 后台线程函数：创建所有需要的servers
	void _WorkerFunc(int targetVer, const std::vector<std::pair<WUID, FILETIME>>& existingServers);

	// 创建单个server（在worker线程中执行）
	bool _CreateServer(Server& server);

	// 销毁单个server
	void _DestroyServer(Server& server);

	// 读取管道剩余内容
	std::string _FlushPipeRead(Server& server);


	// 收集目标servers（enable的Mcp，同名去重）
	void _CollectTargetServers(std::vector<Server>& outServers);

	// 发送MCP请求并读取响应
	bool _SendMcpRequest(Server& server, const std::string& request, std::string& response, int timeoutMs = 5000, bool outputResponse = false);

	// 发送MCP初始化请求
	bool _SendMcpInitialize(Server& server);

	// 获取工具列表
	bool _FetchTools(Server& server);

};

extern CLlmMcpServers g_llmMcpServers;

