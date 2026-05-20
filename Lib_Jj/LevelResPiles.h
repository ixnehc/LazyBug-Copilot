#pragma once

#include "class/class.h"
#include <unordered_map>

#include "LevelDefines.h"

struct LevelResPile
{
public:
	LevelResPile()
	{
		Zero();
	}
	void Zero()
	{
		idOwner=LevelObjID_Invalid;
		tp=LevelResource_None;
		amount=0;
	}

	LevelObjID idOwner;
	LevelResourceType tp;
	int amount;
};


class CLevelResPiles
{
public:
	CLevelResPiles()
	{
		Zero();
	}
	~CLevelResPiles()
	{
		Clear();
	}
	void Zero()
	{
		_level=NULL;
	}
	void Init(CLevel *level);
	void Clear();
	
	void Deposit(LevelObjID idOwner,LevelResourceType tp,int amount);
	int Fetch(LevelObjID idOwner,LevelResourceType tp,int amount);//返回Fetch的数量
	int GetAmount(LevelObjID idOwner,LevelResourceType tp);

protected:

	std::deque<LevelResPile>_piles;

	CLevel *_level;
};

