/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesA_SetType
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSetupSlatesA_SetType.h"
#include "BgnSetupSlatesA_SetType_RandomPick.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesA_SetType
BIND_BGN_CLASS(CBgnSetupSlatesA_SetType,CBgpSetupSlatesA_SetType);


void CBgnSetupSlatesA_SetType::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesA_SetType*pad=_GetPad<CBgpSetupSlatesA_SetType>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);


	LevelSlatesGroup *result=NULL;
	if (pad->result!=StringID_Invalid)
		result=Class_New(LevelSlatesGroup);

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoSlatesA)))
	{
		CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;
		loSlates->Setup_SetType(pad->grp,pad->tp,result?&result->slates:NULL);
	}


	if (pad->result)
		_GetMem()->DepositObj(pad->result,result);

	_OutputOk(outputs,1,"结束");
}
