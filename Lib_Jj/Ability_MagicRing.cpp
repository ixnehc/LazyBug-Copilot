/*!
 * \file Ability_MagicRing.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Ability_MagicRing.h"
#include "LevelAttrs.h"
#include "LevelItemState.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeMagicRing_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeMagicRing_Init);
BOOL CUpgradeMagicRing_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_MagicRing *ability=(CLevelAbility_MagicRing *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_MagicRing

void CLevelAbility_MagicRing::_SaveSync(CDataPacket &dp)
{
	dp.Data_WriteSimple(_nMP);
	dp.Data_WriteSimple(_grade);
}
void CLevelAbility_MagicRing::_LoadSync(CDataPacket &dp,CRecords *records)
{
	dp.Data_ReadSimple(_nMP);
	dp.Data_ReadSimple(_grade);
}

void CLevelAbility_MagicRing::_OnStartDay()
{
//	_nMP=MAX_MAGICRING_MP;
}



class CLevelPlayer;
class CLevelObj;
void CLevelAbility_MagicRing::_OnBuildRT()
{
}

void CLevelAbility_MagicRing::_OnClearRT()
{
	_ClearGradeRT();
}

DWORD CLevelAbility_MagicRing::MakeCost(DWORD cost)
{
	if (cost>_nMP)
	{
		cost=_nMP;
		_nMP=0;
		return cost;
	}
	else
	{
		_nMP-=(short)cost;
		return cost;
	}

}


void CLevelAbility_MagicRing::_OnUpdate(LevelTick dt)
{
	CUpgradeMagicRing_Init *upgradeInitial=(CUpgradeMagicRing_Init *)_upgradeInitial;
// 	if (!_bCrystalHeart)
// 	{
// 		extern BOOL LevelUtil_ExistArtifact(CLevelObj *lo,LevelArtifactType tp);
// 		if (LevelUtil_ExistArtifact(_owner,LevelArtifact_CrystalHeart))
// 		{
// 			_nOrbs+=upgradeInitial->_nCrystalHeartCapacity;
// 			_bCrystalHeart=TRUE;
// 			_ownerAbilities->SetDirty();
// 		}
// 	}

}


void CLevelAbility_MagicRing::_OnEvent(LevelEvent &e0)
{
}

void CLevelAbility_MagicRing::_OnBuildArtifactState(LevelItemState &state)
{
}
