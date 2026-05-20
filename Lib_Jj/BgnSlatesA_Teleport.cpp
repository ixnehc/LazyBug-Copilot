/********************************************************************
	created:	2017/11/04 
	author:		cxi
	
	purpose:	CBgnSlatesA_Teleport
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelDecider.h"
#include "LevelBehavior.h"

#include "LoSlatesA.h"

#include "BgnSlatesA_Teleport.h"

#include "Buff_Teleport.h"



////////////////////////////////////////////////////////////////////////
//CBgnSlatesA_Teleport
BIND_BGN_CLASS(CBgnSlatesA_Teleport,CBgpSlatesA_Teleport);


void CBgnSlatesA_Teleport::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpSlatesA_Teleport*pad=_GetPad<CBgpSlatesA_Teleport>();
	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	if (loSlates->CheckInTeleportCD())
	{
		_OutputOk(outputs,1,"结束");
		return;
	}
	CLevelObj *loTarget=loSlates->GetThreat();
	if (loTarget)
	{
		if (ctx->lo->GetLevel())
		{
			CLevelDecider *decider=ctx->lo->GetLevel()->GetDecider();
			if (decider)
			{
				if (!pad->bLeave)
				{
					LevelSlateIdx idx=loSlates->FindTeleportTarget(ctx->idxSlate);
					if (idx!=LevelSlateIdx_Invalid)
					{
						LevelPos pos;
						if (loSlates->GetSlatePos(idx,pos))
						{
							BuffArg_Teleport arg;
							arg.pos=pos;
							arg.face=loTarget->GetFrameFace();

							decider->MakeBuff(loTarget,pad->idBuff,0,&arg,TRUE);

							loSlates->StartTeleportCD();
							return;
						}
					}
				}
				else
				{
					LevelPos pos;
					if (loSlates->GetTeleLeavePos(pos))
					{
						BuffArg_Teleport arg;
						arg.pos=pos;
						arg.face=loTarget->GetFrameFace();

						decider->MakeBuff(loTarget,pad->idBuff,0,&arg,TRUE);

						return;
					}
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}

void CBgnSlatesA_Teleport::Update(BGNOutputs &outputs)
{
	CBgpSlatesA_Teleport*pad=_GetPad<CBgpSlatesA_Teleport>();

	LevelBehaviorContext *ctx=_GetCtx();

	assert(ctx->lo);
	assert(ctx->lo->GetClass()->CheckName("CLoSlatesA"));

	CLoSlatesA *loSlates=(CLoSlatesA*)ctx->lo;

	CLevelObj *loTarget=loSlates->GetThreat();
	if (loTarget)
	{
		extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
		if (LevelUtil_FindBuffByRecordID(loTarget,pad->idBuff))
			return;
	}

	_OutputOk(outputs,1,"结束");
}
