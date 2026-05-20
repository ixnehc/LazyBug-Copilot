
#include "stdh.h"

#include "Poem_ChartI.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradePoemChartI_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradePoemChartI_Init);
BOOL CUpgradePoemChartI_Init::Init(CLevelAbility *ability)
{
	CLevelPoem_ChartI *abilityPoem=(CLevelPoem_ChartI *)ability;


	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelPoem_ChartI

void CLevelPoem_ChartI::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelPoem_ChartI::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelPoem_ChartI::_SaveSync(CDataPacket &dp)
{

}

void CLevelPoem_ChartI::_LoadSync(CDataPacket &dp,CRecords *records)
{

}


void CLevelPoem_ChartI::_OnUpdate(LevelTick dt)
{
}
