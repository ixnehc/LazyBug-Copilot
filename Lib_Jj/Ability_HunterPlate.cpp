
#include "stdh.h"

#include "Ability_HunterPlate.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeHunterPlate_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHunterPlate_Init);
BOOL CUpgradeHunterPlate_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_HunterPlate *ability=(CLevelAbility_HunterPlate *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_HunterPlate

void CLevelAbility_HunterPlate::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_HunterPlate::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_HunterPlate::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_HunterPlate::_OnEvent(LevelEvent &e)
{

}

void CLevelAbility_HunterPlate::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeHunterPlate_Init *upgrade=_GetInitialUpgrade<CUpgradeHunterPlate_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);

		ItemBuff buff;

		buff.Set_AddPhysDef(upgrade->_deltaPhysDef);	
		state.AddItemBuff(buff);

		buff.Set_AddMoveSpeed(upgrade->_ims);
		state.AddItemBuff(buff);

		buff.Set_AddAttackSpeedBow(upgrade->_iasBow);
		state.AddItemBuff(buff);

		buff.Set_AddPhysDmgRate_Bow(upgrade->_deltaPhysDmgRateBow);
		state.AddItemBuff(buff);

	}

}
