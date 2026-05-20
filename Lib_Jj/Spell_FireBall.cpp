
#include "stdh.h"

#include "Spell_FireBall.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeFireBall_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFireBall_Init);
BOOL CUpgradeFireBall_Init::Init(CLevelAbility *ability)
{
	CLevelSpell_FireBall *abilityFire=(CLevelSpell_FireBall *)ability;

	abilityFire->_idSkill=idSkill;

	return TRUE;
}


IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFireBall_LevelUp);


//////////////////////////////////////////////////////////////////////////
//CLevelSpell_FireBall

void CLevelSpell_FireBall::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_MissileA,_idSkill);
}

void CLevelSpell_FireBall::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelSpell_FireBall::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);

	dp.Data_WriteSimple(_grd);

}

void CLevelSpell_FireBall::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);

	dp.Data_ReadSimple(_grd);
}


void CLevelSpell_FireBall::_OnUpdate(LevelTick dt)
{
}
