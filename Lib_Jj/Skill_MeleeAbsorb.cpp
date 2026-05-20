
#include "stdh.h"


#include "Skill_MeleeAbsorb.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_Absorb

BIND_SKILLPARAM(Skill_MeleeAbsorb,SkillParam_MeleeAbsorb);


void Skill_MeleeAbsorb::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	SkillParam_MeleeAbsorb *param=_rec->GetParam<SkillParam_MeleeAbsorb>();

	_tCasting=0;
	_UpdateDamage(0);
}

void Skill_MeleeAbsorb::_UpdateDamage(AnimTick dt)
{

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

		SkillParam_MeleeAbsorb *param=_rec->GetParam<SkillParam_MeleeAbsorb>();

		if (!_bDealed)
		{
			AnimTick tHitDelay=_rec->HitDelay;

			if (_tCasting+LEVEL_FRAME_TICK>tHitDelay)
			{
				CLevelDecider *decider=GetLevel()->GetDecider();
				if (_owner)
				{
					DealArg arg;
					arg.dir.set(0,0,0);
					arg.link.id=GetLevel()->GenOpLinkID();
					arg.grd=_grd;

					_MakeDeals(_owner,arg);
				}
				_bDealed=TRUE;
			}
		}

		if (_bDealed&&(_tCasting>_rec->CastTime))
			_SetState(SkillState_Finished);
	}
}


void Skill_MeleeAbsorb::_OnUpdate(AnimTick dt)
{
	_UpdateDamage(dt);

}
