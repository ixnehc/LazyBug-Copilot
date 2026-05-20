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

#include "LevelOSB.h"

#include "BgnGA_RollItems.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoItem.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_RollItems
BIND_BGN_CLASS(CBgnGA_RollItems,CBgpGA_RollItems);


void CBgnGA_RollItems::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollItems*pad=_GetPad<CBgpGA_RollItems>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	//清空results
	for (int i=0;i<pad->results.size();i++)
	{
		if (pad->results[i].varItem==StringID_Invalid)
			continue;
		_GetMem()->SetID(pad->results[i].varItem,BehaviorMemType_ItemRecord,RecordID_Invalid);
		_GetMem()->SetNumber(pad->results[i].varCount,0);
	}


	RollItemParam *param=&pad->param;
	if (param)
	{
		int c=0;
		if (TRUE)
		{
			std::vector<RollItemCountEntry*>counts;
			counts.resize(param->counts.size());
			for (int i=0;i<counts.size();i++)
				counts[i]=&param->counts[i];
			RollItemCountEntry *e=CSysRandom::RollWeighted<RollItemCountEntry>(counts);
			if (e)
				c=e->count;
		}
		if (c>0)
		{
			std::vector<RollItemEntry*>entries;
			entries.resize(param->entries.size());
			for (int i=0;i<entries.size();i++)
				entries[i]=&param->entries[i];

			for (int i=0;i<c;i++)
			{
				RollItemEntry *e=CSysRandom::RollWeighted<RollItemEntry>(entries);
				if (!e)
					break;

				_GetMem()->SetID(pad->results[i].varItem,BehaviorMemType_ItemRecord,e->idItem);
				_GetMem()->SetNumber(pad->results[i].varCount,(short)CSysRandom::RandRangeInt<int>(e->nMin,e->nMax));

				VEC_REMOVE_SWAP(entries,e);
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
