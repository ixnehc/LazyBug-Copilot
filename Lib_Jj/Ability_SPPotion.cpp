/*!
 * \file Ability_SPPotion.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"
#include "LevelOSB.h"

#include "Ability_SPPotion.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeSPPotion_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeSPPotion_Init);
BOOL CUpgradeSPPotion_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_SPPotion *ability=(CLevelAbility_SPPotion *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_SPPotion

void CLevelAbility_SPPotion::_SaveSync(CDataPacket &dp)
{
	dp.Data_WriteSimple(_bConsumed);
}

void CLevelAbility_SPPotion::_LoadSync(CDataPacket &dp,CRecords *records)
{
	dp.Data_ReadSimple(_bConsumed);
}

void CLevelAbility_SPPotion::Consume()
{
	CUpgradeSPPotion_Init *upgradeInitial=(CUpgradeSPPotion_Init *)_upgradeInitial;

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
			int nDelta=FloatToNearestInt(((float)lps->base.FullSP)/100.0f*(float)upgradeInitial->_percentAdd);
			_owner->GetLevel()->GetDecider()->MakeCure_FullSP(LevelOSB(_owner),_owner,nDelta,LevelOpLink());
		}
	}
	_ownerAbilities->SetDirty();

}
