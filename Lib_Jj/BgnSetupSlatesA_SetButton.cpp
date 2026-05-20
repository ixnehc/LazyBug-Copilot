/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSetupSlatesA_SetButton
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"
#include "BgnSetupSlatesA_SetType_RandomPick.h"

#include "BgnSetupSlatesA_SetButton.h"



////////////////////////////////////////////////////////////////////////
//CBgnSetupSlatesA_SetButton
BIND_BGN_CLASS(CBgnSetupSlatesA_SetButton,CBgpSetupSlatesA_SetButton);


void CBgnSetupSlatesA_SetButton::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSetupSlatesA_SetButton*pad=_GetPad<CBgpSetupSlatesA_SetButton>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	if ((pad->varGrp!=StringID_Invalid)&&(pad->varLockGrp!=StringID_Invalid))
	{
		LevelSlatesGroup *grp=_GetMem()->GetObj<LevelSlatesGroup>(pad->varGrp);
		LevelSlatesGroup *grpLocks=_GetMem()->GetObj<LevelSlatesGroup>(pad->varLockGrp);
		if (grp&&grpLocks)
			loSlates->Setup_SetButton(grpLocks->slates,grp->slates,pad->channel);
	}

	_OutputOk(outputs,1,"结束");
}
