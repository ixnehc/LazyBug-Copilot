
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoInvoke.h"

#include "LevelRecords.h"

#include "LevelOSB.h"
#include "LevelTalks.h"

#include "LevelUtil.h"


BIND_EOPARAM(EoInvoke,EoParamInvoke);


void EoInvoke::_OnDetroy()
{
}



void EoInvoke::_OnPostCreate()
{
}

void EoInvoke::_OnUpdate()
{
	CLevelSkill *skill=_GetOwnerSkill();
	if (skill)
	{
		LevelSkillTarget &target=skill->GetTarget();
		CLevelObj *loTarget=LevelUtil_GetTargetObj(_level,target);
		if (loTarget)
		{
			CLevelTalks *talks=loTarget->GetTalks();
			if (talks)
			{
				if (_GetOwner())
				{
					talks->Query(_GetOwner()->GetPlayerID());
				}
			}
		}
	}

	DeferDestroy();
}

