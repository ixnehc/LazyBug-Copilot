/********************************************************************
	created:	2019/12/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "BgnGA_宝藏洞穴_Search.h"
#include "BgnGA_RollAwards.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"

#include "Random/Random.h"

struct BMO_FoundProgress:public CBehaviorMemObj
{
	typedef CBgnGA_宝藏洞穴_Search::Found MyFound;
	DECLARE_CLASS(BMO_FoundProgress);
	BEGIN_GOBJ_PURE(BMO_FoundProgress,1);

		GELEM_VARVECTOR(MyFound,founds);
			GELEM_UID(1);
		GELEM_VAR_INIT(AnimTick,tFoundStart,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_UID(2);

	END_GOBJ();

	std::vector<MyFound> founds;
	AnimTick tFoundStart;
};

IMPLEMENT_CLASS(BMO_FoundProgress);

//////////////////////////////////////////////////////////////////////////
//AwardPool_宝藏洞穴
const char *AwardPool_宝藏洞穴::GetBrief(void *param)
{
	FillDescAssist *assist=(FillDescAssist *)param;

	static std::string s;
	s="n/a";
	if (assist)
	{
		if (tp==LevelAward::Item)
			FormatString(s,"%s--%.1f",assist->GetItemName(idItem),wt);
		if (tp==LevelAward::Resource)
			FormatString(s,"%s(%d~%d)",NameFromResourceType(tpRes),nMin,nMax);
		if (((int)tp)==16)
			FormatString(s,"[空] %.1f",wt);
	}
	return s.c_str();
}


//////////////////////////////////////////////////////////////////////////
//AwardEntry_宝藏洞穴
const char *AwardEntry_宝藏洞穴::GetBrief(void *param)
{
	FillDescAssist *assist=(FillDescAssist *)param;

	static std::string s;
	s="n/a";
	if (assist)
	{
		if (pools.size()>0)
		{
			FormatString(s,"[%.2f~%.2f]",difficulty-offDifficulty,difficulty);

			float wtTotal=0.0f;
			for(int i=0;i<pools.size();i++)
			{
				AwardPool_宝藏洞穴 &e=pools[i];
				if (e.tp!=LevelAward::Resource)
					wtTotal+=e.wt;
			}
			if (wtTotal<=0.0f)
				AppendFmtString(s,"empty");
			else
			{
				for(int i=0;i<pools.size();i++)
				{
					if (i>0)
						s+=",";
					AwardPool_宝藏洞穴 &e=pools[i];
					if (e.tp==LevelAward::Resource)
						AppendFmtString(s,"%s(%d~%d)",NameFromResourceType(e.tpRes),e.nMin,e.nMax);
					if (e.tp==LevelAward::Item)
						AppendFmtString(s,"%s %.1f%%",assist->GetItemName(e.idItem),e.wt/wtTotal*100.0f);
				}
			}

		}
	}
	return s.c_str();
}

int AwardEntry_宝藏洞穴::CalcAvarageResourceCount(LevelResourceType tpRes)
{
	int sum=0;
	for (int i=0;i<pools.size();i++)
	{
		if (pools[i].tp==LevelAward::Resource)
		{
			if (pools[i].tpRes==tpRes)
				sum+=(pools[i].nMin+pools[i].nMax)/2;
		}
	}
	return sum;
}

void AwardEntry_宝藏洞穴::CollectItems(std::unordered_set<RecordID>ids)
{
	for (int i=0;i<pools.size();i++)
	{
		if (pools[i].tp==LevelAward::Item)
		{
			if (pools[i].idItem!=RecordID_Invalid)
				ids.insert(pools[i].idItem);
		}
	}
}

float AwardEntry_宝藏洞穴::CalcPossibility(RecordID idItem)
{
	float wtSum=0.0f;
	float wtItem=0.0f;
	for (int i=0;i<pools.size();i++)
	{
		if (pools[i].tp!=LevelAward::Resource)
		{
			wtSum+=pools[i].wt;
			if (pools[i].tp==LevelAward::Item)
			{
				if (pools[i].idItem==idItem)
					wtItem+=pools[i].wt;
			}
		}
	}

	if (wtSum<=0.0f)
		return 0.0f;
	return wtItem/wtSum;
}



//////////////////////////////////////////////////////////////////////////
//Param_宝藏洞穴
int Param_宝藏洞穴::CalcAvarageResourceCount(LevelResourceType tpRes)
{
	int sum=0;
	for (int i=0;i<entries.size();i++)
		sum+=entries[i].CalcAvarageResourceCount(tpRes);
	return sum;
}

void Param_宝藏洞穴::CollectItems(std::unordered_set<RecordID>ids)
{
	ids.clear();
	for (int i=0;i<entries.size();i++)
		entries[i].CollectItems(ids);
}

float Param_宝藏洞穴::CalcPossibility(RecordID idItem)
{
	float result=0.0f;
	for (int i=0;i<entries.size();i++)
	{
		float possibility=entries[i].CalcPossibility(idItem);
		result=1.0f-(1.0f-result)*(1.0f-possibility);
	}
	return result;
}


////////////////////////////////////////////////////////////////////////
//CBgnGA_宝藏洞穴_Search
BIND_BGN_CLASS(CBgnGA_宝藏洞穴_Search,CBgpGA_宝藏洞穴_Search);

static AnimTick CalcFoundTime(AnimTick dur,float difficulty,float off)
{
	float d=difficulty;

	for (int i=0;i<10;i++)
	{
		const int nIterate=3;
		float average=0.0f;
		for (int j=0;j<nIterate;j++)
			average+=CSysRandom::RandRange(difficulty-off,difficulty+off);
		average/=(float)nIterate;
		if (average<=difficulty)
		{
			d=average;
			break;
		}
	}

	return (AnimTick)(d*(float)dur);
}

void CBgnGA_宝藏洞穴_Search::_AddTreasureInfo(CLevelPlayer *player,LevelResourceType tpRes)
{
	CBgpGA_宝藏洞穴_Search*pad=_GetPad<CBgpGA_宝藏洞穴_Search>();
	int count=pad->param.CalcAvarageResourceCount(tpRes);
	if (count>0)
	{
		LevelAgentBrief::TreasureInfos::Entry info;
		info.guidProvider=_GetLo()->GetGUID();
		info.tpRes=tpRes;
		info.count=(WORD)count;

		extern void LevelUtil_AddTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelAgentBrief::TreasureInfos::Entry &info);
		LevelUtil_AddTreasureInfo(player,player->GetLevel()->GetMapID(),_GetLo()->GetGUID(),info);
	}
}


void CBgnGA_宝藏洞穴_Search::_AddTreasureInfos()
{
	CBgpGA_宝藏洞穴_Search*pad=_GetPad<CBgpGA_宝藏洞穴_Search>();
	CLevelObj *lo=_GetLo();
	LevelGUID guid=lo->GetGUID();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();
	if (!player)
		return;

	_AddTreasureInfo(player,LevelResource_Gold);
	_AddTreasureInfo(player,LevelResource_Gem);
	_AddTreasureInfo(player,LevelResource_Labor);

	std::unordered_set<RecordID> idsItem;
	pad->param.CollectItems(idsItem);

	std::unordered_set<RecordID>::iterator it;
	for (it=idsItem.begin();it!=idsItem.end();it++)
	{
		LevelAgentBrief::TreasureInfos::Entry info;
		info.guidProvider=_GetLo()->GetGUID();
		info.idItem=(*it);
		float possibility=pad->param.CalcPossibility(*it);
		info.possibility=FloatToNearestInt(possibility*100.0f);

		extern void LevelUtil_AddTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelAgentBrief::TreasureInfos::Entry &info);
		LevelUtil_AddTreasureInfo(player,player->GetLevel()->GetMapID(),_GetLo()->GetGUID(),info);
	}




}


void CBgnGA_宝藏洞穴_Search::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_宝藏洞穴_Search*pad=_GetPad<CBgpGA_宝藏洞穴_Search>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	_tStart=_GetT();
	_tLast=_tStart;

	RollAwardsResult *result=Class_New(RollAwardsResult);

	if (pad->varResult!=StringID_Invalid)
	{
		_GetMem()->DepositObj(pad->varResult,result);
	}

	_AddTreasureInfos();

	//初始化Founds
	if (TRUE)
	{
		BMO_FoundProgress *progress=NULL;
		if (pad->varProgress!=StringID_Invalid)
			progress=_GetMem()->GetObj<BMO_FoundProgress>(pad->varProgress);

		if (!progress)
		{
			AnimTick dur=pad->param.durSearch;
			std::vector<AwardEntry_宝藏洞穴>&entries=pad->param.entries;
			_founds.reserve(entries.size());
			_founds.clear();
			for (int i=0;i<entries.size();i++)
			{
				AwardEntry_宝藏洞穴 &e=entries[i];
				Found found;
				found.idxEntry=i;
				found.t=CalcFoundTime(dur,e.difficulty,e.offDifficulty);

				_founds.push_back(found);
			}
			_tFoundStart=0;
		}
		else
		{
			_founds=progress->founds;
			_tFoundStart=progress->tFoundStart;
		}

		if (!progress)
		{
			if (pad->varProgress)
			{
				progress=Class_New(BMO_FoundProgress);
				progress->founds=_founds;
				progress->tFoundStart=_tFoundStart;

				_GetMem()->DepositObj(pad->varProgress,progress);
			}
		}
	}

	return;
}

void CBgnGA_宝藏洞穴_Search::_CommitFound(AwardEntry_宝藏洞穴 &e,RollAwardsResult *result)
{
	extern BOOL LevelUtil_ApplyAward(CLevelPlayer *player,LevelAward *award,LevelAwardPrice *price);
	CLevelPlayer *player=_GetTalkPlayer();
	CLevelObj *lo=_GetLo();

	std::vector<AwardPool_宝藏洞穴*> pools;
	for (int i=0;i<e.pools.size();i++)
	{
		AwardPool_宝藏洞穴*pool=&e.pools[i];

		if (pool->tp==LevelAward::Resource)
		{
			BOOL bCanApply=TRUE;
			if (pool->tpRes==LevelResource_Labor)
			{
				if (!LevelUtil_GetActiveAbility(player->GetLoUnit(),LevelAbilityType_UtumTide))
					bCanApply=FALSE;
			}
			if (bCanApply)
			{
				LevelAward award;
				award.bExpendable=0;
				award.bValid=1;
				award.tp=LevelAward::Resource;
				award.tpRes=pool->tpRes;
				award.count=CSysRandom::RandRangeInt(pool->nMin,pool->nMax+1);
				result->awards.push_back(award);
				result->UpdateExpendable(player);
				LevelUtil_ApplyAward(player,&award,NULL);
				extern void LevelUtil_RevealTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelResourceType tpRes,RecordID idItem);
				LevelUtil_RevealTreasureInfo(player,player->GetLevel()->GetMapID(),lo->GetGUID(),pool->tpRes,RecordID_Invalid);
			}
			continue;
		}

		pools.push_back(pool);
	}

	if (pools.size()>0)
	{
		AwardPool_宝藏洞穴 *pool=CSysRandom::RollWeighted<AwardPool_宝藏洞穴>(pools);
		if (pool)
		{
			if (pool->tp==LevelAward::Item)
			{
				if (pool->idItem!=RecordID_Invalid)
				{
					LevelAward award;
					award.bValid=1;
					award.bExpendable=0;
					award.tp=LevelAward::Item;
					award.idRec=pool->idItem;
					award.count=1;
					result->awards.push_back(award);
					result->UpdateExpendable(player);
					LevelUtil_ApplyAward(player,&award,NULL);

					extern void LevelUtil_RevealTreasureInfo(CLevelPlayer *player,RecordID idMap,LevelGUID guidAgent,LevelResourceType tpRes,RecordID idItem);
					LevelUtil_RevealTreasureInfo(player,player->GetLevel()->GetMapID(),lo->GetGUID(),LevelResource_None,pool->idItem);
				}
			}
		}
	}

	result->ApplyMerge();
}

void CBgnGA_宝藏洞穴_Search::Update(BGNOutputs &outputs)
{
	CBgpGA_宝藏洞穴_Search*pad=_GetPad<CBgpGA_宝藏洞穴_Search>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	AnimTick tLocal=_GetT();
	tLocal=ANIMTICK_SAFE_MINUS(tLocal,_tStart);
	AnimTick tFound=_tFoundStart+tLocal;

	AnimTick dt=0;
	if (TRUE)
	{
		AnimTick tCur=_GetT();
		dt=ANIMTICK_SAFE_MINUS(tCur,_tLast);
		_tLast=tCur;
	}

	RollAwardsResult *result=NULL;
	if (pad->varResult!=StringID_Invalid)
		result=_GetMem()->GetObj<RollAwardsResult>(pad->varResult);

	Param_宝藏洞穴 *param=&pad->param;

	BOOL bDirty=FALSE;
	if (TRUE)
	{
		int c=0;
		for (int i=0;i<_founds.size();i++)
		{
			if (_founds[i].t<=tFound)
			{
				if (_founds[i].idxEntry<param->entries.size())
				{
					if (result)
					{
						_CommitFound(param->entries[_founds[i].idxEntry],result);
						bDirty=TRUE;
					}
				}
				continue;
			}
			_founds[c++]=_founds[i];
		}
		_founds.resize(c);
	}

	if (result)
	{
		if (bDirty)
			_GetMem()->DepositObj(pad->varResult,result);
	}

	if (pad->varProgress!=StringID_Invalid)
	{
		BMO_FoundProgress *progress=_GetMem()->GetObj<BMO_FoundProgress>(pad->varProgress);
		if (progress)
		{
			progress->founds=_founds;
			progress->tFoundStart=tFound;
			_GetMem()->DepositObj(pad->varProgress,progress);
		}
	}

	if (pad->varPercent!=StringID_Invalid)
	{
		float percent=((float)tFound)/(float)param->durSearch;
		percent=i_math::clamp_f(percent,0.0f,1.0f);
		_GetMem()->SetFloat(pad->varPercent,percent);
	}

	if (TRUE)
	{
		float costSPMax=param->speedSPCost*ANIMTICK_TO_SECOND(dt);
		float costSP=0.0f;
		CLevelObj *loTalk=_GetTalkLo();
		if (loTalk)
		{
			LevelUtil_ModSPCost(loTalk,costSP,costSPMax);
			if (costSPMax>0.0f)
				level->GetDecider()->MakeCost_MaxSP(loTalk,costSPMax);
		}
	}

	if (tFound>=param->durSearch)
	{
		_OutputOk(outputs,1,"结束");
	}

}
