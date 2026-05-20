/********************************************************************
	created:	2013/4/9 
	author:		cxi
	
	purpose:	掉落产生器
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LevelRecords.h"
 
#include "LoUnit.h"
#include "LoItem.h"

#include "LevelDropper.h"

#include "Log/LogDump.h"


#include "Random/Random.h"


#include "LevelRecordItem.h"
#include "LevelRecordItemClass.h"
#include "LevelRecordItemSeed.h"

#include "LevelRecordUnit.h"
#include "LevelRecordGlobal.h"

static int SeedCompare(const void *l0,const void *r0)
{
	LevelRecordItemSeed **l=(LevelRecordItemSeed **)l0;
	LevelRecordItemSeed **r=(LevelRecordItemSeed **)r0;

	if ((*l)->grdRequire<(*r)->grdRequire)
		return -1;
	else
	{
		if ((*l)->grdRequire>(*r)->grdRequire)
			return 1;
	}
	return 0;
}

void CLevelDropper::Init(CLevelRecords *records)
{
	_records=records;

	//TC
	if (TRUE)
	{
		CRecords *items=_records->GetRecords_Item();

		DWORD c;
		RecordID *ids=items->GetRecords(c);

		for (int i=0;i<c;i++)
		{
			LevelRecordItem *rec=(LevelRecordItem*)items->GetRecord(ids[i]);
			if (!rec)
				continue;

			LevelGrade grd=rec->grdTC;
			if (grd>=ARRAY_SIZE(_tcs))
				continue;

			_tcs[grd].items.push_back(rec);
		}
	}

	//Seeds
	if (TRUE)
	{
		CRecords *seeds=_records->GetRecords_Seed();
		DWORD c;
		RecordID *ids=seeds->GetRecords(c);

		for (int i=0;i<c;i++)
		{
			LevelRecordItemSeed *rec=(LevelRecordItemSeed*)seeds->GetRecord(ids[i]);
			if (!rec)
				continue;

			if (rec->tp==ItemBuff_None)
				continue;
			if (rec->tp>=ARRAY_SIZE(_seeds))
				continue;

			_seeds[rec->tp].push_back(rec);
		}
	}

	//按照等级需求排序
	for (int i=0;i<ARRAY_SIZE(_seeds);i++)
	{
		qsort(&_seeds[i][0],_seeds[i].size(),sizeof(LevelRecordItemSeed*),SeedCompare);
	}
}


void CLevelDropper::Clear()
{
	for (int i=0;i<ARRAY_SIZE(_tcs);i++)
	{
		_tcs[i].items.clear();
	}

	for (int i=0;i<ARRAY_SIZE(_seeds);i++)
		_seeds[i].clear();

	Zero();
}


//计算要掉落几个
DWORD CLevelDropper::_CalcTryCount(LevelGrade grd)
{
	//先判断是否要掉落
	float rate=(((float)grd)/10.0f+1.0f)/20.0f;//XXXXX:Need Tune
	if (!CSysRandom::Roll(rate))
		return 0;

	//再判断要掉落几个
	DWORD nTry=1;
	for (int i=0;i<3;i++)//XXXXX:Need Tune
	{
		if (CSysRandom::Roll(0.2f))//XXXXX:Need Tune
			nTry++;
	}

	return nTry;
}

void CLevelDropper::_GenItemBuff(LevelItemState &state,LevelRecordItemSeed *recSeed)
{
	if (recSeed->tp==ItemBuff_None)
		return;

	ItemBuff buff;
	buff.tp=recSeed->tp;
	switch(recSeed->tp)
	{
		case ItemBuff_None:
			break;
		default:
			buff.sh=CSysRandom::RandVaryUInt(recSeed->base,recSeed->vary);
	}

	BOOL bExist=FALSE;
	for (int i=0;i<state.nBuffs;i++)
	{
		if (state.buffs[i].tp==recSeed->tp)
		{
			bExist=TRUE;
			switch(recSeed->tp)
			{
				case ItemBuff_None:
					break;
				default:
					state.buffs[i].sh+=buff.sh;
			}

			break;
		}
	}

	if (!bExist)
	{
		if (state.nBuffs<ARRAY_SIZE(state.buffs))
		{
			state.buffs[state.nBuffs]=buff;
			state.nBuffs++;
		}
	}
}


void CLevelDropper::_GenItemBuffs(LevelItemState &state,LevelGrade grd)
{
	LevelRecordItem *recItem=_records->GetItem(state.tid);
	if (!recItem)
		return;
	LevelRecordItemClass *recItemClass=_records->GetItemClass(recItem->clss);
	if (!recItemClass)
		return;

	//计算所有可选的Buff的权重和
	float wtTotal=0.0f;
	DWORD nEntries=0;
	if (TRUE)
	{
		for (int i=0;i<recItemClass->seeds.size();i++)
		{
			EItemBuffType tp=recItemClass->seeds[i].tp;
			float wt=recItemClass->seeds[i].wt;

			if (tp>=ItemBuff_Max)
				continue;

			if (_seeds[tp].size()<=0)
				continue;
			if (_seeds[tp][0]->grdRequire>grd)
				continue;

			_entries[nEntries].tp=tp;
			_entries[nEntries].wt=wt;
			nEntries++;

			wtTotal+=wt;
		}
	}

	//尝试产生若干个Buff
	DWORD nTry=4;//XXXXX:Need Tune
	for (int k=0;k<nTry;k++)
	{
		if (CSysRandom::Roll(0.3f))//XXXXX:Need Tune
			break;

		//随机选择ItemBuffType
		float choose=CSysRandom::RandRange<float>(0.0f,wtTotal);

		float wtTotal=0.0f;
		for (int i=0;i<nEntries;i++)
		{
			ItemBuffTypeEntry *entry=&_entries[i];

			wtTotal+=entry->wt;
			if (wtTotal>choose)
			{
				EItemBuffType tp=entry->tp;//选中了type

				//到这个tp的所有seeds里选择一个seed
				std::vector<LevelRecordItemSeed*>&seeds=_seeds[tp];

				//先找到这个tp对应的seed里有几个是满足等级要求的
				DWORD nSeed;
				if (TRUE)
				{
					nSeed=seeds.size();
					for (int j=0;j<seeds.size();j++)
					{
						if (seeds[j]->grdRequire>grd)
						{
							nSeed=j;
							break;
						}
					}
				}

				if(nSeed>0)
				{
					LevelRecordItemSeed *rec=seeds[CSysRandom::RandRangeInt<DWORD>(0,nSeed)];
					_GenItemBuff(state,rec);//将Seed转化成Buff
				}

				break;
			}
		}
	}

}


void CLevelDropper::MakeDrop(CLevelObj *loSrc,CLevelObj *loTarget)
{
	_nResult=0;
	_nSoul=0;

	extern LevelGrade LevelUtil_GetGrade(CLevelObj *lo);
	LevelGrade grdTarget=LevelUtil_GetGrade(loTarget);

	if (grdTarget>=ARRAY_SIZE(_tcs))
		grdTarget=ARRAY_SIZE(_tcs)-1;

	DWORD nTry=_CalcTryCount(grdTarget);

	for (int i=0;i<nTry;i++)
	{
		LevelGrade grd=grdTarget;

		//寻找在哪个TC里产生
		while(grd>0)
		{
			if (CSysRandom::Roll(0.9f))//XXXXX:Need Tune
			{
				if (_tcs[grd].items.size()>0)
					break;
			}
			grd--;
		}

		if (grd<=0)
			continue;

		//TC里面随机挑选一个Item
		int idx=CSysRandom::RandRangeInt<int>(0,_tcs[grd].items.size());
		LevelRecordItem *recItem=_tcs[grd].items[idx];
		if (!recItem)
			continue;

		_results[_nResult].Zero();

		_results[_nResult].tid=recItem->GetID();
		_results[_nResult].nStack=1;

		//产生这个Item的随机参数
		_GenItemBuffs(_results[_nResult],grd);

		_nResult++;
	}

	LevelRecordGlobal *recGlobal=_records->GetGlobal();
	//资源道具的掉落
	if (TRUE)
	{
		extern LevelRecordUnit *LevelUtil_GetUnitRecord(CLevelObj *lo);
		LevelRecordUnit *rec=LevelUtil_GetUnitRecord(loTarget);
		if (rec)
		{
			float rateGold=rec->rateDrop.rateGold;
			float rateGem=rec->rateDrop.rateGem;
			float rateCrystal=rec->rateDrop.rateCrystal;

			if (_nResult<ARRAY_SIZE(_results))
			{
				if (recGlobal->itemsStd.golds.size()>0)
				{
					if (CSysRandom::Roll(rateGold))
					{
						if (recGlobal->itemsStd.golds[0]!=RecordID_Invalid)
						{
							_results[_nResult].Zero();
							_results[_nResult].tid=recGlobal->itemsStd.golds[0];
							_results[_nResult].nStack=1;
							_nResult++;
						}
					}
				}
			}

			if (_nResult<ARRAY_SIZE(_results))
			{
				if (recGlobal->itemsStd.gems.size()>0)
				{
					if (CSysRandom::Roll(rateGem))
					{
						int idx=CSysRandom::RandRangeInt<int>(0,recGlobal->itemsStd.gems.size());
						if (recGlobal->itemsStd.gems[idx]!=RecordID_Invalid)
						{
							_results[_nResult].Zero();
							_results[_nResult].tid=recGlobal->itemsStd.gems[idx];
							_results[_nResult].nStack=1;
							_nResult++;
						}
					}
				}
			}

			if (_nResult<ARRAY_SIZE(_results))
			{
				if (recGlobal->itemsStd.crystals.size()>0)
				{
					if (CSysRandom::Roll(rateCrystal))
					{
						if (recGlobal->itemsStd.crystals[0]!=RecordID_Invalid)
						{
							_results[_nResult].Zero();
							_results[_nResult].tid=recGlobal->itemsStd.crystals[0];
							_results[_nResult].nStack=1;
							_nResult++;
						}
					}
				}
			}

			_nSoul=CSysRandom::RandRangeInt(rec->rateDrop.amntSoul.low,rec->rateDrop.amntSoul.hi);
			_nMP=CSysRandom::RandRangeInt(rec->rateDrop.amntMP.low,rec->rateDrop.amntMP.hi);
			_nCrystal=rec->rateDrop.amntCrystal.hi;//CSysRandom::RandRangeInt(rec->rateDrop.amntCrystal.low,rec->rateDrop.amntCrystal.hi);
		}
	}
		

}
