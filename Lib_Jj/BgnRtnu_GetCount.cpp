/********************************************************************
	created:	2022/07/05 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelRtnus.h"

#include "LevelUtil.h"


#include "BgnRtnu_GetCount.h"


////////////////////////////////////////////////////////////////////////
//CBgnRtnu_GetCount
BIND_BGN_CLASS(CBgnRtnu_GetCount,CBgpRtnu_GetCount);

void CBgnRtnu_GetCount::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpRtnu_GetCount*pad=_GetPad<CBgpRtnu_GetCount>();
	LevelBehaviorContext *ctx=_GetCtx();

	LevelObjID idPlayerLo=LevelObjID_Invalid;
	_GetID(pad->nmPlayerVar,BehaviorMemType_ObjID,idPlayerLo);

	DWORD c=0;
	CLevelObj *loPlayer=LevelUtil_GetAliveLo(ctx->level,idPlayerLo);
	if (loPlayer)
	{
		CLevelPlayer *player=LevelUtil_PlayerFromLo(loPlayer);
		if (player)
		{
			CLevelRtnus *rtnus=player->GetRtnus();
			if (rtnus)
				c=rtnus->GetRetinueCount(pad->idUnit);
		}
	}

	_SetNumber(pad->nmVar,(short)c);

	_OutputOk(outputs,1,"结束");
}
