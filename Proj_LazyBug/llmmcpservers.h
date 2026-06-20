#pragma once
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>
#include "LlmMcps.h"

class CLlmMcpServers
{
public:
	// server的创建状态
	enum class State
	{
		Starting,  // 正在创建（线程运行中）
		Ready,     // 创建成功，tools已获取
		Failed,    // 创建失败
	};

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

		// ---- 每个server独立的线程与中断控制 ----
		std::thread thread;                       // 该server专属的创建线程
		HANDLE hCancelEvent = nullptr;            // 手动复位事件，置位表示请求中断创建
		std::atomic<State> state{ State::Starting };  // 当前状态
		std::atomic<bool> done{ false };          // 线程是否已执行完毕（供主线程join回收）
		bool pendingRemove = false;               // 已请求停止、等待线程退出后销毁
	};

	// 每帧调用，检测版本变化并同步
	bool UpdateSync();//返回server的状态有没有发生变化

	// 将tools回填到g_llmMcps
	void LoadToolsToMcps();

	// 析构时清理所有server
	~CLlmMcpServers();

private:
	std::unordered_map<WUID, std::unique_ptr<Server>> _servers;  // 按uid索引的所有server
	std::vector<std::unique_ptr<Server>> _removing;  // 已请求停止、等待线程退出后销毁（与新建server同uid冲突时迁出至此）
	int _syncedVer = -1;            // 上次完成同步的版本

	// 目标server信息（diff用）
	struct Target
	{
		WUID mcpUid = 0;
		std::string command;
		std::vector<std::string> args;
		std::unordered_map<std::string, std::string> env;
		FILETIME fileTime = { 0 };
	};

	// 检查是否需要同步
	bool _NeedSync() const;

	// 按uid做增量diff：停止不需要的、新建需要的
	void _Sync();

	// 回收已结束线程的server，返回是否有server状态变化
	bool _ReapFinished();

	// 启动单个server的创建线程
	void _StartServer(std::unique_ptr<Server> server);

	// server线程函数：创建并获取tools
	void _ServerThreadFunc(Server* server);

	// 请求停止某个server（置位cancel事件）
	void _RequestStop(Server& server);

	// 等待线程退出并销毁server（释放进程、管道、事件句柄）
	void _JoinAndDestroy(Server& server);

	// 创建单个server（在server线程中执行，可被hCancel打断）
	bool _CreateServer(Server& server, HANDLE hCancel);

	// 销毁单个server的进程与管道
	void _DestroyServer(Server& server);

	// 读取管道剩余内容（可被hCancel打断）
	std::string _FlushPipeRead(Server& server, HANDLE hCancel);

	// 收集目标servers（enable的Mcp，同名去重）
	void _CollectTargetServers(std::vector<Target>& outTargets);

	// 发送MCP请求并读取响应（可被hCancel打断）
	bool _SendMcpRequest(Server& server, HANDLE hCancel, const std::string& request, std::string& response, int timeoutMs = 5000, bool outputResponse = false);

	// 发送MCP初始化请求（可被hCancel打断）
	bool _SendMcpInitialize(Server& server, HANDLE hCancel);

	// 获取工具列表（可被hCancel打断）
	bool _FetchTools(Server& server, HANDLE hCancel);

};

extern CLlmMcpServers g_llmMcpServers;

