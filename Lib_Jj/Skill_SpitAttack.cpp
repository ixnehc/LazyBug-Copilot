
#include "stdh.h"


#include "Skill_SpitAttack.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"
#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_SpitAttack

BIND_SKILLPARAM(Skill_SpitAttack,SkillParam_SpitAttack);


void Skill_SpitAttack::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	SkillParam_SpitAttack *param=_rec->GetParam<SkillParam_SpitAttack>();
	_casting.Init(this);
	_tHit=_rec->HitDelay;

	LevelPos posTarget;
	BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
	if (LevelUtil_CalcTargetPos(_owner->GetLevel(),_target,posTarget))
	{
		float dist=_owner->GetFramePos().getDistanceFrom(posTarget);
		_tHit+=ANIMTICK_FROM_SECOND(dist/param->spd);
	}

	_UpdateDamage(0);
}

void Skill_SpitAttack::_UpdateDamage(AnimTick dt)
{
	_casting.UpdateToCasted(dt);
	if (_casting.NeedCasted())
		_SetState(SkillState_Casted);
	if (_casting.NeedCasted())
		_SetState(SkillState_Finished);

	if (_state!=SkillState_Finished)
	{
		if (_casting.IsFired())
		{
			CLevel *level=_owner->GetLevel();
			AnimTick t=level->GetT_();
			if (t>_tBirth+_tHit)
			{
				CLevelObj*lo=GetLevel()->GetIDs()->LoFromID(_target.ObjID());
				CLevelDecider *decider=GetLevel()->GetDecider();
				if (lo)
				{
					extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
					if (LevelUtil_CheckSkillTarget(_rec,_owner,lo))
					{
						DealArg arg;
						arg.dir.setXZ(lo->GetFramePos()-_owner->GetFramePos());
						arg.link.id=GetLevel()->GenOpLinkID();
						arg.grd=_grd;

						_MakeDeals(lo,arg);
					}
				}
				_SetState(SkillState_Finished);
			}
		}
	}
}


void Skill_SpitAttack::_OnUpdate(AnimTick dt)
{
	_UpdateDamage(dt);

}
