
#include "stdh.h"

#include "Poem_ChartIII.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradePoemChartIII_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradePoemChartIII_Init);
BOOL CUpgradePoemChartIII_Init::Init(CLevelAbility *ability)
{
	CLevelPoem_ChartIII *abilityPoem=(CLevelPoem_ChartIII *)ability;


	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelPoem_ChartIII

void CLevelPoem_ChartIII::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelPoem_ChartIII::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelPoem_ChartIII::_SaveSync(CDataPacket &dp)
{

}

void CLevelPoem_ChartIII::_LoadSync(CDataPacket &dp,CRecords *records)
{

}


void CLevelPoem_ChartIII::_OnUpdate(LevelTick dt)
{
}
