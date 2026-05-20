#pragma once

#include "class/class.h"

#include "micropather/micropather.h"

#include "UnitMgr.h"

class CTileMap:public MPGraph
{
public:
	CTileMap()
	{
		Zero();
	}

	void Zero()
	{
		_lenTile=1.0f;
		_wTile=0;
		_hTile=0;
	}

	void Create(DWORD w,DWORD h,float lenTile);
	void Destroy();

	void SetTile(DWORD x,DWORD y,BYTE v)
	{
		if ((x<_wTile)&&(y<_wTile))
			_tiles[y*_wTile+x]=v;
	}
	BYTE GetTile(DWORD x,DWORD y)
	{
		if ((x<_wTile)&&(y<_hTile))
		{
			return _tiles[y*_wTile+x];
		}
		return 0;
	}

	DWORD GetWidth()	{		return _wTile;	}
	DWORD GetHeight()	{		return _hTile;	}
	float GetTileLen()	{		return _lenTile;	}

	virtual float LeastCostEstimate( void* stateStart, void* stateEnd );
	virtual void AdjacentCost(void* state, std::vector< MPStateCost > *adjacent);
	virtual void  PrintStateInfo(void* state);

protected:


	float _lenTile;
	DWORD _wTile;
	DWORD _hTile;
	std::vector<BYTE>_tiles;

};


class CUnitMgrMP:public CUnitMgr
{
public:
	CUnitMgrMP()
	{
		_pather=NULL;
	}

	BOOL Create(DWORD w,DWORD h,float lenTile);
	void Destroy();
	CTileMap *GetMap()	{		return &_mp;	}

	BOOL FindPath(i_math::pos2d_sh &posSrc,i_math::pos2d_sh& posTarget,std::vector<i_math::pos2d_sh>&path);

	//返回两点间是否有静态障碍
	virtual BOOL StaticObstacleTest(i_math::vector2df &posSrc,float radius,i_math::vector2df &posTarget)
	{
		return FALSE;
	}

protected:

	CTileMap _mp;
	MicroPather *_pather;

};
