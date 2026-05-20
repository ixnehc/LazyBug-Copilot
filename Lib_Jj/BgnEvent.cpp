/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 核心的BGN
*********************************************************************/
#include "stdh.h"
#include "LevelBGs.h"
#include "LevelBehavior.h"

#include "BgnEvent.h"


////////////////////////////////////////////////////////////////////////
//CBgn_SendEvent
BIND_BGN_CLASS(CBgn_SendEvent,CBgp_SendEvent);

void CBgn_SendEvent::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SendEvent*pad=_GetPad<CBgp_SendEvent>();
	LevelBehaviorContext *ctx=_GetCtx();

	ctx->behavior->AddEvent(pad->_e);

	_SetResult(A_Ok);
}

