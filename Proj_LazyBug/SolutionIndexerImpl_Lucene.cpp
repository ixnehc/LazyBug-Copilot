#include "stdh.h"
#include "SolutionIndexerImpl_Lucene.h"
#include "Utils.h"
#include "utils_findinfile.h"
#include "SolutionDB.h"

#include <sstream>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_set>
#include <unordered_map>

// 将字符串中的点号替换为#（用于搜索关键词预处理）
static void PreParseProcess(std::string& str)
{
	for (char& c : str)
	{
		if (c == '.')
			c = '#';
	}
}

// 将字符串中的双引号转义（" -> \"）
static void EscapeQuote(std::string& str)
{
	{
		size_t pos = 0;
		while ((pos = str.find('\\', pos)) != std::string::npos)
		{
			str.replace(pos, 1, "\\\\");
			pos += 2; // 跳过插入的 \"，避免无限循环
		}
	}
	{
		size_t pos = 0;
		while ((pos = str.find('"', pos)) != std::string::npos)
		{
			str.replace(pos, 1, "\\\"");
			pos += 2; // 跳过插入的 \"，避免无限循环
		}
	}
}

// Lucene字段名常量
const Lucene::String CSolutionIndexerImpl_Lucene::FIELD_PATH = L"path";
const Lucene::String CSolutionIndexerImpl_Lucene::FIELD_CONTENT = L"content";
const Lucene::String CSolutionIndexerImpl_Lucene::FIELD_FILENAME = L"filename";
const Lucene::String CSolutionIndexerImpl_Lucene::FIELD_MTIME = L"mtime";

// ==================== CSolutionIndexerImpl_Lucene 实现 ====================

CSolutionIndexerImpl_Lucene::CSolutionIndexerImpl_Lucene()
{
}

CSolutionIndexerImpl_Lucene::~CSolutionIndexerImpl_Lucene()
{
	StopWorkerThread();
	Close();
}

Lucene::String CSolutionIndexerImpl_Lucene::ToLuceneString(const std::string& str)
{
	if (str.empty())
		return Lucene::EmptyString;

	// UTF-8 to UTF-16 (wstring)
	int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
	if (size <= 0)
		return Lucene::EmptyString;

	std::wstring wstr(size - 1, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wstr[0], size);
	return Lucene::String(wstr);
}

std::string CSolutionIndexerImpl_Lucene::FromLuceneString(const Lucene::String& str)
{
	if (str.empty())
		return "";

	// UTF-16 to UTF-8
	int size = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if (size <= 0)
		return "";

	std::string result(size - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.c_str(), -1, &result[0], size, nullptr, nullptr);
	return result;
}

bool CSolutionIndexerImpl_Lucene::Open(const char* indexPath)
{
	try
	{
		_indexPath = indexPath;
		Utils::EnsureFolder(indexPath);

		// 打开或创建Lucene目录
		Lucene::String indexPathW = ToLuceneString(indexPath);
		_directory = Lucene::FSDirectory::open(indexPathW);

		// 创建标准分析器
		_analyzer = Lucene::newLucene<Lucene::StandardAnalyzer>(Lucene::LuceneVersion::LUCENE_CURRENT);

		// 检查索引是否已存在
		bool indexExists = Lucene::IndexReader::indexExists(_directory);

		// 创建IndexWriter
		// create参数：false表示打开已有索引追加，true表示创建新索引（会清空已有数据）
		bool create = !indexExists;  // 只有当索引不存在时才创建
		_indexWriter = Lucene::newLucene<Lucene::IndexWriter>(_directory, _analyzer, create, 
			Lucene::IndexWriter::MaxFieldLengthUNLIMITED);
		_indexWriter->setMaxFieldLength(50000);

		int c = _indexWriter->numDocs();

		_indexWriter->commit();

		c = _indexWriter->numDocs();

		// 重要：如果是打开已存在的索引，不要在这里调用commit()
		// 直接调用commit()会导致IndexWriter将当前空的会话状态写入磁盘，清空原有文档
		// IndexWriter在追加模式下会自动识别已有文档，只需要在添加/删除文档后再commit

		return true;
	}
	catch (const Lucene::LuceneException&)
	{
		// 处理错误
		return false;
	}
}

