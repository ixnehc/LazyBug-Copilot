/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:授予Vita
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "LevelUtil.h"

#include "BgnGA_AssignVita.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"
#include "LoUnit.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_AssignVita
BIND_BGN_CLASS(CBgnGA_AssignVita,CBgpGA_AssignVita);


void CBgnGA_AssignVita::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_AssignVita*pad=_GetPad<CBgpGA_AssignVita>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (TRUE)
	{
		if (pad->idPlayerUnit!=LevelObjID_Invalid)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *loTarget=LevelUtil_GetAliveLo(level,pad->idPlayerUnit);
			if (loTarget->IsPlayer())
			{
				int nDelta=pad->nTotal;

				CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
				if (player)
				{
					LevelPlayerStates *lps=player->GetLPS();
					if (lps)
					{
						if (nDelta+lps->base.vita_>LEVEL_MAX_VITA)
						{
							nDelta=LEVEL_MAX_VITA-(int)lps->base.vita_;
							if (nDelta<0)
								nDelta=0;
						}
						lps->base.vita_+=nDelta;
						lps->base.SetDirtyDB_Urgent();
					}
				}

				CLevelOps *ops=loTarget->GetOps();
				if (ops)
				{
					LevelOp_VitaMod *op=loTarget->NewOp<LevelOp_VitaMod>(LevelOpLink());
					op->delta=nDelta;
					op->idSrcOwner=lo->GetID();
					ops->AddOp(op);
				}

			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
