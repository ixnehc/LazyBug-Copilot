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
#include "LevelRecordGoods.h"

#include "LevelOSB.h"

#include "BgnGA_RollAwards.h"

#include "BgnGA_RollGoods.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoItem.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_RollGoods
BIND_BGN_CLASS(CBgnGA_RollGoods,CBgpGA_RollGoods);


void AddAward(LevelRecordGoods &goods,RollAwardsResult &awards,RollAwardsPrice &prices,int idx)
{
	LevelAward award;
	LevelAwardPrice price;
	award.tp=goods.tp;
	if (goods.tp==LevelAward::Item)
		award.idRec=goods.idItem;
	if (goods.tp==LevelAward::Resource)
		award.tpRes=goods.tpRes;
	award.count=CSysRandom::RandRangeInt(goods.nMin,goods.nMax+1);

	award.bValid=1;

	price.tpRes=LevelResource_Gold;
	price.count=goods.price;

	if (idx<0)
	{
		awards.awards.push_back(award);
		prices.prices.push_back(price);
	}
	else
	{
		awards.awards[idx]=award;
		prices.prices[idx]=price;
	}
}

BOOL CheckSameAward(LevelRecordGoods &goods,LevelAward &award)
{
	if (goods.tp!=award.tp)
		return FALSE;
	if (goods.tp==LevelAward::Item)
	{
		if (award.idRec!=goods.idItem)
			return FALSE;
	}
	if (goods.tp==LevelAward::Resource)
	{
		if (award.tpRes!=goods.tpRes)
			return FALSE;
	}
	if (!((goods.nMin<=award.count)&&(goods.nMax>=award.count)))
		return FALSE;

	return TRUE;
}

void RollGoods(CLevelPlayer *player,BOOL bReplenish,DWORD nAwards,RollAwardsResult &awards,RollAwardsPrice &prices)
{
	CLevel *level=player->GetLevel();

	extern WORD LevelUtil_GetHonor(CLevelObj *lo);
	int honor=LevelUtil_GetHonor((CLevelObj*)player->GetLoUnit());

	if (!bReplenish)
	{
		awards.GClear();
		prices.GClear();
	}

	LevelPlayerStates *lps=player->GetLPS();

	CRecords *records=level->GetRecords()->GetRecords_Goods();

	std::vector<LevelRecordGoods*>candidates;
	{
		DWORD c;
		RecordID *ids=records->GetRecords(c);
		for (int i=0;i<c;i++)
		{
			LevelRecordGoods *rec=(LevelRecordGoods *)records->GetRecord(ids[i]);
			if (!rec)
				continue;
			if (rec->hnrMin>honor)
				continue;

			if ((rec->tp==LevelAward::Resource)&&(rec->tpRes==LevelResource_Labor))
			{
				extern CLevelAbility *LevelUtil_GetActiveAbility(CLevelObj *lo,LevelAbilityType tp);
				if (!LevelUtil_GetActiveAbility((CLevelObj*)player->GetLoUnit(),LevelAbilityType_UtumTide))
					continue;
			}

			if (rec->tp==LevelAward::Item)
			{
				LevelRecordItem *recItem=level->GetRecords()->GetItem(rec->idItem);
				if (recItem)
				{
					if (!recItem->bAllowStack)
					{
						if (LPS_CheckItemMemory(lps,rec->idItem))
							continue;
					}
				}

			}

			if (bReplenish)
			{
				BOOL bExist=FALSE;
				for (int j=0;j<awards.awards.size();j++)
				{
					LevelAward &award=awards.awards[j];
					if (!award.bValid)
						continue;
					if (CheckSameAward(*rec,award))
					{
						bExist=TRUE;
						break;
					}
				}
				if (bExist)
					continue;
			}

			candidates.push_back(rec);
		}
	}

	if (!bReplenish)
	{
		for (int i=0;i<nAwards;i++)
		{
			LevelRecordGoods* rec=CSysRandom::RollWeighted(candidates);
			if (!rec)
				break;
			AddAward(*rec,awards,prices,-1);
			VEC_REMOVE(candidates,rec);
		}
	}
	else
	{
		for (int j=0;j<awards.awards.size();j++)
		{
			LevelAward &award=awards.awards[j];
			if (award.bValid)
				continue;

			LevelRecordGoods* rec=CSysRandom::RollWeighted(candidates);
			if (!rec)
				break;
			AddAward(*rec,awards,prices,j);

			VEC_REMOVE(candidates,rec);
		}
	}

	prices.UpdateAffordable(player);

}


void CBgnGA_RollGoods::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollGoods*pad=_GetPad<CBgpGA_RollGoods>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();

	LevelBehaviorContext *ctx=_GetCtx();

	if (pad->mode==2)
	{
		if (pad->prices!=StringID_Invalid)
		{
			RollAwardsPrice *price=_GetMem()->GetObj<RollAwardsPrice>(pad->prices);
			if (price)
			{
				price->UpdateAffordable(player);
				_GetMem()->DepositObj(pad->prices,price);
			}
		}
	}
	else
	{
		if ((pad->awards!=StringID_Invalid)&&(pad->prices!=StringID_Invalid))
		{
			BOOL bReplenish=(pad->mode==1);
			if (!bReplenish)
			{
				RollAwardsResult *result=Class_New(RollAwardsResult);
				RollAwardsPrice * price=Class_New(RollAwardsPrice);

				RollGoods(player,FALSE,pad->count,*result,*price);

				_GetMem()->DepositObj(pad->awards,result);
				_GetMem()->DepositObj(pad->prices,price);
			}
			else
			{
				RollAwardsResult *result=_GetMem()->GetObj<RollAwardsResult>(pad->awards);
				RollAwardsPrice *price=_GetMem()->GetObj<RollAwardsPrice>(pad->prices);
				if (result&&price)
				{
					RollGoods(player,TRUE,pad->count,*result,*price);

					_GetMem()->DepositObj(pad->awards,result);
					_GetMem()->DepositObj(pad->prices,price);
				}
			}

		}
	}


	_OutputOk(outputs,1,"结束");
	return;
}