void CSolutionIndexerImpl_Lucene::Close()
{
	if (_indexWriter)
	{
		try
		{
			_indexWriter->commit();
			_indexWriter->close();
		}
		catch (const Lucene::LuceneException&)
		{
			// 处理错误
		}
		_indexWriter.reset();
	}

	if (_directory)
	{
		_directory->close();
		_directory.reset();
	}

	_analyzer.reset();
}

bool CSolutionIndexerImpl_Lucene::IsReady() const
{
	return _indexWriter != nullptr;
}

void CSolutionIndexerImpl_Lucene::ProcessSetContent(std::shared_ptr<std::vector<SolutionFile>> filesSnapshot)
{
	if (!filesSnapshot)
		return;

	// 收集当前索引中的所有文件路径，同时复用 reader 用于 GetStoredMTime
	std::unordered_set<std::string> indexedFiles;
	Lucene::IndexReaderPtr reader;
	try
	{
		if (Lucene::IndexReader::indexExists(_directory))
		{
			reader = Lucene::IndexReader::open(_directory);
			int32_t maxDoc = reader->maxDoc();
			for (int32_t i = 0; i < maxDoc; ++i)
			{
				if (!reader->isDeleted(i))
				{
					Lucene::DocumentPtr doc = reader->document(i);
					Lucene::String pathW = doc->get(FIELD_PATH);
					if (!pathW.empty())
					{
						indexedFiles.insert(FromLuceneString(pathW));
					}
				}
			}
		}
	}
	catch (const Lucene::LuceneException&)
	{
		// 忽略错误
	}

	// 获取当前文件列表（使用小写路径作为索引键）
	std::unordered_set<std::string> currentFiles;
	for (const auto& file : *filesSnapshot)
	{
		if (Utils::CheckFileBinary(file.lowerCasedFilePath.c_str()))
			continue;
		currentFiles.insert(file.lowerCasedFilePath);
	}

	// 找出需要删除的文件（在索引中但不在当前文件列表中）
	for (const auto& indexedFile : indexedFiles)
	{
		if (currentFiles.find(indexedFile) == currentFiles.end())
		{
			RemoveDocument(indexedFile);
		}
	}

	// 处理每个文件：检查是否需要更新
	for (const auto& lowerCasedFilePath : currentFiles)
	{
		time_t currentMTime = Utils::GetFileTimeT(lowerCasedFilePath.c_str());
		time_t storedMTime = GetStoredMTime(reader, lowerCasedFilePath);

		if (lowerCasedFilePath == "s:\\tal\\code\\games\\farcry\\game\\rclient\\rclient_agent.cpp")
		{
			int v = 0;
			v++;
		}

		if (currentMTime != storedMTime)
		{
			// 需要更新（使用原始路径读取文件内容，使用小写路径存储索引）
			Utils::FileContentCodingFormat codingFmt;
			std::string content;
			if (Utils::GetFileContentIntoUTF8(lowerCasedFilePath.c_str(), content, codingFmt))
			{
				PreParseProcess(content);
				AddDocument(lowerCasedFilePath, currentMTime, content);
			}
		}
	}

	if (reader)
		reader->close();
}

void CSolutionIndexerImpl_Lucene::AddDocument(const std::string& lowerCasedFilePath, time_t mtime, const std::string& content)
{
	if (lowerCasedFilePath == "d:\\lazybug\\proj_lazybug\\chatdialog.cpp")
	{
		int v = 0;
		v++;
	}
	// 首先删除旧文档（如果存在）
	RemoveDocument(lowerCasedFilePath);

	// 创建新文档
	Lucene::DocumentPtr doc = Lucene::newLucene<Lucene::Document>();

	// 添加路径字段（存储并索引，不用于搜索内容）
	// lowerCasedFilePath 已经是小写路径
	Lucene::String lowerPathW = ToLuceneString(lowerCasedFilePath);
	doc->add(Lucene::newLucene<Lucene::Field>(FIELD_PATH, lowerPathW,
		Lucene::Field::STORE_YES, Lucene::Field::INDEX_NOT_ANALYZED));

	// 存储修改时间
	std::stringstream mtimeStream;
	mtimeStream << mtime;
	Lucene::String mtimeW = ToLuceneString(mtimeStream.str());
	doc->add(Lucene::newLucene<Lucene::Field>(FIELD_MTIME, mtimeW,
		Lucene::Field::STORE_YES, Lucene::Field::INDEX_NO));

	// 提取文件名
	size_t lastSlash = lowerCasedFilePath.find_last_of("/\\");
	std::string fileName = (lastSlash != std::string::npos) ? lowerCasedFilePath.substr(lastSlash + 1) : lowerCasedFilePath;
	Lucene::String fileNameW = ToLuceneString(fileName);

	// 添加文件名字段（索引，用于文件名搜索）
	doc->add(Lucene::newLucene<Lucene::Field>(FIELD_FILENAME, fileNameW,
		Lucene::Field::STORE_YES, Lucene::Field::INDEX_ANALYZED));

	// 添加内容字段（索引但不存储，因为文件内容可能很大）
	Lucene::String contentW = ToLuceneString(content);
	doc->add(Lucene::newLucene<Lucene::Field>(FIELD_CONTENT, contentW,
		Lucene::Field::STORE_NO, Lucene::Field::INDEX_ANALYZED));

	// 将文档添加到索引
	_indexWriter->addDocument(doc);

}

