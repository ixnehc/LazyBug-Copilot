
#include "stdh.h"

#include "Poem_ChartII.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradePoemChartII_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradePoemChartII_Init);
BOOL CUpgradePoemChartII_Init::Init(CLevelAbility *ability)
{
	CLevelPoem_ChartII *abilityPoem=(CLevelPoem_ChartII *)ability;


	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelPoem_ChartII

void CLevelPoem_ChartII::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelPoem_ChartII::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelPoem_ChartII::_SaveSync(CDataPacket &dp)
{

}

void CLevelPoem_ChartII::_LoadSync(CDataPacket &dp,CRecords *records)
{

}


void CLevelPoem_ChartII::_OnUpdate(LevelTick dt)
{
}
