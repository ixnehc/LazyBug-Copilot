
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "LevelExploreMap.h"

#define SEG_UNIT_LEN (8)
struct ExploreSeg
{
	i_math::pos2di off;
	BYTE v;
};

//nRadius以TILE为单位
ExploreSeg *GetExploreSeg(DWORD iBitOff,DWORD &nSegs,DWORD nRadius)
{
	nSegs=0;
	 if (iBitOff>=SEG_UNIT_LEN)
		 return NULL;

	 static std::vector<ExploreSeg> segs[SEG_UNIT_LEN][MAX_EXPLORE_RADIUS+1];

	 if (nRadius>MAX_EXPLORE_RADIUS)
		 return NULL;

	 if (segs[iBitOff][nRadius].size()>0)
	 {
		 nSegs=segs[iBitOff][nRadius].size();
		 return &segs[iBitOff][nRadius][0];
	 }

	 i_math::recti rc;
	 rc.set(-SEG_UNIT_LEN,-(int)nRadius,SEG_UNIT_LEN*2,((int)nRadius)+1);

	 std::vector<BYTE>buf;
	 buf.resize(rc.getArea());
	 VEC_SET(buf,0);

	 i_math::pos2di center;
	 center.x=iBitOff;
	 center.y=0;

	 float ratio=1.0f;//1.4f
	 BYTE *p=buf.data();
	 for (int j=rc.Top();j<rc.Bottom();j++)
	 for (int i=rc.Left();i<rc.Right();i++)
	 {
		 float dist2=(float)(i-center.x)*(float)(i-center.x)+(float)(j-center.y)*(float)(j-center.y)*ratio*ratio;
		 if (dist2<=(float)nRadius*(float)nRadius)
			 *p=1;
		 p++;
	 }

	p=buf.data();
	 for (int j=rc.Top();j<rc.Bottom();j++)
	 {
		 for (int i=rc.Left();i<rc.Right();i+=SEG_UNIT_LEN)
		 {
			 ExploreSeg seg;
			 seg.off.x=i_math::idiv_signed(i,SEG_UNIT_LEN);
			 seg.off.y=j;

			 seg.v=0;
			 for (int k=0;k<SEG_UNIT_LEN;k++)
			 {
				 if (*p)
					 seg.v|=(1<<k);
				 p++;
			 }
			 if (seg.v!=0)
				 segs[iBitOff][nRadius].push_back(seg);
		 }
	 }

	 nSegs=segs[iBitOff][nRadius].size();
	 return &segs[iBitOff][nRadius][0];

}

void InitExploreSeg()
{
	DWORD nSeg;
	for (int j=0;j<MAX_EXPLORE_RADIUS;j++)
	for (int i=0;i<SEG_UNIT_LEN;i++)
		GetExploreSeg(i,nSeg,j);
}



void CLevelExploreMap::Init(i_math::recti &rcMap)
{
	_rcMap=rcMap;

	_rcBlk=rcMap;
	_rcBlk.zeroBase();
	_rcBlk*=EXPLOREMAP_RESO;
	_rcBlk.scale_signed(EXPLOREMAP_BLOCK_LEN);

	_blks.resize(_rcBlk.getArea());
	VEC_SET(_blks,0);
	_masks.resize((_rcBlk.getArea()+7)/8);
	VEC_SET(_masks,0);

	_ver++;
}

void CLevelExploreMap::Clear()
{
	for (int i=0;i<_blks.size();i++)
	{
		Safe_Class_Delete(_blks[i]);
	}
	_masks.clear();
	_blks.clear();
}

void CLevelExploreMap::ClearContent()
{
	for (int i=0;i<_blks.size();i++)
	{
		Safe_Class_Delete(_blks[i]);
	}
	VEC_SET(_masks,0);

	_ver++;
}


void CLevelExploreMap::CopyFrom(CLevelExploreMap *src)
{
	Clear();

	_rcMap=src->_rcMap;
	_rcBlk=src->_rcBlk;
	_masks=src->_masks;
	_blks.resize(src->_blks.size());
	for (int i=0;i<src->_blks.size();i++)
	{
		if (src->_blks[i])
		{
			_blks[i]=Class_New2(ExploreMapBlock);
			_blks[i]->CopyFrom(src->_blks[i]);
		}
	}

	_ver++;
}



void CLevelExploreMap::Save(CDataPacket &dp)
{
	dp.Data_WriteSimpleR(_rcMap);
	DP_WriteVector(dp,_masks);
	BOOL bBlk=0;
	DWORD idxMask=0;
	DWORD mask=1;
	for (int i=0;i<_blks.size();i++)
	{
		bBlk=_masks[idxMask]&mask;
		mask<<=1;
		if (mask>=256)
		{
			mask=1;
			idxMask++;
		}

		if (bBlk)
		{
			ExploreMapBlock*blk=_blks[i];
			assert(blk);
			dp.Data_WriteData(blk->buf,sizeof(blk->buf));
		}
	}
}