void CSolutionIndexerImpl_Lucene::RemoveDocument(const std::string& lowerCasedFilePath)
{
	// lowerCasedFilePath 已经是小写路径
	Lucene::String lowerPathW = ToLuceneString(lowerCasedFilePath);

	try
	{
		// 创建Term用于查找和删除文档
		Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(FIELD_PATH, lowerPathW);
		_indexWriter->deleteDocuments(term);
	}
	catch (const Lucene::LuceneException&)
	{
		// 文档可能不存在，忽略错误
	}
}

void CSolutionIndexerImpl_Lucene::AddDocumentIfChanged(const std::string& lowerCasedFilePath, time_t mtime, const std::string& content)
{
	if (!_indexWriter)
		return;

	// 打开索引读取器以检查存储的修改时间
	time_t storedMTime = 0;
	Lucene::IndexReaderPtr reader;

	if (Lucene::IndexReader::indexExists(_directory))
	{
		reader = Lucene::IndexReader::open(_directory);
		storedMTime = GetStoredMTime(reader, lowerCasedFilePath);
		reader->close();
	}

	// 如果时间不一样，添加或更新文档
	if (mtime != storedMTime)
	{
		std::string newContent = content;
		PreParseProcess(newContent);
		AddDocument(lowerCasedFilePath, mtime, newContent);
	}
}

bool CSolutionIndexerImpl_Lucene::Find(const char* key, int maxResult, FindInFileResults& results)
{
	if (!_indexWriter)
		return false;

	// 检查队列中是否有未完成的任务，最多等待500ms
	{
		const int maxWaitMs = 500;
		const int checkIntervalMs = 50;
		int waitedMs = 0;

		while (waitedMs < maxWaitMs)
		{
			bool isEmpty = false;
			{
				std::lock_guard<std::mutex> lock(_queueMutex);
				isEmpty = _taskQueue.empty();
			}

			if (isEmpty)
			{
				// 队列为空，可以开始搜索
				break;
			}

			// 队列不为空，等待一段时间后再检查
			std::this_thread::sleep_for(std::chrono::milliseconds(checkIntervalMs));
			waitedMs += checkIntervalMs;
		}

		// 最后检查一次队列
		{
			std::lock_guard<std::mutex> lock(_queueMutex);
			if (!_taskQueue.empty())
			{
				// 500ms后仍有任务，返回false
				return false;
			}
		}
	}

	try
	{
		// 提交所有待处理的更改
		_indexWriter->commit();

		// 打开索引读取器
		Lucene::IndexReaderPtr reader = Lucene::IndexReader::open(_directory,true);
		int totalDocs = reader->numDocs();  // 返回索引中的实际文档数
		int maxDocs = reader->maxDoc();     // 返回最大文档ID

		// 创建搜索器
		Lucene::IndexSearcherPtr searcher = Lucene::newLucene<Lucene::IndexSearcher>(reader);

		// 创建查询解析器 - 搜索 content 和 filename 字段
		Lucene::String fields[] = { FIELD_CONTENT, FIELD_FILENAME };
		Lucene::Collection<Lucene::String> fieldCollection = Lucene::Collection<Lucene::String>::newInstance();
		fieldCollection.add(FIELD_CONTENT);
		fieldCollection.add(FIELD_FILENAME);

		Lucene::QueryParserPtr parser = Lucene::newLucene<Lucene::MultiFieldQueryParser>(
			Lucene::LuceneVersion::LUCENE_CURRENT,
			fieldCollection,
			_analyzer);

		parser->setDefaultOperator(Lucene::QueryParser::AND_OPERATOR);

		std::string key2 = key;
		PreParseProcess(key2);
		EscapeQuote(key2);

		// 解析查询
		Lucene::String keyW = ToLuceneString(key2.c_str());
		keyW = Lucene::String(L"\"") + keyW + Lucene::String(L"\"");

		Lucene::QueryPtr query = parser->parse(keyW);

		// 执行搜索
		Lucene::TopScoreDocCollectorPtr collector = Lucene::TopScoreDocCollector::create(maxResult * 5, true);
		searcher->search(query, collector);
		Lucene::Collection<Lucene::ScoreDocPtr> hits = collector->topDocs()->scoreDocs;

		results.Clear();
		int currentCount = 0;

		for (int32_t i = 0; i < hits.size() && currentCount < maxResult; ++i)
		{
			Lucene::DocumentPtr doc = searcher->doc(hits[i]->doc);
			Lucene::String pathW = doc->get(FIELD_PATH);

			std::string lowerCasedFilePath = FromLuceneString(pathW);
			if (lowerCasedFilePath.empty())
				continue;

			// 读取文件内容来查找匹配行
			Utils::FileContentCodingFormat codingFmt;
			std::string fileContent;
			if (Utils::GetFileContentIntoUTF8(lowerCasedFilePath.c_str(), fileContent, codingFmt))
			{
				currentCount += Utils::FindMatchingLines(lowerCasedFilePath, key, fileContent, results, maxResult - currentCount);
			}
		}

		// 关闭资源
		searcher->close();
		reader->close();

		return true;
	}
	catch (const Lucene::LuceneException&e)
	{
		return false;
	}
}

