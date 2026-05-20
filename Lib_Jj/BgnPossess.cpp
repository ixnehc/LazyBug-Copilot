/********************************************************************
	created:	2020/02/25 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnPossess.h"

#include "Buff_Possess.h"

#include "LevelOSB.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Possess
BIND_BGN_CLASS(CBgn_Possess,CBgp_Possess);

  
void CBgn_Possess::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Possess*pad=_GetPad<CBgp_Possess>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelDecider *decider=ctx->level->GetDecider();

	RecordID idBuff=pad->_idBuff;
	if (idBuff!=RecordID_Invalid)
	{
		CBehaviorMem *mem=_GetMem();
		if (mem)
		{
			LevelObjID idTarget;
			if (TRUE==mem->GetID(pad->_varTarget,BehaviorMemType_ObjID,idTarget))
			{
				BuffArg_Possess arg;
				arg.idTarget=idTarget;
				decider->MakeBuff(LevelOSB(ctx->lo),ctx->lo,idBuff,ANIMTICK_INFINITE,&arg,LevelOpLink());
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}
