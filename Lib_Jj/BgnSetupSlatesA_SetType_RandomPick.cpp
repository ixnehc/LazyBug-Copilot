/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesA_SetType_RandomPick
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSetupSlatesA_SetType_RandomPick.h"


IMPLEMENT_CLASS(LevelSlatesGroup);


////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesA_SetType_RandomPick
BIND_BGN_CLASS(CBgnSetupSlatesA_SetType_RandomPick,CBgpSetupSlatesA_SetType_RandomPick);


void CBgnSetupSlatesA_SetType_RandomPick::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesA_SetType_RandomPick*pad=_GetPad<CBgpSetupSlatesA_SetType_RandomPick>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	if (pad->entries.size()>0)
	{
		LevelSlatesGroup *result=NULL;
		if (pad->result!=StringID_Invalid)
			result=Class_New(LevelSlatesGroup);

		loSlates->Setup_SetType_RandomPick(pad->grp,&pad->entries[0],pad->entries.size(),result?&result->slates:NULL);

		if (pad->result)
			_GetMem()->DepositObj(pad->result,result);
	}

	_OutputOk(outputs,1,"结束");
}
