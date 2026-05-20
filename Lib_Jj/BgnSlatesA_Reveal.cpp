/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSlatesA_Reveal
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSlatesA_Reveal.h"



////////////////////////////////////////////////////////////////////////
//CBgnSlatesA_Reveal
BIND_BGN_CLASS(CBgnSlatesA_Reveal,CBgpSlatesA_Reveal);


void CBgnSlatesA_Reveal::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSlatesA_Reveal*pad=_GetPad<CBgpSlatesA_Reveal>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	switch(pad->_op)
	{
		case CBgpSlatesA_Reveal::Op_RevealNearBy:
		{
			loSlates->RevealNearBy(ctx->idxSlate,pad->_radius);
			break;
		}
		case CBgpSlatesA_Reveal::Op_RevealAll:
		{
			loSlates->RevealAll(ctx->idxSlate);
			break;
		}
	}


	_OutputOk(outputs,1,"结束");
}
