
#include "stdh.h"

#include "Ability_WolfSkin.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"
#include "Skill_UtumSummon.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeWolfSkin_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeWolfSkin_Init);
// BOOL CUpgradeWolfSkin_Init::Init(CLevelAbility *ability_)
// {
// 	CLevelAbility_WolfSkin *ability=(CLevelAbility_WolfSkin *)ability_;
// 
// 	return TRUE;
// }


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_WolfSkin
void CLevelAbility_WolfSkin::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_WolfSkin::_OnClearRT()
{
	_ClearGradeRT();
}


void CLevelAbility_WolfSkin::_OnUpdate(LevelTick dt)
{
}


void CLevelAbility_WolfSkin::_OnBuildArtifactState(LevelItemState &state)	
{
	CUpgradeWolfSkin_Init *upgrade=_GetInitialUpgrade<CUpgradeWolfSkin_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);

		ItemBuff buff;

		buff.Set_AddMaxHP(upgrade->_deltaMaxHP);
		state.AddItemBuff(buff);

		buff.Set_AddFullSP(upgrade->_deltaFullSP);
		state.AddItemBuff(buff);

		buff.Set_AddMoveSpeed(upgrade->_ims);
		state.AddItemBuff(buff);

	}
}
