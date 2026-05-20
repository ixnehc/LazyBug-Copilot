
#include "stdh.h"

#include "Protocal.h"

#include "Ability_ToeStone.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

#include "EoToeStoneThrusts.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeToeStone_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeToeStone_Init);
BOOL CUpgradeToeStone_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_ToeStone *ability=(CLevelAbility_ToeStone *)ability_;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_ToeStone

void CLevelAbility_ToeStone::_OnBuildRT()
{
	_BuildGradeRT();
	_durToggleOnCD=ANIMTICK_FROM_SECOND(5.0f);
	_nMaxCharge=_upgradeInitial->nMaxCharge;
	if (_nCharge==0)
		_nCharge=_nMaxCharge;
	if (_nCharge>_nMaxCharge)
		_nCharge=_nMaxCharge;
}

void CLevelAbility_ToeStone::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_ToeStone::_SaveSync(CDataPacket &dp)
{
	dp.Data_NextByte()=_bToggledOn;
	dp.Data_NextByte()=_bInToggleOnCD;
	dp.Data_WriteSimple(_durToggleOnCD);
	dp.Data_NextByte()=_nMaxCharge;
	dp.Data_NextByte()=_nCharge;
}

void CLevelAbility_ToeStone::_LoadSync(CDataPacket &dp,CRecords *records)
{
	BOOL bToggledOn=dp.Data_NextByte();
	if ((bToggledOn==TRUE)&&(_bToggledOn==FALSE))
		_bToggledOnAction=TRUE;

	_bToggledOn=bToggledOn;

	_bInToggleOnCD=dp.Data_NextByte();
	dp.Data_ReadSimple(_durToggleOnCD);

	_nMaxCharge=dp.Data_NextByte();
	_nCharge=dp.Data_NextByte();
}

BOOL CLevelAbility_ToeStone::Toggle(BOOL bOn)
{
	if (bOn==_bToggledOn)
		return TRUE;
	if (_bToggledOn)
	{
//		_bToggledOn=FALSE;
		//目前不支持ToggleOff
		return TRUE;
	}
	if (!CheckInToggleOnCD())
	{
		if (_nCharge>0)
		{
			_bToggledOn=TRUE;
			_nCharge--;
			return TRUE;
		}
	}
	return FALSE;
}


void CLevelAbility_ToeStone::_OnUpdate(LevelTick dt)
{
	_bInToggleOnCD=TRUE;
	if (_tLastThrust==ANIMTICK_INFINITE)
		_bInToggleOnCD=FALSE;
	else
	{
		if (_level->GetT_()>_tLastThrust+_durToggleOnCD)
			_bInToggleOnCD=FALSE;
	}
}

void CLevelAbility_ToeStone::_OnBuildArtifactState(LevelItemState &state)
{
}

void CLevelAbility_ToeStone::StartThrusts(CBToeStoneThrust &msg)
{
	CUpgradeToeStone_Init *upgradeInitial=(CUpgradeToeStone_Init *)_upgradeInitial;
	assert(upgradeInitial->idEo!=RecordID_Invalid);

	_bToggledOn=FALSE;
	_bInToggleOnCD=FALSE;
	_tLastThrust=_level->GetT_();

	if (msg.nSites<=0)
		return;

	LevelPos3D pos=_owner->GetFramePos3D();

	LevelRecordEo *rec=_level->GetRecords()->GetEo(upgradeInitial->idEo);
	CLoEffectObj *eo=NULL;
	if (rec)
	{
		eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
		if (eo)
		{
			i_math::vector3df dir;
			eo->PostCreate(_owner->GetPlayerID(),rec,pos,dir,1,LevelOSB(_owner),LevelOpLink());
			if (eo->GetClass()->IsSameWith(Class_Ptr2(EoToeStoneThrusts)))
			{
				EoToeStoneThrusts *eoThrusts=(EoToeStoneThrusts *)eo;
				eoThrusts->SetSites(msg.base,msg.sites,msg.nSites);
			}
			_level->AddToActives(eo);
		}
	}
	SAFE_RELEASE(eo);
}
