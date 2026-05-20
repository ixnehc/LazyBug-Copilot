/********************************************************************
	created:	2022/4/27 
	author:		cxi

	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_SpawnDailyItems.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoItem.h"

#include "LevelPlayer.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_SpawnDailyItems
BIND_BGN_CLASS(CBgnGA_SpawnDailyItems,CBgpGA_SpawnDailyItems);

void CBgnGA_SpawnDailyItems::_Update(BGNOutputs &outputs)
{
	CBgpGA_SpawnDailyItems*pad=_GetPad<CBgpGA_SpawnDailyItems>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
	CLevelPlayer *player=LevelUtil_GetFirstPlayer(level);
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		if (lps)
		{
			CLevelObjSrc *los=lo->GetLos();
			if (los)
			{
				DailyGoldsDesc &desc=pad->param.descGold;
				if (desc.idItem!=RecordID_Invalid)
				{
					for (int i=0;i<lps->misc.nGoldMines;i++)
					{
						if (i<desc.mats.size())
						{
							LevelRecordItem *recItem=level->GetRecords()->GetItem(desc.idItem);
							if (recItem)
							{
								CLoItem* lo=(CLoItem*)level->CreateObj(Class_Ptr2(CLoItem));

								LevelItemState state;
								state.tid=desc.idItem;
								state.nStack=CSysRandom::RandRangeInt(desc.nMin,desc.nMax+1);
								state.nBuffs=0;

								i_math::matrix43f mat=desc.mats[i];
								mat*=los->GetMat();
								lo->PostCreate(&state,mat.getTranslation().getXZ(),LevelOSB(),LevelOpLink());

								level->AddToActives(lo);

								SAFE_RELEASE(lo);
							}
						}
					}
				}
			}
			_OutputOk(outputs,1,"结束");
		}
	}
}



void CBgnGA_SpawnDailyItems::Start(DWORD iStb,BGNOutputs &outputs)
{
	_Update(outputs);
}

void CBgnGA_SpawnDailyItems::Update(BGNOutputs &outputs)
{
	_Update(outputs);
}
