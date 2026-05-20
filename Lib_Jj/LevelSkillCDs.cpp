
#include "stdh.h"

#include "LevelSkillCDs.h"

#include "Level.h"

#include "LevelObj.h"

#include "LevelRecordSkill.h"

void CLevelSkillCDs::Clear()
{
	_cds.clear();
}

void CLevelSkillCDs::StartCD(LevelRecordSkill *rec)
{
	if (!rec)
		return;
	if (rec->CoolDown>0)
		_cds[FORCE_TYPE(DWORD,rec)]=rec->CoolDown;
}


BOOL CLevelSkillCDs::CheckCDOver(LevelRecordSkill *rec)
{
	std::unordered_map<DWORD,AnimTick>::iterator it=_cds.find(FORCE_TYPE(DWORD,rec));
	if (it==_cds.end())
		return TRUE;

	return FALSE;
}


void CLevelSkillCDs::Update(AnimTick dt)
{
	CLevelBuffs *buffs=_owner->GetBuffs();

	if (buffs)
	{
		float ias=buffs->GetIAS();
		dt=(AnimTick)(((float)dt)*ias);
	}

	std::unordered_map<DWORD,AnimTick>::iterator it=_cds.begin(),itCur;
	while(it!=_cds.end())
	{
		itCur=it;
		it++;

//		SheetRecordSkill *rec=FORCE_TYPE(SheetRecordSkill*,(*it).first);

		if ((*itCur).second>dt)
			(*itCur).second-=dt;
		else
			_cds.erase(itCur);//时间到
	}
	

}
