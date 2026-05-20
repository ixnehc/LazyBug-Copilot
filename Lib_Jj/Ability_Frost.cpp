
#include "stdh.h"

#include "Ability_Frost.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

#include "LevelEvents.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeFrost_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeFrost_Init);
BOOL CUpgradeFrost_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_Frost *ability=(CLevelAbility_Frost *)ability_;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_Frost
void CLevelAbility_Frost::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_Frost::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_Frost::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_Frost::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PostDamage)
	{
		LePostDamage*e=(LePostDamage *)&e0;

		DealArg arg;
		arg.link=e->link;
// 		if (_dealFrostExplode)
// 			_dealFrostExplode->Make(LevelOSB(_owner),_owner,arg);
	}
}


void CLevelAbility_Frost::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeFrost_Init*upgrade=_GetInitialUpgrade<CUpgradeFrost_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);
	}

}
