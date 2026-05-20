#include "stdh.h"

#include "DocIndexer.h"
#include "CoreDefines.h"
#include "Utils.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <cstring>
#include <vector>

// ============================================================
//  持久化格式常量
// ============================================================
static constexpr uint32_t DIDX_MAGIC = 0x44494458u; // 'DIDX'
static constexpr uint32_t DIDX_VERSION = 3u;

// ============================================================
//  内部类型
// ============================================================

using DocId = uint32_t;
static constexpr DocId INVALID_DOC_ID = 0;

struct Posting
{
	DocId            docId = INVALID_DOC_ID;
	std::vector<int> lines; // 1-base，已去重升序
};

using PostingList = std::vector<Posting>;

struct DocMeta
{
	std::string filePath;
	time_t      mtime = 0;
};

// ============================================================
//  工具：判断字符是否为"标识符字符"
// ============================================================
static inline bool IsWordChar(char c)
{
	return (c >= 'a' && c <= 'z') ||
		(c >= 'A' && c <= 'Z') ||
		(c >= '0' && c <= '9') ||
		c == '_';
}

// ============================================================
//  Tokenizer
//  将 UTF-8 文本按"标识符 / 数字"边界分词，保留行号
// ============================================================
struct TokenPos { std::string token; int line; };

static void Tokenize(const std::string& content, std::vector<TokenPos>& out)
{
	out.clear();
	const char* p = content.c_str();
	const char* end = p + content.size();
	int line = 1;

	while (p < end)
	{
		if (*p == '\n') { ++line; ++p; continue; }

		if (IsWordChar(*p))
		{
			const char* s = p;
			while (p < end && IsWordChar(*p)) ++p;
			size_t len = static_cast<size_t>(p - s);
			// 长度 [2, 128]：过短无意义，过长可能是 base64 / 哈希
			if (len >= 2 && len <= 128)
			{
				TokenPos tp;
				tp.line = line;
				tp.token.resize(len);
				for (size_t i = 0; i < len; ++i)
					tp.token[i] = static_cast<char>(
						::tolower(static_cast<unsigned char>(s[i])));
				out.push_back(std::move(tp));
			}
		}
		else
		{
			++p;
		}
	}
}

// ============================================================
//  BinWriter / BinReader  —  小端二进制序列化
// ============================================================
class BinWriter
{
public:
	explicit BinWriter(std::vector<uint8_t>& buf) : _buf(buf) {}

	void WriteU8(uint8_t v) { _buf.push_back(v); }

	void WriteU32(uint32_t v)
	{
		_buf.push_back(uint8_t(v));
		_buf.push_back(uint8_t(v >> 8));
		_buf.push_back(uint8_t(v >> 16));
		_buf.push_back(uint8_t(v >> 24));
	}
	void WriteI32(int32_t v) { WriteU32(uint32_t(v)); }

	void WriteU64(uint64_t v)
	{
		WriteU32(uint32_t(v));
		WriteU32(uint32_t(v >> 32));
	}

	void WriteStr(const std::string& s)
	{
		WriteU32(uint32_t(s.size()));
		for (unsigned char c : s) WriteU8(c);
	}

private:
	std::vector<uint8_t>& _buf;
};

class BinReader
{
public:
	BinReader(const uint8_t* data, size_t size)
		: _data(data), _pos(0), _size(size), _ok(true) {
	}

	bool Ok()  const { return _ok; }

