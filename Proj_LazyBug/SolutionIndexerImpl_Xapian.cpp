#include "stdh.h"

#include "SolutionIndexerImpl_Xapian.h"
#include "Utils.h"
#include "SolutionDB.h"

#include <xapian.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <unordered_set>
#include <unordered_map>

// Xapian前缀常量
constexpr const char* PATH_PREFIX = "P";
constexpr const char* CONTENT_PREFIX = "C";
constexpr const char* FILENAME_PREFIX = "F";


// ==================== CSolutionIndexerImpl_Xapian 实现 ====================

CSolutionIndexerImpl_Xapian::CSolutionIndexerImpl_Xapian()
{
}

CSolutionIndexerImpl_Xapian::~CSolutionIndexerImpl_Xapian()
{
	StopWorkerThread();
	Close();
}

bool CSolutionIndexerImpl_Xapian::Open(const char* indexPath)
{
	try
	{
		_indexPath = indexPath;
		Utils::EnsureFolder(indexPath);

		// 打开或创建Xapian数据库
		_database = std::make_unique<Xapian::WritableDatabase>(indexPath, Xapian::DB_CREATE_OR_OPEN);

		// 初始化TermGenerator用于分词
//             _termGenerator.set_stemmer(Xapian::Stem("en"));

		return true;
	}
	catch (const Xapian::Error&)
	{
		// 处理错误
		return false;
	}
}

void CSolutionIndexerImpl_Xapian::Close()
{
	if (_database)
	{
		try
		{
			_database->commit();
		}
		catch (const Xapian::Error&)
		{
			// 处理错误
		}
		_database.reset();
	}
}

bool CSolutionIndexerImpl_Xapian::IsReady() const
{
	return _database != nullptr;
}

void CSolutionIndexerImpl_Xapian::ProcessSetContent(std::shared_ptr<std::vector<SolutionFile>> filesSnapshot)
{
	if (!filesSnapshot)
		return;

	// 收集当前索引中的所有文件路径
	std::unordered_set<std::string> indexedFiles;
	try
	{
		// 使用 allterms_begin 获取所有以 PATH_PREFIX 开头的 terms
		Xapian::TermIterator it = _database->allterms_begin(std::string(PATH_PREFIX));
		Xapian::TermIterator end = _database->allterms_end(std::string(PATH_PREFIX));
		for (; it != end; ++it)
		{
			// 对每个 term 获取对应的文档（每个路径 term 只对应一个文档）
			Xapian::PostingIterator pit = _database->postlist_begin(*it);
			if (pit != _database->postlist_end(*it))
			{
				Xapian::Document doc = _database->get_document(*pit);
				indexedFiles.insert(doc.get_data());
			}
		}
	}
	catch (const Xapian::Error&)
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
		time_t storedMTime = GetStoredMTime(lowerCasedFilePath);

		if (currentMTime != storedMTime)
		{
			// 需要更新（使用原始路径读取文件内容，使用小写路径存储索引）
			Utils::FileContentCodingFormat codingFmt;
			std::string content;
			if (Utils::GetFileContentIntoUTF8(lowerCasedFilePath.c_str(), content, codingFmt))
			{
				AddDocument(lowerCasedFilePath, currentMTime, content);
			}
		}
	}
}

void CSolutionIndexerImpl_Xapian::AddDocument(const std::string& lowerCasedFilePath, time_t mtime, const std::string& content)
{
	// 首先删除旧文档（如果存在）
	RemoveDocument(lowerCasedFilePath);

	// 创建新文档
	Xapian::Document doc;

	// lowerCasedFilePath 已经是小写路径，直接用于唯一标识
	// 添加路径作为唯一标识（前缀索引，不用于搜索）
	doc.add_boolean_term(std::string(PATH_PREFIX) + lowerCasedFilePath);

	// 存储修改时间
	doc.add_value(0, Xapian::sortable_serialise(static_cast<double>(mtime)));

	// 存储原始文件路径（用于显示）- 需要从完整路径提取
	// 注意：这里需要存储原始路径，但lowerCasedFilePath已经是小写路径
	// 我们通过从lowerCasedFilePath反推原始路径，或者修改接口传递原始路径
	// 暂时使用lowerCasedFilePath作为显示路径（因为Xapian存储的是小写路径，Find时会返回小写路径）
	doc.set_data(lowerCasedFilePath);

	// 使用TermGenerator索引内容
	_termGenerator.set_document(doc);

	// 索引文件名（用于文件名搜索）
	// lowerCasedFilePath 已经是小写路径，从中提取文件名也是小写的
	size_t lastSlash = lowerCasedFilePath.find_last_of("/\\");
	std::string fileName = (lastSlash != std::string::npos) ? lowerCasedFilePath.substr(lastSlash + 1) : lowerCasedFilePath;
	_termGenerator.index_text(fileName, 1, FILENAME_PREFIX);

	// 索引文件内容
	_termGenerator.index_text(content, 1, CONTENT_PREFIX);

	// 将文档添加到数据库
	_database->add_document(doc);
}

