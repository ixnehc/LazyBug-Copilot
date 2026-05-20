#pragma once

#include "LevelDefines.h"



class CLevelTileMap
{
public:
	void Create(i_math::recti &rcMap);
	void Destroy(){}

	DWORD GetWidth()	{		return _w;	}
	DWORD GetHeight()	{		return _h;	}

	float GetHeight(LevelPos &pos)	{		return 0.0f;	}


protected:
	DWORD _w,_h;
	LevelPos _posStart;
};
