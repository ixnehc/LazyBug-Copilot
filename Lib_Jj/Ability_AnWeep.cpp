
#include "stdh.h"

#include "Ability_AnWeep.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeAnWeep_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeAnWeep_Init);
BOOL CUpgradeAnWeep_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_AnWeep *ability=(CLevelAbility_AnWeep *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_AnWeep
void CLevelAbility_AnWeep::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_AnWeep::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_AnWeep::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_AnWeep::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeAnWeep_Init *upgrade=_GetInitialUpgrade<CUpgradeAnWeep_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);

	}

}
