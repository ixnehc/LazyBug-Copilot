#pragma once
#include <xapian.h>

#include "SolutionIndexer.h"


// Xapian实现类
class CSolutionIndexerImpl_Xapian : public CSolutionIndexerImplBase
{
public:
	CSolutionIndexerImpl_Xapian();
	~CSolutionIndexerImpl_Xapian() override;

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
	time_t GetStoredMTime(const std::string& filePath);

	std::unique_ptr<Xapian::WritableDatabase> _database;
	Xapian::TermGenerator _termGenerator;
	std::string _indexPath;
};
