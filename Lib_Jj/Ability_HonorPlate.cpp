
#include "stdh.h"

#include "Ability_HonorPlate.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeHonorPlate_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHonorPlate_Init);
BOOL CUpgradeHonorPlate_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_HonorPlate *ability=(CLevelAbility_HonorPlate *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_HonorPlate
void CLevelAbility_HonorPlate::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_HonorPlate::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_HonorPlate::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_HonorPlate::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeHonorPlate_Init *upgrade=_GetInitialUpgrade<CUpgradeHonorPlate_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);
	}

}
