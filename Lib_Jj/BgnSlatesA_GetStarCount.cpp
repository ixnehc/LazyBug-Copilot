/********************************************************************
	created:	2023/02/17 
	author:		cxi
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSlatesA_GetStarCount.h"



////////////////////////////////////////////////////////////////////////
//CBgnSlatesA_GetStarCount
BIND_BGN_CLASS(CBgnSlatesA_GetStarCount,CBgpSlatesA_GetStarCount);


void CBgnSlatesA_GetStarCount::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSlatesA_GetStarCount*pad=_GetPad<CBgpSlatesA_GetStarCount>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	int c=0;

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;
	if (loSlates)
		c=loSlates->GetStarCount();

	_SetNumber(pad->var,(short)c);

	_OutputOk(outputs,1,"结束");
}
