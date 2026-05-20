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

#include "Bgn_CheckItem.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgn_CheckItem
BIND_BGN_CLASS(CBgn_CheckItem,CBgp_CheckItem);


void CBgn_CheckItem::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckItem*pad=_GetPad<CBgp_CheckItem>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (pad->idItem!=RecordID_Invalid)
	{
		CLevelPlayer *player=_GetTalkPlayer();
		if (pad->idPlayerUnit!=LevelObjID_Invalid)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *lo=LevelUtil_GetAliveLo(level,pad->idPlayerUnit);
			if (lo)
			{
				extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
				player=LevelUtil_PlayerFromLo(lo);
			}
		}
		if (player)
		{
			if (!pad->bCheckMemory)
			{
				extern BOOL LevelUtil_CheckOwningItem(CLevelObj *lo,RecordID idItem);
				if (LevelUtil_CheckOwningItem((CLevelObj*)player->GetLoUnit(),pad->idItem))
				{
					_OutputOk(outputs,1,"有");
					return;
				}
			}
			else
			{
				LevelPlayerStates *lps=player->GetLPS();
				if (lps)
				{
					LPS_CheckItemMemory(lps,pad->idItem);
					_OutputOk(outputs,1,"有");
					return;
				}
			}
		}
	}

	_OutputFail(outputs,2,"没有");
	return;
}
