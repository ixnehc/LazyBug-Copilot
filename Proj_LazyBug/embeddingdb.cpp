#include "stdh.h"
#include "embeddingdb.h"
#include "cppsymbol.h"
#include "treesittersymbol.h"
#include "solutiondb.h"
#include "datapacket/DataPacket.h"
#include "Utils_File.h"
#include "stringparser/stringparser.h"

static const int EMBEDDING_FILES_VERSION = 1;

// ============================================================================
// CEmbeddingChunk
// ============================================================================

CEmbeddingChunk::CEmbeddingChunk()
{
	_startLine = 0;
	_endLine = 0;
	_contentHash = 0;
}

void CEmbeddingChunk::Save(CDataPacket& dp) const
{
	dp.Data_WriteSimple(_startLine);
	dp.Data_WriteSimple(_endLine);
	dp.Data_WriteSimple(_contentHash);

	// 写入 embedding
	DP_WriteVector(dp, _embeddings);
}

void CEmbeddingChunk::Load(CDataPacket& dp)
{
	dp.Data_ReadSimple(_startLine);
	dp.Data_ReadSimple(_endLine);
	dp.Data_ReadSimple(_contentHash);

	DP_ReadVector(dp, _embeddings);
}

// ============================================================================
// CFileChunks
// ============================================================================

CFileChunks::CFileChunks()
{
	_genTime = 0;
}

void CFileChunks::Save(CDataPacket& dp) const
{
	dp.Data_WriteSimple(_key.filePath);
	dp.Data_WriteSimple((uint8_t)_key.dbType);
	dp.Data_WriteSimple(_genTime);
	dp.Data_WriteString(_modelName);

	int chunkCount = (int)_chunks.size();
	dp.Data_WriteSimple(chunkCount);
	for (const CEmbeddingChunk& chunk : _chunks)
		chunk.Save(dp);
}

void CFileChunks::Load(CDataPacket& dp)
{
	dp.Data_ReadSimple(_key.filePath);
	uint8_t dbType;
	dp.Data_ReadSimple(dbType);
	_key.dbType = (SymbolDBType)dbType;
	dp.Data_ReadSimple(_genTime);
	dp.Data_ReadString(_modelName);

	int chunkCount;
	dp.Data_ReadSimple(chunkCount);
	_chunks.resize(chunkCount);
	for (int i = 0; i < chunkCount; i++)
		_chunks[i].Load(dp);
}

// ============================================================================
// CEmbeddingFiles
// ============================================================================

void CEmbeddingFiles::Init(const char* folderPath)
{
	_folderPath = folderPath;
}

bool CEmbeddingFiles::VerifyVersion()
{
	std::string versionPath = _folderPath + "\\_embedding\\ver.txt";

	int fileVersion = 0;
	std::string content;
	if (Utils::LoadFileContent(versionPath.c_str(), content))
		fileVersion = atoi(content.c_str());

	if (fileVersion != EMBEDDING_FILES_VERSION)
	{
		_ClearAllBuckets();
		return false;
	}
	return true;
}

void CEmbeddingFiles::Save(std::unordered_map<FilePathKey, CFileChunks>& fileChunks)
{
	std::vector<std::vector<CFileChunks*>> bucketData(ARRAY_SIZE(_buckets));

	for (auto& pair : fileChunks)
	{
		int bucketIndex = _BucketFromKey(pair.first);
		if (_buckets[bucketIndex].isDirty)
			bucketData[bucketIndex].push_back(&pair.second);
	}

	for (int i = 0; i < ARRAY_SIZE(_buckets); i++)
	{
		if (!_buckets[i].isDirty || bucketData[i].empty())
			continue;

		std::vector<BYTE> buf;
		DP_BeginSave(dp, buf);
		{
			int count = (int)bucketData[i].size();
			dp.Data_WriteSimple(count);
			for (CFileChunks* fc : bucketData[i])
				fc->Save(dp);
		}
		DP_EndSave()

			char bucketFileName[32];
		snprintf(bucketFileName, sizeof(bucketFileName), "bucket_%02d.dat", i);
		std::string bucketPath = _folderPath + "\\_embedding\\" + bucketFileName;

		Utils::SaveFileContent(bucketPath.c_str(), buf);
		_buckets[i].isDirty = false;
	}

	_SaveVersion();
}

