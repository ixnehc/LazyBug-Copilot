#ifndef __FileStream_H__
#define __FileStream_H__

#ifndef USE_STDIO
#include "FileMapView.h"
#endif


class CFileStream
{
public:
	CFileStream();
	~CFileStream();

	enum OpenMode
	{
		OM_NONE = 0,
		OM_WRITE = 0x01,
		OM_READ = 0x02,
		OM_READ_AND_WRITE = OM_WRITE | OM_READ,
	};

public:
	BOOL Open(const char* pszFileName, OpenMode mode, BOOL bMapping = TRUE);
	BOOL IsOpen() const;
	BOOL IsReading() const;
	BOOL IsWriting() const;
	LONG64 Read(void* lpBuf, LONG64 size);
	LONG64 Write(const void* lpBuf, LONG64 size);
	LONG64 Append(const void* lpBuf, LONG64 size);
	void Skip(LONG64 count);
	void Seek(LONG64 position);
	LONG64 Tell(void) const;
	LONG64 GetSize() const;
	BOOL Eof(void) const;
	void Close(void);

public:
#ifndef USE_STDIO
	void OpenMapView(LONG64 postion, LONG64 size);
#endif

public:
#ifndef USE_STDIO
	static LONG64 SetFileCursor(HANDLE hFile, LONG64 distance, DWORD moveMethod);
#endif
	static BOOL FileExists(const char* pszFileName);
	static BOOL SetFileSize(const char* pszFileName, LONG64 newFileSize);
// 	static BOOL RenameFile(const char* pszFileName, const char* pszNewFileName);
// 	static BOOL RemoveFile(const char* pszFileName);

private:
#ifdef USE_STDIO
	FILE *_file;
#else
	HANDLE _hFile;
	HANDLE _hMapFile;
	CFileMapView _fileMapView;
#endif
	LONG64 _fileSize;
	OpenMode _openMode;
};
#endif