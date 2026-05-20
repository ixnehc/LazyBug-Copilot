/********************************************************************
	created:	2008/02/19
	created:	19:2:2008   13:35
	filename: 	f:\IxEngine\Common\FilePackage\FileStream.cpp
	file path:	f:\IxEngine\Common\FilePackage
	file base:	FileStream
	file ext:	cpp
	author:		szg
	
	purpose:	
*********************************************************************/
#include "stdh.h"
#include "FileStream.h"
#include <shellapi.h>

#ifdef USE_STDIO
#include <stdio.h>
#endif

//-----------------------------------------------------------------------
CFileStream::CFileStream() 
	: 
#ifdef USE_STDIO
	_file(NULL)
#else
	_hFile(INVALID_HANDLE_VALUE)
	, _hMapFile(NULL)
#endif
{
}
//-----------------------------------------------------------------------
CFileStream::~CFileStream()
{
	Close();
}
//-----------------------------------------------------------------------
BOOL CFileStream::Open(const char* pszFileName, OpenMode mode, BOOL bMapping)
{
#ifdef USE_STDIO
	if (_file== NULL)
	{
		_openMode = mode;

		const char*flag="rb";
		switch(mode)
		{
			case OM_READ:
				flag="rb";
				break;
			case OM_WRITE:
				flag="wb";
				break;
			case OM_READ_AND_WRITE:
				flag="wb+";
				break;
		}

		_file=fopen(pszFileName,flag);
		if (_file!= NULL)
		{
			LARGE_INTEGER li;
			fseek(_file,0,SEEK_END);
			li.LowPart = ftell(_file);
			fseek(_file,0,SEEK_SET);
			_fileSize = li.QuadPart;			

			return TRUE;
		}
	}
	return FALSE;
#else
	if (_hFile == INVALID_HANDLE_VALUE)
	{
		_openMode = mode;

		DWORD dwDesiredAccess = GENERIC_READ;
//		DWORD dwShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
		DWORD dwShareMode = FILE_SHARE_READ;
		DWORD dwMapProtect = PAGE_READONLY;
		DWORD dwMapDesiredAccess = FILE_MAP_READ;
		if (mode == OM_WRITE || mode == OM_READ_AND_WRITE)
		{
			dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
			dwShareMode = FILE_SHARE_READ;
			dwMapProtect = PAGE_READWRITE;
			dwMapDesiredAccess = FILE_MAP_ALL_ACCESS;
			bMapping=FALSE;//写的话,不要使用FileMapping
		}
		
		_hFile = ::CreateFile(pszFileName, dwDesiredAccess, dwShareMode, NULL, mode==OM_WRITE?CREATE_ALWAYS:OPEN_ALWAYS, 0, NULL);
		if (_hFile != INVALID_HANDLE_VALUE)
		{
			LARGE_INTEGER li;
			li.LowPart = ::GetFileSize(_hFile, (LPDWORD) &li.HighPart);
			_fileSize = li.QuadPart;			

			if (bMapping)
			{
				_hMapFile = ::CreateFileMapping(_hFile, NULL, dwMapProtect, 0, 0, 0);
				if (_hMapFile != NULL)
				{
					_fileMapView.SetUp(_hMapFile, _fileSize, dwMapDesiredAccess);
				}
			}
			
			return TRUE;
		}
	}
	return FALSE;
#endif
}
//-----------------------------------------------------------------------
BOOL CFileStream::IsOpen() const
{
#ifdef USE_STDIO
	return (_file!= NULL);
#else
	return (_hFile != INVALID_HANDLE_VALUE);
#endif
}
//-----------------------------------------------------------------------
BOOL CFileStream::IsReading() const
{
	return (_openMode & OM_READ);
}
//-----------------------------------------------------------------------
BOOL CFileStream::IsWriting() const
{
	return (_openMode & OM_WRITE);
}
//-----------------------------------------------------------------------
LONG64 CFileStream::Read(void* lpBuf, LONG64 size)
{
	LONG64 bytesRead = 0;
#ifdef USE_STDIO
	bytesRead=fread(lpBuf,1,(int)size,_file);
#else
	if (_hMapFile)
	{
		LONG64 curFileCursor = SetFileCursor(_hFile, 0, FILE_CURRENT);

		LONG64 leftSize = _fileSize - curFileCursor;
		LONG64 bytesToRead = size;
		if (bytesToRead > leftSize)
			bytesToRead = leftSize;
		
		bytesRead = _fileMapView.Read(lpBuf, curFileCursor, bytesToRead);
		if (bytesRead > 0)
		{
			curFileCursor += bytesRead;
			SetFileCursor(_hFile, curFileCursor, FILE_BEGIN);
			return bytesRead;
		}
	}	

	//if (_hFile != INVALID_HANDLE_VALUE)
	{
		::ReadFile(_hFile, lpBuf, (DWORD) size, (LPDWORD) &bytesRead, NULL);
	}

#endif
	return bytesRead;
}
//-----------------------------------------------------------------------
LONG64 CFileStream::Write(const void* lpBuf, LONG64 size)
{
	LONG64 bytesWritten = 0;
#ifdef USE_STDIO
	bytesWritten=fwrite(lpBuf,1,(int)size,_file);
#else
	if (_hMapFile)
	{
		LONG64 curFileCursor = SetFileCursor(_hFile, 0, FILE_CURRENT);

		LONG64 leftSize = _fileSize - curFileCursor;
		LONG64 bytesToWrite = size;
		if (bytesToWrite > leftSize)
			bytesToWrite = leftSize;
		
		bytesWritten = _fileMapView.Write(lpBuf, curFileCursor, size);
		if (bytesWritten > 0)
		{
			curFileCursor += bytesWritten;
			SetFileCursor(_hFile, curFileCursor, FILE_BEGIN);
			return bytesWritten;
		}		
	}
	//if (_hFile != INVALID_HANDLE_VALUE)
	{
		::WriteFile(_hFile, lpBuf, (DWORD) size, (LPDWORD) &bytesWritten, NULL);
		LARGE_INTEGER li;
		li.LowPart = ::GetFileSize(_hFile, (LPDWORD) &li.HighPart);
		_fileSize = li.QuadPart;			
	}
#endif
	return bytesWritten;
}
//-----------------------------------------------------------------------
LONG64 CFileStream::Append(const void* lpBuf, LONG64 size)
{
	LONG64 bytesWritten = 0;

#ifdef USE_STDIO
	fseek(_file,0,SEEK_END);
	bytesWritten=fwrite(lpBuf,1,(int)size,_file);
#else
	SetFileCursor(_hFile, 0, FILE_END);
	::WriteFile(_hFile, lpBuf, (DWORD) size, (LPDWORD) &bytesWritten, NULL);
#endif

	_fileSize += bytesWritten;
	return bytesWritten;
}
//-----------------------------------------------------------------------
void CFileStream::Skip(LONG64 count)
{
#ifdef USE_STDIO
	fseek(_file,(int)count,SEEK_CUR);
#else
	SetFileCursor(_hFile, count, FILE_CURRENT);
#endif
}
//-----------------------------------------------------------------------
void CFileStream::Seek(LONG64 position)
{
#ifdef USE_STDIO
	fseek(_file,(int)position,SEEK_SET);
#else
	SetFileCursor(_hFile, position, FILE_BEGIN);
#endif
}
//-----------------------------------------------------------------------
LONG64 CFileStream::Tell(void) const
{
#ifdef USE_STDIO
	return ftell(_file);
#else
	return SetFileCursor(_hFile, 0, FILE_CURRENT);
#endif
}
//-----------------------------------------------------------------------
LONG64 CFileStream::GetSize(void) const
{
	return _fileSize;
}
//-----------------------------------------------------------------------
BOOL CFileStream::Eof(void) const
{
	return (Tell() == GetSize());
}
//-----------------------------------------------------------------------
void CFileStream::Close(void)
{
#ifdef USE_STDIO
	fclose(_file);
	_file=NULL;
	_openMode=OM_NONE;
#else
	if (_hFile != INVALID_HANDLE_VALUE)
	{
		::CloseHandle(_hFile);
		_hFile = INVALID_HANDLE_VALUE;

		_fileMapView.Close();

		if (_hMapFile)
		{
			::CloseHandle(_hMapFile);
			_hMapFile = NULL;
		}

		_openMode = OM_NONE;
	}
#endif
}

