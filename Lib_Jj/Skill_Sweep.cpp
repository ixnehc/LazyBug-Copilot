
#include "stdh.h"


#include "Skill_Sweep.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"
#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_Sweep
BIND_SKILLPARAM(Skill_Sweep,SkillParam_Sweep);


extern BOOL CheckAttackable(CLevelObj *lo);

void Skill_Sweep::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	extern BOOL LevelUtil_ConvertSkillTarget(CLevel *level,LevelSkillTarget &target,LevelSkillTarget::Type tp);
	LevelUtil_ConvertSkillTarget(_owner->GetLevel(),_target,LevelSkillTarget::Target_Aim);


	_SetState(SkillState_Casting);

	_tCasting=0;
	_UpdateDamage(0);
}

void Skill_Sweep::_UpdateDamage(AnimTick dt)
{
	SkillParam_Sweep*param=(SkillParam_Sweep*)_param;

	if (_bDamage)
		return;

	if (_state==SkillState_Casting)
	{
		if (_bBroken)
		{
			_SetState(SkillState_Finished);
			return;
		}

		//累加Cast的时间
		extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		if (_tCasting+LEVEL_FRAME_TICK>_rec->HitDelay)
		{
			_MakeFanDeals(param->fov,_rec->CastRange+_rec->CastRangeTolerance+1.0f);
			_SetState(SkillState_Finished);

			_bDamage=TRUE;
		}
	}
}


void Skill_Sweep::_OnUpdate(AnimTick dt)
{
	_UpdateDamage(dt);

}
