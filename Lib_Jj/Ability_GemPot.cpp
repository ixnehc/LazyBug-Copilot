/*!
 * \file Ability_GemPot.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"

#include "Ability_GemPot.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeGemPot_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeGemPot_Init);
BOOL CUpgradeGemPot_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_GemPot *ability=(CLevelAbility_GemPot *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_GemPot


void CLevelAbility_GemPot::_OnDaily()
{
	CUpgradeGemPot_Init *upgradeInitial=(CUpgradeGemPot_Init *)_upgradeInitial;

	CLevel *level=_owner->GetLevel();
	if (level)
		level->GetDecider()->MakeResModify(_owner,LevelResource_Gem,upgradeInitial->_recoverPerDay);

}

