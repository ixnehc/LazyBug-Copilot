
#include "stdh.h"

#include "Protocal.h"

#include "Banner_Wolf.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeBannerWolf_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeBannerWolf_Init);
BOOL CUpgradeBannerWolf_Init::Init(CLevelAbility *ability_)
{
	CLevelBanner_Wolf *ability=(CLevelBanner_Wolf *)ability_;
// 	ability->_nMaxCharge=nMaxCharge;
// 	ability->_nCharge=ability->_nMaxCharge;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelBanner_Wolf

CLevelBanner_Wolf::~CLevelBanner_Wolf()
{

	GDestructor();
}


void CLevelBanner_Wolf::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelBanner_Wolf::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelBanner_Wolf::_SaveSync(CDataPacket &dp)
{
}

void CLevelBanner_Wolf::_LoadSync(CDataPacket &dp,CRecords *records)
{
}

void CLevelBanner_Wolf::_OnStartDay()
{
	CUpgradeBannerWolf_Init*upgradeInitial=(CUpgradeBannerWolf_Init *)_upgradeInitial;

}


void CLevelBanner_Wolf::_OnUpdate(LevelTick dt)
{
}

void CLevelBanner_Wolf::_OnBuildArtifactState(LevelItemState &state)
{
}

void CLevelBanner_Wolf::_OnEvent(LevelEvent &e0)
{


}

