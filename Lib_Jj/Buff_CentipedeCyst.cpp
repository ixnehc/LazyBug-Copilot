/********************************************************************
	created:	2020/02/24
	file base:	Buff_CentipedeCyst
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LevelRecordBuff.h"

#include "LoCentipede.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "Buff_CentipedeCyst.h"
#include "Buff_CentipedeNode_Move.h"

#include "LevelPlayerStates.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//CBuff_CentipedeCyst

BIND_BUFFPARAM(Buff_CentipedeCyst,BuffParam_CentipedeCyst,BuffArg_CentipedeCyst);



void Buff_CentipedeCyst::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_CentipedeCyst *arg=(BuffArg_CentipedeCyst *)arg0;
	BuffParam_CentipedeCyst*param=_rec->GetParam<BuffParam_CentipedeCyst>();

	CLevelObj *owner=_GetOwner();

	_stage=Stage_Growing;

	if (TRUE)
	{
		float dur=CSysRandom::RandRange(ANIMTICK_TO_SECOND(param->durWaitMin),ANIMTICK_TO_SECOND(param->durWaitMax));
		_durWaiting=ANIMTICK_FROM_SECOND(dur);
	}

}

void Buff_CentipedeCyst::_OnDestroy()
{
}

void Buff_CentipedeCyst::_SetStage(Stage stage)
{
	_stage=stage;
	_AddSyncDataOp();
}

BuffFlag Buff_CentipedeCyst::CalcFlag(AnimTick tAge,AnimTick durGrow,Buff_CentipedeCyst::Stage stage)
{
	if (tAge<(AnimTick)(((float)durGrow)*0.8f))
		return BuffFlag_GhostCollide|BuffFlag_NotAttackable;

	if (stage==Buff_CentipedeCyst::Stage_Exploding)
		return BuffFlag_GhostCollide|BuffFlag_NotAttackable;

	return 0;
}



void Buff_CentipedeCyst::_OnUpdate(AnimTick dt)
{
	BuffParam_CentipedeCyst*param=_rec->GetParam<BuffParam_CentipedeCyst>();

	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	if (LevelUtil_CheckDead(_GetOwner()))
		return;

	if (_stage==Stage_Growing)
	{
		if (GetAge()>param->durGrow)
		{
			_SetStage(Stage_Waiting);
		}
	}

	if (_stage==Stage_Waiting)
	{
		if (GetAge()>param->durGrow+_durWaiting)
		{
			_SetStage(Stage_Activating);
		}
	}

	if (_stage==Stage_Activating)
	{
		if (GetAge()>param->durGrow+_durWaiting+param->durActivating)
		{
			if (TRUE)
			{
				CLevelObj *owner=_GetOwner();
				if (owner)
				{
					LevelFace face=owner->GetFrameFace();
					LevelPos3D pos3D=owner->GetFramePos3D();
					LevelPos dir=LevelFaceToDir(face);

					DealArg arg;
					arg.dir.setXZ(dir);
					_MakeDeals(pos3D,arg);
				}
			}

			_SetStage(Stage_Exploding);
		}
	}

	if (_stage==Stage_Exploding)
	{
		if (GetAge()>param->durGrow+_durWaiting+param->durActivating+ANIMTICK_FROM_SECOND(2.0f))
		{
			_GetOwner()->DeferDestroy();
		}
	}

	_SetFlag(CalcFlag(GetAge(),param->durGrow,_stage));
}

void Buff_CentipedeCyst::_WriteData(CBitPacket *bp)
{
	bp->Bits_Write(_stage,3);
}

void Buff_CentipedeCyst::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_Kill)
	{
		LeKill &e=(LeKill &)e0;

		if (e.loTarget==_GetOwner())
		{
			Buff_CentipedeNode_Move *buffMove=(Buff_CentipedeNode_Move *)_buffs->FindBuff(Class_Ptr2(Buff_CentipedeNode_Move));
			if (buffMove)
			{
				CLoCentipede *loCentipede=buffMove->GetLoCentipede();
				if (loCentipede)
				{
					LevelObjID idNode;
					if (loCentipede->GetNodeFromCyst(e.loTarget->GetID(),idNode))
					{
						DWORD idxNode;
						if (loCentipede->GetNodeIndex(idNode,idxNode))
							loCentipede->GetCombatState().NotifyCystKilled(idxNode,_GetOwner()->GetID());
					}
				}
			}
		}
	}

	if (e0.GetType()==LET_PreKill)
	{
		LePreKill &e=(LePreKill &)e0;

		if (_stage==Stage_Activating)
		{
			if (e.loTarget==_GetOwner())
			{
				Buff_CentipedeNode_Move *buffMove=(Buff_CentipedeNode_Move *)_buffs->FindBuff(Class_Ptr2(Buff_CentipedeNode_Move));
				if (buffMove)
				{
					CLoCentipede *loCentipede=buffMove->GetLoCentipede();
					if (loCentipede)
					{
						LevelObjID idNode;
						if (loCentipede->GetNodeFromCyst(e.loTarget->GetID(),idNode))
						{
							loCentipede->BreakFromCyst(idNode,e.link);
						}
					}
				}
			}
		}
	}

}
