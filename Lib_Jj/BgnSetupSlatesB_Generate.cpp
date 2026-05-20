/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesB_Generate
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesB.h"

#include "BgnSetupSlatesB_Generate.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesB_Generate
BIND_BGN_CLASS(CBgnSetupSlatesB_Generate,CBgpSetupSlatesB_Generate);


void CBgnSetupSlatesB_Generate::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesB_Generate*pad=_GetPad<CBgpSetupSlatesB_Generate>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesB"));

	CLoSlatesB *loSlates=(CLoSlatesB*)ctx->lo;

	loSlates->Setup_Generate(pad->param);

	_OutputOk(outputs,1,"结束");
}
