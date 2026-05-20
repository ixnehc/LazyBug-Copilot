/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesA_SetEdgeLock
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"
#include "BgnSetupSlatesA_SetType_RandomPick.h"

#include "BgnSetupSlatesA_SetEdgeLock.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesA_SetEdgeLock
BIND_BGN_CLASS(CBgnSetupSlatesA_SetEdgeLock,CBgpSetupSlatesA_SetEdgeLock);


void CBgnSetupSlatesA_SetEdgeLock::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesA_SetEdgeLock*pad=_GetPad<CBgpSetupSlatesA_SetEdgeLock>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	if (pad->varGrp!=StringID_Invalid)
	{
		LevelSlatesGroup *grp=_GetMem()->GetObj<LevelSlatesGroup>(pad->varGrp);
		if (grp)
			loSlates->Setup_SetEdgeLock(grp->slates);
	}

	_OutputOk(outputs,1,"结束");
}