	uint8_t ReadU8()
	{
		if (_pos + 1 > _size) { _ok = false; return 0; }
		return _data[_pos++];
	}
	uint32_t ReadU32()
	{
		if (_pos + 4 > _size) { _ok = false; return 0; }
		uint32_t v = uint32_t(_data[_pos])
			| uint32_t(_data[_pos + 1]) << 8
			| uint32_t(_data[_pos + 2]) << 16
			| uint32_t(_data[_pos + 3]) << 24;
		_pos += 4;
		return v;
	}
	int32_t  ReadI32() { return int32_t(ReadU32()); }
	uint64_t ReadU64()
	{
		uint64_t lo = ReadU32();
		uint64_t hi = ReadU32();
		return lo | (hi << 32);
	}
	std::string ReadStr()
	{
		uint32_t len = ReadU32();
		if (!_ok || _pos + len > _size) { _ok = false; return {}; }
		std::string s(reinterpret_cast<const char*>(_data + _pos), len);
		_pos += len;
		return s;
	}

private:
	const uint8_t* _data;
	size_t         _pos;
	size_t         _size;
	bool           _ok;
};

// ============================================================
//  InvertedIndex
//
//  · _postings   : token(小写) → PostingList
//  · _docMeta    : DocId → DocMeta
//  · _pathToDoc  : lowerPath → DocId
//  · _deletedDocs: 懒删除集合（压实时才物理删除）
//  · _nextDocId  : 自增 ID
// ============================================================
class InvertedIndex
{
public:
	InvertedIndex() : _nextDocId(1), _dirty(false) {}

	// ----- 文档操作 -----------------------------------------

	void AddDocument(const std::string& filePath,
		time_t             mtime,
		const std::string& utf8Content)
	{
		// 先删旧版本（若存在）
		RemoveDocument(filePath);

		DocId id = _nextDocId++;
		std::string lp = _LowerPath(filePath);
		_pathToDoc[lp] = id;
		_docMeta[id] = { filePath, mtime };

		// Tokenize → 按 token 聚合行号（去重）
		std::vector<TokenPos> toks;
		Tokenize(utf8Content, toks);

		std::unordered_map<std::string, std::unordered_set<int>> lineMap;
		lineMap.reserve(toks.size());
		for (auto& tp : toks)
			lineMap[tp.token].insert(tp.line);

		for (auto& kv : lineMap)
		{
			Posting p;
			p.docId = id;
			p.lines.assign(kv.second.begin(), kv.second.end());
			std::sort(p.lines.begin(), p.lines.end());
			_postings[kv.first].push_back(std::move(p));
		}

		_dirty = true;
	}

	void RemoveDocument(const std::string& filePath)
	{
		std::string lp = _LowerPath(filePath);
		auto it = _pathToDoc.find(lp);
		if (it == _pathToDoc.end()) return;

		DocId id = it->second;
		// 懒删除：不立即清理 _postings，压实时再做
		_deletedDocs.insert(id);
		_pathToDoc.erase(it);
		_docMeta.erase(id);
		_dirty = true;
	}

	time_t GetMTime(const std::string& filePath) const
	{
		auto it = _pathToDoc.find(_LowerPath(filePath));
		if (it == _pathToDoc.end()) return 0;
		auto jt = _docMeta.find(it->second);
		return jt != _docMeta.end() ? jt->second.mtime : 0;
	}

	size_t GetDocumentCount() const { return _pathToDoc.size(); }

	// ----- 查询候选文档 -------------------------------------
	// keyIsWord=true  → 倒排表精确查找
	// keyIsWord=false → 返回全部活跃文档（线性扫描由上层做）
	std::vector<DocId> QueryCandidates(const std::string& lowerKey,
		bool               keyIsWord) const
	{
		std::vector<DocId> result;
		if (keyIsWord)
		{
			auto it = _postings.find(lowerKey);
			if (it == _postings.end()) return {};
			result.reserve(it->second.size());
			for (const auto& p : it->second)
				if (_deletedDocs.count(p.docId) == 0)
					result.push_back(p.docId);
		}
		else
		{
			result.reserve(_pathToDoc.size());
			for (const auto& kv : _pathToDoc)
				result.push_back(kv.second);
		}
		return result;
	}

	const std::string* GetFilePath(DocId id) const
	{
		auto it = _docMeta.find(id);
		return it != _docMeta.end() ? &it->second.filePath : nullptr;
	}

