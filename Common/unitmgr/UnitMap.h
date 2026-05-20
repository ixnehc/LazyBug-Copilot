#pragma once

#include "class/class.h"

#define UNITMAP_TILE_LEN 2.0f//单位为米
#define UNITMAP_BLOCK_LEN 32.0f//单位为米

#define UNITMAP_TILE_PER_BLOCK ((int)(UNITMAP_BLOCK_LEN/UNITMAP_TILE_LEN))

struct UnitTile;
class CUnitBase
{
public:
	CUnitBase()
	{
		_next=NULL;
		_tile=NULL;
	}

	i_math::vector2df &GetPos()	{		return _pos;	}
	void SetPos(i_math::vector2df &pos)	{		_pos=pos;	}

	i_math::vector2df _pos;

protected:
	i_math::pos2d_sh _ptUnitMapTile;//位于UnitMap中哪一个tile
	CUnitBase *_next;//用于UnitMapTile里的链表
	UnitTile *_tile;

	friend class CUnitMap;
};


class CUnitBlock;
struct UnitTile
{
	CUnitBase *units;//unit的链表
	CUnitBlock *blk;
};

class CUnitBlock
{
public:
	DEFINE_CLASS(CUnitBlock);
	CUnitBlock()
	{
		_nUnits=0;
		memset(_tiles,0,sizeof(_tiles));
		for (int i=0;i<ARRAY_SIZE(_tiles);i++)
			_tiles[i].blk=this;
	}

public:
	UnitTile _tiles[UNITMAP_TILE_PER_BLOCK*UNITMAP_TILE_PER_BLOCK];
	int _nUnits;//block里一共有多少个units

};

class CUnitMap
{
public:
	CUnitMap()
	{
		_w=_h=0;
	}
	void Create(i_math::recti &rcMap);//rcMap单位为米
	void Destroy();

	void AddUnit(CUnitBase *unit);
	void RemoveUnit(CUnitBase *unit);
	void UpdateUnit(CUnitBase *unit);

	//设进去的radius只是一个参考值,不能保证枚举出的CUnit都在这个半径之内,外部需要自行检测
	//注意,Enum时,传入的位置与Rect都是在世界坐标系里
	void Enum(CUnitBase *unitCenter,float radius);
	void Enum(i_math::vector2df &center,float radius);
	void Enum(i_math::rectf &rc0)
	{
		i_math::rectf rc=rc0;
		rc.Left()-=_xStart;
		rc.Right()-=_xStart;
		rc.Top()-=_yStart;
		rc.Bottom()-=_yStart;
		_Enum(rc);
	}
	CUnitBase **GetEnums(DWORD &c)
	{
		c=_enum.size();
		return _enum.data();
	}

	void GarbageCollect();

public:

	void _Enum(i_math::rectf &rc);

	float _xStart,_yStart;
	int _wTile,_hTile;//以tile为单位
	int _w,_h;//以Block为单位

	std::vector<CUnitBlock *> _blocks;

	std::vector<CUnitBase*>_enum;




};