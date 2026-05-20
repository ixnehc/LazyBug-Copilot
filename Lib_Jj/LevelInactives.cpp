
#include "stdh.h"
#include "Level.h"
#include "LevelInactives.h"
#include "LevelObj.h"

#include "commondefines/general_stl.h"

#include "timer/profiler.h"
#include "cyclemap/cyclemap.h"//for UpdateBlockMapCenter(..)

//////////////////////////////////////////////////////////////////////////
//CLevelInactives::Block
void CLevelInactives::Block::Clear()
{
	if (TRUE)
	{
		std::deque<CLevelObj*>::iterator it;
		for (it=objs.begin();it!=objs.end();it++)
		{
			CLevelObj*p=(*it);
			p->Destroy();
		}
	}

	objs.clear();
}

void CLevelInactives::Block::GarbageCollect()
{
	DWORD c=0;
	std::deque<CLevelObj*>::iterator it;
	for (it=objs.begin();it!=objs.end();it++)
	{
		CLevelObj*p=(*it);
		if (!p->IsAlive())
			c++;
	}

	if (c>objs.size()/4)
	{
		DWORD sz=objs.size();
		c=0;
		for (int i=0;i<sz;i++)
		{
			if (objs[i]->IsAlive())
			{
				objs[c]=objs[i];
				c++;
				continue;
			}
			SAFE_RELEASE(objs[i]);
		}
		objs.resize(c);
	}

}



//////////////////////////////////////////////////////////////////////////
//CLevelInactives

void CLevelInactives::Create(i_math::recti &rcMap)
{
	_posStart.x=(float)rcMap.Left();
	_posStart.y=(float)rcMap.Top();

	_w=(int)(((float)rcMap.getWidth())/(float)LEVEL_AOA_BLOCKLEN)+1;
	_h=(int)(((float)rcMap.getHeight())/(float)LEVEL_AOA_BLOCKLEN)+1;

	_blocks.resize(_w*_h);
}

void CLevelInactives::Destroy()
{
	for (int i=0;i<_blocks.size();i++)
	{
		CLevelInactives::Block *blk=&_blocks[i];
		blk->Clear();
	}
	_blocks.clear();
}

CLevelInactives::Block*CLevelInactives::GetBlock(DWORD x,DWORD y)
{
	if ((x>=_w)||(y>=_h))
		return NULL;

	return &_blocks[y*_w+_h];
}

BOOL CLevelInactives::Add(CLevelObj*obj)
{
	if (!obj->IsAlive())
		return FALSE;
	LevelPos pos=obj->GetFramePos();

	DWORD x,y;
	x=(DWORD)((pos.x-_posStart.x)/LEVEL_AOA_BLOCKLEN);
	y=(DWORD)((pos.y-_posStart.y)/LEVEL_AOA_BLOCKLEN);

	if ((x>=_w)||(y>=_h))
		return FALSE;

	Block *blk=&_blocks[y*_w+x];
	obj->AddRef();
	blk->objs.push_back(obj);

	return TRUE;
}

void CLevelInactives::UpdatePlayerAoa(AoaCenter&center,LevelPos&posCur0,CLevel *level)
{
	LevelPos posCur=posCur0;
	posCur-=_posStart;
	AovCenter centerOld=center;
	if (!UpdateBlockMapCenter(center,posCur.x,posCur.y,LEVEL_AOA_BLOCKLEN,LEVEL_AOA_BLOCKLEN))
		return;

	int radiusInBlk=(int)(LEVEL_AOA_RADIUS/LEVEL_AOA_BLOCKLEN);

	i_math::recti rcClip(0,0,_w,_h);

	i_math::recti rc;
	rc.set(center.x,center.y,center.x,center.y);
	rc.inflate(radiusInBlk,radiusInBlk,radiusInBlk+1,radiusInBlk+1);

	i_math::recti *rcs;
	DWORD nRCs;
	if (centerOld==AoaCenter_Invalid)
	{
		rcs=&rc;
		nRCs=1;
	}
	else
	{
		i_math::recti rcOld;
		rcOld.set(centerOld.x,centerOld.y,centerOld.x,centerOld.y);
		rcOld.inflate(radiusInBlk,radiusInBlk,radiusInBlk+1,radiusInBlk+1);

		extern i_math::recti *DifferRect(DWORD &nRc,i_math::recti &rcOrg,i_math::recti &rcNew);
		rcs=DifferRect(nRCs,rcOld,rc);
	}

	for (int i=0;i<nRCs;i++)
	{
		i_math::recti rcT=rcs[i];
		rcT.clipAgainst(rcClip);
		for (DWORD jj=rcT.Top();jj<rcT.Bottom();jj++)
		for (DWORD ii=rcT.Left();ii<rcT.Right();ii++)
		{
			Block *blk=&_blocks[jj*_w+ii];

			std::deque<CLevelObj*>::iterator it;
			for (it=blk->objs.begin();it!=blk->objs.end();it++)
			{
				CLevelObj *obj=(*it);
				level->AddToActives(obj);
				SAFE_RELEASE(obj);
			}
			blk->objs.clear();
		}
	}
}



void CLevelInactives::GarbageCollect()
{
	DWORD nStep=_blocks.size()/20+1;
	for (int i=0;i<nStep;i++)
	{
		int idx=(_idxGC+i)%_blocks.size();
		_blocks[idx].GarbageCollect();
	}
	_idxGC=(_idxGC+nStep)%_blocks.size();
}
