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

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoItem.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//RollAwardsResult
void RollAwardsResult::UpdateExpendable(CLevelPlayer *player)
{
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		if (lps)
		{
			for (int i=0;i<awards.size();i++)
			{
				LevelAward *award=&awards[i];

				extern void LevelUtil_UpdateAwardExpendable(LevelPlayerStates *lps,LevelAward *award);
				LevelUtil_UpdateAwardExpendable(lps,award);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//RollAwardsPrice
void RollAwardsPrice::UpdateAffordable(CLevelPlayer *player)
{
	LevelPlayerStates *lps=player->GetLPS();
	if (lps)
	{
		for (int i=0;i<prices.size();i++)
		{
			LevelAwardPrice *price=&prices[i];

			extern void LevelUtil_UpdateAwardPriceAffordable(LevelPlayerStates *lps,LevelAwardPrice *price);
			LevelUtil_UpdateAwardPriceAffordable(lps,price);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//RollAwardsResult
void RollAwardsResult::ApplyMerge()
{
	if (awards.size()<=1)
		return;

	int c=1;
	for (int i=1;i<awards.size();i++)
	{
		BOOL bMerged=FALSE;
		for (int j=0;j<c;j++)
		{
			if (awards[j].MergeFrom(awards[i]))
			{
				bMerged=TRUE;
				break;
			}
		}

		if (bMerged)
			continue;
		awards[c++]=awards[i];
	}
	awards.resize(c);
}


////////////////////////////////////////////////////////////////////////
//CBgnGA_RollAwards
BIND_BGN_CLASS(CBgnGA_RollAwards,CBgpGA_RollAwards);
IMPLEMENT_CLASS(RollAwardsResult);
IMPLEMENT_CLASS(RollAwardsPrice);


RollAwardsPrice *CBgnGA_RollAwards::_EvalPrice(RollAwardsResult *result,CLevelPlayer *player)
{
	if (!result)
		return NULL;

	CLevel *level=_GetLevel();
	CLevelRecords *records=level->GetRecords();

	RollAwardsPrice * priceAwards=Class_New(RollAwardsPrice);

	priceAwards->prices.resize(result->awards.size());

	for (int i=0;i<result->awards.size();i++)
	{
		LevelAward *award=&result->awards[i];
		LevelAwardPrice *price=&priceAwards->prices[i];
		if (award->tp==LevelAward::Item)
		{
			LevelRecordItem *item=records->GetItem(award->idRec);
			if (item)
			{
				if (item->priceVendor.gold>0)
				{
					price->tpRes=LevelResource_Gold;
					price->count=CSysRandom::RandVary(item->priceVendor.gold,item->priceVendor.goldVary);
				}
				if (item->priceVendor.gem>0)
				{
					price->tpRes=LevelResource_Gem;
					price->count=CSysRandom::RandVary(item->priceVendor.gem,item->priceVendor.gemVary);
				}
			}
		}
	}

	priceAwards->UpdateAffordable(player);

	return priceAwards;
}

void RollAwards(CLevelPlayer *player,RollAwardParam &param,RollAwardsResult &result)
{
	CLevel *level=player->GetLevel();

	result.GClear();

	int c=0;
	if (TRUE)
	{
		std::vector<RollAwardCountEntry*>counts;
		counts.resize(param.counts.size());
		for (int i=0;i<counts.size();i++)
			counts[i]=&param.counts[i];
		RollAwardCountEntry *e=CSysRandom::RollWeighted<RollAwardCountEntry>(counts);
		if (e)
			c=e->count;
	}
	if (c>0)
	{
		std::vector<RollAwardEntry*>entries;
		entries.reserve(param.entries.size());
		for (int i=0;i<param.entries.size();i++)
		{
			RollAwardEntry *e=&param.entries[i];

			//检测这个奖励是否可以实施
			if (TRUE)
			{
				switch(e->tp)
				{
					case LevelAward::Item:
					{
						extern BOOL LevelUtil_CheckAwardAvailable(CLevelPlayer *player,RecordID idItem);
						if (!LevelUtil_CheckAwardAvailable(player,e->idItem))
							continue;

						break;
					}
					case LevelAward::Upgrade:
					{
						BOOL bAvailable=FALSE;
						LevelRecordUpgrade *rec=level->GetRecords()->GetUpgrade(e->idUpgrade);
						if (rec)
						{
							if (rec->upgrade)
							{
								if (player)
								{
									if (rec->upgrade->GetUpgradeType()==CLevelUpgrade::Ability)
									{
										CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
										CLevelAbility *ability=player->GetAbilities().GetActiveAbility(upgrade->GetAbilityType());
										if (ability)
										{
											if (!upgrade->GetChannel()==CLevelAbilityUpgrade::Channel_Initial)
											{
												if(upgrade->CanUpgrade(ability))
													bAvailable=TRUE;
											}
										}
										else
										{
											if (upgrade->GetChannel()==CLevelAbilityUpgrade::Channel_Initial)
												bAvailable=TRUE;
										}
									}
									else
									{
										//xxxxxxxxxxxxxx
									}
								}
							}
						}
						if (!bAvailable)
							continue;
						break;
					}
					case LevelAward::LevelUp_Weapon:
					{
						BOOL bAvailable=FALSE;
						extern LevelAbilityType LevelUtil_GetEquipingAbility(CLevelObj *lo);
						LevelAbilityType tpAbility=LevelUtil_GetEquipingAbility((CLevelObj *)player->GetLoUnit());
						if (tpAbility!=LevelAbilityType_None)
						{
							LevelRecordUpgrade *recUpgrade=level->GetRecords()->GetLevelUpUpgrade(tpAbility);
							CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tpAbility);
							if (ability)
							{
								if (recUpgrade)
								{
									CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)recUpgrade->upgrade;
									if (upgrade->CanUpgrade(ability))
										bAvailable=TRUE;
								}
							}
						}
						if (!bAvailable)
							continue;
						break;
					}
				}
			}

			entries.push_back(e);
		}

		int idxVar=0;

		result.awards.reserve(c);

		for (int i=0;i<c;i++)
		{
			RollAwardEntry *e=CSysRandom::RollWeighted<RollAwardEntry>(entries);
			if (!e)
				break;

			LevelAward award;

			award.tp=e->tp;
			award.bValid=1;
			award.bExpendable=0;

			switch(e->tp)
			{
			case LevelAward::Resource:
				{
					award.tpRes=e->tpRes;
					award.count=(short)CSysRandom::RandRangeInt<int>(e->nMin,e->nMax);
					break;
				}
			case LevelAward::Item:
				{
					award.idRec=e->idItem;
					award.count=(short)CSysRandom::RandRangeInt<int>(e->nMin,e->nMax);
					break;
				}
			case LevelAward::Upgrade:
				{
					award.idRec=e->idUpgrade;
					award.count=1;

					LevelRecordUpgrade *rec=level->GetRecords()->GetUpgrade(e->idUpgrade);
					if (rec)
					{
						if (rec->upgrade)
						{
							if (player)
							{
								if (rec->upgrade->GetUpgradeType()==CLevelUpgrade::Ability)
								{
									CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
									CLevelAbility *ability=player->GetAbilities().GetAbility(upgrade->GetAbilityType());
									if (ability)
										upgrade->MakeSeed(ability,award.seed);
								}
								else
								{
									//xxxxxxxxxxxxxx
								}
							}
						}
					}
					break;
				}
			case LevelAward::LevelUp_Weapon:
				{
					extern LevelAbilityType LevelUtil_GetEquipingAbility(CLevelObj *lo);
					LevelAbilityType tpAbility=LevelUtil_GetEquipingAbility((CLevelObj*)player->GetLoUnit());
					if (tpAbility!=LevelAbilityType_None)
					{
						LevelRecordUpgrade *recUpgrade=level->GetRecords()->GetLevelUpUpgrade(tpAbility);
						if (recUpgrade)
						{
							award.idRec=recUpgrade->GetID();
							award.count=1;

							CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tpAbility);
							if (ability)
							{
								if (recUpgrade->upgrade)
									((CLevelAbilityUpgrade *)recUpgrade->upgrade)->MakeSeed(ability,award.seed);
							}
						}
					}
					break;
				}

			}
			VEC_REMOVE_SWAP(entries,e);

			result.awards.push_back(award);
		}
		result.UpdateExpendable(player);
	}

}


void CBgnGA_RollAwards::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollAwards*pad=_GetPad<CBgpGA_RollAwards>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();

	LevelBehaviorContext *ctx=_GetCtx();

	//清空results
	BOOL bAward=FALSE;
	if (pad->awards!=StringID_Invalid)
		_GetMem()->DepositObj(pad->awards,NULL);
	bAward=TRUE;

	RollAwardsResult *result=NULL;

	if (FALSE)
	{
		RollAwardParam *param=&pad->param;
		if (param)
		{
			int c=0;
			if (TRUE)
			{
				std::vector<RollAwardCountEntry*>counts;
				counts.resize(param->counts.size());
				for (int i=0;i<counts.size();i++)
					counts[i]=&param->counts[i];
				RollAwardCountEntry *e=CSysRandom::RollWeighted<RollAwardCountEntry>(counts);
				if (e)
					c=e->count;
			}
			if (c>0)
			{
				std::vector<RollAwardEntry*>entries;
				entries.reserve(param->entries.size());
				for (int i=0;i<param->entries.size();i++)
				{
					RollAwardEntry *e=&param->entries[i];

					//检测这个奖励是否可以实施
					if (TRUE)
					{
						switch(e->tp)
						{
							case LevelAward::Item:
							{
								BOOL bAvailable=TRUE;
								LevelRecordItem *rec=level->GetRecords()->GetItem(e->idItem);
								if (rec)
								{
									if (!rec->bAllowStack)
									{
										if (rec->tpAbility!=LevelAbilityType_None)
										{
											if (player)
											{
												CLevelAbility *ability=player->GetAbilities().GetActiveAbility(rec->tpAbility);
												if (ability)
													bAvailable=FALSE;//已经有这个Ability了
											}
										}
									}
								}
								if (!bAvailable)
									continue;

								break;
							}
							case LevelAward::Upgrade:
							{
								BOOL bAvailable=FALSE;
								LevelRecordUpgrade *rec=level->GetRecords()->GetUpgrade(e->idUpgrade);
								if (rec)
								{
									if (rec->upgrade)
									{
										if (player)
										{
											if (rec->upgrade->GetUpgradeType()==CLevelUpgrade::Ability)
											{
												CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
												CLevelAbility *ability=player->GetAbilities().GetActiveAbility(upgrade->GetAbilityType());
												if (ability)
												{
													if (!upgrade->GetChannel()==CLevelAbilityUpgrade::Channel_Initial)
													{
														if(upgrade->CanUpgrade(ability))
															bAvailable=TRUE;
													}
												}
												else
												{
													if (upgrade->GetChannel()==CLevelAbilityUpgrade::Channel_Initial)
														bAvailable=TRUE;
												}
											}
											else
											{
												//xxxxxxxxxxxxxx
											}
										}
									}
								}
								if (!bAvailable)
									continue;
								break;
							}
							case LevelAward::LevelUp_Weapon:
							{
								BOOL bAvailable=FALSE;
								extern LevelAbilityType LevelUtil_GetEquipingAbility(CLevelObj *lo);
								LevelAbilityType tpAbility=LevelUtil_GetEquipingAbility((CLevelObj *)player->GetLoUnit());
								if (tpAbility!=LevelAbilityType_None)
								{
									LevelRecordUpgrade *recUpgrade=level->GetRecords()->GetLevelUpUpgrade(tpAbility);
									CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tpAbility);
									if (ability)
									{
										if (recUpgrade)
										{
											CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)recUpgrade->upgrade;
											if (upgrade->CanUpgrade(ability))
												bAvailable=TRUE;
										}
									}
								}
								if (!bAvailable)
									continue;
								break;
							}
						}
					}

					entries.push_back(e);
				}

				int idxVar=0;
				result=Class_New(RollAwardsResult);

				result->awards.reserve(c);

				for (int i=0;i<c;i++)
				{
					RollAwardEntry *e=CSysRandom::RollWeighted<RollAwardEntry>(entries);
					if (!e)
						break;

					LevelAward award;

					award.tp=e->tp;
					award.bExpendable=0;
					award.bValid=TRUE;

					switch(e->tp)
					{
						case LevelAward::Resource:
						{
							award.tpRes=e->tpRes;
							award.count=(short)CSysRandom::RandRangeInt<int>(e->nMin,e->nMax);
							break;
						}
						case LevelAward::Item:
						{
							award.idRec=e->idItem;
							award.count=(short)CSysRandom::RandRangeInt<int>(e->nMin,e->nMax);
							break;
						}
						case LevelAward::Upgrade:
						{
							award.idRec=e->idUpgrade;
							award.count=1;

							LevelRecordUpgrade *rec=level->GetRecords()->GetUpgrade(e->idUpgrade);
							if (rec)
							{
								if (rec->upgrade)
								{
									if (player)
									{
										if (rec->upgrade->GetUpgradeType()==CLevelUpgrade::Ability)
										{
											CLevelAbilityUpgrade *upgrade=(CLevelAbilityUpgrade *)rec->upgrade;
											CLevelAbility *ability=player->GetAbilities().GetAbility(upgrade->GetAbilityType());
											if (ability)
												upgrade->MakeSeed(ability,award.seed);
										}
										else
										{
											//xxxxxxxxxxxxxx
										}
									}
								}
							}
							break;
						}
						case LevelAward::LevelUp_Weapon:
						{
							extern LevelAbilityType LevelUtil_GetEquipingAbility(CLevelObj *lo);
							LevelAbilityType tpAbility=LevelUtil_GetEquipingAbility((CLevelObj*)player->GetLoUnit());
							if (tpAbility!=LevelAbilityType_None)
							{
								LevelRecordUpgrade *recUpgrade=level->GetRecords()->GetLevelUpUpgrade(tpAbility);
								if (recUpgrade)
								{
									award.idRec=recUpgrade->GetID();
									award.count=1;

									CLevelAbility *ability=player->GetAbilities().GetActiveAbility(tpAbility);
									if (ability)
									{
										if (recUpgrade->upgrade)
											((CLevelAbilityUpgrade *)recUpgrade->upgrade)->MakeSeed(ability,award.seed);
									}
								}
							}
							break;
						}

					}
					VEC_REMOVE_SWAP(entries,e);

					result->awards.push_back(award);
				}
				result->UpdateExpendable(player);
				_GetMem()->DepositObj(pad->awards,result);
			}
		}
	}
	else
	{
		result=Class_New(RollAwardsResult);
		RollAwards(player,pad->param,*result);
		_GetMem()->DepositObj(pad->awards,result);
	}

	if (pad->prices!=StringID_Invalid)
	{
		_GetMem()->DepositObj(pad->prices,NULL);

		if (result)
		{
			RollAwardsPrice *price=_EvalPrice(result,player);
			_GetMem()->DepositObj(pad->prices,price);
		}
	}



	_OutputOk(outputs,1,"结束");
	return;
}
