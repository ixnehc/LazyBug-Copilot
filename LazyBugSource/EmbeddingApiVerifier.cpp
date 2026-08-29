#include "stdh.h"
#include "embeddingapiverifier.h"
#include "utils_embedding.h"
#include "llmlib.h"

// ============================================================================
// 构造 / 析构
// ============================================================================

CEmbeddingApiVerifier::CEmbeddingApiVerifier()
{
}

CEmbeddingApiVerifier::~CEmbeddingApiVerifier()
{
	Stop();
}

// ============================================================================
// 公共接口
// ============================================================================

void CEmbeddingApiVerifier::Start()
{
	if (_running.load())
		return;

	_running.store(true);
	_available.store(false);

	_thread = std::thread(&CEmbeddingApiVerifier::_WorkerThread, this);
}

void CEmbeddingApiVerifier::Stop()
{
	_running.store(false);
	_cv.notify_all();
	if (_thread.joinable())
		_thread.join();
}

void CEmbeddingApiVerifier::SetStatusChangedCallback(StatusChangedCallback callback)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_callback = std::move(callback);
}

// ============================================================================
// 工作线程
// ============================================================================

void CEmbeddingApiVerifier::_WorkerThread()
{
	while (_running.load())
	{
		bool available = false;

		EmbedModelParam param;
		if (_BuildModelParam(param))
			available = _VerifyOnce(param);

		// 状态变化时触发回调
		if (available != _available.exchange(available))
		{
			StatusChangedCallback cb;
			{
				std::lock_guard<std::mutex> lock(_mutex);
				cb = _callback;
			}
			if (cb)
				cb(available);
		}

		// 等待下一轮（成功 60s，失败 30s）
		int waitSec = available ? VERIFY_INTERVAL_OK : VERIFY_INTERVAL_FAIL;
		std::unique_lock<std::mutex> lock(_mutex);
		_cv.wait_for(lock, std::chrono::seconds(waitSec),
			[this] { return !_running.load(); });
	}
}

// ============================================================================
// 从 g_llmLib 构建 EmbedModelParam
// ============================================================================

bool CEmbeddingApiVerifier::_BuildModelParam(EmbedModelParam& outParam)
{
	std::string embeddingApiName = g_llmLib.GetEmbeddingApi();
	if (embeddingApiName.empty() || embeddingApiName == EMBEDDING_API_DISABLE)
		return false;

	const LlmApi* api = g_llmLib.GetApi(embeddingApiName);
	if (!api)
		return false;

	const LlmApiProvider* provider = g_llmLib.GetProvider(api->providerTypeName);
	if (!provider)
		return false;

	outParam._modelName = api->model;
	outParam._endpoint = provider->endpoint;
	outParam._apiKey = provider->key;
	outParam._timeoutSeconds = VERIFY_TIMEOUT;

	return outParam.IsValid();
}

// ============================================================================
// 调用 CallEmbeddingApi 做一次探测
// ============================================================================

bool CEmbeddingApiVerifier::_VerifyOnce(const EmbedModelParam& param)
{
	std::vector<std::string> texts = { "test" };
	std::vector<std::vector<float>> outEmbeddings;

	return Utils::CallEmbeddingApi(param, texts, outEmbeddings, &_running);
}
