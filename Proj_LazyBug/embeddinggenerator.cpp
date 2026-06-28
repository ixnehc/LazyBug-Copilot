#include "stdh.h"
#include "embeddinggenerator.h"
#include "Utils_File.h"
#include "Utils.h"
#include "stringparser/stringparser.h"

#include <set>
#include <curl/curl.h>

// 用于 curl 写回调的上下文
struct EmbedApiResponse
{
	std::string data;
};

static size_t _EmbedWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t totalSize = size * nmemb;
	EmbedApiResponse* response = static_cast<EmbedApiResponse*>(userp);
	response->data.append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

// ============================================================================
// CEmbeddingGenerator
// ============================================================================

CEmbeddingGenerator::CEmbeddingGenerator()
{
	_running          = false;
	_threadPriority   = ThreadPriority::LOWEST;
	_activeCount      = 0;
	_nextRequestId    = 1;
}

CEmbeddingGenerator::~CEmbeddingGenerator()
{
	Close();
}

void CEmbeddingGenerator::Init(const EmbedModelParam& modelParam,
                               int numThreads, ThreadPriority priority)
{
	if (_running)
		return;

	_modelParam     = modelParam;
	_threadPriority = priority;
	_running         = true;

	for (int i = 0; i < numThreads; i++)
	{
		_threads.emplace_back(&CEmbeddingGenerator::_WorkerThread, this);
	}

	// 设置线程优先级
	for (auto& t : _threads)
	{
		HANDLE h = t.native_handle();
		int winPriority = THREAD_PRIORITY_LOWEST;
		if (priority == ThreadPriority::BELOW_NORMAL)  winPriority = THREAD_PRIORITY_BELOW_NORMAL;
		else if (priority == ThreadPriority::NORMAL)    winPriority = THREAD_PRIORITY_NORMAL;
		else if (priority == ThreadPriority::ABOVE_NORMAL) winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
		else if (priority == ThreadPriority::HIGHEST)   winPriority = THREAD_PRIORITY_HIGHEST;
		SetThreadPriority(h, winPriority);
	}
}

void CEmbeddingGenerator::Close()
{
	if (!_running)
		return;

	_running = false;
	_requestCV.notify_all();

	for (auto& t : _threads)
	{
		if (t.joinable())
			t.join();
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

	_activeCount   = 0;
	_nextRequestId = 1;
	_modelParam = EmbedModelParam();
}

bool CEmbeddingGenerator::Request(EmbedRequest& request)
{
	if (!_running)
		return false;

	request.requestId = _nextRequestId.fetch_add(1);

	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_requestQueue.push_back(std::move(request));
		_activeCount.fetch_add(1);
	}

	_requestCV.notify_one();
	return true;
}

bool CEmbeddingGenerator::FetchResult(EmbedResult& result)
{
	std::lock_guard<std::mutex> lock(_resultMutex);
	if (_resultQueue.empty())
		return false;

	result = std::move(_resultQueue.front());
	_resultQueue.pop_front();
	return true;
}

bool CEmbeddingGenerator::IsFlushed() const
{
	return _activeCount.load() <= 0;
}

int CEmbeddingGenerator::GetActiveCount() const
{
	return _activeCount.load();
}

// ---- 工作线程 ----

void CEmbeddingGenerator::_WorkerThread()
{
	while (_running)
	{
		EmbedRequest request;

		{
			std::unique_lock<std::mutex> lock(_requestMutex);
			_requestCV.wait(lock, [this]
			{
				return !_requestQueue.empty() || !_running;
			});

			if (!_running)
				return;

			request = std::move(_requestQueue.front());
			_requestQueue.pop_front();
		}

		EmbedResult result = _ProcessRequest(request);

		{
			std::lock_guard<std::mutex> lock(_resultMutex);
			_resultQueue.push_back(std::move(result));
		}

		_activeCount.fetch_sub(1);
	}
}

// ---- 请求处理 ----

