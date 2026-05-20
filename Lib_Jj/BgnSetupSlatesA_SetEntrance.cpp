/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesA_SetEntrance
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSetupSlatesA_SetEntrance.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesA_SetEntrance
BIND_BGN_CLASS(CBgnSetupSlatesA_SetEntrance,CBgpSetupSlatesA_SetEntrance);


void CBgnSetupSlatesA_SetEntrance::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesA_SetEntrance*pad=_GetPad<CBgpSetupSlatesA_SetEntrance>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoSlatesA)))
	{
		CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

		if (pad->bEntrance)
			loSlates->Setup_SetEntrance(pad->grp);
		else
			loSlates->Setup_SetExit(pad->grp);
	}
	_OutputOk(outputs,1,"结束");
}
