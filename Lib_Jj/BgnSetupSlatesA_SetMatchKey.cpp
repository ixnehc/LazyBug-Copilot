/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesA_SetMatchKey
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"
#include "BgnSetupSlatesA_SetType_RandomPick.h"

#include "BgnSetupSlatesA_SetMatchKey.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesA_SetMatchKey
BIND_BGN_CLASS(CBgnSetupSlatesA_SetMatchKey,CBgpSetupSlatesA_SetMatchKey);


void CBgnSetupSlatesA_SetMatchKey::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesA_SetMatchKey*pad=_GetPad<CBgpSetupSlatesA_SetMatchKey>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	if (pad->varGrp!=StringID_Invalid)
	{
		LevelSlatesGroup *grp=_GetMem()->GetObj<LevelSlatesGroup>(pad->varGrp);
		if (grp)
			loSlates->Setup_SetMatchKey(grp->slates,pad->key);
	}

	_OutputOk(outputs,1,"结束");
}
