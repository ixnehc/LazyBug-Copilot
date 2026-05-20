
#pragma once

#include "../class/class.h"

#include "../mempool/mempool.h"

template <typename T,int T_width,unsigned char initfill,BOOL bUseInitFill>
struct SparseBlock
{
	typedef SparseBlock<T,T_width,initfill,bUseInitFill> SparseBlockThisType;
//	DEFINE_CLASS(SparseBlockThisType)

	SparseBlock()
	{
		if (bUseInitFill)
			memset(data,initfill,sizeof(data));
	}

	T data[T_width][T_width];
	DWORD tRecent;//最近一次访问的时间
};

template <typename T,unsigned int T_blockwidth,unsigned char initfill=0,BOOL bUseInitFill=TRUE>
struct SparseArray2D
{
	typedef SparseBlock<T,T_blockwidth,initfill,bUseInitFill>BlockType;

	struct Area
	{
		T *buf;
		int idxStart;
		i_math::recti rc;//世界空间,以T为单位
		i_math::pos2di ptBlk;//Block的坐标
		BlockType *blk;
	};

	void Reset()
	{
		rcBlks.set(0,0,0,0);

		pool.Reset(FALSE);//不需要检查是否有内存泄漏

		blocks.clear();

		last=0;
	}

	T *Obtain(int x,int y)
	{
		return Obtain(x,y,0);
	}

	T *Obtain(int x,int y,DWORD t)
	{
		i_math::pos2di ptBlk;
		ptBlk.x=i_math::idiv_signed(x,T_blockwidth);
		ptBlk.y=i_math::idiv_signed(y,T_blockwidth);

		BlockType *pBlk=_ObtainBlock(ptBlk);
		pBlk->tRecent=t;//记录最近一次访问的时间

		return &pBlk->data[x-ptBlk.x*T_blockwidth][y-ptBlk.y*T_blockwidth];
	}

	T *Get(int x,int y)
	{
		i_math::pos2di ptBlk;
		ptBlk.x=i_math::idiv_signed(x,T_blockwidth);
		ptBlk.y=i_math::idiv_signed(y,T_blockwidth);

		if (!rcBlks.isPointInside(ptBlk))
			return NULL;

		BlockType *pBlk=blocks[(ptBlk.y-rcBlks.Top())*rcBlks.getWidth()+ptBlk.x-rcBlks.Left()];
		if (!pBlk)
			return NULL;

		return &pBlk->data[x-ptBlk.x*T_blockwidth][y-ptBlk.y*T_blockwidth];
	}

	void Obtain(i_math::recti &rc,std::vector<Area>&areas,DWORD t)
	{
		areas.clear();

		i_math::recti rcBlk=rc;
		rcBlk.scale_signed(T_blockwidth);

		for (int i=rcBlk.Left();i<=rcBlk.Right();i++)
		for (int j=rcBlk.Top();j<=rcBlk.Bottom();j++)
		{
			i_math::recti rc2;
			rc2.set(i,j,i+1,j+1);
			rc2*=T_blockwidth;

			if (!rc2.isRectCollided(rc))
				continue;

			Area area;

			area.rc=rc2;
			area.rc.clipAgainst(rc);

			i_math::recti rcLocal=area.rc;
			rcLocal-=rc2.UpperLeftCorner;

			i_math::pos2di ptBlk(i,j);
			BlockType *pBlk=_ObtainBlock(ptBlk);
			pBlk->tRecent=t;

			area.buf=&pBlk->data[0][0];
			area.idxStart=rcLocal.Top()*T_blockwidth+rcLocal.Left();
			area.ptBlk=ptBlk;
			area.blk=pBlk;

			areas.push_back(area);
		}
	}

	//清除掉所有在tBefore后没有被访问过的block
	void DiscardInactive(DWORD tBefore)
	{
		for (int i=0;i<256;i++)
		{
			last=(last+1)%blocks.size();
			if (blocks[last])
			{
				if (blocks[last]->tRecent<tBefore)
				{
					pool.Free(blocks[last]);
					blocks[last]=NULL;
				}
			}
		}
	}
	void DiscardAll()
	{
		for (int i=0;i<blocks.size();i++)
		{
			if (blocks[i])
			{
				pool.Free(blocks[i]);
				blocks[i]=NULL;
			}
		}
	}

protected:

	BlockType *_ObtainBlock(i_math::pos2di &ptBlk)
	{
		if (!rcBlks.isPointInside(ptBlk))
		{
			i_math::recti rcNew;
			rcNew=rcBlks;
			rcNew.merge(ptBlk);

			std::vector<BlockType *>blksNew;
			blksNew.resize(rcNew.getArea());

			memset(blksNew.data(),0,sizeof(BlockType *)*blksNew.size());

			//从旧的数组中移到新的数组
			for (int j=0;j<rcBlks.getHeight();j++)
			{
				int x,y;
				x=rcBlks.Left()-rcNew.Left();
				y=rcBlks.Top()+j-rcNew.Top();

				BlockType **p,**q;
				q=&blksNew[y*rcNew.getWidth()+x];
				p=&blocks[j*rcBlks.getWidth()];

				memcpy(q,p,rcBlks.getWidth()*sizeof(BlockType *));
			}

			rcBlks=rcNew;
			blocks.swap(blksNew);
		}

		BlockType *&pBlk=blocks[(ptBlk.y-rcBlks.Top())*rcBlks.getWidth()+ptBlk.x-rcBlks.Left()];
		if (!pBlk)
			pBlk=pool.Alloc();

		return pBlk;
	}

public:

	i_math::recti rcBlks;
	std::vector<BlockType *>blocks;
	DWORD last;//最近一次检查过的要不要discard的block

	CMemPool<BlockType> pool;
};