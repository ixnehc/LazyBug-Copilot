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

#include "BgnGA_断桥_StartRepair.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "Ability_UtumTide.h"

  
////////////////////////////////////////////////////////////////////////
//CBgnGA_断桥_StartRepair
BIND_BGN_CLASS(CBgnGA_断桥_StartRepair,CBgpGA_断桥_StartRepair);


void CBgnGA_断桥_StartRepair::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_断桥_StartRepair*pad=_GetPad<CBgpGA_断桥_StartRepair>();
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
			int nRequiredLabor=pad->_nRequiredLabor; 
			if (ability->GetAvailableCount()>=nRequiredLabor)
			{
				ability->StartRepairBridge(lo->GetID(),nRequiredLabor);
				bFail=FALSE;
			}
		}
	}

	if (bFail)
		_OutputFail(outputs,2,"失败");
	else
		_OutputOk(outputs,1,"成功");

	return;
}