#ifndef USE_STDIO
void CFileStream::OpenMapView(LONG64 postion, LONG64 size)
{
	if (!_fileMapView.IsLoaded(postion, size))
	{
		_fileMapView.Open(postion, size);
	}	
}
#endif

#ifndef USE_STDIO
// From msdn 'myFileSeek'
LONG64 CFileStream::SetFileCursor(HANDLE hFile, LONG64 distance, DWORD moveMethod)
{
	LARGE_INTEGER li;

	li.QuadPart = distance;

	li.LowPart = ::SetFilePointer (hFile, li.LowPart, &li.HighPart, moveMethod);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		li.QuadPart = -1;
	}

	return li.QuadPart;
}
#endif

BOOL CFileStream::FileExists(const char* pszFileName)
{
#ifdef USE_STDIO
	FILE *fl=fopen(pszFileName,"rb");
	if (!fl)
		return FALSE;
	fclose(fl);
	return TRUE;
#else
	return (INVALID_FILE_ATTRIBUTES != ::GetFileAttributes(pszFileName));
#endif
}

BOOL CFileStream::SetFileSize(const char* pszFileName, LONG64 newFileSize)
{
#ifdef USE_STDIO
	return FALSE;//just不支持
#else
	HANDLE hFile = ::CreateFile(pszFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER li;
		li.LowPart = ::GetFileSize(hFile, (LPDWORD) &li.HighPart);
		if (li.QuadPart > newFileSize)
		{
			SetFileCursor(hFile, newFileSize, FILE_BEGIN);
			::SetEndOfFile(hFile);
			::CloseHandle(hFile);
			return TRUE;
		}
	}
	return FALSE;
#endif
}

// BOOL CFileStream::RenameFile(const char* pszFileName, const char* pszNewFileName)
// {
// 	SHFILEOPSTRUCT fo;
// 	fo.hwnd = NULL;
// 	fo.wFunc = FO_RENAME;
// 	fo.pFrom = pszFileName;
// 	fo.pTo = pszNewFileName;
// 	fo.hNameMappings = NULL;
// 	fo.lpszProgressTitle = NULL;
// 	fo.fAnyOperationsAborted = FALSE;
// 	fo.fFlags = FOF_NOERRORUI | FOF_NOCONFIRMMKDIR | FOF_SILENT;
// 	return (0 == ::SHFileOperation(&fo));
// }

// BOOL CFileStream::RemoveFile(const char* pszFileName)
// {
// 	return ::DeleteFile(pszFileName);
// }