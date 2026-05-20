
#include "stdh.h"

#include "Ability_PushSlideway.h"

#include "LevelPlayer.h"
#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordItem.h"
 
//////////////////////////////////////////////////////////////////////////
//CUpgradePushSlideway_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradePushSlideway_Init);
BOOL CUpgradePushSlideway_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_PushSlideway *abilityFire=(CLevelAbility_PushSlideway *)ability;

	return TRUE;
}

 

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_Fist

void CLevelAbility_PushSlideway::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_AttackA,_upgradeInitial->idSkill);
}

void CLevelAbility_PushSlideway::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_PushSlideway::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_PushSlideway::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);
}


void CLevelAbility_PushSlideway::_OnUpdate(LevelTick dt)
{

}
