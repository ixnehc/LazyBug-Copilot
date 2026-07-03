#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <shared_mutex>
#include <time.h>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "coredefines.h"
#include "StringPool.h"
#include "embeddingtypes.h"
#include "embeddinggenerator.h"

class CDataPacket;
class CSolutionFiles;
struct SolutionFile;

namespace CppSymbol { class CSymbolDB; }
namespace TreeSitterSymbol { class CSymbolDB; }

// ============================================================================
// CEmbeddingFiles — bucket 持久化管理（类似 CSymbolFiles）
// ============================================================================
class CEmbeddingFiles
{
public:
	static const int NUM_BUCKETS = CStringPool::NUM_BUCKETS;  // 必须与 CStringPool 一致

	void Init(const char* folderPath);

	// 版本校验,版本不匹配则清除所有 bucket
	bool VerifyVersion();

	void Save(std::unordered_map<FilePathKey, CFileChunks>& fileChunks);
	void Load(std::unordered_map<FilePathKey, CFileChunks>& fileChunks);

	void SetDirty(const FilePathKey& key);
	void SetAllDirty();

private:
	void _ClearAllBuckets();
	void _SaveVersion();
	int  _BucketFromKey(const FilePathKey& key) const;

	struct Bucket
	{
		Bucket() : isDirty(false) {}
		bool isDirty;
	};

	Bucket      _buckets[NUM_BUCKETS];
	std::string _folderPath;
};

// ============================================================================
// CEmbeddingDB — 项目文件的 embedding 数据库
// ============================================================================
class CEmbeddingDB
{
public:
	CEmbeddingDB();

	// ---- 生命周期 ----
	// folderPath: embedding db 的文件夹路径
	// cppSymbolDB / tsSymbolDB: 复用其 strPool 和 symbol 分片信息
	// modelParam: 生成 embedding 使用的模型 API 参数
	void Init(const char* folderPath,
	          CppSymbol::CSymbolDB& cppSymbolDB,
	          TreeSitterSymbol::CSymbolDB& tsSymbolDB,
	          const EmbedModelParam& modelParam = EmbedModelParam());
	void Clear();

	// 设置当前模型名，若与已有 chunk 不一致则下一轮轮询触发重新生成
	const char* GetModelName() const;
	void SetModelName(const char* modelName);

	// 运行时更新模型参数（modelName + generator 的 endpoint/key/timeout）
	void SetModelParam(const EmbedModelParam& modelParam);

	// ---- 内容管理（由 SolutionDB 调用）- 通知 solution 文件变化 ----
	// 全量设置 solution 文件列表
	void SetContent(const CSolutionFiles& files);
	// 增量更新 solution 文件列表
	void SetDeltaContent(const std::vector<SolutionFile*>& newFiles,
	                     const std::vector<SolutionFile*>& updatedFiles,
	                     std::vector<std::string>& removedFiles);

	// ---- 激活管理（由外部控制哪些文件需要 embedding） ----
	// 只有被激活且在 solution 中的文件才生成/维护 embedding
	void ActivateFile(const char* filePath);

	// ---- embedding 读写 ----
	bool GetChunkEmbeddings(const FilePathKey& key,
	                        std::vector<std::vector<float>>& outEmbeddings) const;

	// 检查指定文件是否还有未生成的 embedding
	bool HasPendingChunks(const FilePathKey& key) const;

	// ---- 相似度查询（仅在已激活文件中搜索） ----
	struct SimilarResult
	{
		FilePathKey key;
		int         chunkIndex;
		float       similarity;   // [0, 1]
	};

	// 查询与指定文件(所有chunk)最相似的 topK 个 chunk
	void QuerySimilar(const FilePathKey& key,
	                  int topK, std::vector<SimilarResult>& results) const;

	// 用给定向量查询最相似的 topK 个 chunk
	void QuerySimilar(const std::vector<float>& queryVec,
	                  int topK, std::vector<SimilarResult>& results) const;

	// ---- 工具方法 ----
	void GetStr(const FilePathKey& key, std::string& ret) const;
	FilePathKey MakeKey(const char* filePath) const;

	// ---- 持久化 ----
	void Save();

public:
	std::string                  _folderPath;
	CppSymbol::CSymbolDB*        _cppSymbolDB;
	TreeSitterSymbol::CSymbolDB* _tsSymbolDB;
	CEmbeddingFiles              _files;
	std::string                  _modelName;       // 当前使用的 LLM 模型名

	mutable std::shared_mutex     _modelNameMutex;  // 保护 _modelName（与 _mutex 分离，避免锁嵌套）

	// 仅包含已激活文件的 chunks（线程安全: _mutex 保护）
	std::unordered_map<FilePathKey, CFileChunks>  _fileChunks;
	mutable std::shared_mutex                      _mutex;

private:
	// ---- 后台更新线程 ----
	void _StartUpdateThread();
	void _StopUpdateThread();
	void _UpdateThreadProc();
	bool _NeedResetThreadLoop();

	std::thread             _updateThread;
	std::mutex              _updateMutex;
	std::condition_variable _updateCV;
	std::atomic<bool>       _updateThreadRunning;
	std::atomic<bool>       _resetThreadLoop;

	FilePathKey _cursorCheckEmb;  // 轮询游标

	// ---- embedding 生成线程池 ----
	CEmbeddingGenerator _generator;

	// ---- 轮询: 检查文件是否需要重新分片 ----
	// 对比 _fileChunks[key]._genTime 与对应 SymbolDB 中该文件的 _parsedTime,
	// 对比 _fileChunks[key]._modelName 与当前 _modelName,
	// 返回 SymbolDB 的 _parsedTime（0 表示不需要更新）
	time_t _CheckOutOfDate(const FilePathKey& key) const;

	// 轮询游标,遍历 _fileChunks
	typedef std::unordered_map<FilePathKey, CFileChunks>::iterator _FileChunksIt;
	bool _StepCursor(_FileChunksIt& cursorIt, int& nSteps);  // 返回是否已遍历一圈

	// ---- 分片 ----
	// 从对应 SymbolDB 获取文件中所有 symbol 的类型、行范围及 _parsedTime
	bool _GetSymbolRanges(const FilePathKey& key,
	                      std::vector<SymbolRangeInfo>& outRanges,
	                      time_t& outParsedTime) const;

	// 根据 FilePathKey 获取实际文件系统路径
	std::string _GetRealFilePath(const FilePathKey& key) const;

	// ---- 查找 ----
	CFileChunks* _FindFileChunks(const FilePathKey& key);

	// ---- 相似度 ----
	static float _CosineSimilarity(const std::vector<float>& a,
	                               const std::vector<float>& b);
};
