/*!
 * \file Ability_HPAmulet.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"
#include "LevelOSB.h"

#include "Ability_HPAmulet.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeHPAmulet_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHPAmulet_Init);
BOOL CUpgradeHPAmulet_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_HPAmulet *ability=(CLevelAbility_HPAmulet *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_HPAmulet


void CLevelAbility_HPAmulet::_OnDaily()
{
	CUpgradeHPAmulet_Init *upgradeInitial=(CUpgradeHPAmulet_Init *)_upgradeInitial;

	CLevel *level=_owner->GetLevel();
	if (level)
		level->GetDecider()->MakeCure_MaxHP(LevelOSB(_owner),_owner,upgradeInitial->_recoverPerDay,LevelOpLink());

}

