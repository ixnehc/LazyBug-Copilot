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

#include "BgnCheckSkillStage.h"
#include "Skill_GeneralAdvS.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckSkillStage,CBgp_CheckSkillStage);

void CBgn_CheckSkillStage::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckSkillStage*pad=_GetPad<CBgp_CheckSkillStage>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	if (CLevelSkill *skill=LevelUtil_GetCastingSkill(lo))
	{
		if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
		{
			Skill_GeneralAdvS *skillGeneral=(Skill_GeneralAdvS *)skill;

			if (!skillGeneral->ExistStage())
			{
				outputs.Add(1,_thrd);
				_SetResult(A_Ok);
				return;
			}
			else
			{
				for (int i=0;i<pad->nms.size();i++)
				{
					if (skillGeneral->CheckInStage(pad->nms[i]))
					{
						outputs.Add(i+2,_thrd);
						_SetResult(A_Ok);
						return;
					}
				}
				outputs.Add(pad->nms.size()+2,_thrd);
				_SetResult(A_Ok);
				return;
			}
		}
	}
	_SetResult(A_Fail);
}

