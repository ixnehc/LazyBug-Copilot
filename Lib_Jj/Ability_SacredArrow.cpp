
#include "stdh.h"

#include "Protocal.h"

#include "Ability_SacredArrow.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"


//////////////////////////////////////////////////////////////////////////
//CUpgradeSacredArrow_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeSacredArrow_Init);
BOOL CUpgradeSacredArrow_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_SacredArrow *ability=(CLevelAbility_SacredArrow *)ability_;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_SacredArrow

void CLevelAbility_SacredArrow::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_SacredArrow::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_SacredArrow::_SaveSync(CDataPacket &dp)
{
	dp.Data_NextByte()=_bToggledOn;
}

void CLevelAbility_SacredArrow::_LoadSync(CDataPacket &dp,CRecords *records)
{
	BOOL bToggledOn=dp.Data_NextByte();
	_bToggledOn=bToggledOn;
}

BOOL CLevelAbility_SacredArrow::Toggle(BOOL bOn)
{
	if (bOn==_bToggledOn)
		return TRUE;
	if (_bToggledOn)
	{
		_bToggledOn=FALSE;
		return TRUE;
	}

	extern LevelItemState *LevelUtil_GetRawArtifactItemState(CLevelObj *lo,LevelArtifactType tp);
	LevelItemState * is=LevelUtil_GetRawArtifactItemState(_owner,LevelArtifact_SacredArrow);
	if (is)
	{
		if (is->nStack>0)
		{
			_bToggledOn=TRUE;
			return TRUE;
		}
	}

	return FALSE;
}


void CLevelAbility_SacredArrow::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_SacredArrow::_OnBuildArtifactState(LevelItemState &state)
{
}

