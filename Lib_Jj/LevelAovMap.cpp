
#include "stdh.h"

#include "cyclemap/cyclemap.h"

#include "LevelAovMap.h"


//Note:the 2 rects need to be same in size(w,h)
//返回在rcNew而不在rcOrg里的区域
i_math::recti *DifferRect(DWORD &nRc,i_math::recti &rcOrg,i_math::recti &rcNew)
{
	static i_math::recti ret[2];
	nRc=0;
	i_math::recti rc=rcNew;
	if (!rc.isValid())
		return NULL;
	rc.clipAgainst(rcOrg);

	if (!rc.isValid())
	{
		ret[0]=rcNew;
		nRc=1;
		return ret;
	}

#define DIFFERRECT_ADD_RC(l,t,r,b) \
		ret[nRc].set(l,t,r,b);\
		if (ret[nRc].isValid())\
			nRc++;

	//		xxxooo
	//		xxxooo
	//		oooooo
	//		oooooo
	if ((rc.Left()==rcNew.Left())&&(rc.Top()==rcNew.Top()))
	{
		DIFFERRECT_ADD_RC(rc.Right(),rc.Top(),rcNew.Right(),rc.Bottom());
		DIFFERRECT_ADD_RC(rcNew.Left(),rc.Bottom(),rcNew.Right(),rcNew.Bottom());
		return ret;
	}

	//		oooxxx
	//		oooxxx
	//		oooooo
	//		oooooo

	if ((rc.Right()==rcNew.Right())&&(rc.Top()==rcNew.Top()))
	{
		DIFFERRECT_ADD_RC(rcNew.Left(),rc.Top(),rc.Left(),rc.Bottom());
		DIFFERRECT_ADD_RC(rcNew.Left(),rc.Bottom(),rcNew.Right(),rcNew.Bottom());
		return ret;
	}

	//		oooooo
	//		oooooo
	//		xxxooo
	//		xxxooo

	if ((rc.Left()==rcNew.Left())&&(rc.Bottom()==rcNew.Bottom()))
	{
		DIFFERRECT_ADD_RC(rcNew.Left(),rcNew.Top(),rcNew.Right(),rc.Top());
		DIFFERRECT_ADD_RC(rc.Right(),rc.Top(),rcNew.Right(),rcNew.Bottom());
		return ret;
	}

	//		oooooo
	//		oooooo
	//		oooxxx
	//		oooxxx

	if ((rc.Right()==rcNew.Right())&&(rc.Bottom()==rcNew.Bottom()))
	{
		DIFFERRECT_ADD_RC(rcNew.Left(),rcNew.Top(),rcNew.Right(),rc.Top());
		DIFFERRECT_ADD_RC(rcNew.Left(),rc.Top(),rc.Left(),rcNew.Bottom());
		return ret;
	}

	assert(FALSE);
	return NULL;
}


void CLevelAovMap::Create(i_math::recti &rcMap)
{
	_posStart.x=(float)rcMap.Left();
	_posStart.y=(float)rcMap.Top();

	_w=(int)(((float)rcMap.getWidth())/(float)LEVEL_AOVMAP_BLOCKLEN)+1;
	_h=(int)(((float)rcMap.getHeight())/(float)LEVEL_AOVMAP_BLOCKLEN)+1;

	_buf.resize(_w*_h);

}

LevelPlayerMask CLevelAovMap::GetPlayerMask(LevelPos &pos0)
{
	LevelPos pos=pos0;
	pos-=_posStart;

	int x,y;
	x=(int)(pos.x/LEVEL_AOVMAP_BLOCKLEN);
	y=(int)(pos.y/LEVEL_AOVMAP_BLOCKLEN);

	i_math::recti rcClip(0,0,_w,_h);

	if (!rcClip.isPointInside(x,y))
		return 0;

	return _buf[y*_w+x];
}


void CLevelAovMap::UpdatePlayerAov(LevelPlayerMask mask,AovCenter&center,LevelPos&posCur0)
{
	LevelPos posCur=posCur0;
	posCur-=_posStart;
	AovCenter centerOld=center;
	if (!UpdateBlockMapCenter(center,posCur.x,posCur.y,LEVEL_AOVMAP_BLOCKLEN,LEVEL_AOVMAP_BLOCKLEN))
		return;

	int radiusInBlk=(int)(LEVEL_AOV_RADIUS/LEVEL_AOVMAP_BLOCKLEN);

	i_math::recti rcClip(0,0,_w,_h);

	i_math::recti rc;
	rc.set(center.x,center.y,center.x,center.y);
	rc.inflate(radiusInBlk,radiusInBlk,radiusInBlk+1,radiusInBlk+1);

	i_math::recti *rcs;
	DWORD nRCs;
	if (centerOld==AovCenter_Invalid)
	{
		rcs=&rc;
		nRCs=1;
	}
	else
	{
		i_math::recti rcOld;
		rcOld.set(centerOld.x,centerOld.y,centerOld.x,centerOld.y);
		rcOld.inflate(radiusInBlk,radiusInBlk,radiusInBlk+1,radiusInBlk+1);

		LevelPlayerMask maskInv=~mask;
		rcs=DifferRect(nRCs,rc,rcOld);
		for (int i=0;i<nRCs;i++)
		{
			i_math::recti rcT=rcs[i];
			rcT.clipAgainst(rcClip);
			for (DWORD jj=rcT.Top();jj<rcT.Bottom();jj++)
			for (DWORD ii=rcT.Left();ii<rcT.Right();ii++)
				_buf[jj*_w+ii]&=maskInv;
		}
		rcs=DifferRect(nRCs,rcOld,rc);
	}

	for (int i=0;i<nRCs;i++)
	{
		i_math::recti rcT=rcs[i];
		rcT.clipAgainst(rcClip);
		for (DWORD jj=rcT.Top();jj<rcT.Bottom();jj++)
		for (DWORD ii=rcT.Left();ii<rcT.Right();ii++)
			_buf[jj*_w+ii]|=mask;
	}

}

void CLevelAovMap::ClearPlayerAov(LevelPlayerMask mask,AovCenter&center)
{
	if (center==AovCenter_Invalid)
		return;

	int radiusInBlk=(int)(LEVEL_AOV_RADIUS/LEVEL_AOVMAP_BLOCKLEN);

	i_math::recti rcClip(0,0,_w,_h);

	i_math::recti rc;
	rc.set(center.x,center.y,center.x,center.y);
	rc.inflate(radiusInBlk,radiusInBlk,radiusInBlk+1,radiusInBlk+1);
	rc.clipAgainst(rcClip);

	LevelPlayerMask maskInv=~mask;
	for (DWORD jj=rc.Top();jj<rc.Bottom();jj++)
	for (DWORD ii=rc.Left();ii<rc.Right();ii++)
		_buf[jj*_w+ii]&=maskInv;

}