	// ----- 持久化 -------------------------------------------
	void Serialize(std::vector<uint8_t>& out) const
	{
		// 序列化前先在副本上做压实
		InvertedIndex tmp = *this;
		tmp._Compact();

		out.clear();
		BinWriter w(out);

		w.WriteU32(DIDX_MAGIC);
		w.WriteU32(DIDX_VERSION);
		w.WriteU32(uint32_t(tmp._nextDocId));

		// DocMeta
		w.WriteU32(uint32_t(tmp._docMeta.size()));
		for (const auto& kv : tmp._docMeta)
		{
			w.WriteU32(kv.first);
			w.WriteStr(kv.second.filePath);
			w.WriteU64(uint64_t(kv.second.mtime));
		}

		// 倒排表
		// 先统计非空 token 数量
		uint32_t nonEmpty = 0;
		for (const auto& kv : tmp._postings)
			if (!kv.second.empty()) ++nonEmpty;

		w.WriteU32(nonEmpty);
		for (const auto& kv : tmp._postings)
		{
			if (kv.second.empty()) continue;
			w.WriteStr(kv.first);
			w.WriteU32(uint32_t(kv.second.size()));
			for (const auto& p : kv.second)
			{
				w.WriteU32(p.docId);
				w.WriteU32(uint32_t(p.lines.size()));
				for (int ln : p.lines) w.WriteI32(ln);
			}
		}
	}

	bool Deserialize(const uint8_t* data, size_t size)
	{
		BinReader r(data, size);

		if (r.ReadU32() != DIDX_MAGIC)   return false;
		if (r.ReadU32() != DIDX_VERSION) return false;
		if (!r.Ok()) return false;

		_nextDocId = r.ReadU32();

		uint32_t metaCnt = r.ReadU32();
		if (!r.Ok()) return false;
		_docMeta.clear();
		_pathToDoc.clear();
		for (uint32_t i = 0; i < metaCnt; ++i)
		{
			DocId       id = r.ReadU32();
			std::string path = r.ReadStr();
			time_t      mtime = time_t(r.ReadU64());
			if (!r.Ok()) return false;
			_docMeta[id] = { path, mtime };
			_pathToDoc[_LowerPath(path)] = id;
		}

		uint32_t tokCnt = r.ReadU32();
		if (!r.Ok()) return false;
		_postings.clear();
		for (uint32_t i = 0; i < tokCnt; ++i)
		{
			std::string token = r.ReadStr();
			uint32_t    plSize = r.ReadU32();
			if (!r.Ok()) return false;

			PostingList pl;
			pl.reserve(plSize);
			for (uint32_t j = 0; j < plSize; ++j)
			{
				Posting p;
				p.docId = r.ReadU32();
				uint32_t lineCnt = r.ReadU32();
				if (!r.Ok()) return false;
				p.lines.reserve(lineCnt);
				for (uint32_t k = 0; k < lineCnt; ++k)
					p.lines.push_back(r.ReadI32());
				if (!r.Ok()) return false;
				pl.push_back(std::move(p));
			}
			_postings[token] = std::move(pl);
		}

		_deletedDocs.clear();
		_dirty = false;
		return r.Ok();
	}

	bool IsDirty()  const { return _dirty; }
	void ClearDirty() { _dirty = false; }

private:
	static std::string _LowerPath(const std::string& p)
	{
		std::string lp = p;
		std::transform(lp.begin(), lp.end(), lp.begin(),
			[](unsigned char c) { return char(::tolower(c)); });
		// 路径分隔符统一（Windows 下 \ → /）
		std::replace(lp.begin(), lp.end(), '\\', '/');
		return lp;
	}

	// 物理删除 _deletedDocs 中所有 docId 对应的 Posting 条目
	void _Compact()
	{
		if (_deletedDocs.empty()) return;
		for (auto& kv : _postings)
		{
			auto& pl = kv.second;
			pl.erase(
				std::remove_if(pl.begin(), pl.end(),
					[this](const Posting& p) {
				return _deletedDocs.count(p.docId) != 0;
			}),
				pl.end());
		}
		// 清除空 token
		for (auto it = _postings.begin(); it != _postings.end(); )
			it = it->second.empty() ? _postings.erase(it) : ++it;

		_deletedDocs.clear();
	}

