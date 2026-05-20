/********************************************************************
	created:	2019/10/21 
	author:		cxi
	
	purpose:	快闪
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelDeal.h"

#include "BgnJink.h"

#include "Buff_Jink.h"

#include "LevelOSB.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Jink
BIND_BGN_CLASS(CBgn_Jink,CBgp_Jink);

  
void CBgn_Jink::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Jink*pad=_GetPad<CBgp_Jink>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelDecider *decider=ctx->level->GetDecider();

	CLevelDecider::MakeJinkContext *ctxJink=decider->GetJinkContext();

	if (ctxJink)
	{
		RecordID idBuff=pad->_idBuff;
		if (idBuff!=RecordID_Invalid)
		{
			CBehaviorMem *mem=_GetMem();
			if (mem)
			{
				LevelPos pos;
				if (TRUE==mem->GetPos(pad->_varPos,pos))
				{
					BuffArg_Jink arg;
					arg.pos=pos;
					arg.strike=*ctxJink->strike;
					arg.face=0.0f;
					decider->MakeBuff(*(ctxJink->osbSrc),ctx->lo,idBuff,ANIMTICK_INFINITE,&arg,*ctxJink->link);
				}
			}
		}

		if (pad->_dealsOnPos.size()>0)
		{
			DealArg arg;
			arg.link=*ctxJink->link;
			arg.dir.setXZ(ctxJink->strike->GetDir());

			MakeDeals(pad->_dealsOnPos,LevelOSB(ctx->lo),ctx->lo->GetFramePos3D(),arg,NULL);
		}

	}

	_OutputOk(outputs,1,"结束");
}
