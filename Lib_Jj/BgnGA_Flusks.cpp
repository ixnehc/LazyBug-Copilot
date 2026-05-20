/********************************************************************
	created:	2017/2/15 
	author:		cxi
	
	purpose:	GA功能:修改SP
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_Flusks.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "Ability_HPFlusk.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_FillFlusks
BIND_BGN_CLASS(CBgnGA_FillFlusks,CBgpGA_FillFlusks);


void CBgnGA_FillFlusks::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_FillFlusks*pad=_GetPad<CBgpGA_FillFlusks>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			CLevelObj *lo=_GetTalkLo();
			extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp);
			CLevelAbility_HPFlusk *ability=(CLevelAbility_HPFlusk *)LevelUtil_GetActiveAbility(lo,LevelAbilityType_HPFlusk);
			if (ability)
				ability->Refill();
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}



////////////////////////////////////////////////////////////////////////
//CBgnGA_CheckFilledFlusks
BIND_BGN_CLASS(CBgnGA_CheckFilledFlusks,CBgpGA_CheckFilledFlusks);


void CBgnGA_CheckFilledFlusks::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_CheckFilledFlusks*pad=_GetPad<CBgpGA_CheckFilledFlusks>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			CLevelObj *lo=_GetTalkLo();
			extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp);
			CLevelAbility_HPFlusk *ability=(CLevelAbility_HPFlusk *)LevelUtil_GetActiveAbility(lo,LevelAbilityType_HPFlusk);
			if (ability)
			{
				if (ability->GetFilledCount()>0)
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
	}

	_OutputFail(outputs,2,"否");
	return;
}



////////////////////////////////////////////////////////////////////////
//CBgnGA_DecFilledFlusks
BIND_BGN_CLASS(CBgnGA_DecFilledFlusks,CBgpGA_DecFilledFlusks);


void CBgnGA_DecFilledFlusks::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_DecFilledFlusks*pad=_GetPad<CBgpGA_DecFilledFlusks>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			CLevelObj *lo=_GetTalkLo();
			extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp);
			CLevelAbility_HPFlusk *ability=(CLevelAbility_HPFlusk *)LevelUtil_GetActiveAbility(lo,LevelAbilityType_HPFlusk);
			if (ability)
				ability->DecFilled();
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}

