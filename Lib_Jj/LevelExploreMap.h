#pragma once

#include "LevelDefines.h"

#define EXPLOREMAP_RESO 4//每米有多少个像素

#define EXPLOREMAP_BLOCK_LEN (32*EXPLOREMAP_RESO) //以像素为单位,32米一个block
#define EXPLOREMAP_METER_PER_TILE (4) //4米一个tile
#define EXPLOREMAP_TILE_LEN (EXPLOREMAP_METER_PER_TILE*EXPLOREMAP_RESO) //以像素为单位,4米一个tile
#define EXPLOREMAP_TILE_PER_BLOCK (EXPLOREMAP_BLOCK_LEN/EXPLOREMAP_TILE_LEN)

#define MAX_EXPLORE_RADIUS (16) //In Tile

struct ExploreMapBlock
{
	DEFINE_CLASS(ExploreMapBlock);

	void CopyFrom(ExploreMapBlock *src)
	{
		memcpy(buf,src->buf,sizeof(buf));
	}

	BYTE buf[EXPLOREMAP_TILE_PER_BLOCK*EXPLOREMAP_TILE_PER_BLOCK/8];
};


class CLevelExploreMap
{
public:
	DEFINE_CLASS(CLevelExploreMap);
	CLevelExploreMap()
	{
		_ver=0;
	}
	void Init(i_math::recti &rcMap);//rcMap以米为单位
	void Clear();

	void ClearContent();//清空所有已被Explore的区域

	DWORD GetVer()	{		return _ver;	}

	void Save(CDataPacket &dp);
	void Load(CDataPacket &dp);

	void CopyFrom(CLevelExploreMap *src);

	i_math::pos2di TilePosFromWorldPos(LevelPos3D &center);

	void AddExplore(LevelPos3D &center,DWORD nRadiusInTile=2);//nRadius以tile为单位,一个Tile为8米

	i_math::recti &GetMapRect()	{		return _rcMap;	}

	//如果是TRUE,表示这个tile已经explore了
	//注意这个函数目前没有作是否在地图范围内的检查,调用时要小心
	BOOL GetTileValue(int xTile,int yTile)
	{
		i_math::pos2di posBlk(xTile,yTile);
		posBlk.scale_signed(EXPLOREMAP_TILE_PER_BLOCK);
		ExploreMapBlock*blk=_blks[(posBlk.y-_rcBlk.Top())*_rcBlk.getWidth()+posBlk.x-_rcBlk.Left()];
		if (!blk)
			return FALSE;
		int xOff=xTile-posBlk.x*EXPLOREMAP_TILE_PER_BLOCK;
		int yOff=yTile-posBlk.y*EXPLOREMAP_TILE_PER_BLOCK;

		BYTE *p=&blk->buf[yOff*(EXPLOREMAP_TILE_PER_BLOCK/8)+xOff/8];
		if ((*p)&(1<<(xOff%8)))
			return TRUE;
		return FALSE;
	}

	//做边界检测
	BOOL GetSafeTileValue(LevelPos &pos)
	{
		i_math::pos2di posTile;
		posTile.x=(int)floor(pos.x/(float)EXPLOREMAP_METER_PER_TILE);
		posTile.y=(int)floor(pos.y/(float)EXPLOREMAP_METER_PER_TILE);
		return GetSafeTileValue(posTile);
	}

	BOOL GetSafeTileValue(i_math::pos2di posTile)
	{
		i_math::pos2di pos2;
		pos2=posTile*EXPLOREMAP_METER_PER_TILE;
		if (!_rcMap.isPointInside(pos2))
			return FALSE;//边界外,未探索
		return GetTileValue(posTile.x,posTile.y);
	}


protected:
	void _ClearContent();
	std::vector<BYTE>_masks;
	std::vector<ExploreMapBlock*> _blks;

	i_math::recti _rcMap;//以米为单位
	i_math::recti _rcBlk;//以Block为单位

	DWORD _ver;

};
