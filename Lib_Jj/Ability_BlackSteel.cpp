
#include "stdh.h"

#include "Ability_BlackSteel.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeBlackSteel_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeBlackSteel_Init);
BOOL CUpgradeBlackSteel_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_BlackSteel *ability=(CLevelAbility_BlackSteel *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_BlackSteel
void CLevelAbility_BlackSteel::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_BlackSteel::_OnClearRT()
{
	_ClearGradeRT();
}
void CLevelAbility_BlackSteel::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_BlackSteel::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeBlackSteel_Init *upgrade=_GetInitialUpgrade<CUpgradeBlackSteel_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);

	}

}