EmbedResult CEmbeddingGenerator::_ProcessRequest(const EmbedRequest& request)
{
	EmbedResult result;
	result.key       = request.key;
	result.requestId = request.requestId;
	result.success   = false;

	constexpr int MAX_LINES_PER_CHUNK = 100;

	// 1. 读取整个文件并拆分为行
	std::string fileContent;
	Utils::FileContentCodingFormat codingFmt;
	if (!Utils::GetFileContentIntoUTF8(request.filePath.c_str(), fileContent, codingFmt))
		return result;

	std::vector<std::string> lines;
	SplitLines(fileContent, lines);
	int totalLines = (int)lines.size();

	// 2. 校验文件修改时间是否与 symbolParseTime 一致
	time_t currentModifyTime = Utils::GetFileTimeT(request.filePath.c_str());
	if (currentModifyTime == 0 || currentModifyTime != request.symbolParseTime)
		return result;

	// 3. 收集切割点（来自 symbol range 的 start/end，加上文件首尾）
	std::set<int> splitPoints;
	splitPoints.insert(0);
	splitPoints.insert(totalLines);

	for (const auto& info : request.symbolRanges)
	{
		int s = (int)info._lineRange.start;
		int e = (int)info._lineRange.end;
		if (s >= e)
			continue;
		if (s < 0) s = 0;
		if (e > totalLines) e = totalLines;
		splitPoints.insert(s);
		splitPoints.insert(e);
	}

	// 4. 根据切割点生成 segments（原子分段，不可再拆分）
	struct Segment { int start; int end; };
	std::vector<Segment> segments;
	{
		auto it = splitPoints.begin();
		int prev = *it;
		for (++it; it != splitPoints.end(); ++it)
		{
			int cur = *it;
			if (cur > prev)
				segments.push_back({prev, cur});
			prev = cur;
		}
	}

	// 5. 将 segments 打包成 chunks（遵守 MAX_LINES_PER_CHUNK，不拆分 segment）
	std::vector<CEmbeddingChunk> newChunks;
	{
		int chunkStart = -1;
		int chunkEnd   = -1;
		int chunkLines = 0;

		auto flushChunk = [&]()
		{
			if (chunkStart < 0)
				return;
			CEmbeddingChunk chunk;
			chunk._startLine = chunkStart;
			chunk._endLine   = chunkEnd;
			newChunks.push_back(std::move(chunk));
			chunkStart = -1;
			chunkLines = 0;
		};

		for (const auto& seg : segments)
		{
			int segLines = seg.end - seg.start;

			if (chunkStart >= 0 && chunkLines + segLines > MAX_LINES_PER_CHUNK)
				flushChunk();

			if (chunkStart < 0)
			{
				chunkStart = seg.start;
				chunkEnd   = seg.end;
				chunkLines = segLines;
			}
			else
			{
				chunkEnd   = seg.end;
				chunkLines += segLines;
			}
		}
		flushChunk();
	}

	// 6. 为每个 chunk 构建内容文本、计算 hash，并收集待 embedding 的文本
	std::vector<std::string> textsToEmbed;
	textsToEmbed.reserve(newChunks.size());

	for (auto& chunk : newChunks)
	{
		std::string content;
		for (int i = chunk._startLine; i < chunk._endLine; i++)
		{
			if (i > chunk._startLine)
				content += "\n";
			content += lines[i];
		}
		chunk._contentHash = _ComputeHash(content);
		textsToEmbed.push_back(std::move(content));
	}

	// 7. 调用 embedding API
	if (!textsToEmbed.empty())
	{
		std::vector<std::vector<float>> modelEmbeddings;
		if (_CallEmbeddingApi(textsToEmbed, modelEmbeddings))
		{
			for (size_t i = 0; i < newChunks.size() && i < modelEmbeddings.size(); i++)
				newChunks[i]._embeddings = std::move(modelEmbeddings[i]);
		}
	}

	result.chunks  = std::move(newChunks);
	result.symbolParseTime = request.symbolParseTime;
	result.success = true;
	return result;
}

// ---- Embedding API 调用 ----

bool CEmbeddingGenerator::_CallEmbeddingApi(const std::vector<std::string>& texts,
                                            std::vector<std::vector<float>>& outEmbeddings)
{
	outEmbeddings.clear();

	if (texts.empty())
		return true;

	if (!_modelParam.IsValid())
		return false;

	// 构造 embedding endpoint URL
	std::string embedEndpoint = _modelParam._endpoint;
	if (!embedEndpoint.empty() && embedEndpoint.back() == '/')
		embedEndpoint.pop_back();
	embedEndpoint += "/embeddings";

	// 构造请求 JSON
	json requestJson;
	requestJson["model"] = _modelParam._modelName;
	requestJson["input"] = texts;

	std::string requestBody = requestJson.dump();

	// 初始化 CURL
	CURL* curl = curl_easy_init();
	if (!curl)
		return false;

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

	std::string authHeader = "Authorization: Bearer " + _modelParam._apiKey;
	headers = curl_slist_append(headers, authHeader.c_str());

	EmbedApiResponse response;

	curl_easy_setopt(curl, CURLOPT_URL, embedEndpoint.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _EmbedWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	if (_modelParam._timeoutSeconds > 0)
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, _modelParam._timeoutSeconds);

	CURLcode res = curl_easy_perform(curl);

	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (res != CURLE_OK)
		return false;

	// 解析响应 JSON
	try
	{
		auto respJson = json::parse(response.data);

		// OpenAI 格式: {"data": [{"embedding": [...], "index": 0}, ...]}
		if (respJson.contains("data") && respJson["data"].is_array())
		{
			const auto& dataArr = respJson["data"];
			outEmbeddings.resize(dataArr.size());

			for (size_t i = 0; i < dataArr.size(); i++)
			{
				const auto& item = dataArr[i];
				if (item.contains("embedding") && item["embedding"].is_array())
				{
					const auto& emb = item["embedding"];
					outEmbeddings[i].reserve(emb.size());
					for (const auto& val : emb)
						outEmbeddings[i].push_back(val.get<float>());
				}
			}

			return true;
		}
	}
	catch (...) {}

	return false;
}

// ---- Hash ----

uint64_t CEmbeddingGenerator::_ComputeHash(const std::string& content)
{
	// FNV-1a 64-bit hash
	uint64_t hash = 14695981039346656037ULL;
	for (unsigned char c : content)
	{
		hash ^= c;
		hash *= 1099511628211ULL;
	}
	return hash;
}

