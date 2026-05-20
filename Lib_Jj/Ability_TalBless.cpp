
#include "stdh.h"

#include "Ability_TalBless.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

#include "LevelPlayer.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeTalBless_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeTalBless_Init);
BOOL CUpgradeTalBless_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_TalBless *ability=(CLevelAbility_TalBless *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_TalBless
void CLevelAbility_TalBless::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_TalBless::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_TalBless::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_TalBless::_OnEndDay()
{
	CUpgradeTalBless_Init *upgrade=_GetInitialUpgrade<CUpgradeTalBless_Init>();
	if (upgrade)
	{
		extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
		CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
		if (player)
		{
			LevelPlayerStates *lps=player->GetLPS();
			if (lps)
			{
				lps->base.MaxHP+=upgrade->_deltaMaxHPPerDay;
				lps->base.SetDirtyDB_Urgent();
			}

		}
	}
}

void CLevelAbility_TalBless::_OnBuildArtifactState(LevelItemState &state)
{
	CUpgradeTalBless_Init *upgrade=_GetInitialUpgrade<CUpgradeTalBless_Init>();
	if (upgrade)
	{
		_ApplyDefendMods(upgrade,state,_grdRT);
	}
}
