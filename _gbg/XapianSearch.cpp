#include "stdh.h"

#include "xapiansearch.h"
#include <fstream>
#include <sstream>
#include <algorithm>

XapianFileIndex::XapianFileIndex(const std::string& indexPath)
	: _indexPath(indexPath) 
{

	// 创建或打开可写数据库
	_writeDb = std::make_unique<Xapian::WritableDatabase>(
		indexPath,
		Xapian::DB_CREATE_OR_OPEN
	);

	// 创建可读数据库用于搜索
	_readDb = std::make_unique<Xapian::Database>(*_writeDb);

	initStemmer();
}

XapianFileIndex::~XapianFileIndex() 
{
	if (_writeDb)
	{
		_writeDb->commit();
	}
}

void XapianFileIndex::initStemmer()
{
	// 使用英语词干提取器 (支持其他语言: english, french, german, etc.)
	_stemmer  = std::make_unique<Xapian::Stem>("english");
	_termGenerator.set_stemmer(*_stemmer );
}

void XapianFileIndex::addFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file)
	{
		return;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();

	addFile(filePath, buffer.str());
}

void XapianFileIndex::addFile(const std::string& filePath, const std::string& content)
{
	Xapian::Document doc;

	// 设置文档数据 (存储在索引中)
	doc.set_data(filePath + "\n" + content);

	// 添加文件路径作为可搜索字段
	doc.add_value(0, filePath);  // 值槽0: 文件路径

	// 配置词项生成器
	_termGenerator.set_document(doc);

	// 索引文件内容
	_termGenerator.index_text(content);

	// 索引文件路径 (方便按文件名搜索)
	_termGenerator.index_text(filePath, 1, "P");  // 前缀P表示路径字段

	// 添加唯一标识 (用于后续删除/更新)
	doc.add_term("U" + filePath);

	// 替换或添加文档
	std::string uniqueTerm = "U" + filePath;
	_writeDb->replace_document(uniqueTerm, doc);
}

void XapianFileIndex::removeFile(const std::string& filePath)
{
	std::string uniqueTerm = "U" + filePath;
	_writeDb->delete_document(uniqueTerm);
}

void XapianFileIndex::updateFile(const std::string& filePath, const std::string& content)
{
	// 与addFile相同，replace_document会自动处理
	addFile(filePath, content);
}

void XapianFileIndex::beginBatchIndex()
{
	// 开始事务
	_writeDb->begin_transaction();
}

void XapianFileIndex::commitBatchIndex()
{
	// 提交事务
	_writeDb->commit_transaction();
	// 重新打开读数据库以看到更新
	_readDb->reopen();
}

std::vector<SearchResult> XapianFileIndex::search(const std::string& query, int maxResults)
{
	std::vector<SearchResult> results;

	// 创建查询解析器
	Xapian::QueryParser queryParser;
	queryParser.set_stemmer(*_stemmer );
	queryParser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
	queryParser.add_prefix("path", "P");  // path: 前缀搜索路径

	// 解析查询
	Xapian::Query parsedQuery = queryParser.parse_query(query);

	// 执行搜索
	Xapian::Enquire enquire(*_readDb);
	enquire.set_query(parsedQuery);

	// 获取结果
	Xapian::MSet matches = enquire.get_mset(0, maxResults);

	for (auto it = matches.begin(); it != matches.end(); ++it)
	{
		Xapian::Document doc = it.get_document();
		std::string data = doc.get_data();

		// 解析数据 (第一行是路径，剩余是内容)
		size_t newlinePos = data.find('\n');
		std::string path = data.substr(0, newlinePos);
		std::string content = (newlinePos != std::string::npos) ?
			data.substr(newlinePos + 1) : "";

		SearchResult result;
		result.filePath = path;
		result.snippet = content.substr(0, 200);  // 前200字符作为摘要
		result.score = it.get_percent();
		result.rank = it.get_rank();

		results.push_back(result);
	}

	return results;
}

std::vector<SearchResult> XapianFileIndex::searchWithSnippet(const std::string& query, int maxResults)
{
	std::vector<SearchResult> results;

	Xapian::QueryParser queryParser;
	queryParser.set_stemmer(*_stemmer );
	Xapian::Query parsedQuery = queryParser.parse_query(query);

	Xapian::Enquire enquire(*_readDb);
	enquire.set_query(parsedQuery);

	Xapian::MSet matches = enquire.get_mset(0, maxResults);

	for (auto it = matches.begin(); it != matches.end(); ++it)
	{
		Xapian::Document doc = it.get_document();
		std::string data = doc.get_data();

		size_t newlinePos = data.find('\n');
		std::string path = data.substr(0, newlinePos);
		std::string content = (newlinePos != std::string::npos) ?
			data.substr(newlinePos + 1) : "";

		SearchResult result;
		result.filePath = path;
		result.score = it.get_percent();
		result.rank = it.get_rank();

		// 提取包含查询词的片段
		result.snippet = extractSnippet(content, matches);

		results.push_back(result);
	}

	return results;
}

std::string XapianFileIndex::extractSnippet(const std::string& content, const Xapian::MSet& matches)
{
	// 简化版: 返回包含匹配词的前后100字符
	// 实际可以用 Xapian::MSet::snippet() 功能

	if (content.length() <= 200)
	{
		return content;
	}

	// 找到第一个匹配位置
	// 实际实现需要跟踪词项位置
	return content.substr(0, 200) + "...";
}

std::vector<SearchResult> XapianFileIndex::searchPrefix(const std::string& prefix, int maxResults)
{
	std::vector<SearchResult> results;

	// 前缀查询
	Xapian::Query query(Xapian::Query::OP_WILDCARD, prefix);

	Xapian::Enquire enquire(*_readDb);
	enquire.set_query(query);

	Xapian::MSet matches = enquire.get_mset(0, maxResults);


	for (auto it = matches.begin(); it != matches.end(); ++it)
	{
		Xapian::Document doc = it.get_document();
		std::string data = doc.get_data();

		size_t newlinePos = data.find('\n');
		SearchResult result;
		result.filePath = data.substr(0, newlinePos);
		result.score = it.get_percent();
		results.push_back(result);
	}

	return results;
}

void XapianFileIndex::compactIndex()
{
	// 提交当前更改
	_writeDb->commit();

	// 压缩到临时目录
	std::string tmpPath = _indexPath + ".compact";
	_writeDb->compact(tmpPath, Xapian::DBCOMPACT_MULTIPASS);

	// 关闭数据库
	_writeDb->close();

	// 删除原索引目录
	std::string cmd = "rmdir /s /q \"" + _indexPath + "\"";
	system(cmd.c_str());

	// 重命名压缩后的目录为原索引目录
	std::string cmd2 = "move /y \"" + tmpPath + "\" \"" + _indexPath + "\"";
	system(cmd2.c_str());

	// 重新打开
	_writeDb = std::make_unique<Xapian::WritableDatabase>(_indexPath, Xapian::DB_OPEN);
	_readDb = std::make_unique<Xapian::Database>(*_writeDb);
}

size_t XapianFileIndex::getDocumentCount() const
{
	return _readDb->get_doccount();
}