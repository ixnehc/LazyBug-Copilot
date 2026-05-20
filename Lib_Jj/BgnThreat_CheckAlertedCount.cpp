/********************************************************************
	created:	2016/09/15 
	author:		cxi
	
	purpose:	 检测Threat范围
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnThreat_CheckAlertedCount.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelAIContext.h"

////////////////////////////////////////////////////////////////////////
//CBgnThreat_CheckAlertedCount
BIND_BGN_CLASS(CBgnThreat_CheckAlertedCount,CBgpThreat_CheckAlertedCount);

void CBgnThreat_CheckAlertedCount::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_CheckAlertedCount*pad=_GetPad<CBgpThreat_CheckAlertedCount>();

	CLevelObj *target=_GetThreat();
	if (!target)
	{
		_OutputFail(outputs,2,"否");
		return;
	}

	int nAlerted=0;
	if (TRUE)
	{
		LevelAIContext *ctx=target->GetAIContext();
		if (ctx)
			nAlerted=ctx->GetAlertedCount();
	}

	if ((nAlerted<=pad->_rng.hi)&&(nAlerted>=pad->_rng.low))
		_OutputOk(outputs,1,"是");
	else
		_OutputOk(outputs,2,"否");

}