void CLevelExploreMap::Load(CDataPacket &dp)
{
	i_math::recti rcMapOrg=_rcMap;

	dp.Data_ReadSimple(_rcMap);
	_rcBlk=_rcMap;
	_rcBlk.zeroBase();
	_rcBlk*=EXPLOREMAP_RESO;
	_rcBlk.scale_signed(EXPLOREMAP_BLOCK_LEN);
	_blks.resize(_rcBlk.getArea());

	DP_ReadVector(dp,_masks);
	BOOL bBlk=0;
	DWORD idxMask=0;
	DWORD mask=1;
	for (int i=0;i<_blks.size();i++)
	{
		bBlk=_masks[idxMask]&mask;
		mask<<=1;
		if (mask>=256)
		{
			mask=1;
			idxMask++;
		}

		if (bBlk)
		{
			if (!_blks[i])
				_blks[i]=Class_New2(ExploreMapBlock);
			dp.Data_ReadData(_blks[i]->buf,sizeof(_blks[i]->buf));
		}
		else
		{
			Safe_Class_Delete(_blks[i]);
		}
	}

	_ver++;
}

LevelPos WorldPos2TexPos(LevelPos3D &pos3D,i_math::recti &rcMap)
{
	i_math::vector3df vX,vY,vZ;
	GetMainGameCameraAxes(vX,vY,vZ);

	i_math::plane3df pl;
	i_math::pos2di ptCenter=rcMap.getCenter();
	i_math::vector3df posCenter;
	posCenter.set((float)ptCenter.x,0.0f,(float)ptCenter.y);
	pl.setPlane(posCenter,vZ);

	i_math::vector3df posProj;
	pl.getProjectionOf(pos3D,posProj);

	i_math::vector3df dirProj;
	dirProj=posProj-posCenter;
	float x=dirProj.dotProduct(vX);
	float y=dirProj.dotProduct(vY);

	x=x+0.5f*(float)rcMap.getWidth();
	y=0.5f*(float)rcMap.getHeight()-y;

	x*=(float)EXPLOREMAP_RESO;
	y*=(float)EXPLOREMAP_RESO;

	return LevelPos(x,y);
}

i_math::pos2di CLevelExploreMap::TilePosFromWorldPos(LevelPos3D &center)
{
	i_math::pos2di posTile;
	if (TRUE)
	{
		LevelPos posTex=WorldPos2TexPos(center,_rcMap);
		posTile.x=(int)floor(posTex.x/(float)EXPLOREMAP_TILE_LEN);
		posTile.y=(int)floor(posTex.y/(float)EXPLOREMAP_TILE_LEN);
	}
	return posTile;
}


void CLevelExploreMap::AddExplore(LevelPos3D &center,DWORD nRadiusInTile)
{
	i_math::pos2di posTile=TilePosFromWorldPos(center);

	DWORD iBitOff=i_math::imod_signed(posTile.x,SEG_UNIT_LEN);

	DWORD nSegs;
	ExploreSeg *segs=GetExploreSeg(iBitOff,nSegs,nRadiusInTile);
	if (!segs)
		return;

	for (int i=0;i<nSegs;i++)
	{
		i_math::pos2di posTile2;
		posTile2.x=i_math::idiv_signed(posTile.x,SEG_UNIT_LEN)*SEG_UNIT_LEN+segs[i].off.x*SEG_UNIT_LEN;
		posTile2.y=posTile.y+segs[i].off.y;

		i_math::pos2di posBlk=posTile2;
		posBlk.scale_signed(EXPLOREMAP_TILE_PER_BLOCK);
		if (!_rcBlk.isPointInside(posBlk))
			continue;

		DWORD idx=(posBlk.y-_rcBlk.Top())*_rcBlk.getWidth()+posBlk.x-_rcBlk.Left();
		ExploreMapBlock*&blk=_blks[idx];

		if (!blk)
		{
			blk=Class_New2(ExploreMapBlock);
			memset(blk->buf,0,sizeof(blk->buf));
			_masks[idx/8]|=(1<<(idx%8));
		}

		i_math::pos2di ptTileOff;
		ptTileOff.x=posTile2.x-posBlk.x*EXPLOREMAP_TILE_PER_BLOCK;
		ptTileOff.y=posTile2.y-posBlk.y*EXPLOREMAP_TILE_PER_BLOCK;

		BYTE *p=&blk->buf[ptTileOff.y*(EXPLOREMAP_TILE_PER_BLOCK/8)+ptTileOff.x/8];

		(*(DWORD*)p)|=segs[i].v;
	}

	_ver++;

}
