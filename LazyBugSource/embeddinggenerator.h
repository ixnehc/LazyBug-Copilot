#pragma once

#include <vector>
#include <string>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "coredefines.h"
#include "embeddingtypes.h"

// ============================================================================
// EmbedRequest — 提交给线程池的构建请求（一个文件一次请求）
// ============================================================================
struct EmbedRequest
{
	FilePathKey                        key;              // 文件标识
	std::string                        filePath;         // 实际文件系统路径(用于读文件内容)
	std::vector<SymbolRangeInfo>        symbolRanges;     // 从 SymbolDB 获取的 symbol 类型及行范围
	time_t                             symbolParseTime;  // 获取 symbolRanges 时 SymbolDB 中该文件的 _parsedTime
	std::vector<CEmbeddingChunk>       oldChunks;        // 旧分片（用于 diff,找出变化 chunk）
	uint64_t                           requestId;
};

// ============================================================================
// EmbedResult — 线程池返回的构建结果
// ============================================================================
struct EmbedResult
{
	FilePathKey                   key;
	std::vector<CEmbeddingChunk>  chunks;           // 新分片（含已生成的 embedding）
	time_t                        symbolParseTime;  // 生成时使用的 symbolParseTime，回写时作为 _genTime
	std::string                   modelName;        // 生成时使用的模型名
	uint64_t                      requestId;
	bool                          success;
};

// ============================================================================
// CEmbeddingGenerator — 生成 embedding 的线程池
// ============================================================================
class CEmbeddingGenerator
{
public:
	CEmbeddingGenerator();
	~CEmbeddingGenerator();

	// 初始化线程池
	// modelParam: 生成 embedding 使用的模型 API 参数
	void Init(const EmbedModelParam& modelParam,
	          int numThreads = 4, ThreadPriority priority = ThreadPriority::LOWEST);
	// 运行时更新模型参数（线程安全）
	void SetModelParam(const EmbedModelParam& modelParam);
	// 重新激活 generator（仅翻转 _enable 并唤醒 worker, 不更新 _modelParam）
	void ReEnable();
	// 关闭线程池
	void Close();

	// 提交生成请求（非阻塞）
	bool Request(EmbedRequest& request);
	// 获取生成结果,有结果则返回 true
	bool FetchResult(EmbedResult& result);

	bool IsFlushed() const;
	int  GetActiveCount() const;
	bool IsEnabled() const;

private:
	// 工作线程函数
	void _WorkerThread();

	// 处理一个请求（由工作线程调用）
	EmbedResult _ProcessRequest(const EmbedRequest& request);

	// 调用 LLM API 生成 embedding
	// modelParam: 调用方在外部加锁拷贝后传入, 确保与 result.modelName 一致
	// texts: 需要生成 embedding 的文本列表
	// outEmbeddings: 返回的 embedding 向量列表（与 texts 一一对应）
	bool _CallEmbeddingApi(const EmbedModelParam& modelParam,
	                       const std::vector<std::string>& texts,
	                       std::vector<std::vector<float>>& outEmbeddings);

	// curl 进度回调: 当 _enable == false 时返回非零, 中止正在进行的 curl 传输
	static int _CurlProgressCb(void* userp, double, double, double, double);

	// 计算内容 hash
	static uint64_t _ComputeHash(const std::string& content);

	// 单次请求内立即重试的最大次数
	static constexpr int MAX_RETRIES = 3;

private:
	bool                                        _running;
	std::vector<std::thread>                    _threads;
	ThreadPriority                              _threadPriority;

	std::mutex                                  _requestMutex;
	std::condition_variable                     _requestCV;
	std::deque<EmbedRequest>                    _requestQueue;

	std::mutex                                  _resultMutex;
	std::deque<EmbedResult>                     _resultQueue;

	std::atomic<int>                            _activeCount;
	std::atomic<uint64_t>                       _nextRequestId;

	std::atomic<bool>                           _enable;        // true=激活, false=失活(模型/API不可用)

	EmbedModelParam                             _modelParam;   // 使用中的模型参数
	mutable std::mutex                          _modelParamMutex; // 保护 _modelParam
};

