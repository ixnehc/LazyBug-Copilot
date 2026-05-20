
#include "stdh.h"

#include "Ability_Fire.h"

#include "Deal_Dmg.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeFire_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFire_Init);
BOOL CUpgradeFire_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_Fire *abilityFire=(CLevelAbility_Fire *)ability;

	abilityFire->_idSkill=idSkill;
	abilityFire->_attackFire=attackFire;
	abilityFire->_cost=cost;
	abilityFire->_nBullets=1;

	return TRUE;
}


IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFire_IncDmg);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFire_DecCost);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFire_IncBullet);


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_Fire

void CLevelAbility_Fire::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_AttackA,_idSkill);

	LevelRecordSkill *recSkill=_skillsRT.GetSkillRecord(LevelAbilityAction_AttackA);
	if (recSkill)
	{
		LevelRecordSkill *rec=recSkill;

		rec->cost.crystal_=_cost;
// 		SkillParam_BulletAttack *param=rec->GetParam<SkillParam_BulletAttack>();
// 		if (param)
// 		{
// 			param->Count=_nBullets;
// 		}

		Deal_Dmg *deal=rec->GetDeal<Deal_Dmg>();
		if (deal)
		{
			LevelAttr_Damages *attacks=deal->_attacks.Get();
			attacks->damages[DamageAttrType_Fire].lo=
				attacks->damages[DamageAttrType_Fire].hi=(short)_attackFire;
		}
	}

}

void CLevelAbility_Fire::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_Fire::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);

	dp.Data_WriteSimple(_cost);
	dp.Data_WriteSimple(_attackFire);
	dp.Data_WriteSimple(_nBullets);
}

void CLevelAbility_Fire::_LoadSync(CDataPacket &dp,CRecords *recordsSkill)
{
	_LoadSync_SkillsRT(dp,recordsSkill);

	dp.Data_ReadSimple(_cost);
	dp.Data_ReadSimple(_attackFire);
	dp.Data_ReadSimple(_nBullets);
}


void CLevelAbility_Fire::_OnUpdate(LevelTick dt)
{
}

