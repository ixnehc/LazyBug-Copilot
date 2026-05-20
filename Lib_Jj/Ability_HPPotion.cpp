/*!
 * \file Ability_HPPotion.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"
#include "LevelOSB.h"

#include "Ability_HPPotion.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeHPPotion_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHPPotion_Init);
BOOL CUpgradeHPPotion_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_HPPotion *ability=(CLevelAbility_HPPotion *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_HPPotion

void CLevelAbility_HPPotion::_SaveSync(CDataPacket &dp)
{
	dp.Data_WriteSimple(_bConsumed);
}

void CLevelAbility_HPPotion::_LoadSync(CDataPacket &dp,CRecords *records)
{
	dp.Data_ReadSimple(_bConsumed);
}

void CLevelAbility_HPPotion::Consume()
{
	CUpgradeHPPotion_Init *upgradeInitial=(CUpgradeHPPotion_Init *)_upgradeInitial;

	if (_bConsumed)
		return;

	_bConsumed=TRUE;

	extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
	CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
	if (player)
	{
		LevelPlayerStates *lps=	player->GetLPS();
		if (lps)
		{
			int nDelta=FloatToNearestInt(((float)lps->base.MaxHP)/100.0f*(float)upgradeInitial->_percentAdd);
			_owner->GetLevel()->GetDecider()->MakeCure_MaxHP(LevelOSB(_owner),_owner,nDelta,LevelOpLink());
		}
	}
	_ownerAbilities->SetDirty();

}
