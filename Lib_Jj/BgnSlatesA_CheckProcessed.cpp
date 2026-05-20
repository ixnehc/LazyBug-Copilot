/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSlatesA_CheckProcessed
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSlatesA_CheckProcessed.h"



////////////////////////////////////////////////////////////////////////
//CBgnSlatesA_CheckProcessed
BIND_BGN_CLASS(CBgnSlatesA_CheckProcessed,CBgpSlatesA_CheckProcessed);


void CBgnSlatesA_CheckProcessed::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSlatesA_CheckProcessed*pad=_GetPad<CBgpSlatesA_CheckProcessed>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;
	if (loSlates->CheckProcessed(ctx->idxSlate))
		_OutputOk(outputs,1,"是");
	else
		_OutputFail(outputs,2,"否");
}
