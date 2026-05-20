#pragma once

#include "class/class.h"

#include "LevelDefines.h"



class CUnit;
class CLevelObj;
class CLevel;
class CLevelInactives
{
public:
	CLevelInactives()
	{
		_w=_h=0;
		_idxGC=0;
	}

	struct Block
	{
	public:
		void Clear();
		void GarbageCollect();

		std::deque<CLevelObj*>objs;
	};

	void Create(i_math::recti &rcMap);//rcMap单位为米
	void Destroy();

	float GetBlockLen()	{		return LEVEL_AOA_BLOCKLEN;	}

	BOOL Add(CLevelObj *obj);

	DWORD GetWidth()	{		return _w;	}
	DWORD GetHeight()	{		return _h;	}

	//更新激活区域,并激活那些新激活区域中的obj
	void UpdatePlayerAoa(AoaCenter&center,LevelPos&posCur0,CLevel *level);

	Block*GetBlock(DWORD x,DWORD y);

	void GarbageCollect();

public:
	LevelPos _posStart;
	int _w,_h;//以Block为单位

	std::vector<Block> _blocks;

	DWORD _idxGC;


};