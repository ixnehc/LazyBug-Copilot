/*!
 * \file Ability_HPFlusk.cpp
 *
 * \author cxi
 * \date 2018/04
 *
 * 
 */

#include "stdh.h"

#include "Level.h"
#include "LevelOSB.h"

#include "Ability_HPFlusk.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeHPFlusk_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeHPFlusk_Init);
BOOL CUpgradeHPFlusk_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_HPFlusk *ability=(CLevelAbility_HPFlusk *)ability_;

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//CLevelAbility_HPFlusk

void CLevelAbility_HPFlusk::_OnUpdate(LevelTick dt)
{
	int nFlusk=0;
	extern LevelItemState *LevelUtil_GetRawArtifactItemState(CLevelObj *lo,LevelArtifactType tp);
	LevelItemState *is=LevelUtil_GetRawArtifactItemState(_owner,LevelArtifact_HPFlusk);
	if (is)
		nFlusk=is->nStack;

	if (nFlusk!=_nFlusks)
	{
		if (nFlusk>_nFlusks)
			_nFilled+=nFlusk-_nFlusks;
		_nFlusks=nFlusk;
		if (_nFilled>_nFlusks)
			_nFilled=_nFlusks;
		_ownerAbilities->SetDirty();
	}
}


void CLevelAbility_HPFlusk::_SaveSync(CDataPacket &dp)
{
	dp.Data_WriteSimple(_nFlusks);
	dp.Data_WriteSimple(_nFilled);
}

void CLevelAbility_HPFlusk::_LoadSync(CDataPacket &dp,CRecords *records)
{
	dp.Data_ReadSimple(_nFlusks);
	dp.Data_ReadSimple(_nFilled);
}

void CLevelAbility_HPFlusk::Consume()
{
	CUpgradeHPFlusk_Init *upgradeInitial=(CUpgradeHPFlusk_Init *)_upgradeInitial;

	if (_nFilled<=0)
		return;

	_nFilled--;

	extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
	CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
	if (player)
	{
		LevelPlayerStates *lps=	player->GetLPS();
		if (lps)
		{
			LevelStrike strike;
			_owner->GetLevel()->GetDecider()->MakeCure(LevelOSB(_owner),_owner,1000000,strike,LevelOpLink());
		}
	}

	_ownerAbilities->SetDirty();
}

void CLevelAbility_HPFlusk::Refill()
{
	_nFilled=_nFlusks;
	_ownerAbilities->SetDirty();
}

void CLevelAbility_HPFlusk::DecFilled()
{
	if (_nFilled>0)
	{
		_nFilled--;
		_ownerAbilities->SetDirty();
	}
}

