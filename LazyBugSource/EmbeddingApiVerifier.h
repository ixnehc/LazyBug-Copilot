#pragma once

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "embeddingtypes.h"
#include "CoreDefines.h"

// ============================================================================
// CEmbeddingApiVerifier — 周期性检测 embedding API 是否可用
// 内部启动一个工作线程，定期调用 CallEmbeddingApi 做一次探测。
// ============================================================================
class CEmbeddingApiVerifier
{
public:
	CEmbeddingApiVerifier();
	~CEmbeddingApiVerifier();

	// 启动验证线程
	void Start();

	// 停止验证线程（唤醒并 join）
	void Stop();

	// 线程安全读取最近一次 embedding 请求状态（success + time）
	EmbeddingRequestStatus GetLastRequestStatus() const { return _lastRequestStatus.load(std::memory_order_relaxed); }

private:
	// 工作线程主循环
	void _WorkerThread();

	// 从 g_llmLib 构建 EmbedModelParam，返回是否构建成功
	bool _BuildModelParam(EmbedModelParam& outParam);

	// 调用 CallEmbeddingApi 做一次探测
	bool _VerifyOnce(const EmbedModelParam& param);

private:
	std::thread                 _thread;
	std::mutex                  _mutex;
	std::condition_variable     _cv;
	std::atomic<bool>           _running{false};
	std::atomic<EmbeddingRequestStatus> _lastRequestStatus{EmbeddingRequestStatus{true, 0}};

	// 验证间隔（秒）
	static constexpr int        VERIFY_INTERVAL_OK   = 5;   // 上次成功时的间隔
	static constexpr int        VERIFY_INTERVAL_FAIL = 5;   // 上次失败时的间隔
	static constexpr int        VERIFY_TIMEOUT       = 30;   // 探测请求超时秒数
};
