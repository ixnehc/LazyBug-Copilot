/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesA_SetSwitch
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"
#include "BgnSetupSlatesA_SetType_RandomPick.h"

#include "BgnSetupSlatesA_SetSwitch.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesA_SetSwitch
BIND_BGN_CLASS(CBgnSetupSlatesA_SetSwitch,CBgpSetupSlatesA_SetSwitch);


void CBgnSetupSlatesA_SetSwitch::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesA_SetSwitch*pad=_GetPad<CBgpSetupSlatesA_SetSwitch>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	if ((pad->varGrp!=StringID_Invalid)&&(pad->varLockGrp!=StringID_Invalid))
	{
		LevelSlatesGroup *grp=_GetMem()->GetObj<LevelSlatesGroup>(pad->varGrp);
		LevelSlatesGroup *grpLocks=_GetMem()->GetObj<LevelSlatesGroup>(pad->varLockGrp);
		if (grp&&grpLocks)
			loSlates->Setup_SetSwitch(grpLocks->slates,grp->slates,pad->channel);
	}

	if (pad->varPointerGrp!=StringID_Invalid)
	{
		LevelSlatesGroup *grp=_GetMem()->GetObj<LevelSlatesGroup>(pad->varPointerGrp);
		if (grp)
			loSlates->Setup_SetSwitchPointer(grp->slates,pad->channel);
	}

	_OutputOk(outputs,1,"结束");
}
