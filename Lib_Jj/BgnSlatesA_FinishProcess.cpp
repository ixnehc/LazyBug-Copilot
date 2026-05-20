/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSlatesA_FinishProcess
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSlatesA_FinishProcess.h"



////////////////////////////////////////////////////////////////////////
//CBgnSlatesA_FinishProcess
BIND_BGN_CLASS(CBgnSlatesA_FinishProcess,CBgpSlatesA_FinishProcess);


void CBgnSlatesA_FinishProcess::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSlatesA_FinishProcess*pad=_GetPad<CBgpSlatesA_FinishProcess>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	loSlates->FinishProcess();

	_OutputOk(outputs,1,"结束");
}