time_t CSolutionIndexerImpl_Lucene::GetStoredMTime(Lucene::IndexReaderPtr reader, const std::string& lowerCasedFilePath)
{
	if (!reader)
		return 0;

	// lowerCasedFilePath 已经是小写路径
	Lucene::String lowerPathW = ToLuceneString(lowerCasedFilePath);

	try
	{
		Lucene::TermPtr term = Lucene::newLucene<Lucene::Term>(FIELD_PATH, lowerPathW);
		Lucene::TermDocsPtr termDocs = reader->termDocs(term);

		time_t mtime = 0;
		if (termDocs->next())
		{
			Lucene::DocumentPtr doc = reader->document(termDocs->doc());
			Lucene::String mtimeW = doc->get(FIELD_MTIME);
			if (!mtimeW.empty())
			{
				std::string mtimeStr = FromLuceneString(mtimeW);
				mtime = static_cast<time_t>(std::stoll(mtimeStr));
			}
		}

		termDocs->close();
		return mtime;
	}
	catch (const Lucene::LuceneException&)
	{
		// 忽略错误
	}

	return 0;
}

void CSolutionIndexerImpl_Lucene::ProcessUpdateIfExists(const std::string& lowerCasedFilePath)
{
	try
	{
		// 先提交之前的修改，确保索引状态是最新的
		if (_indexWriter)
		{
			_indexWriter->commit();
		}

		// 检查文件是否已被索引
		Lucene::IndexReaderPtr reader;
		time_t storedMTime = 0;
		bool fileExistsInIndex = false;

		if (Lucene::IndexReader::indexExists(_directory))
		{
			reader = Lucene::IndexReader::open(_directory);
			storedMTime = GetStoredMTime(reader, lowerCasedFilePath);
			fileExistsInIndex = (storedMTime != 0);
			reader->close();
		}

		// 如果文件不在索引中，直接返回
		if (!fileExistsInIndex)
			return;

		// 获取当前文件的修改时间
		time_t currentMTime = Utils::GetFileTimeT(lowerCasedFilePath.c_str());

		// 如果时间不一样，更新索引
		if (currentMTime != storedMTime)
		{
			Utils::FileContentCodingFormat codingFmt;
			std::string content;
			if (Utils::GetFileContentIntoUTF8(lowerCasedFilePath.c_str(), content, codingFmt))
			{
				PreParseProcess(content);
				AddDocument(lowerCasedFilePath, currentMTime, content);
			}
		}
	}
	catch (const Lucene::LuceneException&)
	{
		// 忽略错误
	}
}

