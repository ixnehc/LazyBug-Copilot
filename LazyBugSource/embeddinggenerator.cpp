#include "stdh.h"
#include "embeddinggenerator.h"
#include "Utils_File.h"
#include "Utils.h"
#include "stringparser/stringparser.h"

#include <set>
#include <algorithm>
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
// Segment — 文件中的一个行区间 [start, end)
// ============================================================================
struct Segment { int start; int end; bool isTarget; };

// ============================================================================
// BuildSegmentsFromSymbols
// 从指定类型 symbol 的 range 生成独立 segment，并补全文件剩余部分为 complement segment
// - 每个 _kind ∈ targetKinds 的 symbol range 直接作为一个 segment（不合并，即便重叠/相邻）
// - 文件中去除这些 segment 后的剩余部分也生成 complement segment
// - 输出按 start 升序排列，可能包含重叠的 segment
// ============================================================================
static void BuildSegmentsFromSymbols(
    const std::vector<SymbolRangeInfo>& symbolRanges,
    int totalLines,
    const std::set<SymbolKind>& targetKinds,
    std::vector<Segment>& outSegments)
{
	outSegments.clear();

	// 1. 收集 + 裁剪 target range
	std::vector<std::pair<int, int>> targetRanges;
	for (const auto& info : symbolRanges)
	{
		if (targetKinds.find(info._kind) == targetKinds.end())
			continue;

		int s = (int)info._lineRange.start;
		int e = (int)info._lineRange.end;
		if (s >= e)
			continue;
		if (s < 0) s = 0;
		if (e > totalLines) e = totalLines;
		if (s >= e)
			continue;

		// 单行的 Method 跳过（太短，不值得单独 embedding）
		if (info._kind == SymbolKind::Method && e - s <= 1)
			continue;

		targetRanges.push_back({s, e});
	}

	// 2. 每个 target range 直接输出为 segment（不合并）
	for (const auto& r : targetRanges)
		outSegments.push_back({r.first, r.second, true});

	// 3. 输出 complement segment（排序后跟踪 prev 即可，无需显式合并）
	{
		std::vector<std::pair<int, int>> sortedRanges = targetRanges;
		std::sort(sortedRanges.begin(), sortedRanges.end());

		int prev = 0;
		for (const auto& r : sortedRanges)
		{
			if (r.first > prev)
				outSegments.push_back({prev, r.first, false});
			prev = (std::max)(prev, r.second);
		}
		if (prev < totalLines)
			outSegments.push_back({prev, totalLines, false});
	}

	// 4. 按 start 排序
	std::sort(outSegments.begin(), outSegments.end(),
	          [](const Segment& a, const Segment& b) { return a.start < b.start; });
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
	_enable           = true;
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
	_enable          = true;

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

void CEmbeddingGenerator::SetModelParam(const EmbedModelParam& modelParam)
{
	{
		std::lock_guard<std::mutex> lock(_modelParamMutex);
		_modelParam = modelParam;
	}

	// 模型参数有效时恢复激活状态
	if (modelParam.IsValid())
	{
		_enable.store(true);
		_requestCV.notify_all();
	}
}

void CEmbeddingGenerator::ReEnable()
{
	_enable.store(true);
	_requestCV.notify_all();
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
	_enable        = true;
	_modelParam = EmbedModelParam();
}

bool CEmbeddingGenerator::Request(EmbedRequest& request)
{
	if (!_running)
		return false;

	request.requestId = _nextRequestId.fetch_add(1);

	// 失活状态: 直接塞入失败结果, 不入请求队列
	if (!_enable.load())
	{
		EmbedResult result;
		result.key       = request.key;
		result.requestId = request.requestId;
		result.success   = false;
		{
			std::lock_guard<std::mutex> lock(_modelParamMutex);
			result.modelName = _modelParam._modelName;
		}

		std::lock_guard<std::mutex> lock(_resultMutex);
		_resultQueue.push_back(std::move(result));
		return true;
	}

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

bool CEmbeddingGenerator::IsEnabled() const
{
	return _enable.load();
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

		EmbedResult result;

		if (_enable.load())
			result = _ProcessRequest(request);
		else
		{
			// 失活状态: 直接返回失败结果
			result.key       = request.key;
			result.requestId = request.requestId;
			result.success   = false;
			{
				std::lock_guard<std::mutex> lock(_modelParamMutex);
				result.modelName = _modelParam._modelName;
			}
		}

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

	constexpr size_t MAX_BYTES_PER_CHUNK = 8192;

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

	// 3. 生成 segments：特定类型 symbol 的 range 各自独立成段，文件剩余部分生成 complement segment
	static const std::set<SymbolKind> TARGET_KINDS = {
		SymbolKind::Class, SymbolKind::Struct, SymbolKind::Enum,
		SymbolKind::Function, SymbolKind::Method,
		SymbolKind::Constructor, SymbolKind::Destructor
	};
	std::vector<Segment> segments;
	BuildSegmentsFromSymbols(request.symbolRanges, totalLines, TARGET_KINDS, segments);

	// 4. 每个 segment 生成 chunk + 构建内容文本，跳过空白 chunk
	//    - target segment：超限时截断尾部（单 chunk）
	//    - complement segment：超限时拆分为多个 chunk
	std::vector<CEmbeddingChunk> newChunks;
	std::vector<std::string> textsToEmbed;

	auto isBlank = [](const std::string& s) -> bool {
		for (unsigned char c : s)
		{
			if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
				return false;
		}
		return true;
	};

	auto flushChunk = [&](std::string& content, int startLine, int endLine) {
		if (isBlank(content))
		{
			content.clear();
			return;
		}
		CEmbeddingChunk chunk;
		chunk._startLine   = startLine;
		chunk._endLine     = endLine;
		chunk._contentHash = _ComputeHash(content);
		newChunks.push_back(std::move(chunk));
		textsToEmbed.push_back(std::move(content));
		content.clear();
	};

	for (const auto& seg : segments)
	{
		if (seg.start >= seg.end)
			continue;

		if (seg.isTarget)
		{
			// target segment：逐行追加，超限时截断尾部
			std::string content;
			int lastLine = seg.start;
			for (int i = seg.start; i < seg.end; i++)
			{
				size_t addSize = (content.empty() ? 0 : 1) + lines[i].size();
				if (content.size() + addSize > MAX_BYTES_PER_CHUNK)
				{
					if (content.empty())
					{
						// 首行即超限 — 仍包含该行
						content += lines[i];
						lastLine = i + 1;
					}
					break;
				}
				if (!content.empty())
					content += "\n";
				content += lines[i];
				lastLine = i + 1;
			}
			if (!content.empty())
				flushChunk(content, seg.start, lastLine);
		}
		else
		{
			// complement segment：逐行追加，超限时 flush 后开启新 chunk
			std::string content;
			int chunkStart = seg.start;
			int lastLine   = seg.start;
			for (int i = seg.start; i < seg.end; i++)
			{
				size_t addSize = (content.empty() ? 0 : 1) + lines[i].size();
				if (!content.empty() && content.size() + addSize > MAX_BYTES_PER_CHUNK)
				{
					flushChunk(content, chunkStart, lastLine);
					chunkStart = i;
				}
				if (!content.empty())
					content += "\n";
				content += lines[i];
				lastLine = i + 1;
			}
			if (!content.empty())
				flushChunk(content, chunkStart, lastLine);
		}
	}

	// 6. 调用 embedding API（带重试）
	if (!textsToEmbed.empty())
	{
		// 在此点加锁拷贝 modelParam, 确保 result.modelName 与实际使用的一致
		EmbedModelParam modelParam;
		{
			std::lock_guard<std::mutex> lock(_modelParamMutex);
			modelParam = _modelParam;
		}
		result.modelName = modelParam._modelName;

		bool apiOk = false;
		for (int retry = 0; retry < MAX_RETRIES && _enable.load(); retry++)
		{
			std::vector<std::vector<float>> modelEmbeddings;
			if (_CallEmbeddingApi(modelParam, textsToEmbed, modelEmbeddings))
			{
				for (size_t i = 0; i < newChunks.size() && i < modelEmbeddings.size(); i++)
					newChunks[i]._embeddings = std::move(modelEmbeddings[i]);
				apiOk = true;
				break;
			}
		}

		if (!apiOk)
		{
			// 重试耗尽: 标记整个 generator 为失活
			_enable.store(false);
			_requestCV.notify_all();
			return result;   // success = false
		}
	}

	result.chunks  = std::move(newChunks);
	result.symbolParseTime = request.symbolParseTime;
	result.success = true;
	return result;
}

// ---- Embedding API 调用 ----

bool CEmbeddingGenerator::_CallEmbeddingApi(const EmbedModelParam& modelParam,
                                            const std::vector<std::string>& texts,
                                            std::vector<std::vector<float>>& outEmbeddings)
{
	outEmbeddings.clear();

	if (texts.empty())
		return true;

	if (!modelParam.IsValid())
		return false;

	// 构造 embedding endpoint URL
	std::string embedEndpoint = modelParam._endpoint;
	if (!embedEndpoint.empty() && embedEndpoint.back() == '/')
		embedEndpoint.pop_back();
	// 只有当结尾不是 "/embeddings" 时才添加
	if (embedEndpoint.size() < 11 || embedEndpoint.compare(embedEndpoint.size() - 11, 11, "/embeddings") != 0)
		embedEndpoint += "/embeddings";

	// 构造请求 JSON
	json requestJson;
	requestJson["model"] = modelParam._modelName;
	requestJson["input"] = texts;

	std::string requestBody = requestJson.dump();

	// 初始化 CURL
	CURL* curl = curl_easy_init();
	if (!curl)
		return false;

	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

	std::string authHeader = "Authorization: Bearer " + modelParam._apiKey;
	headers = curl_slist_append(headers, authHeader.c_str());

	EmbedApiResponse response;

	curl_easy_setopt(curl, CURLOPT_URL, embedEndpoint.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, requestBody.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _EmbedWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
	if (modelParam._timeoutSeconds > 0)
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, modelParam._timeoutSeconds);

	// 进度回调: 失活时中止 curl 传输
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &CEmbeddingGenerator::_CurlProgressCb);
	curl_easy_setopt(curl, CURLOPT_XFERINFODATA, this);

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

// ---- curl 进度回调 ----

int CEmbeddingGenerator::_CurlProgressCb(void* userp, double, double, double, double)
{
	auto* self = static_cast<CEmbeddingGenerator*>(userp);
	return self->_enable.load() ? 0 : 1;   // 非0 → curl 立即中止传输
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

