/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSlatesA_OpenFence
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelDecider.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSlatesA_OpenFence.h"



////////////////////////////////////////////////////////////////////////
//CBgnSlatesA_OpenFence
BIND_BGN_CLASS(CBgnSlatesA_OpenFence,CBgpSlatesA_OpenFence);


void CBgnSlatesA_OpenFence::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSlatesA_OpenFence*pad=_GetPad<CBgpSlatesA_OpenFence>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	loSlates->OpenFenceWithSwitch(ctx->idxSlate);

	_OutputOk(outputs,1,"结束");
}

