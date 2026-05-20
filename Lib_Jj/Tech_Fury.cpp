
#include "stdh.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

#include "LevelAttrs_DamageAttr.h"

#include "Tech_Fury.h"

#include "LevelAbility.h"


BIND_TECH(CTech_Fury,TechParam_Fury,TechSync_Fury);

void CTech_Fury::OnBuildRT()
{
	TechParam_Fury *param=(TechParam_Fury *)_param;
	LevelGrade grdAbility=_owner->GetGradeRT();

	_durActiveLastingRT=param->durActiveLasting+grdAbility*param->durActiveLastingAdd;

	_dmgFireRT=(short)((float)param->dmgFire*(1.0f+param->dmgFireRate*(float)grdAbility));


}

void CTech_Fury::OnClearRT()
{
	_durActiveLastingRT=0;
	_dmgFireRT=0;
}


void CTech_Fury::OnUpdate(AnimTick dt)
{
	TechParam_Fury *param=(TechParam_Fury *)_param;


	if (_bActive)
	{
		_durActive+=dt;
		if (_durActive>_durActiveLastingRT)
		{
			_bActive=FALSE;
			_durActive=0;
		}
		else
		{
			if (_owner)
			{
				if (_owner->GetOwner())
				{
					CLevelObj *loOwner=_owner->GetOwner();
					extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
					CLevelSkill *skill=LevelUtil_GetCastingSkill(loOwner);
					if (skill)
					{
						LevelRecordSkill *recSkill=skill->GetRec();
						if (_owner->CheckAbilityActionSkillRecord_Fury(recSkill))
						{
							_bActive=FALSE;
							_fury=0.0f;
						}
					}
				}
			}

		}
	}

	if (!_bActive)
	{
		_fury-=param->furyCD*ANIMTICK_TO_SECOND(dt);
		if (_fury<0.0f)
			_fury=0.0f;
	}

}

void CTech_Fury::OnEvent(LevelEvent &e0)
{
	TechParam_Fury *param=(TechParam_Fury *)_param;
	if (e0.GetType()==LET_Damage)
	{
		AnimTick t=0;
		if (_owner)
		{
			if (_owner->GetOwner())
				t=_owner->GetOwner()->GetT();
		}

		if (t>_tLastDamage)
		{
			LeDamage &e=(LeDamage &)e0;
			if (e.osbSrc)
			{
				CLevelSkill *skill=e.osbSrc->GetRootSkill();
				if (skill)
				{
					LevelRecordSkill *recSkill=skill->GetRec();
					if (_owner)
					{
						if (_owner->CheckAbilityActionSkillRecord_Attack(recSkill))
						{
							_fury+=param->furyPerHit;
							if (_fury>1.0f)
							{
								_fury=1.0f;
								_bActive=TRUE;
								_durActive=0;//น้0
							}
							_tLastDamage=t;
						}
					}
				}
			}
		}
	}
	if (e0.GetType()==LET_ModDamageAttr)
	{
		LeModDamageAttr &e=(LeModDamageAttr&)e0;
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetSkill();
			if (skill)
			{
				LevelRecordSkill *recSkill=skill->GetRec();
				if (_owner)
				{
					if (_owner->GetSkillRecordRT(LevelAbilityAction_AttackA)==recSkill)
					{
						if (_bActive)
						{
							e.modsAttack->modsHit.atkRate+=param->rateHit;
							e.modsAttack->modsDamage[DamageAttrType_Crush].atkRate+=param->rateCrush;
							e.modsAttack->modsDamage[DamageAttrType_Fire].atkAdd+=_dmgFireRT;
							e.bAttackMods=TRUE;
						}
					}
				}
			}
		}
	}
}

void CTech_Fury::SaveSync(LevelTechSync &sync)
{
	TechSync_Fury &syncFury=(TechSync_Fury &)sync;
	syncFury.fury=(BYTE)(i_math::clamp_f(_fury,0.0f,1.0f)*100.0f);
	syncFury.bActive=_bActive;
}

