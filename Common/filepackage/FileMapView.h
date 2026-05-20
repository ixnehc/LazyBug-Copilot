#ifndef __FileMapView_H__
#define __FileMapView_H__

const DWORD PACKAGE_MAPVIEW_SIZE	= 64 * 1024 * 1024;	// 500MB

class CFileMapView
{
public:
	CFileMapView(HANDLE hMapFile = NULL) : _hMapFile(hMapFile), _lpMapAddress(NULL)
	{
		_liBasePosition = 0;
		_liViewSize = PACKAGE_MAPVIEW_SIZE;
		_liMapFileSize = 0;

		SYSTEM_INFO si;
		::GetSystemInfo(&si);
		_dwAllocGranula = si.dwAllocationGranularity;
	}

	~CFileMapView()
	{
		Close();
	}

public:
	void SetUp(HANDLE hMapFile, ULONG64 liMapFileSize, DWORD dwDesiredAccess)
	{
		_hMapFile = hMapFile;
		_liMapFileSize = liMapFileSize;
		_dwDesiredAccess = dwDesiredAccess;
	}

public:
	ULONG64 Read(void* lpBuf, ULONG64 position, ULONG64 size)
	{
		ULONG64 bytesRead = 0;
		if (Open(position, size))
		{
			ULONG64 dist = position - _liBasePosition;
			char* lpBaseAddress = reinterpret_cast<char*>(_lpMapAddress);
			lpBaseAddress += dist;
			memcpy(lpBuf, lpBaseAddress, (size_t) size);
			bytesRead = size;
		}
		return bytesRead;
	}

	ULONG64 Write(const void* lpBuf, ULONG64 position, ULONG64 size)
	{
		ULONG64 bytesWritten = 0;
		if (Open(position, size))
		{
			ULONG64 dist = position - _liBasePosition;
			char* lpBaseAddress = reinterpret_cast<char*>(_lpMapAddress);
			lpBaseAddress += dist;
			memcpy(lpBaseAddress, lpBuf, (size_t) size);
			::FlushViewOfFile(lpBaseAddress, (SIZE_T) size);
			bytesWritten = size;			
		}
		return bytesWritten;
	}

public:
	inline BOOL Open(LONG64 position, ULONG64 size)
	{
		if (IsLoaded(position, size))
		{
			return TRUE;
		}

		// Release the old view
		Close();

		_liBasePosition = (position / _dwAllocGranula) * _dwAllocGranula;
		_liViewSize = (position - _liBasePosition) + size;
		if (_liViewSize < PACKAGE_MAPVIEW_SIZE)
			_liViewSize = PACKAGE_MAPVIEW_SIZE;
		ULONG64 viewSize = _liMapFileSize - _liBasePosition;
		if (_liViewSize > viewSize)
			_liViewSize = viewSize;

		LARGE_INTEGER li;
		li.QuadPart = _liBasePosition;
		_lpMapAddress = ::MapViewOfFile(_hMapFile, _dwDesiredAccess, 
			li.HighPart, li.LowPart, (SIZE_T) _liViewSize);

		return (_lpMapAddress != NULL);
	}

	inline void Close()
	{
		if (_lpMapAddress)
		{
			//::FlushViewOfFile(_lpMapAddress, (SIZE_T) _liViewSize);

			::UnmapViewOfFile(_lpMapAddress);
			_lpMapAddress = NULL;
		}
	}
	inline BOOL IsLoaded(LONG64 position, ULONG64 size)
	{
		if (_lpMapAddress)
		{
			if (position<_liBasePosition)
				return FALSE;
			ULONG64 viewSize = (position - _liBasePosition) + size;
			if (_liViewSize > viewSize)
			{
				return TRUE;
			}
		}
		return FALSE;
	}

private:
	HANDLE _hMapFile;
	LPVOID _lpMapAddress;
	DWORD _dwDesiredAccess;
	DWORD _dwAllocGranula;
	ULONG64 _liBasePosition;
	ULONG64 _liViewSize;
	ULONG64 _liMapFileSize;
};
#endif