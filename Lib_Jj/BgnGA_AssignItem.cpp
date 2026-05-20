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

#include "BgnGA_AssignItem.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_AssignItem
BIND_BGN_CLASS(CBgnGA_AssignItem,CBgpGA_AssignItem);


void CBgnGA_AssignItem::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_AssignItem*pad=_GetPad<CBgpGA_AssignItem>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (TRUE)
	{
		RecordID idItem=pad->idItem;
		if (idItem!=RecordID_Invalid)
		{
			LevelRecordItem *recItem=_GetLevel()->GetRecords()->GetItem(idItem);
			if (recItem)
			{
				LevelBehaviorContext *ctx=_GetCtx();

				CLevelPlayer *player=_GetTalkPlayer();
				if (player)
				{
					LevelPlayerStates *lps=player->GetLPS();

					if (recItem->tpArtifact!=LevelArtifact_None)
					{
						extern BOOL LevelUtil_AddArtifact(CLevelPlayer *player,RecordID idItem,int nStack);
						LevelUtil_AddArtifact(player,idItem,1);
					}
				}

			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
