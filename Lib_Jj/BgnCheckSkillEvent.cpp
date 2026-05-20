/********************************************************************
	created:	2019/09/23 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelObj.h"

#include "BgnCheckSkillEvent.h"

#include "Skill_GeneralS.h"
#include "Skill_GeneralAdvS.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckSkillEvent

BIND_BGN_CLASS(CBgn_CheckSkillEvent,CBgp_CheckSkillEvent);

void CBgn_CheckSkillEvent::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckSkillEvent*pad=_GetPad<CBgp_CheckSkillEvent>();

	if (!_Update(outputs))
	{
		if (!pad->bWait)
			_OutputFail(outputs,2,"未检测到");
	}
	else
		_OutputOk(outputs,1,"检测到");
}

BOOL CBgn_CheckSkillEvent::_Update(BGNOutputs &outputs)
{
	CBgp_CheckSkillEvent*pad=_GetPad<CBgp_CheckSkillEvent>();

	LevelBehaviorContext *ctx=_GetCtx();
	CLevel *level=ctx->level;
	CLevelObj *lo=NULL;

	lo=_GetLo();
	if (!lo)
		return FALSE;

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
	if (!skill)
		return FALSE;

	if (pad->e==StringID_Invalid)
		return FALSE;

	if(skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
	{
		return ((Skill_GeneralAdvS*)skill)->CheckCastingEvent(pad->e);
	}
	return FALSE;
}


void CBgn_CheckSkillEvent::Update(BGNOutputs &outputs)
{
	if (_Update(outputs))
		_OutputOk(outputs,1,"检测到");
}
