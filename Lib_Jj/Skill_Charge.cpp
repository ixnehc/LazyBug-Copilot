
#include "stdh.h"


#include "Skill_Charge.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelDecider.h"

#include "LevelObjMove.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_Charge
BIND_SKILLPARAM(Skill_Charge,SkillParam_Charge);


void Skill_Charge::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);

	extern BOOL LevelUtil_ConvertSkillTarget(CLevel *level,LevelSkillTarget &target,LevelSkillTarget::Type tp);
	LevelUtil_ConvertSkillTarget(_owner->GetLevel(),_target,LevelSkillTarget::Target_Aim);

	LevelPos src=_owner->GetFramePos();
	if (_target.bOrg)
		src=_target.org;

	LevelPos target;
	assert(_target.tp==LevelSkillTarget::Target_Aim);
	target=_target.Aim();

	LevelPos dir=target-src;
	dir.normalize();

	_dur=ANIMTICK_FROM_SECOND(0.4f);
	float distCharge=8.0f;

	LevelPos posTeleport;
	if (TRUE)
	{
		posTeleport=src+dir*distCharge;

		LevelPos hit;
		if (_owner->GetLevel()->GetUnitMgr()->StaticRayCast(UnitFindPath_Walkable,src,target,hit))
			posTeleport=hit;
	}

	LevelTeleportID idTeleport=_owner->GenTeleportID();

	//进行Teleport
	if (TRUE)
	{
		CLevelObjMove *move=_owner->GetMove();
		if (move)
			move->Teleport(idTeleport,posTeleport,atan2f(dir.y,dir.x));

		if (_owner->IsPlayer())
		{
			CLevelPlayer *player=_owner->GetLevel()->GetPlayer(_owner->GetPlayerID());
			if (player)
			{
				player->GetMove().PauseMove();
				player->GetMove().AuthorizeTeleport(idTeleport,posTeleport);
			}
		}
	}


	LevelOp_SkillTeleport *op=NewOp<LevelOp_SkillTeleport>(LevelOpLink());
	op->id=idTeleport;
	op->target=target;
	op->dur=_dur;
	_owner->AddOp(op);

}



void Skill_Charge::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		_dur=ANIMTICK_SAFE_MINUS(_dur,dt);
		if (_dur<=0)
			_Finish();
	}

}


void Skill_Charge::_Finish()
{
	_SetState(SkillState_Finished);
}
