/********************************************************************
	created:	2019/10/0 1
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordItemClass.h"

#include "BgnCheckSkillStageAge.h"
#include "Skill_GeneralAdvS.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckSkillStageAge,CBgp_CheckSkillStageAge);

void CBgn_CheckSkillStageAge::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckSkillStageAge*pad=_GetPad<CBgp_CheckSkillStageAge>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	if (CLevelSkill *skill=LevelUtil_GetCastingSkill(lo))
	{
		if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
		{
			Skill_GeneralAdvS *skillGeneral=(Skill_GeneralAdvS *)skill;

			if (skillGeneral->ExistStage())
			{
				AnimTick t=skillGeneral->GetStageAge();
				float fT=ANIMTICK_TO_SECOND(t);
				if((fT>=pad->tMin)&&(fT<=pad->tMax))
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
	}
	_OutputFail(outputs,2,"否");
}

