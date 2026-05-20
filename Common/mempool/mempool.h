#pragma once

#include <vector>
#include <list>
#include <deque>

#include <assert.h>

#include <stdio.h>

#include "../datapacket/DataPacket.h"


typedef DWORD MemHandle;
#define MemHandle_Null (NULL)

#define MemHandle_Make(iPool,idx) (((iPool)<<28)|(((idx)+1)&0x0fffffff))
#define MemHandle_GetPool(h) ((h)>>28)
#define MemHandle_GetIdx(h) (((h)&0x0fffffff)-1)

struct DummyMutex
{
	struct Lock
	{
		Lock(DummyMutex &mu,BOOL bWriter=TRUE)
		{
		}
	};
};


//Note for CMemPool_v:
//1.containee should be simple type,that is to say,the object of this type
//should be std::vector-containable
//2.no memory limit
template <class T>
class CMemPool_v//v for vector,use vector as internal containor
{
public:
	void Reset()
	{
		_buf.clear();
		_frees.clear();
	}
	MemHandle Alloc()
	{
		MemHandle h;
		if (_frees.size()>0)
		{
			h=MemHandle_Make(0,_frees[_frees.size()-1]);
			_frees.resize(_frees.size()-1);
		}
		else
		{
			h=MemHandle_Make(0,_buf.size());
			_buf.resize(_buf.size()+1);//increase buffer by 1 unit
		}
		_buf[MemHandle_GetIdx(h)].bFreed=FALSE;
		return h;
	}
	void Free(MemHandle h)
	{
		_frees.push_back(MemHandle_GetIdx(h));
		_buf[MemHandle_GetIdx(h)].bFreed=TRUE;
	}
	T *ObtainPtr(MemHandle h)
	{
		return &_buf[MemHandle_GetIdx(h)].v;
	}

	//for browsing
	DWORD GetCount()	{		return _buf.size();	}
	T *Get(DWORD idx)
	{
		if (_buf[idx].bFreed)
			return NULL;
		return &_buf[idx].v;
	}

	void CollectStat(DWORD &nUsed,DWORD &nAlloc)
	{
		nAlloc=_buf.size()*sizeof(Node);
		nUsed=nAlloc-_frees.size()*sizeof(Node);
	}

	void Save(CDataPacket &dp)
	{
		DP_WriteVector(dp,_buf);
		DP_WriteVector(dp,_frees);
	}

	void Load(CDataPacket &dp)
	{
		DP_ReadVector(dp,_buf);
		DP_ReadVector(dp,_frees);
	}

protected:
	struct Node
	{
		T v;
		bool bFreed;
	};
	std::vector<Node>_buf;
	std::vector<DWORD>_frees;
};



//Note for CMemPool_fv
//1.containee could be complex type
//2.containee count is limited
template<typename T,typename T_Mutex=DummyMutex,typename T_Lock=DummyMutex::Lock>
class CMemPool_fv//a for fixed length vector,use fixed length vector as internal container
{
public:

	CMemPool_fv()
	{
		_count=0;
	}

	void Reset(DWORD capacity)
	{
		//Call the destructor for un-freeed containee
		for (int i=0;i<_count;i++)
		{
			T *p=(T *)&_buf[i*sizeof(Node)];
			if (!((Node*)p)->bFree)
				p->T::~T();
		}

		_count=0;
		_frees.clear();
		_buf.resize(capacity*sizeof(Node));
		_capacity=capacity;
	}
	T* Alloc()
	{
		T* p;
		if (_frees.size()>0)
		{
			p=_frees[_frees.size()-1];
			_frees.pop_back();
		}
		else
		{
			if (_count>=_capacity)
				return NULL;//buffer used up
			p=(T*)&_buf[_count*sizeof(Node)];
			_count++;
		}
		p=new(p)T;//call the constructor
		((Node*)p)->bFree=FALSE;
		return p;
	}
	void Free(T *p)
	{
		p->T::~T();
		((Node*)p)->bFree=TRUE;
		_frees.push_back(p);
	}

	void CollectStat(DWORD &nUsed,DWORD &nAlloc)
	{
		nAlloc=_buf.size();
		nUsed=nAlloc-_frees.size()*sizeof(Node);
	}

	DWORD GetCount()	{		return _capacity;	}

	T *Get(DWORD idx)
	{
		if (idx>=_capacity)
			return NULL;

		Node *p=(Node*)(&_buf[idx*sizeof(Node)]);
		if (p->bFree)
			return NULL;

		return (T *)p;
	}

protected:
	struct Node
	{
		char v[sizeof(T)];
		bool bFree;
	};
	std::vector<BYTE> _buf;
	DWORD _count;
	std::vector<T*> _frees;
	DWORD _capacity;

};


inline std::vector<void *> *GetMemPoolInstanceBuf()
{
	static std::vector<void *>buf;
	return &buf;
}

