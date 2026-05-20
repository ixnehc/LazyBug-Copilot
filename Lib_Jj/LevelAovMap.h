#pragma once

#include "LevelDefines.h"


//一个Block的map,每个Block记录一个LevelPlayerMask,表示这个Block所对应的区域被哪些Player所看到
class CLevelAovMap
{
public:
	void Create(i_math::recti &rcMap);

	void UpdatePlayerAov(LevelPlayerMask mask,AovCenter&center,LevelPos&posCur);
	void ClearPlayerAov(LevelPlayerMask mask,AovCenter&center);

	LevelPlayerMask GetPlayerMask(LevelPos &pos);

	DWORD GetWidth()	{		return _w;	}
	DWORD GetHeight()	{		return _h;	}


protected:
	std::vector<LevelPlayerMask> _buf;
	DWORD _w,_h;
	LevelPos _posStart;
};
