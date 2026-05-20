/********************************************************************
	created:	2019/10/0 1
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnSwitchSkillStage.h"

#include "Skill_GeneralAdvS.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_SwitchSkillStage,CBgp_SwitchSkillStage);

void CBgn_SwitchSkillStage::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SwitchSkillStage*pad=_GetPad<CBgp_SwitchSkillStage>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	if (CLevelSkill *skill=LevelUtil_GetCastingSkill(lo))
	{
		if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
		{
			Skill_GeneralAdvS *skillGeneral=(Skill_GeneralAdvS *)skill;
			skillGeneral->DoSwitchStage(pad->nm);
		}
	}

	_OutputOk(outputs,1,"结束");
}

