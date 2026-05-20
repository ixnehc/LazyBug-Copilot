/********************************************************************
	created:	2007/3/15   11:36
	filename: 	e:\IxEngine\Common\mempool\mempool.cpp
	author:		cxi
	
	purpose:	memory pool
*********************************************************************/
#include "stdh.h"

#include "mempool.h"

#include "../commondefines/general_stl.h"



//////////////////////////////////////////////////////////////////////////
//CMemPool
void CMemPoolEx::Clear()
{
	for (int i=0;i<_pools.size();i++)
		SAFE_DELETE(_pools[i]);
	_pools.clear();
}

void CMemPoolEx::SetName(const char *name)
{
	_name=name;
}

void CMemPoolEx::DumpTrack()
{
#ifdef _DEBUG

	std::string name=_name;
	if (name=="")
		name="<noname>";
	DWORD nUsed,nAlloc;
	CollectStat(nUsed,nAlloc);
	if (nUsed>0)
	{
		char buf[256];
		sprintf(buf,
			"\n"
			"****************************************************\n"
			"Memory leak found in CMemPoolEx \"%s\"(%d bytes unfreed)!\n"
			"****************************************************\n"
			"\n"
			,name.c_str(),nUsed);
		OutputDebugStringA(buf);
	}
#endif
}



void CMemPoolEx::Reset(DWORD BaseUnitSize,DWORD level)
{
	std::vector<DWORD>poolunits;
	DWORD sz=BaseUnitSize;
	poolunits.resize(level);
	for (int i=0;i<level;i++)
	{
		poolunits[i]=sz;
		sz*=2;
	}
	Reset(poolunits);
}


void CMemPoolEx::Reset(DWORD *poolunits0,DWORD count)
{
	Clear();
	assert(count<16);

	std::vector<DWORD>poolunits;
	poolunits.resize(count);
	memcpy(poolunits.data(),poolunits0,count*sizeof(DWORD));

	VEC_ASCEND(poolunits,DWORD);

	_pools.resize(poolunits.size());

	for (int i=0;i<poolunits.size();i++)
	{
		_pools[i]=new Pool;
		_pools[i]->szUnit=poolunits[i];
	}
}


//re-create pools using the given unit sizes
void CMemPoolEx::Reset(std::vector<DWORD>&poolunits0)
{
	Reset(poolunits0.data(),poolunits0.size());
}

//empty all the existing pools
void CMemPoolEx::Reset()
{
	for (int i=0;i<_pools.size();i++)
	{
		_pools[i]->buf.clear();
		_pools[i]->frees.clear();
	}
}

void CMemPoolEx::CollectStat(DWORD &nUsed,DWORD &nAlloc)
{
	nUsed=0;
	nAlloc=0;
	for (int i=0;i<_pools.size();i++)
	{
		nAlloc+=_pools[i]->buf.size();
		nUsed+=_pools[i]->buf.size()-_pools[i]->frees.size()*_pools[i]->szUnit;
	}
}
