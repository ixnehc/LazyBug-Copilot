
#include "stdh.h"

#include "Protocal.h"

#include "Banner_Fire.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeBannerFire_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeBannerFire_Init);
BOOL CUpgradeBannerFire_Init::Init(CLevelAbility *ability_)
{
	CLevelBanner_Fire *ability=(CLevelBanner_Fire *)ability_;
// 	ability->_nMaxCharge=nMaxCharge;
// 	ability->_nCharge=ability->_nMaxCharge;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelBanner_Fire

CLevelBanner_Fire::~CLevelBanner_Fire()
{

	GDestructor();
}


void CLevelBanner_Fire::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelBanner_Fire::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelBanner_Fire::_SaveSync(CDataPacket &dp)
{
}

void CLevelBanner_Fire::_LoadSync(CDataPacket &dp,CRecords *records)
{
}

void CLevelBanner_Fire::_OnStartDay()
{
	CUpgradeBannerFire_Init*upgradeInitial=(CUpgradeBannerFire_Init *)_upgradeInitial;

}


void CLevelBanner_Fire::_OnUpdate(LevelTick dt)
{
}

void CLevelBanner_Fire::_OnBuildArtifactState(LevelItemState &state)
{
}

void CLevelBanner_Fire::_OnEvent(LevelEvent &e0)
{


}

