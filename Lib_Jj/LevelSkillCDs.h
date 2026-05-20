#pragma once

#include "class/class.h"
#include "anim/animdefines.h"

#include "LevelDefines.h"

#include <unordered_map>

struct LevelRecordSkill;

//技能CoolDown的管理
class CLevelObj;
class CLevelSkillCDs
{
public:
	CLevelSkillCDs()
	{
		_owner=NULL;
	}
	void Init(CLevelObj *lo)
	{
		_owner=lo;
	}

	void Clear();

	void Update(AnimTick dt);

	void StartCD(LevelRecordSkill *rec);
	BOOL CheckCDOver(LevelRecordSkill *rec);


protected:
	std::unordered_map<DWORD,AnimTick> _cds;

	CLevelObj *_owner;


};

