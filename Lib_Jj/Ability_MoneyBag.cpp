/*!
 * \file Ability_MoneyBag.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"

#include "Ability_MoneyBag.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeMoneyBag_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeMoneyBag_Init);
BOOL CUpgradeMoneyBag_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_MoneyBag *ability=(CLevelAbility_MoneyBag *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_MoneyBag


void CLevelAbility_MoneyBag::_OnDaily()
{
	CUpgradeMoneyBag_Init *upgradeInitial=(CUpgradeMoneyBag_Init *)_upgradeInitial;

	CLevel *level=_owner->GetLevel();
	if (level)
		level->GetDecider()->MakeResModify(_owner,LevelResource_Gold,upgradeInitial->_recoverPerDay);

}

