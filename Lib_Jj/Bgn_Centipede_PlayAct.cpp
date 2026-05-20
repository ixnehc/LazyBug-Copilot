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

#include "Bgn_Centipede_PlayAct.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoCentipede.h"



////////////////////////////////////////////////////////////////////////
//CBgn_Centipede_PlayAct
BIND_BGN_CLASS(CBgn_Centipede_PlayAct,CBgp_Centipede_PlayAct);

void CBgn_Centipede_PlayAct::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Centipede_PlayAct*pad=_GetPad<CBgp_Centipede_PlayAct>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
	{
		CLoCentipede *loCentipede=(CLoCentipede *)ctx->lo;
		loCentipede->GetCombatState().PlayAct(pad->_nmAct,pad->_bLoop,level->GetT_());
	}

	if (pad->_bLoop)
		_OutputOk(outputs,1,"结束");
}

void CBgn_Centipede_PlayAct::Update(BGNOutputs &outputs)
{
	CBgp_Centipede_PlayAct*pad=_GetPad<CBgp_Centipede_PlayAct>();
	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
	{
		CLoCentipede *loCentipede=(CLoCentipede *)ctx->lo;
		AnimTick tCur,dur;
		if (loCentipede->GetCombatState().GetTopActProgress(tCur,dur))
		{
			AnimTick remain=ANIMTICK_SAFE_MINUS(dur,tCur);
			if (remain<ANIMTICK_FROM_SECOND(0.4f))
				_OutputOk(outputs,1,"结束");
			return;
		}
		else
		{
			_OutputOk(outputs,1,"结束");
			return;
		}
	}

	_OutputOk(outputs,1,"结束");
	return;


}
