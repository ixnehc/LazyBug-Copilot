/********************************************************************
	created:	2022/05/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "BgnGA_断桥_EndRepair.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "Ability_UtumTide.h"

  
////////////////////////////////////////////////////////////////////////
//CBgnGA_断桥_EndRepair
BIND_BGN_CLASS(CBgnGA_断桥_EndRepair,CBgpGA_断桥_EndRepair);


void CBgnGA_断桥_EndRepair::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_断桥_EndRepair*pad=_GetPad<CBgpGA_断桥_EndRepair>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	BOOL bFail=TRUE;
	CLevelPlayer *player=_GetTalkPlayer();
	if (player)
	{
		CLevelAbility_UtumTide *ability= (CLevelAbility_UtumTide *)LevelUtil_GetActiveAbility(player,LevelAbilityType_UtumTide);
		if (ability)
		{
			ability->EndRepairBridge(lo->GetID(),pad->bAbort);
		}
	}

	_OutputOk(outputs,1,"结束");

	return;
}