//Note for CMemPool
//1.containee could be complex type
//2.no memory limit
//3.return direct pointer
template <typename T,typename T_Mutex=DummyMutex,typename T_Lock=DummyMutex::Lock>
class CMemPool
{
public:
	CMemPool()
	{
	}
	CMemPool(const char *poolname)
	{
#ifdef _DEBUG
		SetName(poolname);
#endif
	}
	~CMemPool()
	{
		DumpTrack();
	}

	void Reset(BOOL bDumpTrack=TRUE)
	{
		T_Lock lock(_mu,TRUE);//write lock

		if (bDumpTrack)
			DumpTrack();

		//Call the destructor for un-freeed containee
		std::deque<Node>::iterator it;
		for(it=_buf.begin();it!=_buf.end();it++)
		{
			T *p=(T *)&(*it);
			if (!((Node*)p)->bFreed)
				p->T::~T();
		}
		_buf.clear();
		_frees.clear();
	}
	T* Alloc()
	{
		T_Lock lock(_mu,TRUE);//write lock

		T* p;
		if (_frees.size()>0)
		{
			p=_frees[_frees.size()-1];
			_frees.resize(_frees.size()-1);
		}
		else
		{
			_buf.resize(_buf.size()+1);//increase buffer by 1 unit
			p=(T*)&_buf.back();
		}
		p=new(p)T;
		((Node*)p)->bFreed=FALSE;
		return p;
	}
	void Free(T *p)
	{
		T_Lock lock(_mu,TRUE);//write lock

		assert(!((Node*)p)->bFreed);
		p->T::~T();
		((Node*)p)->bFreed=TRUE;
		_frees.push_back(p);
	}
	void FreeAll()
	{
		std::deque<Node>::iterator it;
		for (it=_buf.begin();it!=_buf.end();it++)
		{
			if ((*it).bFreed)
				continue;
			Free((T*)&(*it));
		}
	}

	void **GetInstances(DWORD &count)
	{
		std::vector<void *> *buf=GetMemPoolInstanceBuf();

		buf->clear();
		std::deque<Node>::iterator it;
		for (it=_buf.begin();it!=_buf.end();it++)
		{
			Node *p=&(*it);
			if (!p->bFreed)
				buf->push_back(p);
		}
		count=buf->size();
		if (count <= 0)
			return NULL;

		return &(*buf)[0];
	}

	DWORD GetCount()	
	{		
		T_Lock lock(_mu,FALSE);//read lock
		return _buf.size();	
	}
	T *Get(DWORD idx)
	{
		T_Lock lock(_mu,FALSE);//read lock

		if (((Node*)&_buf[idx])->bFreed)
			return NULL;
		return (T*)&_buf[idx];
	}

	void CollectStat(DWORD &nUsed,DWORD &nAlloc)
	{
		T_Lock lock(_mu,FALSE);//read lock

		nAlloc=_buf.size()*sizeof(T);
		nUsed=nAlloc-_frees.size()*sizeof(T);
	}


	void SetName(const char *poolname)
	{
#ifdef _DEBUG
		T_Lock lock(_mu,TRUE);//write lock
		_poolname=poolname;
#endif
	}
	void DumpTrack()
	{
#ifdef _DEBUG
		T_Lock lock(_mu,FALSE);//read lock

		std::string name=_poolname;
		if (name=="")
			name="<noname>";
		if (_frees.size()!=_buf.size())
		{
			assert(0);
			char buf[256];
			sprintf(buf,
				"\n"
				"****************************************************\n"
				"Memory leak found in CMemPool \"%s\"(%d unfreed)!\n"
				"****************************************************\n"
				"\n"
				,name.c_str(),(int)(_buf.size()-_frees.size()));
			OutputDebugStringA(buf);

			DWORD c=GetCount();
			for (int i=0;i<c;i++)
			{
				T *p=Get(i);
				if (p)
				{
					int v=0;
					v++;
				}
			}
		}
#endif
	}


protected:
	struct Node
	{
		char v[sizeof(T)];
		bool bFreed;
	};
	std::deque<Node>_buf;
	std::vector<T*>_frees;

	T_Mutex _mu;

#ifdef _DEBUG
	std::string _poolname;
#endif

};


//Note for CMemPool_a
//1.containee could be complex type
//2.containee count is limited by T_count
template<typename T,int T_count,typename T_Mutex=DummyMutex,typename T_Lock=DummyMutex::Lock>
class CMemPool_a//a for array,use fixed length array as internal container
{
public:

	CMemPool_a()
	{
		_count=0;
		_countFree=0;
	}

