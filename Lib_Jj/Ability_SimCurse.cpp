
#include "stdh.h"

#include "Ability_SimCurse.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeSimCurse_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeSimCurse_Init);
BOOL CUpgradeSimCurse_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_SimCurse *ability=(CLevelAbility_SimCurse *)ability_;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_SimCurse
void CLevelAbility_SimCurse::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_SimCurse::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_SimCurse::_OnUpdate(LevelTick dt)
{
}


void CLevelAbility_SimCurse::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeSimCurse_Init *upgrade=_GetInitialUpgrade<CUpgradeSimCurse_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);
	}
}
