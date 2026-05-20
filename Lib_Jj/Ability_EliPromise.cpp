
#include "stdh.h"

#include "Ability_EliPromise.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

#include "LevelOSB.h"
#include "LevelEvents.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeEliPromise_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeEliPromise_Init);
BOOL CUpgradeEliPromise_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_EliPromise *ability=(CLevelAbility_EliPromise *)ability_;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_EliPromise
void CLevelAbility_EliPromise::_OnBuildRT()
{
	_BuildGradeRT();

	CUpgradeEliPromise_Init *upgrade=_GetInitialUpgrade<CUpgradeEliPromise_Init>();
	if (upgrade)
	{
		_dealCureRT=upgrade->_dealCure->Clone();
		_dealShockwaveRT=upgrade->_dealShockwave->Clone();
	}

}

void CLevelAbility_EliPromise::_OnClearRT()
{
	_ClearGradeRT();
	
	Safe_Class_Delete(_dealCureRT);
	Safe_Class_Delete(_dealShockwaveRT);

}

void CLevelAbility_EliPromise::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_EliPromise::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PreKill)
	{
		LePreKill *e=(LePreKill *)&e0;

		DealArg arg;
		arg.link=e->link;
		if (_dealCureRT)
			_dealCureRT->Make(LevelOSB(_owner),_owner,arg,NULL);

		if (_dealShockwaveRT)
			_dealShockwaveRT->Make(LevelOSB(_owner),_owner,arg,NULL);

		e->bAbandon=TRUE;
	}
}


void CLevelAbility_EliPromise::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeEliPromise_Init *upgrade=_GetInitialUpgrade<CUpgradeEliPromise_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);

	}

}
