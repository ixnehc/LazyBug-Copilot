/*!
 * \file Ability_SPAmulet.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"

#include "LevelOSB.h"

#include "Ability_SPAmulet.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeSPAmulet_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeSPAmulet_Init);
BOOL CUpgradeSPAmulet_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_SPAmulet *ability=(CLevelAbility_SPAmulet *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_SPAmulet


void CLevelAbility_SPAmulet::_OnDaily()
{
	CUpgradeSPAmulet_Init *upgradeInitial=(CUpgradeSPAmulet_Init *)_upgradeInitial;

	CLevel *level=_owner->GetLevel();
	if (level)
		level->GetDecider()->MakeCure_FullSP(LevelOSB(_owner),_owner,upgradeInitial->_recoverPerDay,LevelOpLink());

}