	void Reset()
	{
		//Call the destructor for un-freeed containee
		for (int i=0;i<_count;i++)
		{
			T *p=(T *)&_buf[i];
			if (!((Node*)p)->bFree)
				p->T::~T();
		}

		_count=0;
		_countFree=0;
	}
	T* Alloc()
	{
		T* p;
		if (_countFree>0)
		{
			p=_frees[_countFree-1];
			_countFree--;
		}
		else
		{
			if (_count>=T_count)
				return NULL;//buffer used up
			p=(T*)&_buf[_count];
			_count++;
		}
		p=new(p)T;//call the constructor
		((Node*)p)->bFree=FALSE;
		return p;
	}
	void Free(T *p)
	{
		p->T::~T();
		((Node*)p)->bFree=TRUE;
		_frees[_countFree]=p;
		_countFree++;
	}

	void CollectStat(DWORD &nUsed,DWORD &nAlloc)
	{
		nAlloc=_count.size()*sizeof(T);
		nUsed=nAlloc-_countFree*sizeof(T);
	}

	DWORD GetCount()	{		return T_count;	}

	T *Get(DWORD idx)
	{
		if (idx>=T_count)
			return NULL;

		if (_buf[idx].bFree)
			return NULL;

		return (T *)_buf[idx].v;
	}

protected:
	struct Node
	{
		char v[sizeof(T)];
		bool bFree;
	};
	Node _buf[T_count];
	DWORD _count;
	T* _frees[T_count];
	DWORD _countFree;

};



//IMPORTANT: each pool could contain at most 0xfffffff(256M) byte of memory
class CMemPoolEx
{
public:
	~CMemPoolEx()
	{
		DumpTrack();
		Clear();
	}
	void Clear();
	void Reset(std::vector<DWORD>&poolunits);//re-create pools using the given unit sizes
	void Reset(DWORD *poolunits,DWORD count);

	//For example,if BaseUnitSize is 5,level is 4,the unit size will be 5,5*2,5*4,5*8
	void Reset(DWORD BaseUnitSize,DWORD level);
	void Reset();//empty all the existing pools

	void DumpTrack();

	void SetName(const char *name);
	MemHandle Alloc(DWORD sz)
	{
		for (int i=0;i<_pools.size();i++)
		{
			if (_pools[i]->szUnit>=sz)
			{
				MemHandle h;
				Pool *pool=_pools[i];
				if (pool->frees.size()>0)
				{
					h=MemHandle_Make(i,pool->frees[pool->frees.size()-1]);
					pool->frees.resize(pool->frees.size()-1);
				}
				else
				{
					h=MemHandle_Make(i,pool->buf.size());
					pool->buf.resize(pool->buf.size()+pool->szUnit);//increase buffer by 1 unit
					assert(pool->buf.size()<0xfffffff);
				}
				return h;
			}
		}
		return MemHandle_Null;
	}
	void Free(MemHandle h)
	{
		DWORD iPool=MemHandle_GetPool(h);
		assert(iPool<_pools.size());
		_pools[iPool]->frees.push_back(MemHandle_GetIdx(h));
	}

	//will keep the original data in hOld
	//if return FALSE,hOld will not be freed
	BOOL ReAlloc(MemHandle &hOld,DWORD sz)
	{
		DWORD iPool=MemHandle_GetPool(hOld);
		DWORD szOld=_pools[iPool]->szUnit;
		if (szOld>=sz)
			return TRUE;

		MemHandle hNew=Alloc(sz);
		if (!hNew)
			return FALSE;

		//Copy the data
		BYTE *p,*q;
		p=(BYTE*)ObtainPtr(hNew);
		q=(BYTE*)ObtainPtr(hOld);

		if (sz>szOld)
			sz=szOld;
		memcpy(p,q,sz);

		Free(hOld);
		hOld=hNew;

		return TRUE;
	}

	void *ObtainPtr(MemHandle h)
	{
		DWORD iPool=MemHandle_GetPool(h);
		assert(iPool<_pools.size());
		return &_pools[iPool]->buf[MemHandle_GetIdx(h)];
	}

	void CollectStat(DWORD &nUsed,DWORD &nAlloc);

	void Save(CDataPacket &dp)
	{
		dp.Data_NextDword()=_pools.size();
		for (int i=0;i<_pools.size();i++)
		{
			dp.Data_NextDword()=_pools[i]->szUnit;
			
			DP_WriteVector(dp,_pools[i]->buf);
			DP_WriteVector(dp,_pools[i]->frees);
		}
	}

	void Load(CDataPacket &dp)
	{
		Clear();

		DWORD sz=dp.Data_NextDword();
		_pools.resize(sz);

		for (int i=0;i<sz;i++)
		{
			_pools[i]=new Pool;

			_pools[i]->szUnit=dp.Data_NextDword();
			DP_ReadVector(dp,_pools[i]->buf);
			DP_ReadVector(dp,_pools[i]->frees);
		}
	}


protected:
	struct Pool
	{
		DWORD szUnit;
		std::vector<BYTE>buf;
		std::vector<DWORD>frees;
	};
	std::vector<Pool*>_pools;
	std::string _name;
};