void CSolutionIndexerImpl_Xapian::RemoveDocument(const std::string& lowerCasedFilePath)
{
	// lowerCasedFilePath 已经是小写路径
	std::string term = std::string(PATH_PREFIX) + lowerCasedFilePath;

	try
	{
		Xapian::PostingIterator it = _database->postlist_begin(term);
		if (it != _database->postlist_end(term))
		{
			Xapian::docid docId = *it;
			_database->delete_document(docId);
		}
	}
	catch (const Xapian::Error&)
	{
		// 文档可能不存在，忽略错误
	}
}

void CSolutionIndexerImpl_Xapian::AddDocumentIfChanged(const std::string& lowerCasedFilePath, time_t mtime, const std::string& content)
{
	if (!_database)
		return;

	// 检查存储的修改时间
	time_t storedMTime = GetStoredMTime(lowerCasedFilePath);

	// 如果时间不一样，添加或更新文档
	if (mtime != storedMTime)
	{
		AddDocument(lowerCasedFilePath, mtime, content);
	}
}

bool CSolutionIndexerImpl_Xapian::Find(const char* key, int maxResult, FindInFileResults& results)
{
	if (!_database)
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
		_database->commit();

		// 创建查询解析器
		Xapian::QueryParser parser;
		//            parser.set_stemmer(Xapian::Stem("en"));
		parser.set_database(*_database);

		// 添加前缀支持
		parser.add_prefix("file", FILENAME_PREFIX);
		parser.add_prefix("content", CONTENT_PREFIX);

		// 默认搜索内容前缀
		Xapian::Query query = parser.parse_query(key, Xapian::QueryParser::FLAG_DEFAULT, CONTENT_PREFIX);

		// 创建查询器
		Xapian::Enquire enquire(*_database);
		enquire.set_query(query);

		// 获取结果
		Xapian::MSet mset = enquire.get_mset(0, maxResult * 5);

		results.Clear();
		int currentCount = 0;

		for (Xapian::MSetIterator it = mset.begin(); it != mset.end() && currentCount < maxResult; ++it)
		{
			Xapian::Document doc = it.get_document();
			std::string lowerCasedFilePath = doc.get_data();

			// 读取文件内容来查找匹配行
			Utils::FileContentCodingFormat codingFmt;
			std::string fileContent;
			if (Utils::GetFileContentIntoUTF8(lowerCasedFilePath.c_str(), fileContent, codingFmt))
			{
				currentCount += Utils::FindMatchingLines(lowerCasedFilePath, key, fileContent, results, maxResult - currentCount);
			}
		}

		return true;
	}
	catch (const Xapian::Error&)
	{
		return false;
	}
}

time_t CSolutionIndexerImpl_Xapian::GetStoredMTime(const std::string& lowerCasedFilePath)
{
	if (!_database)
		return 0;

	// lowerCasedFilePath 已经是小写路径
	std::string term = std::string(PATH_PREFIX) + lowerCasedFilePath;

	try
	{
		Xapian::PostingIterator it = _database->postlist_begin(term);
		if (it != _database->postlist_end(term))
		{
			Xapian::Document doc = _database->get_document(*it);
			std::string value = doc.get_value(0);
			if (!value.empty())
			{
				return static_cast<time_t>(Xapian::sortable_unserialise(value));
			}
		}
	}
	catch (const Xapian::Error&)
	{
		// 忽略错误
	}

	return 0;
}

void CSolutionIndexerImpl_Xapian::ProcessUpdateIfExists(const std::string& lowerCasedFilePath)
{
	try
	{
		// 先提交之前的修改，确保索引状态是最新的
		if (_database)
		{
			_database->commit();
		}

		// 检查文件是否已被索引
		time_t storedMTime = GetStoredMTime(lowerCasedFilePath);
		bool fileExistsInIndex = (storedMTime != 0);

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
				AddDocument(lowerCasedFilePath, currentMTime, content);
			}
		}
	}
	catch (const Xapian::Error&)
	{
		// 忽略错误
	}
}

