#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class CChatTask_CLI : public CChatTask
{
public:
	CChatTask_CLI(const std::string& shellType = "cmd.exe");
	~CChatTask_CLI();

	const char* GetType() override { return "CLI"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return false; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();
	void _ThreadFunc();
	void _StartExecution();  // 启动实际执行（工作线程）

	// === 公用辅助方法 ===
	
	// 设置线程结果（线程安全）
	void _SetThreadResult(const std::string& result, const std::string& message, bool success);
	
	// 获取线程结果（线程安全）
	std::string _GetThreadResult();
	
	// 终止进程并清理句柄
	void _TerminateProcessAndCleanupHandles();
	
	// 清理工作线程
	void _CleanupWorkerThread();
	
	// 完成 CLI 显示并清理状态
	void _CompleteCliAndCleanup(int exitCode);
	
	// 追加输出到 CLI 显示区域
	void _AppendOutputToDisplay(const std::string& output);

	std::wstring _cliId;  // CLI 显示元素的 ID
	bool _isPending;      // 是否为 pending 状态（等待用户点击播放按钮）
	bool _executionStarted;  // 是否已经启动执行
	std::string _shellType;  // shell 类型（cmd.exe, bash.exe, python.exe），初始化时由外部传入

	std::thread* _workerThread;
	std::atomic<bool> _shouldStop;
	std::atomic<bool> _threadFinished;
	std::atomic<bool> _processStarted;  // 进程已启动，需要显示输入框
	std::mutex _resultMutex;

	std::string _threadResult;
	std::string _threadMessage;
	bool _threadSuccess;
	
	// 实时输出相关
	std::vector<std::string> _outputChunks;  // 增量输出队列
	std::mutex _outputMutex;  // 保护输出队列的互斥锁
	
	// 输入框显示控制
	std::atomic<__int64> _lastOutputTime;  // 上次收到输出的时间戳（毫秒）
	std::atomic<bool> _inputAreaShown;  // 输入框是否已显示
	
	// 进程句柄，用于中断
	HANDLE _processHandle;
	HANDLE _hWriteInput;  // 用于向子进程发送输入的管道写入端
	
	// 输入等待标志
	std::atomic<bool> _waitingForInput;  // 标记是否正在等待用户输入
};
