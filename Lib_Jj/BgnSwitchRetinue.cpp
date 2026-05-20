/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"


#include "Level.h"
#include "LevelRtnus.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnSwitchRetinue.h"

#include "LevelObj.h"

#include "LevelNPCs.h"

#include "LoUnit.h"

////////////////////////////////////////////////////////////////////////
//CBgn_SwitchRetinue
BIND_BGN_CLASS(CBgn_SwitchRetinue,CBgp_SwitchRetinue);

void CBgn_SwitchRetinue::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SwitchRetinue*pad=_GetPad<CBgp_SwitchRetinue>();

	LevelBehaviorContext *ctx=_GetCtx();

	LevelObjID id=LevelObjID_Invalid;
	_GetID(pad->_nmVar,BehaviorMemType_ObjID,id);

	CLevelObj *lo=ctx->level->GetIDs()->LoFromID(id);
	if (lo)
	{
		LevelPlayerID idPlayer=lo->GetPlayerID();
		CLevelPlayer *player=ctx->level->GetPlayer(idPlayer);
		if (player)
		{
			CLevelObj *loMe=ctx->lo;
			if (!loMe->IsRetinue())
			{
				if (loMe->GetType()==LevelObjType_Unit)
				{
					CLevelNPC *npc=ctx->npc;
					if (npc)
					{
						extern BOOL LevelUtil_SwitchNPCRetinue(CLevelPlayer *player,RecordID idNPC);
						LevelUtil_SwitchNPCRetinue(player,npc->GetRecID());
					}
					else
					{
						if (player->GetRtnus())
							player->GetRtnus()->Add_New((CLoUnit*)loMe,TRUE);
					}
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}
