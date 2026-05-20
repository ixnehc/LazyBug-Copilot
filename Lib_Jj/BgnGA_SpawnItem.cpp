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

#include "BgnGA_SpawnItem.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoItem.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

REGISTER_BCC_CLASS(BccSpawnItems,"SpawnItem参数")

////////////////////////////////////////////////////////////////////////
//CBgnGA_SpawnItem
BIND_BGN_CLASS(CBgnGA_SpawnItem,CBgpGA_SpawnItem);


void CBgnGA_SpawnItem::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_SpawnItem*pad=_GetPad<CBgpGA_SpawnItem>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (pad->nmSites!=StringID_Invalid)
	{
		LevelPos pos;
		extern BOOL LevelUtil_GenRndGAPos(CLevelObj *lo,StringID nm,LevelPos &pos);
		if (LevelUtil_GenRndGAPos(lo,pad->nmSites,pos))
		{
			RecordID idItem=RecordID_Invalid;
			if (pad->nmItems!=StringID_Invalid)
			{
				extern RecordID LevelUtil_GenRndGAItem(CLevelObj *lo,StringID nm);
				idItem=LevelUtil_GenRndGAItem(lo,pad->nmItems);
			}
			else
				idItem=pad->idItem;

			if (idItem!=RecordID_Invalid)
			{
				LevelBehaviorContext *ctx=_GetCtx();

				LevelRecordItem *recItem=level->GetRecords()->GetItem(idItem);
				if (recItem)
				{
					CLoItem* lo=(CLoItem*)level->CreateObj(Class_Ptr2(CLoItem));

					LevelItemState state;
					state.tid=recItem->GetID();
					state.nStack=1;
					state.nBuffs=0;

					lo->PostCreate(&state,pos,LevelOSB(),LevelOpLinkID_Invalid);

					level->AddToActives(lo);


					if (pad->nmVar!=StringID_Invalid)
						_SetID(pad->nmVar,BehaviorMemID_LevelObj,lo->GetID());

					SAFE_RELEASE(lo);
				}
			}
		}
	}


	_OutputOk(outputs,1,"结束");
	return;
}
