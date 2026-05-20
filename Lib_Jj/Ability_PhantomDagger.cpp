
#include "stdh.h"

#include "LevelEvents.h"

#include "LevelOSB.h"

#include "Ability_PhantomDagger.h"

/*
BloodTeeth:

DeathCall:

FlameBlade:

FlashSwing:

HonorSword:

LightningBow:

ObliterateBow:

PhantomDagger:

SkullSword:

TeleportSword:

*/

 
//////////////////////////////////////////////////////////////////////////
//CUpgradePhantomDagger_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradePhantomDagger_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradePhantomDagger_LevelUp);

BOOL CUpgradePhantomDagger_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_PhantomDagger *ability=(CLevelAbility_PhantomDagger *)ability_;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_PhantomDagger

void CLevelAbility_PhantomDagger::_InitTechs()
{
}

void CLevelAbility_PhantomDagger::_OnBuildRT()
{
	_BuildGradeRT();

	CUpgradePhantomDagger_Init*upgradeInitial=_upgradeInitial;
	_BuildSkillRT(upgradeInitial->settings);
}

void CLevelAbility_PhantomDagger::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_PhantomDagger::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);

	dp.Data_WriteSimple(_energy);
}

void CLevelAbility_PhantomDagger::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);

	dp.Data_ReadSimple(_energy);
}




void CLevelAbility_PhantomDagger::_OnUpdate(LevelTick dt0)
{
}

BOOL CLevelAbility_PhantomDagger::TestStartSkill(LevelSkillType &tpSkill)
{
	if (tpSkill.tpAbility_!=GetType())
		return FALSE;
	if (tpSkill.actionAbility==LevelAbilityAction_FuryA)
	{
		if (_energy<1.0f)
			return FALSE;
	}
	return TRUE;
}

void CLevelAbility_PhantomDagger::NotifyStartSkill(LevelSkillType &tpSkill)
{
	if (tpSkill.tpAbility_==GetType())
	{
		if (tpSkill.actionAbility==LevelAbilityAction_FuryA)
			_energy=0.0f;
	}
}


void CLevelAbility_PhantomDagger::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_Damage)
	{
		LeDamage &e=(LeDamage &)e0;
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetSkill();
			if (skill)
			{
				if (CheckAbilityActionSkillRecord_Attack(skill->GetRec()))
					_energy+=CSysRandom::RandRange(0.1f,0.2f);
			}
		}
	}
	if (e0.GetType()==LET_Kill)
	{
		LeKill &e=(LeKill &)e0;
		if (e.osbSrc)
		{
			CLevelSkill *skill=e.osbSrc->GetSkill();
			if (skill)
			{
				if (CheckAbilityActionSkillRecord_Attack(skill->GetRec()))
					_energy+=CSysRandom::RandRange(0.4f,0.6f);
			}
		}
	}

}