	std::unordered_map<std::string, PostingList> _postings;
	std::unordered_map<DocId, DocMeta>           _docMeta;
	std::unordered_map<std::string, DocId>       _pathToDoc;
	std::unordered_set<DocId>                    _deletedDocs;
	DocId                                        _nextDocId;
	bool                                         _dirty;
};


// ============================================================
//  CDocIndexer::Impl
// ============================================================
class CDocIndexer::Impl
{
public:
	// ---- 任务 ----------------------------------------------
	struct Task
	{
		enum Type { Add, Remove } type;
		std::string filePath;
		time_t      mtime = 0;
		std::string content; // 仅 Add 时有效
	};

	// ---- 构造 / 析构 ---------------------------------------
	Impl() : _workerRunning(false), _isOpen(false) {}
	~Impl() { _StopWorker(); }

	// ---- Open / Close --------------------------------------
	bool Open(const char* indexPath)
	{
		std::lock_guard<std::mutex> lk(_indexMutex);
		_indexPath = indexPath;
		Utils::EnsureFolder(indexPath);
		_idxFilePath = _indexPath + "/index.didx";
		_TryLoad();
		_isOpen = true;
		_StartWorker();
		return true;
	}

	void Close()
	{
		_StopWorker();
		std::lock_guard<std::mutex> lk(_indexMutex);
		if (_isOpen && _index.IsDirty())
			_Save();
		_isOpen = false;
	}

	// ---- 异步入队 ------------------------------------------
	void EnqueueAdd(const std::string& fp, time_t mt, const std::string& content)
	{
		Task t;
		t.type = Task::Add;
		t.filePath = fp;
		t.mtime = mt;
		t.content = content;
		_PushTask(std::move(t));
	}

	void EnqueueRemove(const std::string& fp)
	{
		Task t;
		t.type = Task::Remove;
		t.filePath = fp;
		_PushTask(std::move(t));
	}

	// ---- 同步查询（无需等待队列）---------------------------
	time_t GetStoredMTime(const std::string& fp)
	{
		std::lock_guard<std::mutex> lk(_indexMutex);
		return _index.GetMTime(fp);
	}

	size_t GetDocumentCount()
	{
		std::lock_guard<std::mutex> lk(_indexMutex);
		return _index.GetDocumentCount();
	}

	// ---- FlushAndWait --------------------------------------
	bool FlushAndWait(int timeoutMs)
	{
		auto deadline = std::chrono::steady_clock::now()
			+ std::chrono::milliseconds(timeoutMs);
		while (true)
		{
			{ std::lock_guard<std::mutex> lk(_queueMutex); if (_taskQueue.empty()) break; }
			if (std::chrono::steady_clock::now() > deadline) return false;
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}
		std::lock_guard<std::mutex> lk(_indexMutex);
		if (_index.IsDirty()) _Save();
		return true;
	}

	// ---- Find ----------------------------------------------
	bool Find(const char* key, int maxResult, FindInFileResults& results)
	{
		if (!_isOpen || !key || key[0] == '\0') return false;

		// 最多等待 500 ms 让队列清空
		{
			const int maxWaitMs = 500, stepMs = 30;
			int waited = 0;
			while (waited < maxWaitMs)
			{
				bool empty = false;
				{ std::lock_guard<std::mutex> lk(_queueMutex); empty = _taskQueue.empty(); }
				if (empty) break;
				std::this_thread::sleep_for(std::chrono::milliseconds(stepMs));
				waited += stepMs;
			}
		}

		std::string keyStr(key);

		// 判断是否为纯 word-char key
		bool keyIsWord = true;
		for (char c : keyStr)
			if (!IsWordChar(c)) { keyIsWord = false; break; }

		// 小写 key 用于倒排表查找
		std::string lowerKey = keyStr;
		std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(),
			[](unsigned char c) { return char(::tolower(c)); });

