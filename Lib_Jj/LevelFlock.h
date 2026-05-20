#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "LevelObj.h"


class CLevelFlock
{
public:
	CLevelFlock()
	{
		Zero();
	}
	void Zero()
	{
	}
	void Init(CLevelObj*owner);
	void Clear();

	void Build(UnitTypeID_ tid,float radius,DWORD count);//一次性建立一定规模的Flock
	void Add(UnitTypeID_ tid,i_math::vector2df &pos,DWORD count);//

	void Update(AnimTick t);

protected:
	LevelPlayerID _idPlayer;
	std::vector<CLevelObj*> _members;
};