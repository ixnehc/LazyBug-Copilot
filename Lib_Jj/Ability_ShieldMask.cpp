
#include "stdh.h"

#include "Protocal.h"

#include "Ability_ShieldMask.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelRecordEO.h"

#include "LevelEvents.h"
#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeShieldMask_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeShieldMask_Init);
BOOL CUpgradeShieldMask_Init::Init(CLevelAbility *ability_)
{
	CLevelAbility_ShieldMask *ability=(CLevelAbility_ShieldMask *)ability_;
	ability->_nMaxCharge=nMaxCharge;
	ability->_nCharge=ability->_nMaxCharge;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//CLevelAbility_ShieldMask

CLevelAbility_ShieldMask::~CLevelAbility_ShieldMask()
{
	_abortsDmg.clear();

	GDestructor();
}


void CLevelAbility_ShieldMask::_OnBuildRT()
{
	_BuildGradeRT();
}

void CLevelAbility_ShieldMask::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_ShieldMask::_SaveSync(CDataPacket &dp)
{
	dp.Data_NextByte()=_nMaxCharge;
	dp.Data_NextByte()=_nCharge;
}

void CLevelAbility_ShieldMask::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_nMaxCharge=dp.Data_NextByte();
	_nCharge=dp.Data_NextByte();
}

void CLevelAbility_ShieldMask::_OnStartDay()
{
	CUpgradeShieldMask_Init*upgradeInitial=(CUpgradeShieldMask_Init *)_upgradeInitial;

	_nCharge=_nMaxCharge=upgradeInitial->nMaxCharge;
	_ownerAbilities->SetDirty();
}


void CLevelAbility_ShieldMask::_OnUpdate(LevelTick dt)
{
}

void CLevelAbility_ShieldMask::_OnBuildArtifactState(LevelItemState &state)
{
}

void CLevelAbility_ShieldMask::_OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PreDamage)
	{
		LePreDamage &e=(LePreDamage&)e0;	
		if (e.loTarget==_owner)
		{
			if (_nCharge>0)
			{
				LevelOp_DmgAbort *op=e.osb->NewOp<LevelOp_DmgAbort>(e.link);
				if (e.strike)
					op->v.strike=*e.strike;
				op->v.tp=LevelDmgAbort::ShieldMask;
				_owner->AddOp(op);

				e.scaleDmg=0.0f;
				_nCharge--;
				_ownerAbilities->SetDirty();
			}
		}
	}


}


void CLevelAbility_ShieldMask::DepositDmgAbort(LevelDmgAbort &abort)
{
	_abortsDmg.push_back(abort);
}

BOOL CLevelAbility_ShieldMask::FetchDmgAbort(LevelDmgAbort &abort)
{
	if (_abortsDmg.size()>0)
	{
		abort=_abortsDmg[0];
		_abortsDmg.pop_front();
		return TRUE;
	}
	return FALSE;
}
