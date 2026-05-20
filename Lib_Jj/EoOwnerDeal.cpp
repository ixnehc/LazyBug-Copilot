
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoOwnerDeal.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoOwnerDeal,EoParamOwnerDeal);


void EoOwnerDeal::_OnDetroy()
{
}



void EoOwnerDeal::_OnPostCreate()
{
	CLevelObj *loOwner=_GetOwner();
	if (loOwner)
		_idOwner=loOwner->GetID();

	_tCasting=_GetSkillCastingTime();
}

void EoOwnerDeal::_OnUpdate()
{
	if (_idOwner!=LevelObjID_Invalid)
	{
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		CLevelObj *loOwner=LevelUtil_GetAliveLo(_level,_idOwner);
		if (loOwner)
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
			_MakeDeals(LevelOSB(loOwner),loOwner,arg);
		}
	}

	DeferDestroy();
}

