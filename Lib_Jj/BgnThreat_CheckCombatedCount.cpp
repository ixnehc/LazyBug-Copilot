/********************************************************************
	created:	2016/09/15 
	author:		cxi
	
	purpose:	 检测Threat范围
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnThreat_CheckCombatedCount.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelAIContext.h"

////////////////////////////////////////////////////////////////////////
//CBgnThreat_CheckCombatedCount
BIND_BGN_CLASS(CBgnThreat_CheckCombatedCount,CBgpThreat_CheckCombatedCount);

void CBgnThreat_CheckCombatedCount::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_CheckCombatedCount*pad=_GetPad<CBgpThreat_CheckCombatedCount>();

	CLevelObj *target=_GetThreat();
	if (!target)
	{
		_OutputFail(outputs,2,"否");
		return;
	}

	int nCombated=0;
	if (TRUE)
	{
		LevelAIContext *ctx=target->GetAIContext();
		if (ctx)
			nCombated=ctx->GetCombatedCount();
	}

	if ((nCombated<=pad->_rng.hi)&&(nCombated>=pad->_rng.low))
		_OutputOk(outputs,1,"是");
	else
		_OutputOk(outputs,2,"否");

}


