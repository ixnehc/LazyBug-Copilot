/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesB_SetEntrance
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesB.h"

#include "BgnSetupSlatesB_SetEntrance.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesB_SetEntrance
BIND_BGN_CLASS(CBgnSetupSlatesB_SetEntrance,CBgpSetupSlatesB_SetEntrance);


void CBgnSetupSlatesB_SetEntrance::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesB_SetEntrance*pad=_GetPad<CBgpSetupSlatesB_SetEntrance>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoSlatesB)))
	{
		CLoSlatesB *loSlates=(CLoSlatesB*)ctx->lo;

		loSlates->Setup_SetExit(pad->grp);
	}

	_OutputOk(outputs,1,"结束");
}
