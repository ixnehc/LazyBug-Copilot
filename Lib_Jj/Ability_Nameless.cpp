
#include "stdh.h"

#include "Ability_Nameless.h"


/*
BloodTeeth:

DeathCall:

FlameBlade:

FlashSwing:

Nameless:

LightningBow:

ObliterateBow:

PhantomDagger:

SkullSword:

TeleportSword:

*/

//////////////////////////////////////////////////////////////////////////
//CUpgradeNameless_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeNameless_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeNameless_LevelUp);

BOOL CUpgradeNameless_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_Nameless *abilityNameless=(CLevelAbility_Nameless *)ability;


	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_Nameless

void CLevelAbility_Nameless::_InitTechs()
{
}


void CLevelAbility_Nameless::_OnBuildRT()
{
	_BuildGradeRT();
	CUpgradeNameless_Init *upgradeInitial=_upgradeInitial;
	_BuildSkillRT(upgradeInitial->settings);
}

void CLevelAbility_Nameless::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_Nameless::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_Nameless::_LoadSync(CDataPacket &dp,CRecords *recordsSkill)
{
	_LoadSync_SkillsRT(dp,recordsSkill);
}


void CLevelAbility_Nameless::_OnUpdate(LevelTick dt0)
{
}

