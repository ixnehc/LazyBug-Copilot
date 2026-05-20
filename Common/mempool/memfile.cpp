/********************************************************************
	created:	2007/4/19   9:32
	filename: 	e:\IxEngine\Common\mempool\memfile.cpp
	author:		cxi
	
	purpose:	a memory pool could be swap to a file
*********************************************************************/
#include "stdh.h"
#include "../commondefines/general_stl.h"

#include "memfile.h"

//////////////////////////////////////////////////////////////////////////
//CFileMemPool
BOOL CFileMemPool::Init(const char *pathSwapFile,DWORD cachelimit)
{
	HANDLE hFile;
	hFile = CreateFile(pathSwapFile,GENERIC_WRITE|GENERIC_READ,0,NULL,
						CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	if (hFile == INVALID_HANDLE_VALUE) 
		return FALSE;

	_hFile=hFile;
	_szCacheLimit=cachelimit;
	_path=pathSwapFile;

	//128 byte to 2M byte
	std::vector<DWORD>poolunits;
	for (DWORD i=7;i<=21;i++)
		poolunits.push_back(1<<i);
	Reset(poolunits);

	return TRUE;
}

//will delete the swap file
void CFileMemPool::Clear()
{
	_ClearContent(TRUE);

	//remove the swap file
	CloseHandle(_hFile);
	DeleteFile(_path.c_str());
	_path="";
	_hFile=NULL;
	_szCacheLimit=0;
}


void CFileMemPool::_ClearContent(BOOL bClearPoolStruct)
{
	if (bClearPoolStruct)
	{
		for (int i=0;i<_pools.size();i++)
			SAFE_DELETE(_pools[i]);
		_pools.clear();
	}
	else
	{
		for (int i=0;i<_pools.size();i++)
			_pools[i]->frees.clear();
	}

	_entries.clear();

	_queueCache.clear();

	std::map<MemHandle,Cache>::iterator it;
	for (it=_mapCache.begin();it!=_mapCache.end();it++)
		_UnloadCache((*it).second,TRUE);
	_mapCache.clear();

	if (_hFile)
		SetFileValidData(_hFile,0);
}

//re-create pools using the given unit sizes
void CFileMemPool::Reset(std::vector<DWORD>&poolunits0)
{
	_ClearContent(TRUE);

	assert(poolunits0.size()<16);

	std::vector<DWORD>poolunits;
	poolunits=poolunits0;


	VEC_ASCEND(poolunits,DWORD);

	_pools.resize(poolunits.size());

	for (int i=0;i<poolunits.size();i++)
	{
		_pools[i]=new Pool;
		_pools[i]->szUnit=poolunits[i];
	}
}

//empty all the existing pools
void CFileMemPool::Reset()
{
	_ClearContent(FALSE);
}

BOOL CFileMemPool::_LoadCache(Entry &e,DWORD sz,Cache &c)
{
	assert(_hFile);
	c.e=e;
	c.szBuf=sz;
	c.buf=new BYTE[sz];

	DWORD szRead;
	if (INVALID_SET_FILE_POINTER==SetFilePointer(_hFile,e.low,(PLONG)&e.high,FILE_BEGIN))
		return FALSE;
	if (FALSE==ReadFile(_hFile,c.buf,sz,&szRead,NULL))
	{
		SAFE_DELETE(c.buf);
		return FALSE;
	}

	c.bDirty=FALSE;

	return TRUE;
}

BOOL CFileMemPool::_UnloadCache(Cache &c,BOOL bDiscard)
{
	if(!bDiscard)
	{
		if (c.bDirty)
		{//need to save back to file
			DWORD szWritten;
			if (INVALID_SET_FILE_POINTER==SetFilePointer(_hFile,
				c.e.low,(PLONG)&c.e.high,FILE_BEGIN))
				return FALSE;
			if (FALSE==WriteFile(_hFile,c.buf,c.szBuf,&szWritten,NULL))
				return FALSE;
		}
	}

	SAFE_DELETE(c.buf);
	c.szBuf=0;
	c.e.high=0;
	c.e.low=0;

	return TRUE;
}


MemHandle CFileMemPool::Alloc(DWORD sz)
{
	if (!_hFile)
		return MemHandle_Null;
	for (int i=0;i<_pools.size();i++)
	{
		if (_pools[i]->szUnit>=sz)
		{
			MemHandle h;
			Pool *pool=_pools[i];
			if (pool->frees.size()>0)
			{
				h=MemHandle_Make(i,pool->frees[0]);
				pool->frees.pop_front();
			}
			else
			{
				LARGE_INTEGER sz;
				GetFileSizeEx(_hFile,&sz);
				Entry e;
				e.high=sz.HighPart;
				e.low=sz.LowPart;

				SetFilePointer(_hFile,0,0,FILE_END);

				std::vector<BYTE>buf;
				buf.resize(pool->szUnit);
				VEC_SET(buf,0);
				DWORD szWritten;
				if (FALSE==WriteFile(_hFile,buf.data(),buf.size(),&szWritten,NULL))
					return MemHandle_Null;

				h=MemHandle_Make(i,_entries.size());
				_entries.resize(_entries.size()+1);//increase by 1 entry
				assert(_entries.size()<0xfffffff);

				_entries[_entries.size()-1]=e;
			}
			return h;
		}
	}
	return MemHandle_Null;
}

void CFileMemPool::Free(MemHandle h)
{
	DWORD iPool=MemHandle_GetPool(h);
	assert(iPool<_pools.size());
	_pools[iPool]->frees.push_front(MemHandle_GetIdx(h));

	//remove from the cache
	std::map<MemHandle,Cache>::iterator it;
	it=_mapCache.find(h);
	if (it!=_mapCache.end())
	{
		VEC_REMOVE(_queueCache,h);

		_UnloadCache((*it).second,TRUE);//discard the cache

		_mapCache.erase(it);
	}
}

CFileMemPool::Cache *CFileMemPool::_ObtainCache(MemHandle h)
{
	if (h==NULL)
		return NULL;
	std::map<MemHandle,Cache>::iterator it;
	it=_mapCache.find(h);

	if (it!=_mapCache.end())
		return &(*it).second;
#pragma message("------------------------------need to move this memhandle to the end of the queue")

	if (_mapCache.size()>=_szCacheLimit)
	{//cache overflow,discard the oldest
		MemHandle hUnload;
		hUnload=_queueCache[0];

		it=_mapCache.find(hUnload);
		assert(it!=_mapCache.end());

		if (FALSE==_UnloadCache((*it).second,FALSE))
			return NULL;//fail to save back the cache to file

		_queueCache.pop_front();
		_mapCache.erase(it);
	}

	//now make a new cache
	DWORD idx=MemHandle_GetIdx(h);
	DWORD iPool=MemHandle_GetPool(h);

	Cache c;
	if (FALSE==_LoadCache(_entries[idx],_pools[iPool]->szUnit,c))
		return NULL;

	_queueCache.push_back(h);
	Cache *pc=&_mapCache[h];
	*pc=c;
	return pc;
}


void *CFileMemPool::GetPtr(MemHandle h)
{
	Cache *c=_ObtainCache(h);
	if (!c)
		return NULL;
	return c->buf;
}
void *CFileMemPool::QueryPtr(MemHandle h)
{
	Cache *c=_ObtainCache(h);
	if (!c)
		return NULL;
	c->bDirty=TRUE;
	return c->buf;
}