		// 取候选文档列表（加锁保护索引读）
		std::vector<DocId> candidates;
		{
			std::lock_guard<std::mutex> lk(_indexMutex);
			candidates = _index.QueryCandidates(lowerKey, keyIsWord);
		}
		if (candidates.empty()) return true;

		results.Clear();
		int fileCount = 0;

		for (DocId docId : candidates)
		{
			if (fileCount >= maxResult) break;

			std::string filePath;
			{
				std::lock_guard<std::mutex> lk(_indexMutex);
				const std::string* pPath = _index.GetFilePath(docId);
				if (!pPath) continue;
				filePath = *pPath;
			}

			Utils::FileContentCodingFormat fmt;
			std::string content;
			if (!Utils::GetFileContentIntoUTF8(filePath.c_str(), content, fmt))
				continue;

			int added = _FindMatchingLines(
				filePath, keyStr, keyIsWord, content,
				results, maxResult - fileCount);
			if (added > 0) ++fileCount;
		}

		return true;
	}

private:
	// ---- 工作线程 ------------------------------------------
	void _StartWorker()
	{
		if (!_workerRunning.load())
		{
			_workerRunning.store(true);
			_workerThread = std::thread(&Impl::_WorkerLoop, this);
		}
	}

	void _StopWorker()
	{
		if (_workerRunning.load())
		{
			{ std::lock_guard<std::mutex> lk(_queueMutex); _workerRunning.store(false); }
			_queueCv.notify_all();
			if (_workerThread.joinable()) _workerThread.join();
		}
	}

	void _PushTask(Task&& t)
	{
		{ std::lock_guard<std::mutex> lk(_queueMutex); _taskQueue.push(std::move(t)); }
		_queueCv.notify_one();
	}

	void _WorkerLoop()
	{
		// 每次最多取 BATCH_SIZE 个任务一起处理；
		// 队列空闲 IDLE_FLUSH_MS 毫秒后自动刷盘
		static constexpr int BATCH_SIZE = 64;
		static constexpr int IDLE_FLUSH_MS = 2000;

		auto lastFlush = std::chrono::steady_clock::now();

		while (true)
		{
			std::vector<Task> batch;
			{
				std::unique_lock<std::mutex> lk(_queueMutex);
				_queueCv.wait_for(lk, std::chrono::milliseconds(200),
					[this] { return !_taskQueue.empty() || !_workerRunning.load(); });

				while (!_taskQueue.empty() && int(batch.size()) < BATCH_SIZE)
				{
					batch.push_back(std::move(_taskQueue.front()));
					_taskQueue.pop();
				}

				if (!_workerRunning.load() && _taskQueue.empty() && batch.empty())
					break;
			}

			// 写索引
			if (!batch.empty())
			{
				std::lock_guard<std::mutex> lk(_indexMutex);
				for (auto& t : batch)
				{
					if (t.type == Task::Add)
						_index.AddDocument(t.filePath, t.mtime, t.content);
					else
						_index.RemoveDocument(t.filePath);
				}
			}

			// 空闲刷盘
			bool qEmpty = false;
			{ std::lock_guard<std::mutex> lk(_queueMutex); qEmpty = _taskQueue.empty(); }
			if (qEmpty)
			{
				auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::steady_clock::now() - lastFlush).count();
				if (elapsed >= IDLE_FLUSH_MS || !_workerRunning.load())
				{
					std::lock_guard<std::mutex> lk(_indexMutex);
					if (_index.IsDirty()) _Save();
					lastFlush = std::chrono::steady_clock::now();
				}
			}
		}

		// 退出前最后刷盘
		std::lock_guard<std::mutex> lk(_indexMutex);
		if (_index.IsDirty()) _Save();
	}

	// ---- IO（调用方须持有 _indexMutex）--------------------
	void _Save()
	{
		std::vector<uint8_t> buf;
		_index.Serialize(buf);

		std::ofstream ofs(_idxFilePath, std::ios::binary | std::ios::trunc);
		if (!ofs.is_open()) return;
		if (!buf.empty())
			ofs.write(reinterpret_cast<const char*>(buf.data()),
				std::streamsize(buf.size()));
		_index.ClearDirty();
	}

	void _TryLoad()
	{
		std::ifstream ifs(_idxFilePath, std::ios::binary | std::ios::ate);
		if (!ifs.is_open()) return;
		std::streamsize sz = ifs.tellg();
		if (sz <= 0) return;
		ifs.seekg(0, std::ios::beg);

		std::vector<unsigned char> buf((size_t)sz);

		if (!ifs.read((char*)(buf.data()), sz)) 
			return;

		InvertedIndex tmp;
		if (tmp.Deserialize(buf.data(), buf.size()))
			_index = std::move(tmp);
		// 反序列化失败则保留空索引，下次重建
	}

	// ---- 行级精确匹配（区分大小写）------------------------
	// 返回向 results 新增的行数
	static int _FindMatchingLines(const std::string& filePath,
		const std::string& key,
		bool               keyIsWord,
		const std::string& content,
		FindInFileResults& results,
		int                maxLines)
	{
		if (maxLines <= 0) return 0;

		std::istringstream ss(content);
		std::string line;
		int lineNo = 0, added = 0;

		while (std::getline(ss, line) && added < maxLines)
		{
			++lineNo;
			size_t pos = 0;
			while ((pos = line.find(key, pos)) != std::string::npos)
			{
				bool match;
				if (!keyIsWord)
				{
					match = true; // 含特殊符号：子串即命中
				}
				else
				{
					bool leftOk = (pos == 0) || !IsWordChar(line[pos - 1]);
					bool rightOk = (pos + key.size() >= line.size()) ||
						!IsWordChar(line[pos + key.size()]);
					match = leftOk && rightOk;
				}

				if (match)
				{
					results.AddResult(filePath, lineNo, line);
					++added;
					break; // 同行只记录一次
				}
				++pos;
			}
		}
		return added;
	}

	// ---- 成员变量 ------------------------------------------
	std::string   _indexPath;
	std::string   _idxFilePath;
	bool          _isOpen;

	InvertedIndex _index;
	std::mutex    _indexMutex;

	std::thread             _workerThread;
	std::atomic<bool>       _workerRunning;
	std::queue<Task>        _taskQueue;
	std::mutex              _queueMutex;
	std::condition_variable _queueCv;
};


// ============================================================
//  CDocIndexer  —  公共接口
// ============================================================

CDocIndexer::CDocIndexer() : _impl(std::make_unique<Impl>()) {}
CDocIndexer::~CDocIndexer() = default;

bool CDocIndexer::Open(const char* indexPath)
{
	return _impl->Open(indexPath);
}

void CDocIndexer::Close()
{
	_impl->Close();
}

void CDocIndexer::AddDocument(const std::string& filePath,
	time_t             mtime,
	const std::string& utf8Content)
{
	_impl->EnqueueAdd(filePath, mtime, utf8Content);
}

void CDocIndexer::RemoveDocument(const std::string& filePath)
{
	_impl->EnqueueRemove(filePath);
}

time_t CDocIndexer::GetStoredMTime(const std::string& filePath)
{
	return _impl->GetStoredMTime(filePath);
}

bool CDocIndexer::FlushAndWait(int timeoutMs)
{
	return _impl->FlushAndWait(timeoutMs);
}

bool CDocIndexer::Find(const char* key, int maxResult, FindInFileResults& results)
{
	return _impl->Find(key, maxResult, results);
}

size_t CDocIndexer::GetDocumentCount() const
{
	return _impl->GetDocumentCount();
}