void CEmbeddingFiles::Load(std::unordered_map<FilePathKey, CFileChunks>& fileChunks)
{
	for (int i = 0; i < ARRAY_SIZE(_buckets); i++)
	{
		char bucketFileName[32];
		snprintf(bucketFileName, sizeof(bucketFileName), "bucket_%02d.dat", i);
		std::string bucketPath = _folderPath + "\\_embedding\\" + bucketFileName;

		if (!Utils::IsFileExist(bucketPath.c_str()))
			continue;

		std::vector<BYTE> buf;
		if (!Utils::LoadFileContent(bucketPath.c_str(), buf))
			continue;

		{
			CDataPacket dp;
			dp.SetDataBufferPointer(buf.data());

			int count;
			dp.Data_ReadSimple(count);

			for (int j = 0; j < count; j++)
			{
				CFileChunks temp;
				temp.Load(dp);
				fileChunks[temp._key] = std::move(temp);
			}
		}
		_buckets[i].isDirty = false;
	}
}

void CEmbeddingFiles::SetDirty(const FilePathKey& key)
{
	int bucket = _BucketFromKey(key);
	_buckets[bucket].isDirty = true;
}

void CEmbeddingFiles::SetAllDirty()
{
	for (int i = 0; i < NUM_BUCKETS; i++)
		_buckets[i].isDirty = true;
}

void CEmbeddingFiles::_ClearAllBuckets()
{
	for (int i = 0; i < ARRAY_SIZE(_buckets); i++)
	{
		char bucketFileName[32];
		snprintf(bucketFileName, sizeof(bucketFileName), "bucket_%02d.dat", i);
		std::string bucketPath = _folderPath + "\\_embedding\\" + bucketFileName;

		if (Utils::IsFileExist(bucketPath.c_str()))
			Utils::RemoveFile(bucketPath.c_str());

		_buckets[i].isDirty = false;
	}
}

void CEmbeddingFiles::_SaveVersion()
{
	std::string versionPath = _folderPath + "\\_embedding\\ver.txt";
	char versionStr[32];
	snprintf(versionStr, sizeof(versionStr), "%d", EMBEDDING_FILES_VERSION);
	Utils::SaveFileContent(versionPath.c_str(), versionStr);
}

int CEmbeddingFiles::_BucketFromKey(const FilePathKey& key) const
{
	return (key.filePath >> CStringPool::BUCKET_INDEX_SHIFT) % ARRAY_SIZE(_buckets);
}

// ============================================================================
// CEmbeddingDB
// ============================================================================

CEmbeddingDB::CEmbeddingDB()
{
	_cppSymbolDB = nullptr;
	_tsSymbolDB = nullptr;
	_cursorCheckEmb.filePath = StringIndex_Null;
	_updateThreadRunning = false;
	_resetThreadLoop = false;
}

void CEmbeddingDB::Init(const char* folderPath,
	CppSymbol::CSymbolDB& cppSymbolDB,
	TreeSitterSymbol::CSymbolDB& tsSymbolDB,
	const EmbedModelParam& modelParam)
{
	if (true)
	{
		std::string path = std::string(folderPath) + "\\_embedding";
		Utils::EnsureFolder(path.c_str());
	}

	_cppSymbolDB = &cppSymbolDB;
	_tsSymbolDB = &tsSymbolDB;
	_folderPath = folderPath;
	if (!modelParam._modelName.empty())
		_modelName = modelParam._modelName;
	_files.Init(folderPath);

	_generator.Init(modelParam, 4, ThreadPriority::LOWEST);
}

