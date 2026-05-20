/********************************************************************
	created:	2017/01/26 
	author:		cxi
	
	purpose:	GA功能:随机Spell
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"
#include "LevelRecordUpgrade.h"

#include "LevelOSB.h"

#include "BgnGA_RollSpells.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_RollSpells
BIND_BGN_CLASS(CBgnGA_RollSpells,CBgpGA_RollSpells);



void CBgnGA_RollSpells::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollSpells*pad=_GetPad<CBgpGA_RollSpells>();
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

	if (bAward)
	{
		RollSpellParam *param=&pad->param;
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
				std::vector<RollSpellEntry*>entries;
				entries.reserve(param->entries.size());
				for (int i=0;i<param->entries.size();i++)
				{
					RollSpellEntry *e=&param->entries[i];

					//检测这个奖励是否可以实施
					if (TRUE)
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
											bAvailable=TRUE;
									}
								}
							}
						}
						if (!bAvailable)
							continue;
					}

					entries.push_back(e);
				}

				int idxVar=0;
				result=Class_New(RollAwardsResult);

				result->awards.reserve(c);

				for (int i=0;i<c;i++)
				{
					RollSpellEntry *e=CSysRandom::RollWeighted<RollSpellEntry>(entries);
					if (!e)
						break;

					LevelAward award;

					award.tp=LevelAward::Upgrade;
					award.bExpendable=0;
					award.bValid=TRUE;

					award.idRec=e->idUpgrade;
					award.count=1;

					VEC_REMOVE_SWAP(entries,e);

					result->awards.push_back(award);
				}
				result->UpdateExpendable(player);
				_GetMem()->DepositObj(pad->awards,result);

			}
		}
	}


	_OutputOk(outputs,1,"结束");
	return;
}
