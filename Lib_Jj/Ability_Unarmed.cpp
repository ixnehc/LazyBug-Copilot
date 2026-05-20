
#include "stdh.h"

#include "Ability_Unarmed.h"

#include "LevelPlayer.h"
#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordItem.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeUnarmed_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeUnarmed_Init);
BOOL CUpgradeUnarmed_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_Unarmed *abilityFire=(CLevelAbility_Unarmed *)ability;

	return TRUE;
}

 

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_Fist

void CLevelAbility_Unarmed::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_AttackA,_upgradeInitial->idSkill);
}

void CLevelAbility_Unarmed::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_Unarmed::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_Unarmed::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);
}


void CLevelAbility_Unarmed::_OnUpdate(LevelTick dt)
{

}
