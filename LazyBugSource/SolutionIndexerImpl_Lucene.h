#pragma once
#include "SolutionIndexer.h"
#include "lucene++/LuceneHeaders.h"

// Lucene实现类
class CSolutionIndexerImpl_Lucene : public CSolutionIndexerImplBase
{
public:
	CSolutionIndexerImpl_Lucene();
	~CSolutionIndexerImpl_Lucene() override;

	bool Open(const char* indexPath) override;
	void Close() override;
	bool Find(const char* key, int maxResult, FindInFileResults& results) override;

protected:
	bool IsReady() const override;
	void AddDocument(const std::string& filePath, time_t mtime, const std::string& content) override;
	void RemoveDocument(const std::string& filePath) override;
	void AddDocumentIfChanged(const std::string& filePath, time_t mtime, const std::string& content) override;
	void ProcessSetContent(std::shared_ptr<std::vector<SolutionFile>> filesSnapshot) override;
	void ProcessUpdateIfExists(const std::string& lowerCasedFilePath) override;

private:
	time_t GetStoredMTime(Lucene::IndexReaderPtr reader, const std::string& filePath);
	
	// 将std::string转换为Lucene::String (wstring)
	static Lucene::String ToLuceneString(const std::string& str);
	// 将Lucene::String转换为std::string
	static std::string FromLuceneString(const Lucene::String& str);

	Lucene::IndexWriterPtr _indexWriter;
	Lucene::DirectoryPtr _directory;
	Lucene::AnalyzerPtr _analyzer;
	std::string _indexPath;
	
	// 字段名常量
	static const Lucene::String FIELD_PATH;
	static const Lucene::String FIELD_CONTENT;
	static const Lucene::String FIELD_FILENAME;
	static const Lucene::String FIELD_MTIME;
};

