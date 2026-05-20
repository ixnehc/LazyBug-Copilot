/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSlatesA_SpawnAgent
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSlatesA_SpawnAgent.h"



////////////////////////////////////////////////////////////////////////
//CBgnSlatesA_SpawnAgent
BIND_BGN_CLASS(CBgnSlatesA_SpawnAgent,CBgpSlatesA_SpawnAgent);


void CBgnSlatesA_SpawnAgent::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSlatesA_SpawnAgent*pad=_GetPad<CBgpSlatesA_SpawnAgent>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	loSlates->SpawnAgent(ctx->idxSlate,pad->idAgent,TRUE);

	_OutputOk(outputs,1,"结束");
}
