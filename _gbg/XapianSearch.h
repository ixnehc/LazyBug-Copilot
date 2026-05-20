#pragma once
#include <xapian.h>
#include <string>
#include <vector>
#include <memory>

struct SearchResult 
{
	std::string filePath;
	std::string snippet;      // 匹配的文本片段
	int rank;                 // 相关性排名
	double score;             // 匹配分数
	int lineNumber;           // 行号 (从snippet推断)
};

class XapianFileIndex 
{
public:
	// 构造函数: indexPath 是索引存储目录
	explicit XapianFileIndex(const std::string& indexPath);
	~XapianFileIndex();

	// 索引操作
	void addFile(const std::string& filePath);
	void addFile(const std::string& filePath, const std::string& content);
	void removeFile(const std::string& filePath);
	void updateFile(const std::string& filePath, const std::string& content);

	// 批量索引 (更快)
	void beginBatchIndex();
	void commitBatchIndex();

	// 搜索
	std::vector<SearchResult> search(const std::string& query, int maxResults = 20);
	std::vector<SearchResult> searchWithSnippet(const std::string& query, int maxResults = 20);

	// 高级搜索
	std::vector<SearchResult> searchPrefix(const std::string& prefix, int maxResults = 20);
	std::vector<SearchResult> searchBoolean(const std::string& query, int maxResults = 20);

	// 索引维护
	void compactIndex();           // 压缩索引
	size_t getDocumentCount() const;

private:
	std::string _indexPath;
	std::unique_ptr<Xapian::WritableDatabase> _writeDb;
	std::unique_ptr<Xapian::Database> _readDb;
	Xapian::TermGenerator _termGenerator;
	std::unique_ptr<Xapian::Stem> _stemmer ;

	void initStemmer();
	std::string extractSnippet(const std::string& content, const Xapian::MSet& matches);
};