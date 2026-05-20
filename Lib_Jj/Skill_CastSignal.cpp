
#include "stdh.h"


#include "Skill_CastSignal.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_CastSignal

BIND_SKILLPARAM(Skill_CastSignal,SkillParam_CastSignal);


void Skill_CastSignal::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	_casting.Init(this);

	_UpdateDamage(0);
}

void Skill_CastSignal::_UpdateDamage(AnimTick dt)
{
	_casting.UpdateToFinished(dt);
	if (_casting.NeedFire())
	{
		SkillParam_CastSignal *param=_rec->GetParam<SkillParam_CastSignal>();
		if (param->nm!=StringID_Invalid)
		{
			CLevel *level=GetLevel();

			extern LevelObjID GetSkillTargetObjID(LevelSkillTarget &target);
			LevelObjID idLo=GetSkillTargetObjID(_target);
			if (idLo!=LevelObjID_Invalid)
				level->GetEventMap()->AddSignal(param->nm,idLo,_owner->GetID());
			else
				level->GetEventMap()->AddSignal(param->nm,_owner->GetFramePos(),param->radius,_owner->GetID());
		}
	}

	if (_casting.NeedFinished())
		_SetState(SkillState_Finished);

}


void Skill_CastSignal::_OnUpdate(AnimTick dt)
{
	_UpdateDamage(dt);

}
