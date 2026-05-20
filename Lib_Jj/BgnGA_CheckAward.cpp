/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建道具
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"
#include "LevelRecordUpgrade.h"

#include "LevelOSB.h"

#include "BgnGA_RollAwards.h"
#include "BgnGA_CheckAward.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelAbility.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_CheckAward
BIND_BGN_CLASS(CBgnGA_CheckAward,CBgpGA_CheckAward);


void CBgnGA_CheckAward::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_CheckAward*pad=_GetPad<CBgpGA_CheckAward>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (TRUE)
	{
		RollAwardsResult *result=_GetMem()->GetObj<RollAwardsResult>(pad->award);
		if (result)
		{
			if (pad->op==CBgpGA_CheckAward::IsEmpty)
			{
				if (result->GetValidCount()<=0)
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
	}

	_OutputOk(outputs,2,"否");
	return;
}
