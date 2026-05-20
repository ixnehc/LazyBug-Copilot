
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoObjDeal.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoObjDeal,EoParamObjDeal);


void EoObjDeal::_OnDetroy()
{
}



void EoObjDeal::_OnPostCreate()
{
	EoParamObjDeal *param=GetParam<EoParamObjDeal>();
	if (param)
	{
		if (param->tpTarget==0)
		{
			CLevelObj *loOwner=_GetOwner();
			if (loOwner)
				_idObj=loOwner->GetID();
		}
		if (param->tpTarget==1)
		{
			LevelSkillTarget *targetSkill=_GetSkillTarget();
			if (targetSkill)
				_idObj=targetSkill->ObjID();
		}
	}

	_tCasting=_GetSkillCastingTime();

	if (_idObj!=LevelObjID_Invalid)
	{
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,_idObj);
		if (lo)
		{
			DealArg arg;
			arg.grd=1;
			if (_opBirth)
				arg.link=_opBirth->GetDesc().link;
			else
			{
				arg.link.id=GetLevel()->GenOpLinkID();
				arg.link.t=_tCasting;
			}
			CLevelObj *loOwner=_GetOwner();
			if (loOwner)
				_MakeDeals(LevelOSB(loOwner),lo,arg);
			else
				_MakeDeals(lo,arg);
		}
	}
}

void EoObjDeal::_OnUpdate()
{

	DeferDestroy();
}

