
#include "stdh.h"


#include "Skill_JumpAttack.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelDecider.h"

#include "LevelObjMove.h"

#include "LevelOSB.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_JumpAttack
BIND_SKILLPARAM(Skill_JumpAttack,SkillParam_JumpAttack);


void Skill_JumpAttack::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);

	assert((_target.tp==LevelSkillTarget::Target_Pos)||(_target.tp==LevelSkillTarget::Target_DefObj));
	LevelPos src=_owner->GetFramePos();
	if (_target.bOrg)
		src=_target.org;

	BOOL bTarget=FALSE;
	LevelPos target;
	if (_arg)
	{
		if (_arg->sites.size()>0)
		{
			target=_arg->sites[0];
			bTarget=TRUE;
		}
	}

	if (!bTarget)
	{
		_SetState(SkillState_Fail);
		return;
	}

	float face=_owner->GetFrameFace();
	if (bTarget)
	{
		LevelPos dir=target-src;
		if (dir.getLengthSQ()>0.0001f)
			face=atan2f(dir.y,dir.x);
	}


// 	if (_target.tp==LevelSkillTarget::Target_Pos)
// 	else
// 	{
// 		CLevelObj *lo=LevelUtil_GetAliveLo(GetLevel(),_target.ObjID());
// 		if (!lo)
// 		{
// 			_SetState(SkillState_Fail);
// 			return;
// 		}
// 		float radius=lo->GetRadius_();
// 		float radiusMe=_owner->GetRadius_();
// 
// 		target=lo->GetFramePos();
// 
// 		LevelPos dir=target-src;
// 		dir.safe_normalize();
// 		target-=dir*(radiusMe+radius);
// 	}

	SkillParam_JumpAttack *param=(SkillParam_JumpAttack *)_param;


	LevelTeleportID idTeleport=_owner->GenTeleportID();

	//进行Teleport
	if (TRUE)
	{
		CLevelObjMove *move=_owner->GetMove();
		if (move)
			move->Teleport(idTeleport,target,face);

		if (_owner->IsPlayer())
		{
			CLevelPlayer *player=_owner->GetLevel()->GetPlayer(_owner->GetPlayerID());
			if (player)
			{
				player->GetMove().PauseMove();
				player->GetMove().AuthorizeTeleport(idTeleport,target);
			}
		}
	}


	LevelOp_SkillTeleport *op=NewOp<LevelOp_SkillTeleport>(LevelOpLink());
	op->id=idTeleport;
	op->target=target;
	op->dur=param->durJump;
	_owner->AddOp(op);

}

extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);


void Skill_JumpAttack::_OnUpdate(AnimTick dt)
{
	SkillParam_JumpAttack *param=(SkillParam_JumpAttack *)_param;

	if (_state==SkillState_Casting)
	{
		if (!_bJumpReach)
		{
			if (_GetAge()>=param->durJump)
			{
				_bJumpReach=TRUE;
				LevelUtil_AccumCastingTime(_owner,_GetAge()-param->durJump,_tCasting);
			}
		}
		else
			LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		if (!_bDealed)
		{
			if (_bJumpReach)
			{
				if (_tCasting+LEVEL_FRAME_TICK>_rec->HitDelay)
				{
					if (_target.tp==LevelSkillTarget::Target_DefObj)
					{
						CLevelObj*lo=LevelUtil_GetAliveLo(GetLevel(),_target.ObjID());
						CLevelDecider *decider=GetLevel()->GetDecider();
						if (lo)
						{
							extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
							if (LevelUtil_CheckSkillTarget(_rec,_owner,lo))
							{
								if (decider->CheckInRange(_owner,lo,param->rangeDmg))
								{
									DealArg arg;
									arg.dir.setXZ(lo->GetFramePos()-_owner->GetFramePos());
									arg.link.id=GetLevel()->GenOpLinkID();
									arg.grd=_grd;

									_MakeDeals(lo,arg);
								}
							}
						}
					}
					_bDealed=TRUE;
				}
			}
		}


		if (_bJumpReach&&_tCasting>_rec->CastTime)
		{
			_Finish();
		}

	}

}


void Skill_JumpAttack::_Finish()
{
	_SetState(SkillState_Finished);
}
