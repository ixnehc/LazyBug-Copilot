/********************************************************************
	created:	2020/02/24
	file base:	Buff_Possess
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LevelRecordBuff.h"


#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "Buff_Possess.h"

#include "LevelPlayerStates.h"

//////////////////////////////////////////////////////////////////////////
//CBuff_Possess

BIND_BUFFPARAM(Buff_Possess,BuffParam_Possess,BuffArg_Possess);



void Buff_Possess::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_Possess *arg=(BuffArg_Possess *)arg0;
	BuffParam_Possess *param=(BuffParam_Possess *)_param;

	CLevelObj *owner=_GetOwner();

	_idTarget=arg->idTarget;
	_dur=ANIMTICK_INFINITE;

}

void Buff_Possess::_OnDestroy()
{
}

void Buff_Possess::_OnUpdate(AnimTick dt)
{
	BuffParam_Possess *param=(BuffParam_Possess *)_param;

	if (!_bEntered)
	{
		if (_tAge>param->durEnter)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *loTarget=LevelUtil_GetAliveLo(_GetLevel(),_idTarget);
			if (loTarget)
			{
				extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
				CLevelPlayer *player=LevelUtil_PlayerFromLo(loTarget);
				if (player)
				{
					LevelPlayerStates *lps=player->GetLPS();
					if (lps)
					{
						lps->base.worm++;
						lps->base.SetDirtyDB_Urgent();
					}
				}

				CLevelOps *ops=loTarget->GetOps();
				if (ops)
				{
					LevelOp_WormMod *op=loTarget->NewOp<LevelOp_WormMod>(LevelOpLink());
					op->delta=1;
					ops->AddOp(op);
				}

				_bEntered=TRUE;
			}
		}
	}

}



void Buff_Possess::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_idTarget);
}
