#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <time.h>
#include <stdint.h>

#include "coredefines.h"
#include "StringPool.h"

class CDataPacket;

// ============================================================================
// 标识 FilePathKey 中的 StringIndex 属于哪个 SymbolDB 的 strPool
// ============================================================================
enum class SymbolDBType : uint8_t
{
	CppSymbol = 0,
	TreeSitterSymbol = 1
};

// ============================================================================
// FilePathKey — 复用已有 strPool 中的文件路径索引,并记录所属 strPool
// ============================================================================
struct FilePathKey
{
	StringIndex   filePath = StringIndex_Null;
	SymbolDBType  dbType   = SymbolDBType::CppSymbol;

	bool operator==(const FilePathKey& o) const
	{
		return filePath == o.filePath && dbType == o.dbType;
	}

	bool operator!=(const FilePathKey& o) const
	{
		return !(*this == o);
	}

	size_t Hash() const
	{
		return ((size_t)(uint32_t)filePath) ^ ((size_t)(uint8_t)dbType << 31);
	}
};

namespace std
{
	template<>
	struct hash<FilePathKey>
	{
		size_t operator()(const FilePathKey& k) const
		{
			return k.Hash();
		}
	};
}

// ============================================================================
// CEmbeddingChunk — 一个文件片段及其在多个模型下的 embedding 向量
// ============================================================================
class CEmbeddingChunk
{
public:
	CEmbeddingChunk();

	void Save(CDataPacket& dp) const;
	void Load(CDataPacket& dp);

public:
	int         _startLine;      // 片段起始行 (0-based, inclusive)
	int         _endLine;        // 片段结束行 (0-based, exclusive)
	uint64_t    _contentHash;    // 片段内容的哈希,用于检测内容是否发生变化

	// embedding 向量 (固定 1024 维)
	std::vector<float> _embeddings;
};

// ============================================================================
// CFileChunks — 单个文件的全部分片及其 embedding 数据
// ============================================================================
class CFileChunks
{
public:
	CFileChunks();

	void Save(CDataPacket& dp) const;
	void Load(CDataPacket& dp);

public:
	FilePathKey                  _key;        // 文件标识（含所属 strPool）
	time_t                       _genTime;    // 生成此分片时,对应 SymbolDB 中该文件的 _parsedTime
	std::string                  _modelName;  // 生成时使用的模型名
	std::vector<CEmbeddingChunk> _chunks;     // 按行号顺序排列的所有片段
};

// ============================================================================
// EmbedModelParam — 调用 embedding API 所需的参数
// ============================================================================
struct EmbedModelParam
{
	std::string _modelName;       // 模型名
	std::string _endpoint;        // API endpoint（如 https://api.openai.com/v1，不含尾部斜杠）
	std::string _apiKey;          // API key
	int         _timeoutSeconds;  // 请求超时秒数

	EmbedModelParam()
		: _timeoutSeconds(600)
	{
	}

	bool IsValid() const
	{
		return !_modelName.empty() && !_endpoint.empty() && !_apiKey.empty();
	}
};
