
#include "stdh.h"


#include "Ability_FlameBlade.h"

/*
BloodTeeth:
加速Fury累积速度,命中后使敌人进入Bleeding状态
DeathCall:
使用右键释放直线火焰墙,并释放Fury
FlameBlade:
n/a
FlashSwing:
Fury满时,可以使用右键发射火焰刀远程攻击
HonorSword:
Honor加伤害,并延长着火时间
LightningBow:
Fury满时,右键发射火焰弧
ObliterateBow:
杀死敌人后,产生一个持续一段时间的火焰足迹(走到哪,烧到哪)
PhantomDagger:
只要PhatomDagger是Active的,自动按照一定速度ChargeFury,同时按右键可以释放Fury,命中敌人后,产生爆炸
SkullSword:
Fury满时,可以右键招唤一个在玩家头顶的持续一段时间的火焰骷髅,使周围的敌人进入燃烧状态
TeleportSword:
命中后,在附近产生一次爆炸
*/


//////////////////////////////////////////////////////////////////////////
//CUpgradeFlameBlade_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFlameBlade_Init);
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFlameBlade_LevelUp);
BOOL CUpgradeFlameBlade_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_FlameBlade *abilityFire=(CLevelAbility_FlameBlade *)ability;

	abilityFire->_tcpFury.bValid=TRUE;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_FlameBlade

void CLevelAbility_FlameBlade::_InitTechs()
{
	_AddTech(&_tcpFury);
}


void CLevelAbility_FlameBlade::_OnUpdate(LevelTick dt0)
{
}


void CLevelAbility_FlameBlade::_OnBuildRT()
{
	_BuildGradeRT();

	CUpgradeFlameBlade_Init *upgradeInitial=_upgradeInitial;
	_BuildSkillRT(upgradeInitial->settings);
}

void CLevelAbility_FlameBlade::_OnClearRT()
{
	_ClearSkillsRT();
	_ClearGradeRT();
}

void CLevelAbility_FlameBlade::_SaveSync(CDataPacket &dp)
{
	_SaveSync_SkillsRT(dp);
}

void CLevelAbility_FlameBlade::_LoadSync(CDataPacket &dp,CRecords *recordsSkill)
{
	_LoadSync_SkillsRT(dp,recordsSkill);
}

BOOL CLevelAbility_FlameBlade::CanFury()
{
	TechSync_Fury *sync=FindTechSync<TechSync_Fury>();
	if (sync)
		return sync->bActive!=0;
	return FALSE;
}
