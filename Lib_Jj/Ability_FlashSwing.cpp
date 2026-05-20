
#include "stdh.h"

#include "Ability_FlashSwing.h"

/*
BloodTeeth:
每命中一个单位,飞出一个利齿
DeathCall:
命中后,有几率产生一个死亡Aura,这些Aura最多可以累加至[3]个,在切换为DeathCall后,这些Aura可以100%产生必杀效果
FlameBlade:
累加FlameBlade的Fury,Fury满时,每次发动可以带火焰伤害
FlashSwing:
n/a
HonorSword:
根据Honor提升攻击力,杀敌累加Honor值
LightningBow:
向[5]米范围内敌人发出攻击
ObliterateBow:
一次挥击中成功的杀死敌人后,剩余的挥击产生爆炸伤害(不同颜色的刀光)
PhantomDagger:

SkullSword:

TeleportSword:

*/

//////////////////////////////////////////////////////////////////////////
//CUpgradeFlashSwing_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFlashSwing_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFlashSwing_LevelUp);
BOOL CUpgradeFlashSwing_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_FlashSwing *abilityFire=(CLevelAbility_FlashSwing *)ability;

	abilityFire->_idSkill=idSkill;
	abilityFire->_idDefSkill=idDefaultSkill;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_FlashSwing

void CLevelAbility_FlashSwing::_OnBuildRT()
{
	_BuildGradeRT();
	_AddSkillRT(LevelAbilityAction_AttackA,_idDefSkill);
	_AddSkillRT(LevelAbilityAction_FuryA,_idSkill);
}

void CLevelAbility_FlashSwing::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_FlashSwing::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}
void CLevelAbility_FlashSwing::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_LoadSync_SkillsRT(dp,records);
}


void CLevelAbility_FlashSwing::_OnUpdate(LevelTick dt)
{
}

