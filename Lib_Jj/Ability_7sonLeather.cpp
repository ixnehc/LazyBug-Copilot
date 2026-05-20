
#include "stdh.h"

#include "Ability_7sonLeather.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

//////////////////////////////////////////////////////////////////////////
//CUpgrade7sonLeather_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgrade7sonLeather_Init);
BOOL CUpgrade7sonLeather_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_7sonLeather *ability=(CLevelAbility_7sonLeather *)ability_;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_7sonLeather

void CLevelAbility_7sonLeather::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_7sonLeather::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_7sonLeather::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_7sonLeather::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgrade7sonLeather_Init *upgrade=_GetInitialUpgrade<CUpgrade7sonLeather_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);

	}

}
