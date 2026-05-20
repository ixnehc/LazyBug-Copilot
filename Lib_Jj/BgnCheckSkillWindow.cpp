/********************************************************************
	created:	2019/09/23 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelObj.h"

#include "BgnCheckSkillWindow.h"

#include "Skill_GeneralS.h"
#include "Skill_GeneralAdvS.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckSkillWindow

BIND_BGN_CLASS(CBgn_CheckSkillWindow,CBgp_CheckSkillWindow);

void CBgn_CheckSkillWindow::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckSkillWindow*pad=_GetPad<CBgp_CheckSkillWindow>();

	if (!_Update(outputs))
	{
		if (!pad->bWait)
			_OutputFail(outputs,2,"未检测到");
	}
	else
		_OutputOk(outputs,1,"检测到");
}

BOOL CBgn_CheckSkillWindow::_Update(BGNOutputs &outputs)
{
	CBgp_CheckSkillWindow*pad=_GetPad<CBgp_CheckSkillWindow>();

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

	if (pad->open==StringID_Invalid)
		return FALSE;

	if(skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralS)))
	{
		return ((Skill_GeneralS*)skill)->CheckEventWindow(pad->open,pad->close);
	}
	if(skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
	{
		return ((Skill_GeneralAdvS*)skill)->CheckEventWindow(pad->open,pad->close);
	}
	return FALSE;
}


void CBgn_CheckSkillWindow::Update(BGNOutputs &outputs)
{
	if (_Update(outputs))
		_OutputOk(outputs,1,"检测到");
}
