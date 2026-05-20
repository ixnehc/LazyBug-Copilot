/********************************************************************
	created:	2022/4/18 
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

#include "BgnGA_RollPoemAwards.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LevelUtil.h"

#include "LevelPoem.h"




////////////////////////////////////////////////////////////////////////
//CBgnGA_RollPoemAwards
BIND_BGN_CLASS(CBgnGA_RollPoemAwards,CBgpGA_RollPoemAwards);

extern void RollAwards(CLevelPlayer *player,RollAwardParam &param,RollAwardsResult &result);


void CBgnGA_RollPoemAwards::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollPoemAwards*pad=_GetPad<CBgpGA_RollPoemAwards>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();

	LevelBehaviorContext *ctx=_GetCtx();

	//清空results
	RollAwardsResult *result=NULL;
	if (pad->awards!=StringID_Invalid)
	{
		_GetMem()->DepositObj(pad->awards,NULL);
	}

	BOOL bChoose=FALSE;
	LevelAbilityType tpAbility=LevelUtil_AbilityFromArtifact(player,pad->tpArtifact);
	if (tpAbility!=LevelAbilityType_None)
	{
		CLevelAbility *ability=LevelUtil_GetActiveAbility(player,tpAbility);
		if (ability)
		{
			PoemAwards *awards=ability->FindPoemAwards(pad->nm);
			if (awards)
			{
				result=Class_New(RollAwardsResult);
				RollAwards(player,awards->award,*result);
				_GetMem()->DepositObj(pad->awards,result);
				bChoose=awards->bAllowChoose;
			}
		}
	}

	if (!bChoose)
		_OutputOk(outputs,1,"不需要选择");
	else
		_OutputOk(outputs,2,"需要选择");

	return;
}
