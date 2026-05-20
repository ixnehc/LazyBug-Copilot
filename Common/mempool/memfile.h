#pragma once

#include <assert.h>

#include "mempool.h"

#include <map>
#include <deque>

class CFileMemPool
{
public:
	CFileMemPool()
	{
		_hFile=NULL;

		_szCacheLimit=10;
	}

	BOOL Init(const char *pathSwapFile,DWORD cachelimit=10);
	void Clear();//will delete the swap file

	void Reset(std::vector<DWORD>&poolunits);//re-create pools using the given unit sizes
	void Reset();//empty all the existing pools
	MemHandle Alloc(DWORD sz);
	void Free(MemHandle h);
	void *GetPtr(MemHandle h);
	void *QueryPtr(MemHandle h);

protected:
	struct Entry
	{
		DWORD low;
		DWORD high;
	};
	struct Pool
	{
		DWORD szUnit;
		std::deque<DWORD> frees;
	};
	struct Cache
	{
		Entry e;
		void *buf;
		DWORD szBuf;
		BOOL bDirty;
    };

	void _ClearContent(BOOL bClearPoolStruct);
	Cache *_ObtainCache(MemHandle h);
	BOOL _LoadCache(Entry &e,DWORD sz,Cache &c);
	BOOL _UnloadCache(Cache &c,BOOL bDiscard);
	std::vector<Pool*>_pools;

	std::vector<Entry>_entries;

	std::deque<MemHandle> _queueCache;
	std::map<MemHandle,Cache> _mapCache;

	HANDLE _hFile;

	DWORD _szCacheLimit;
	std::string _path;




};

