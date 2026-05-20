#ifndef __FilePackage_H__
#define __FilePackage_H__
#include "FilePackageDefines.h"
#include "FilePackageFile.h"
#include "FilePackageSearchFilter.h"
#include "FileStream.h"

const char* const FILEPACKAGEEXT	= ".pack";

class CProgress;

class CFilePackage
{
	friend class CFilePackageFile;

public:
	CFilePackage();
	~CFilePackage();

public:
	BOOL Open(const char* pszPackage, CFileStream::OpenMode mode);
	void Close();

	BOOL IsExclusive() const
	{
		return (_exclusiveWriter.key != 0);
	}
	int GetOpenFileCount() const
	{
		return static_cast<int>(_openFileList.size());
	}

public:
	CFilePackageFile* CreateFile(const char* pszPath);
	CFilePackageFile* OpenFile(const char* pszPath);
	CFilePackageFile* OpenFile(DWORD pathHashKey);
	void CloseFile(CFilePackageFile* pFile);

	BOOL FileExists(const char* pszPath) const;
	BOOL GetFileSize(const char* pszPath,DWORD &sz) const;
	int SearchFiles(const char* pszPath, FilePathList& rSearchResult) const;

public:
	BOOL AddFile(const char* pszFileName, const char* pszPath);
	BOOL RemoveFile(const char* pszPath);
	BOOL ReplaceFile(const char* pszPath, const char* pszFileName);

public:
	static BOOL IsFilePackage(const char* pszPackage);
	static BOOL CleanupPackage(const char* pszPackage, CProgress* progress);
	
protected:
	PackageFileNode* _FindFileNodeByAbsPath(const char* pszAbsPath) const;
	PackageFileNode* _FindFileNodeByPath(const char* pszPath) const;
	PackageFileNode* _FindFileNodeByPosition(LONG64 dwPosition) const;

	PackageFileNode* _FindFileNodeByPathKey(const PathKey& key);
	void _RemoveFileNodeByPathKey(const PathKey& key, BOOL bRelease = TRUE);
	void _AddFileNode(PackageFileNode* node);

	BOOL _RemoveFile(PackageFileNode* node);

	BOOL _IsLastFile(PackageFileNode* node) const
	{
		return (node->pos == _fileHeader.last);
	}

	inline BOOL _CreateFilePackage();

	LONG64 _GetActualSize() const;

private:
	PathKey _exclusiveWriter;
	PackageFileNodeList _fileNodeList;
	CFileStream* _pStream;
	PackageFileHeader _fileHeader;
	CFilePackageSearchFilter* _pSearchFilter;
	std::vector<CFilePackageFile*> _openFileList;
};
#endif