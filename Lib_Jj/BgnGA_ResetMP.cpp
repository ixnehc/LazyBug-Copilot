/********************************************************************
	created:	2022/7/16 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "behaviorgraph/BehaviorGraphs.h"

#include "BgnGA_ResetMP.h"

#include "Ability_MagicRing.h"


 

////////////////////////////////////////////////////////////////////////
//CBgnGA_ResetMP
BIND_BGN_CLASS(CBgnGA_ResetMP,CBgpGA_ResetMP);


void CBgnGA_ResetMP::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ResetMP*pad=_GetPad<CBgpGA_ResetMP>(); 
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelObj *lo=_GetTalkLo();
		if(lo)
		{
			extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp);
			CLevelAbility_MagicRing *ability=(CLevelAbility_MagicRing *)LevelUtil_GetActiveAbility(lo,LevelAbilityType_MagicRing);
			if (ability)
				ability->ResetMP();
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
