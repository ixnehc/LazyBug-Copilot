
#include "stdh.h"

#include "Ability_WeaponInductionStone.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeWeaponInductionStone_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeWeaponInductionStone_Init);
BOOL CUpgradeWeaponInductionStone_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_WeaponInductionStone *ability=(CLevelAbility_WeaponInductionStone *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_WeaponInductionStone

class CLevelPlayer;
class CLevelObj;
void CLevelAbility_WeaponInductionStone::_OnBuildRT()
{
	extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
	extern BOOL LevelUtil_ExistArtifact(CLevelPlayer *player,LevelArtifactType tp);

	_grdRT=0;
	CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
	if (player)
	{
		if (LevelUtil_ExistArtifact(player,LevelArtifact_WeaponInductionStone_A))
			_grdRT++;
		if (LevelUtil_ExistArtifact(player,LevelArtifact_WeaponInductionStone_B))
			_grdRT++;
		if (LevelUtil_ExistArtifact(player,LevelArtifact_WeaponInductionStone_C))
			_grdRT++;
	}
}

void CLevelAbility_WeaponInductionStone::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_WeaponInductionStone::_OnUpdate(LevelTick dt)
{
}


void CLevelAbility_WeaponInductionStone::_OnEvent(LevelEvent &e0)
{
}

void CLevelAbility_WeaponInductionStone::_OnBuildArtifactState(LevelItemState &state)
{
}
