/********************************************************************
	created:	2010/8/13   11:14
	file path:	d:\IxEngine\Common\mempool
	author:		chenxi
	
	purpose:	专为class使用的内存池
*********************************************************************/
#pragma once


#include <vector>
#include <deque>
#include <unordered_set>

#include <assert.h>

#include <stdio.h>

#include "../datapacket/DataPacket.h"


class CClassPoolRawBase
{
public:
	virtual int GetSize()=0;
	virtual void *Alloc(void *owner)=0;
	virtual void Free(void*p)=0;
	virtual void CollectStat(DWORD &nUsed,DWORD &nAlloc)=0;
	virtual void **GetPtrs(void *owner,DWORD &count)=0;
};

template <int T_size>
class CClassPoolRaw:public CClassPoolRawBase
{
public:
	virtual int GetSize()	{		return T_size;	}

	virtual void* Alloc(void *owner)
	{
		Node* p;
		if (_frees.size()>0)
		{
			p=*_frees.begin();
			_frees.pop_front();
		}
		else
		{
			_buf.resize(_buf.size()+1);//increase buffer by 1 unit
			p=(Node*)&_buf.back();
		}
		((Node*)p)->owner=owner;
		return (void*)(((Node*)p));
	}
	virtual void Free(void*p)
	{
		Node *node=(Node *)p;
		assert(node->owner);
		node->owner=NULL;
		_frees.push_front(node);
	}

	virtual void CollectStat(DWORD &nUsed,DWORD &nAlloc)
	{
		nAlloc=_buf.size()*T_size;
		nUsed=nAlloc-_frees.size()*T_size;
	}

	virtual void **GetPtrs(void *owner,DWORD &count)
	{
		_temp.clear();
		std::deque<Node>::iterator it;
		for (it=_buf.begin();it!=_buf.end();it++)
		{
			Node *p=&(*it);
			if (p->owner==owner)
				_temp.push_back((void*)p);
		}
		count=_temp.size();
		return _temp.data();
	}

protected:
	struct Node
	{
		char v[T_size];
		void *owner;
	};
	std::deque<Node>_buf;
	std::deque<Node*>_frees;
	std::vector<void *>_temp;
};

#define DEFINE_CLASSPOOL(size) {static CClassPoolRaw<size>t;pools.push_back(&t);}
inline CClassPoolRawBase *FindRawPool(int sz)
{
	static std::vector<CClassPoolRawBase*> pools;
	static BOOL bLoaded=FALSE;
	if (!bLoaded)
	{
		DEFINE_CLASSPOOL(16);
		DEFINE_CLASSPOOL(32);
		DEFINE_CLASSPOOL(64);
		DEFINE_CLASSPOOL(128);
		DEFINE_CLASSPOOL(256);
		DEFINE_CLASSPOOL(512);
		DEFINE_CLASSPOOL(1024);
		DEFINE_CLASSPOOL(2048);
		DEFINE_CLASSPOOL(4096);

		bLoaded=TRUE;
	}

	for (int i=0;i<pools.size();i++)
	{
		if (pools[i]->GetSize()>sz)
			return pools[i];
	}
	return NULL;
}


template <typename T>
class CClassPool
{
public:
	CClassPool()
	{
		_pool=FindRawPool(sizeof(T));
		_c=0;
	}
	CClassPool(const char *poolname)
	{
		_pool=FindRawPool(sizeof(T));
		_c=0;
#ifdef _DEBUG
		SetName(poolname);
#endif
	}
	~CClassPool()
	{
		DumpTrack();
	}

	void Reset(BOOL bDumpTrack=TRUE)
	{
		if (bDumpTrack)
			DumpTrack();

		FreeAll();
	}
	T* Alloc()
	{
		_c++;
		if (_pool)
		{
			T* p=(T*)_pool->Alloc(this);
			p=new(p)T;
			return p;
		}
		return new T;
	}

	void Free(T *p)
	{
		_c--;
		if (_pool)
		{
			p->T::~T();
			_pool->Free(p);
		}
		else
			delete p;
	}

	void FreeAll()
	{
		if (_c==0)
			return;
		if (_pool)
		{
			DWORD c;
			void **ptrs=_pool->GetPtrs(this,c);
			for (int i=0;i<c;i++)
			{
				T *p=(T *)ptrs[i];
				p->T::~T();
				_pool->Free(p);
			}
		}
		_c=0;
	}

	T **GetInstances(DWORD &count)
	{
		if (_pool)
			return (T**)_pool->GetPtrs(this,count);
		count=0;
		return NULL;
	}

	void SetName(const char *poolname)
	{
#ifdef _DEBUG
		_poolname=poolname;
#endif
	}
	void DumpTrack()
	{
#ifdef _DEBUG

		std::string name=_poolname;
		if (name=="")
			name="<noname>";

		if (_c!=0)
		{
			char buf[256];
			sprintf(buf,
				"\n"
				"****************************************************\n"
				"Memory leak found in CClassPool \"%s\"(%d unfreed)!\n"
				"****************************************************\n"
				"\n"
				,name.c_str(),_c);
			OutputDebugStringA(buf);
		}
#endif
	}

	CClassPoolRawBase *_pool;
	int _c;

#ifdef _DEBUG
	std::string _poolname;
#endif

};
