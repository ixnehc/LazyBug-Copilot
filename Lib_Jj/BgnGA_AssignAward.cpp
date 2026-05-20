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
#include "BgnGA_AssignAward.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelAbility.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_AssignAward
BIND_BGN_CLASS(CBgnGA_AssignAward,CBgpGA_AssignAward);

BOOL CBgnGA_AssignAward::_Assign(CLevelPlayer *player,LevelAward *award,LevelAwardPrice *price)
{
	CBgpGA_AssignAward*pad=_GetPad<CBgpGA_AssignAward>();

	extern BOOL LevelUtil_ApplyAward(CLevelPlayer *player,LevelAward *award,LevelAwardPrice *price);
	if (!LevelUtil_ApplyAward(player,award,price))
		return FALSE;

	if (pad->refTreasurePickAgent.IsValid())
	{
		extern void LevelUtil_RevealTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelResourceType tpRes,RecordID idItem);
		if (award->tp==LevelAward::Item)
		{
			if (award->idRec!=RecordID_Invalid)
				LevelUtil_RevealTreasureInfo(player,pad->refTreasurePickAgent.idMap,pad->refTreasurePickAgent.guid,LevelResource_None,award->idRec);
		}
		if (award->tp==LevelAward::Resource)
		{
			if (award->tpRes!=LevelResource_None)
				LevelUtil_RevealTreasureInfo(player,pad->refTreasurePickAgent.idMap,pad->refTreasurePickAgent.guid,award->tpRes,RecordID_Invalid);
		}
		
	}

	award->bValid=FALSE;

	return TRUE;
}

void CBgnGA_AssignAward::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_AssignAward*pad=_GetPad<CBgpGA_AssignAward>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();

	LevelBehaviorContext *ctx=_GetCtx();

	if (TRUE)
	{
		RollAwardsResult *result=_GetMem()->GetObj<RollAwardsResult>(pad->award);
		RollAwardsPrice *priceAward=_GetMem()->GetObj<RollAwardsPrice>(pad->price);
		if (priceAward)
			priceAward->UpdateAffordable(player);

		if (result)
		{
			BOOL bAnyAssigned=FALSE;
			if (pad->idxAward<(int)result->awards.size())
			{
				LevelBehaviorContext *ctx=_GetCtx();
				if (player)
				{
					LevelPlayerStates *lps=player->GetLPS();
					if (lps)
					{
						if (pad->idxAward<0)
						{
							for (int i=0;i<result->awards.size();i++)
							{
								LevelAwardPrice *price=NULL;
								if (priceAward)
								{
									if (i<priceAward->prices.size())
										price=&priceAward->prices[i];
								}
								if (_Assign(player,&result->awards[i],price))
								{
									bAnyAssigned=TRUE;
									if (priceAward)
										priceAward->UpdateAffordable(player);
								}
							}
						}
						else
						{
							LevelAwardPrice *price=NULL;
							if (priceAward)
							{
								if (pad->idxAward<priceAward->prices.size())
									price=&priceAward->prices[pad->idxAward];
							}
							if (_Assign(player,&result->awards[pad->idxAward],price))
							{
								bAnyAssigned=TRUE;
								if (priceAward)
									priceAward->UpdateAffordable(player);
							}
						}
					}
				}
			}
			if (bAnyAssigned)
			{
				_GetMem()->DepositObj(pad->award,result);
				if (priceAward)
					_GetMem()->DepositObj(pad->price,priceAward);
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
