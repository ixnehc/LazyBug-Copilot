
#include "stdh.h"

#include "Ability_UtumTide.h"
#include "LevelAttrs.h"
#include "Level.h"
#include "LevelRecords.h"
#include "LevelUtil.h"

#include "LevelRecordEO.h"
#include "EoUtumAttack.h"
#include "EoUtumRepairBridge.h"

//////////////////////////////////////////////////////////////////////////
//CUpgradeUtumTide_Init
IMPLEMENT_ABILITY_UPGRADE_CLASS(CUpgradeUtumTide_Init);
BOOL CUpgradeUtumTide_Init::Init(CLevelAbility *ability)
{
	CLevelAbility_UtumTide *abilityFire=(CLevelAbility_UtumTide *)ability;


	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAbility_UtumTide
void CLevelAbility_UtumTide::_SetMode(WorkingMode mode)
{
	if (_mode!=mode)
	{
		_mode=mode;
		_tModeStart=_owner->GetT();
	}
}

void CLevelAbility_UtumTide::_OnBuildRT()
{
	_BuildGradeRT();

	_SetMode(Mode_Default);

	_UpdateAvailable();



}

void CLevelAbility_UtumTide::_OnClearRT()
{
	_ClearGradeRT();
}

void CLevelAbility_UtumTide::_SaveSync(CDataPacket &dp)
{
	dp.Data_NextByte()=(BYTE)_mode;

	if (_mode==Mode_Default)
	{
		dp.Data_NextByte()=(BYTE)_bToggledOn;
	}
	if (_mode==Mode_RepairBridge)
	{
		dp.Data_WriteSimple(_idBridge);
	}

	dp.Data_NextWord()=(WORD)_nAvailable;
}

void CLevelAbility_UtumTide::_LoadSync(CDataPacket &dp,CRecords *records)
{
	_mode=(WorkingMode)dp.Data_NextByte();
	if (_mode==Mode_Default)
	{
		_bToggledOn=dp.Data_NextByte();
	}
	if (_mode==Mode_RepairBridge)
	{
		dp.Data_ReadSimple(_idBridge);
	}
	_nAvailable=dp.Data_NextWord();
}

BOOL CLevelAbility_UtumTide::Toggle(BOOL bOn)
{
	if (_bToggledOn==bOn)
		return TRUE;

	_bToggledOn=bOn;
	if (_bToggledOn)
	{
		_tToggledOn=_owner->GetT();
		_nSummonedAttack=0;
	}

	return TRUE;
}


void CLevelAbility_UtumTide::_UpdateAttack()
{
	CUpgradeUtumTide_Init*upgradeInitial=(CUpgradeUtumTide_Init*)_upgradeInitial;
	LevelPos3D pos=_owner->GetFramePos3D();
	LevelRecordEo *rec=_level->GetRecords()->GetEo(upgradeInitial->idAttackEo);
	AnimTick tCur=_owner->GetT();

	const float speedSummon=3.0f;
	int nToSummon=0;
	if (TRUE)
	{
		AnimTick dur=ANIMTICK_SAFE_MINUS(tCur,_tToggledOn);
		nToSummon=1+(int)(ANIMTICK_TO_SECOND(dur)*speedSummon);
	}

	while (nToSummon>_nSummonedAttack)
	{
		if (_mode==Mode_Default)
		{
			if (_nAvailable>0)
			{
				CLoEffectObj *eo=NULL;
				if (rec)
				{
					eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
					if (eo)
					{
						i_math::vector3df dir;
						eo->PostCreate(_owner->GetPlayerID(),rec,pos,dir,1,LevelOSB(_owner),LevelOpLink());
						_level->AddToActives(eo);
						_flyings[eo->GetID()]=FALSE;
						_UpdateAvailable();
					}
				}
				SAFE_RELEASE(eo);
			}
		}

		_nSummonedAttack++;
	}
}

void CLevelAbility_UtumTide::_UpdateRepairBridge()
{
	CUpgradeUtumTide_Init*upgradeInitial=(CUpgradeUtumTide_Init*)_upgradeInitial;
	LevelPos3D pos=_owner->GetFramePos3D();
	LevelRecordEo *rec=_level->GetRecords()->GetEo(upgradeInitial->idRepairBridgeEo);
	AnimTick tCur=_owner->GetT();

	const float speedSummon=20.0f;
	int nToSummon=0;
	if (TRUE)
	{
		AnimTick dur=ANIMTICK_SAFE_MINUS(tCur,_tModeStart);
		nToSummon=1+(int)(ANIMTICK_TO_SECOND(dur)*speedSummon);
	}

	while ((nToSummon>_nSummonedRepair)&&(_nSummonedRepair<_nRequiredLabor))
	{
		if (_nAvailable>0)
		{
			CLoEffectObj *eo=NULL;
			if (rec)
			{
				eo=(CLoEffectObj*)_level->CreateObj(rec->param->GetEoClass());
				if (eo)
				{
					if (eo->GetClass()->IsSameWith(Class_Ptr2(EoUtumRepairBridge)))
					{
						((EoUtumRepairBridge*)eo)->SetInfo(_idBridge,_nRequiredLabor-1-_nSummonedRepair);

						i_math::vector3df dir;
						eo->PostCreate(_owner->GetPlayerID(),rec,pos,dir,1,LevelOSB(_owner),LevelOpLink());
						_level->AddToActives(eo);
						_flyings[eo->GetID()]=FALSE;
						_UpdateAvailable();
					}
				}
			}
			SAFE_RELEASE(eo);
		}

		_nSummonedRepair++;
	}
}



void CLevelAbility_UtumTide::_OnUpdate(LevelTick dt)
{
	CUpgradeUtumTide_Init*upgradeInitial=(CUpgradeUtumTide_Init*)_upgradeInitial;
	assert(upgradeInitial->idAttackEo!=RecordID_Invalid);

	_UpdateExausted();
	_UpdateAvailable();

	if (_owner) 
	{
		AnimTick tCur=_owner->GetT();

		DWORD nUtum=0;
		LevelAttr_Resource *attrRes=_owner->GetAttr_Resource();
		if (attrRes)
		{
			Lav *lav=attrRes->Get(LevelResource_Labor);
			if (lav)
				nUtum=(int)lav->GetCur_Int();
		}

		if (_bToggledOn)
			_UpdateAttack();
		if (_mode==Mode_RepairBridge)
			_UpdateRepairBridge();
	}
}


void CLevelAbility_UtumTide::_UpdateExausted()
{
	DWORD nExaustedOld=_nExausted;
	if (TRUE)
	{
		std::unordered_map<LevelObjID,BOOL>::iterator it=_flyings.begin();
		while(it!=_flyings.end())
		{
			std::unordered_map<LevelObjID,BOOL>::iterator itCur=it;
			it++;

			CLevelObj *lo=LevelUtil_GetAliveLo(_level,(*itCur).first);
			if (lo)
			{
				if ((*itCur).second)
					continue;//已经exausted了

				if (lo->GetClass()->IsSameWith(Class_Ptr2(EoUtumAttack)))
				{
					if (((EoUtumAttack*)lo)->HasDamaged())
					{
						_nExausted++;
						(*itCur).second=TRUE;
					}
				}
				if (lo->GetClass()->IsSameWith(Class_Ptr2(EoUtumRepairBridge)))
				{
					if (((EoUtumRepairBridge*)lo)->IsRepaired())
					{
						_nExausted++;
						(*itCur).second=TRUE;
					}
				}

			}
			else
				_flyings.erase(itCur);
		}
	}
	if (_nExausted!=nExaustedOld)
		_ownerAbilities->SetDirty();
}


void CLevelAbility_UtumTide::_UpdateAvailable()
{
	if (_owner) 
	{
		AnimTick tCur=_owner->GetT();

		int nUtum=0;
		LevelAttr_Resource *attrRes=_owner->GetAttr_Resource();
		if (attrRes)
		{
			Lav *lav=attrRes->Get(LevelResource_Labor);
			if (lav)
				nUtum=(int)lav->GetCur_Int();
		}

		int nFlying=0;//在飞行但是还未exausted的utum的个数
		std::unordered_map<LevelObjID,BOOL>::iterator it;
		for (it=_flyings.begin();it!=_flyings.end();it++)
		{
			if ((*it).second)
				continue;//已经exausted了
			nFlying++;
		}

		_nAvailable=nUtum-_nExausted-nFlying;
		if (_nAvailable<0)
			_nAvailable=0;
	}

}

void CLevelAbility_UtumTide::NotifyUtumReturn(CBUtumReturn&msg)
{
	_UpdateExausted();

	std::unordered_map<LevelObjID,BOOL>::iterator it=_flyings.find(msg.idEo);
	if (it!=_flyings.end())
		_flyings.erase(it);

	_UpdateAvailable();
}

void CLevelAbility_UtumTide::StartRepairBridge(LevelObjID idBridge,DWORD nRequiredLabor)
{
	if (_mode!=Mode_RepairBridge)
	{
		_SetMode(Mode_RepairBridge);
		_idBridge=idBridge;
		_nRequiredLabor=nRequiredLabor;
		_nSummonedRepair=0;
	}
}

void CLevelAbility_UtumTide::EndRepairBridge(LevelObjID idBridge,BOOL bAbort)
{
	if (_mode==Mode_RepairBridge)
	{
		if (_idBridge==idBridge)
			_SetMode(Mode_Default);
	}
}

void CLevelAbility_UtumTide::_OnStartDay()
{
	_nExausted=0;
	_UpdateExausted();
	_UpdateAvailable();

}