void CEmbeddingDB::Clear()
{
	_StopUpdateThread();
	_generator.Close();

	Save();

	if (true)
	{
		std::unique_lock<std::shared_mutex> lock(_mutex);
		_fileChunks.clear();
		_files.SetAllDirty();
	}

	_cppSymbolDB = nullptr;
	_tsSymbolDB = nullptr;
}

// ---- 内容管理（通知 solution 文件变化） ----

void CEmbeddingDB::SetContent(const CSolutionFiles& files)
{
	if (files._lowerCasedFiles.size() <= 0)
		return;

	// 加载已有数据
	if (true)
	{
		std::unique_lock<std::shared_mutex> lock(_mutex);
		_files.VerifyVersion();
		_files.Load(_fileChunks);
	}

	// 先获取 files 的读锁，再获取 _mutex 的写锁（固定锁顺序避免死锁）
	{
		CSolutionFiles::ReadLock filesLock(files._filesMutex);
		std::unique_lock<std::shared_mutex> lock(_mutex);

		auto it = _fileChunks.begin();
		while (it != _fileChunks.end())
		{
			std::string filePath;
			GetStr(it->first, filePath);
			if (files._lowerCasedFiles.find(filePath) == files._lowerCasedFiles.end())
			{
				_files.SetDirty(it->first);
				it = _fileChunks.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	_StartUpdateThread();
}

void CEmbeddingDB::SetDeltaContent(
	const std::vector<SolutionFile*>& newFiles,
	const std::vector<SolutionFile*>& updatedFiles,
	std::vector<std::string>& removedFiles)
{
	std::unique_lock<std::shared_mutex> lock(_mutex);

	auto it = _fileChunks.begin();
	while (it != _fileChunks.end())
	{
		std::string filePath;
		GetStr(it->first, filePath);

		bool removed = false;
		for (auto& path : removedFiles)
		{
			if (filePath == path)
			{
				removed = true;
				break;
			}
		}

		if (removed)
		{
			_files.SetDirty(it->first);
			it = _fileChunks.erase(it);
		}
		else
		{
			++it;
		}
	}
}

const char* CEmbeddingDB::GetModelName() const
{
	std::shared_lock<std::shared_mutex> lock(_modelNameMutex);
	return _modelName.c_str();
}

void CEmbeddingDB::SetModelName(const char* modelName)
{
	{
		std::unique_lock<std::shared_mutex> lock(_modelNameMutex);
		if (_modelName == modelName)
			return;
		_modelName = modelName;
	}
	_resetThreadLoop.store(true);
	_updateCV.notify_all();
}

void CEmbeddingDB::SetModelParam(const EmbedModelParam& modelParam)
{
	SetModelName(modelParam._modelName.c_str());
	_generator.SetModelParam(modelParam);
}

// ---- 激活管理 ----

void CEmbeddingDB::ActivateFile(const char* filePath)
{
	std::string lowerPath = filePath;
	StringLower(lowerPath);

	FilePathKey key = MakeKey(lowerPath.c_str());
	if (key.filePath == StringIndex_Null)
		return;

	// 创建或更新 _fileChunks 条目
	if (true)
	{
		std::unique_lock<std::shared_mutex> lock(_mutex);
		if (_fileChunks.find(key) == _fileChunks.end())
		{
			CFileChunks fileChunks;
			fileChunks._key = key;
			fileChunks._genTime = 0;  // 触发首次分片
			_fileChunks[key] = fileChunks;
			_files.SetDirty(key);
		}
	}

	_resetThreadLoop.store(true);
	_updateCV.notify_all();
}

// ---- embedding 读写 ----

bool CEmbeddingDB::GetChunkEmbeddings(const FilePathKey& key,
	std::vector<std::vector<float>>& outEmbeddings) const
{
	std::shared_lock<std::shared_mutex> lock(_mutex);

	auto it = _fileChunks.find(key);
	if (it == _fileChunks.end())
		return false;

	const CFileChunks& fc = it->second;
	outEmbeddings.clear();
	outEmbeddings.reserve(fc._chunks.size());

	for (const CEmbeddingChunk& chunk : fc._chunks)
	{
		if (!chunk._embeddings.empty())
			outEmbeddings.push_back(chunk._embeddings);
		else
			outEmbeddings.push_back({});  // 尚未生成
	}

	return true;
}

bool CEmbeddingDB::HasPendingChunks(const FilePathKey& key) const
{
	std::shared_lock<std::shared_mutex> lock(_mutex);

	auto it = _fileChunks.find(key);
	if (it == _fileChunks.end())
		return false;

	const CFileChunks& fc = it->second;
	for (const CEmbeddingChunk& chunk : fc._chunks)
	{
		if (chunk._embeddings.empty())
			return true;
	}
	return false;
}

// ---- 相似度查询 ----

void CEmbeddingDB::QuerySimilar(const FilePathKey& key,
	int topK, std::vector<SimilarResult>& results) const
{
	results.clear();

	// 获取目标文件的所有 embedding
	std::vector<std::vector<float>> queryVecs;
	if (!GetChunkEmbeddings(key, queryVecs))
		return;

	// 收集所有候选 embedding
	struct Candidate
	{
		FilePathKey fileKey;
		int         chunkIdx;
		float       similarity;
	};

	std::shared_lock<std::shared_mutex> lock(_mutex);

	for (const auto& filePair : _fileChunks)
	{
		const FilePathKey& fkey = filePair.first;
		if (fkey == key)
			continue;  // 跳过自己

		const CFileChunks& fc = filePair.second;
		for (int ci = 0; ci < (int)fc._chunks.size(); ci++)
		{
			const auto& emb = fc._chunks[ci]._embeddings;
			if (emb.empty())
				continue;

			// 与 query 的每个 embedding 比较，取最大相似度
			float bestSim = 0;
			for (const auto& qv : queryVecs)
			{
				if (qv.empty()) continue;
				float sim = _CosineSimilarity(qv, emb);
				if (sim > bestSim) bestSim = sim;
			}

			results.push_back({ fkey, ci, bestSim });
		}
	}

	// 按相似度降序排序
	std::sort(results.begin(), results.end(),
		[](const SimilarResult& a, const SimilarResult& b)
	{ return a.similarity > b.similarity; });

	if ((int)results.size() > topK)
		results.resize(topK);
}

void CEmbeddingDB::QuerySimilar(const std::vector<float>& queryVec,
	int topK, std::vector<SimilarResult>& results) const
{
	results.clear();
	if (queryVec.empty())
		return;

	std::shared_lock<std::shared_mutex> lock(_mutex);

	for (const auto& filePair : _fileChunks)
	{
		const FilePathKey& fkey = filePair.first;
		const CFileChunks& fc = filePair.second;

		for (int ci = 0; ci < (int)fc._chunks.size(); ci++)
		{
			const auto& emb = fc._chunks[ci]._embeddings;
			if (emb.empty())
				continue;

			float sim = _CosineSimilarity(queryVec, emb);
			results.push_back({ fkey, ci, sim });
		}
	}

	std::sort(results.begin(), results.end(),
		[](const SimilarResult& a, const SimilarResult& b)
	{ return a.similarity > b.similarity; });

	if ((int)results.size() > topK)
		results.resize(topK);
}

// ---- 工具方法 ----

void CEmbeddingDB::GetStr(const FilePathKey& key, std::string& ret) const
{
	if (key.dbType == SymbolDBType::CppSymbol)
		_cppSymbolDB->GetStr(key.filePath, ret);
	else
		_tsSymbolDB->GetStr(key.filePath, ret);
}

FilePathKey CEmbeddingDB::MakeKey(const char* filePath) const
{
	std::string lowerPath = filePath;
	StringLower(lowerPath);

	std::string suffix = GetFileSuffix(lowerPath);
	StringLower(suffix);

	FilePathKey key;
	key.dbType = Utils::IsCppFile(suffix) ? SymbolDBType::CppSymbol
		: SymbolDBType::TreeSitterSymbol;

	if (key.dbType == SymbolDBType::CppSymbol)
		key.filePath = _cppSymbolDB->FindStr(lowerPath.c_str());
	else
		key.filePath = _tsSymbolDB->FindStr(lowerPath.c_str());

	return key;
}

std::string CEmbeddingDB::_GetRealFilePath(const FilePathKey& key) const
{
	std::string ret;
	GetStr(key, ret);
	return ret;
}

// ---- 持久化 ----

void CEmbeddingDB::Save()
{
	std::unique_lock<std::shared_mutex> lock(_mutex);
	_files.Save(_fileChunks);
}

// ---- 后台更新线程 ----

void CEmbeddingDB::_StartUpdateThread()
{
	if (_updateThreadRunning.load())
		return;

	_updateThreadRunning.store(true);
	_updateThread = std::thread(&CEmbeddingDB::_UpdateThreadProc, this);

	HANDLE threadHandle = _updateThread.native_handle();
	SetThreadPriority(threadHandle, THREAD_PRIORITY_LOWEST);
}

void CEmbeddingDB::_StopUpdateThread()
{
	if (!_updateThreadRunning.load())
		return;

	_updateThreadRunning.store(false);
	_updateCV.notify_all();

	if (_updateThread.joinable())
	{
		_updateThread.join();
	}
}

void CEmbeddingDB::_UpdateThreadProc()
{
	while (_updateThreadRunning.load())
	{
		_resetThreadLoop = false;

		bool isAnyAction = false;

		// Fetch embedding results from generator thread pool
		if (true)
		{
			const AbsTick budgetDur = 20;
			AbsTick startFetch = GetAbsTick();
			EmbedResult result;
			while (_generator.FetchResult(result))
			{
				if (result.success)
				{
					// generator 返回完整的 CFileChunks，直接替换
					{
						std::unique_lock<std::shared_mutex> lock(_mutex);
						auto it = _fileChunks.find(result.key);
						if (it != _fileChunks.end())
						{
							it->second._chunks = std::move(result.chunks);
							it->second._genTime = result.symbolParseTime;
							it->second._modelName = _modelName;
							_files.SetDirty(result.key);
						}
					}
				}
				isAnyAction = true;
				if (GetAbsTick() > startFetch + budgetDur)
					break;
				if (_NeedResetThreadLoop())
					break;
			}
			if (_NeedResetThreadLoop())
				continue;
		}

		// 轮询检查过期文件，提交构建请求到 generator
		if (true)
		{
			const AbsTick budgetDur = 2000000000000;
			AbsTick startT = GetAbsTick();

			std::shared_lock<std::shared_mutex> lock(_mutex);

			auto cursorIt = _fileChunks.end();
			if (_cursorCheckEmb.filePath != StringIndex_Null)
				cursorIt = _fileChunks.find(_cursorCheckEmb);
			if (cursorIt == _fileChunks.end())
				cursorIt = _fileChunks.begin();

			int nSteps = 0;
			while (cursorIt != _fileChunks.end())
			{
				if (_NeedResetThreadLoop())
					break;
				if (GetAbsTick() > startT + budgetDur)
					break;

				const FilePathKey& key = cursorIt->first;

				time_t parsedTime = _CheckOutOfDate(key);
				if (parsedTime != 0)
				{
					// 准备请求：从 SymbolDB 获取 symbol 行范围
					EmbedRequest request;
					request.key = key;
					request.filePath = _GetRealFilePath(key);
					request.oldChunks = cursorIt->second._chunks;

					if (!_GetSymbolRanges(key, request.symbolRanges, request.symbolParseTime))
					{
						// 获取 symbol range 失败（文件可能已被删除或解析有问题），
						// 直接设空 chunks 并标记为已生成，避免反复重试
						cursorIt->second._chunks.clear();
						cursorIt->second._genTime = parsedTime;
						cursorIt->second._modelName = _modelName;
						_files.SetDirty(key);
						isAnyAction = true;
					}
					else
					{
						_generator.Request(request);
						isAnyAction = true;
					}
				}

				if (_StepCursor(cursorIt, nSteps))
					break;
			}

			if (cursorIt == _fileChunks.end())
				_cursorCheckEmb.filePath = StringIndex_Null;
			else
				_cursorCheckEmb = cursorIt->first;
		}

		if (_NeedResetThreadLoop())
			continue;

		if (!_generator.IsFlushed())
			continue;
		if (isAnyAction)
			continue;

		Save();

		std::unique_lock<std::mutex> lock(_updateMutex);
		_updateCV.wait_for(lock, std::chrono::milliseconds(100), [this]
		{
			return _NeedResetThreadLoop();
		});
	}
}

bool CEmbeddingDB::_NeedResetThreadLoop()
{
	if (!_updateThreadRunning.load())
		return true;
	if (_resetThreadLoop.load())
		return true;
	return false;
}

// ---- 轮询 ----

time_t CEmbeddingDB::_CheckOutOfDate(const FilePathKey& key) const
{
	if (_modelName.empty())
		return 0;
	time_t parsedTime = 0;
	if (key.dbType == SymbolDBType::CppSymbol)
		parsedTime = _cppSymbolDB->GetParsedTime(key.filePath);
	else
		parsedTime = _tsSymbolDB->GetParsedTime(key.filePath);

	if (parsedTime == 0)
		return 0;  // 尚未解析过

	auto it = _fileChunks.find(key);
	if (it == _fileChunks.end())
		return 0;  // 不在 embedding 跟踪范围内

	if (it->second._genTime != parsedTime || it->second._modelName != _modelName)
		return parsedTime;  // 需要更新

	return 0;
}

bool CEmbeddingDB::_StepCursor(_FileChunksIt& cursorIt, int& nSteps)
{
	if (cursorIt == _fileChunks.end())
		return true;
	++cursorIt;
	++nSteps;
	if (cursorIt == _fileChunks.end())
		cursorIt = _fileChunks.begin();
	return nSteps >= (int)_fileChunks.size();
}

// ---- 分片 ----

bool CEmbeddingDB::_GetSymbolRanges(const FilePathKey& key,
	std::vector<SymbolRangeInfo>& outRanges,
	time_t& outParsedTime) const
{
	if (key.dbType == SymbolDBType::CppSymbol)
		return _cppSymbolDB->GetSymbolLineRanges(key.filePath, outRanges, outParsedTime);
	else
		return _tsSymbolDB->GetSymbolLineRanges(key.filePath, outRanges, outParsedTime);
}

// ---- 查找 ----

CFileChunks* CEmbeddingDB::_FindFileChunks(const FilePathKey& key)
{
	auto it = _fileChunks.find(key);
	if (it != _fileChunks.end())
		return &it->second;
	return nullptr;
}

// ---- 相似度 ----

float CEmbeddingDB::_CosineSimilarity(const std::vector<float>& a,
	const std::vector<float>& b)
{
	if (a.size() != b.size() || a.empty())
		return 0.0f;

	float dot = 0, normA = 0, normB = 0;
	for (size_t i = 0; i < a.size(); i++)
	{
		dot += a[i] * b[i];
		normA += a[i] * a[i];
		normB += b[i] * b[i];
	}

	if (normA == 0 || normB == 0)
		return 0.0f;

	return dot / (std::sqrt(normA) * std::sqrt(normB));
}
