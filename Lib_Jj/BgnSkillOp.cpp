/********************************************************************
	created:	2019/10/0 1
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnSkillOp.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_SkillOp,CBgp_SkillOp);

void CBgn_SkillOp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SkillOp*pad=_GetPad<CBgp_SkillOp>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	if (pad->_op!=SkillParam_GeneralAdvS::OpEntry::Op_None)
	{
		if (CLevelSkill *skill=LevelUtil_GetCastingSkill(lo))
		{
			if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
			{
				Skill_GeneralAdvS *skillGeneral=(Skill_GeneralAdvS *)skill;
				SkillParam_GeneralAdvS::OpEntry entryOp;
				entryOp.op=pad->_op;
				entryOp.weaks.CopyFrom(pad->weaks);
				entryOp.bWeaksCanTakeOver=FALSE;//目前设置的弱点不支持接管
				skillGeneral->OnOp(entryOp,NULL);
			}
		}
	}
	_OutputOk(outputs,1,"结束");
}

