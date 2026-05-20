/********************************************************************
	created:	2023/2/19 
	author:		cxi
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
#include "BgnGA_RollSlatesAwards.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoSlatesA.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_RollSlatesAwards
BIND_BGN_CLASS(CBgnGA_RollSlatesAwards,CBgpGA_RollSlatesAwards);

void CBgnGA_RollSlatesAwards::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollSlatesAwards*pad=_GetPad<CBgpGA_RollSlatesAwards>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();

	LevelBehaviorContext *ctx=_GetCtx();
 

	//清空results
	if (pad->awards!=StringID_Invalid)
		_GetMem()->DepositObj(pad->awards,NULL);

	extern CLoSlatesA *LevelUtil_GetSlatesAFromEmbed(CLevelObj *lo);
	CLoSlatesA *loSlates=LevelUtil_GetSlatesAFromEmbed(lo);
	if (loSlates)
	{
		RollAwardParam *param=loSlates->GetRollAwardParam();
		if (param)
		{
			RollAwardsResult *result=Class_New(RollAwardsResult);
			extern void RollAwards(CLevelPlayer *player,RollAwardParam &param,RollAwardsResult &result);
			RollAwards(player,*param,*result);
			_GetMem()->DepositObj(pad->awards,result);
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
