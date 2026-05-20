/********************************************************************
	created:	2020/02/24 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LoAgent.h"
#include "LevelUtil.h"

#include "BgnGetSkillDynObstacle.h"

#include "Skill_GeneralAdvS.h"

////////////////////////////////////////////////////////////////////////
//CBgn_GetSkillDynObstacle
BIND_BGN_CLASS(CBgn_GetSkillDynObstacle,CBgp_GetSkillDynObstacle);
void CBgn_GetSkillDynObstacle::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetSkillDynObstacle*pad=_GetPad<CBgp_GetSkillDynObstacle>();

	LevelObjID idObstacle=LevelObjID_Invalid;

	CLevelObj *lo=_GetLo();
	if (lo)
	{
		extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
		CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
		if (!skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
			return;
		Skill_GeneralAdvS *skillG=(Skill_GeneralAdvS *)skill;

		idObstacle=skillG->GetRecentDynObstacle();
	}

	if (pad->nmVar!=StringID_Invalid)
		_SetID(pad->nmVar,BehaviorMemType_ObjID,idObstacle);

	if (idObstacle!=LevelObjID_Invalid)
		_OutputOk(outputs,1,"成功");
	else
		_OutputOk(outputs,2,"失败");
}
