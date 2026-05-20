
#include "stdh.h"

#include "Ability_HonorSword.h"


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
//CUpgradeHonorSword_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHonorSword_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHonorSword_LevelUp);

BOOL CUpgradeHonorSword_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_HonorSword *abilityFire=(CLevelAbility_HonorSword *)ability;

	abilityFire->_idSkill=idSkill;
	abilityFire->_idDefSkill=idDefaultSkill;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_HonorSword

void CLevelAbility_HonorSword::_InitTechs()
{
}


void CLevelAbility_HonorSword::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_AttackA,_idDefSkill);
	_AddSkillRT(LevelAbilityAction_FuryA,_idSkill);
}

void CLevelAbility_HonorSword::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_HonorSword::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_HonorSword::_LoadSync(CDataPacket &dp,CRecords *recordsSkill)
{
	_LoadSync_SkillsRT(dp,recordsSkill);
}


void CLevelAbility_HonorSword::_OnUpdate(LevelTick dt0)
{
}

