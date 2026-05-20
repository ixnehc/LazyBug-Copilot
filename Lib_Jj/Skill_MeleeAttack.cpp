
#include "stdh.h"


#include "Skill_MeleeAttack.h"

#include "LevelRecordSkill.h"
#include "LevelRecordUnit.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_MeleeAttack

BIND_SKILLPARAM(Skill_MeleeAttack,SkillParam_MeleeAttack);


void Skill_MeleeAttack::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	SkillParam_MeleeAttack *param=_rec->GetParam<SkillParam_MeleeAttack>();
	_nToDamages=param->hitsEx.size()+1;

	_tCasting=0;
	_UpdateDamage(0);
}

void Skill_MeleeAttack::_UpdateDamage(AnimTick dt)
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

		SkillParam_MeleeAttack *param=_rec->GetParam<SkillParam_MeleeAttack>();

		AnimTick tHitDelay=_rec->HitDelay;
		if (_nToDamages>1)
			tHitDelay=param->hitsEx[_nToDamages-1-1];

		if (_nDamages<_nToDamages)
		{
//			if (_tCasting+LEVEL_FRAME_TICK>_tLastHit+tHitDelay)
			if (_tCasting>_tLastHit+tHitDelay)
			{
				CLevelObj*lo=GetLevel()->GetIDs()->LoFromID(_target.ObjID());
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

				_nDamages++;
				_tLastHit+=tHitDelay;

			}
		}

		if ((_nDamages>=_nToDamages)&&(_tCasting>_rec->CastTime))
		{
			_SetState(SkillState_Finished);
		}

	}
}


void Skill_MeleeAttack::_OnUpdate(AnimTick dt)
{
	_UpdateDamage(dt);

}
