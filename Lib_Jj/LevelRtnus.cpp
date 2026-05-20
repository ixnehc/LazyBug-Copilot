
#include "stdh.h"

#include "LevelRtnus.h"

#include "LoUnit.h"
#include "LevelPlayer.h"
#include "Level.h"

#include "LevelRecordUnit.h"
#include "LevelBehavior.h"

#include "LevelRtnuCircum.h"

#define CURVER 1


BOOL CLevelRtnu::CreateNew(CLoUnit *lo,BOOL bPersist,CLevelPlayer *player)
{
	_loUnit=lo;
	SAFE_ADDREF(_loUnit);
	_loUnit->SetRtnu(this);
	_loUnit->SetPlayerID(player->GetPlayerID());

	//更新unit的collide,如果需要的话
	if (TRUE)
	{
		CLevelObjMove *move=_loUnit->GetMove();
		if (move)
		{
			CUnit *unit=move->GetGroundUnit();
			if (unit)
			{
				extern void UnitCollide_SetAlly(CUnit *unit,DWORD ally);
				UnitCollide_SetAlly(unit,player->GetPlayerID());
			}
		}
	}
	_idPlayer=player->GetPlayerID();

//	_loUnit->ResetAI();

	if (bPersist)
	{
		LevelPlayerStates *lps=player->GetLevel()->GetLPS(player->GetPlayerID());
		if (lps)
		{
			extern LPSRetinueData *LPS_NewRetinue(LevelPlayerStates *lps,RetinueType tp,RecordID idRec,LevelGrade grd,EquipSetPick iPickedEquipSet,LevelMoveMethod method);
			extern LevelGrade LevelUtil_GetGrade(CLevelObj *lo);
			LPSRetinueData *data=LPS_NewRetinue(lps,Retinue_Unit,_loUnit->GetRec()->GetID(),LevelUtil_GetGrade(_loUnit),_loUnit->GetPickedEquipSet(),_loUnit->GetMoveMethod());
			if (data)
				_uid=data->uid;
		}
	}
	return TRUE;
}

void CLevelRtnu::_CreateFromData(LPSRetinueData *data,CLoUnit *lo,CLevelPlayer *player)
{
	_idPlayer=player->GetPlayerID();
	_loUnit=lo;
	if (_loUnit)
		_loUnit->SetRtnu(this);

	_uid=data->uid;

}

BOOL CLevelRtnu::CreateFromData(LPSRetinueData *data,CLevelPlayer *player,LevelPos &pos)
{
	//创建lo
	assert(data->tp==Retinue_Unit);
	CLevel *level=player->GetLevel();
	CLoUnit *lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
	if (data->method!=LevelMoveMethod_Flying)
		lo->PostCreate(player->GetPlayerID(),NULL,data->idRec,data->grd_,NULL,data->iPickedEquipSet,pos);
	else
	{
		extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
		LevelPos3D pos3D=LevelUtil_GetGroundHeight(level,pos.x,pos.y,FALSE);
		lo->PostCreate(player->GetPlayerID(),NULL,data->idRec,data->grd_,NULL,data->iPickedEquipSet,pos3D);
	}
	level->AddToActives(lo);

	_CreateFromData(data,lo,player);

	return TRUE;
}

BOOL CLevelRtnu::CreateTeleport(CLevelRtnu *rtnuOrg,CLevelPlayer *player,LevelPos &pos)
{
	_idPlayer=player->GetPlayerID();
	CLevel *level=player->GetLevel();
	_loUnit=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
	_loUnit->PostCreate_Teleport(rtnuOrg->_loUnit,pos);
	level->AddToActives(_loUnit);

	_loUnit->SetRtnu(this);

	_uid=rtnuOrg->_uid;


	return TRUE;
}

CLoUnit *CLevelRtnu::Dismiss()
{
	CLoUnit *lo=_loUnit;
	SAFE_ADDREF(lo);

	if (_loUnit)
		_loUnit->SetRtnu(NULL);
	SAFE_RELEASE(_loUnit);

	Zero();

	if (lo)
	{
		if (lo->IsAlive())
		{
			lo->ResetAI();
			lo->SetPlayerID(LevelPlayerID_Wild);//设一个非Player的ID
		}
	}

	return lo;
}

void CLevelRtnu::Discard()
{
	if (_loUnit)
		_loUnit->SetRtnu(NULL);
	SAFE_RELEASE(_loUnit);

	Zero();
}

void CLevelRtnu::Destroy()
{
	CLoUnit *lo=Dismiss();
	if (lo)
		lo->DeferDestroy();
	SAFE_RELEASE(lo);
}

void CLevelRtnu::Update()
{
}

LevelRtnuRank CLevelRtnu::GetRank()
{
	if (_loUnit)
		return _loUnit->GetRtnuRank();
	return LevelRtnuRank_None;
}


//////////////////////////////////////////////////////////////////////////
//CLevelRtnus

void CLevelRtnus::Init(CLevelPlayer *player)
{
	_owner=player;
}

void CLevelRtnus::Clear()
{
	_UpdateRetinuesGC();
	for (int i=0;i<_rtnus.size();i++)
	{
		_rtnus[i]->Destroy();
		Safe_Class_Delete(_rtnus[i]);
	}
	_rtnus.clear();

	Zero();
}



BOOL CLevelRtnus::Add_New(CLoUnit *lo,BOOL bPersist)
{
	if (lo->IsRetinue())
		return FALSE;//已经是某个Player的随从了
	CLevelRtnu *rtnu=Class_New2(CLevelRtnu);
	rtnu->CreateNew(lo,bPersist,_owner);
	_rtnus.push_back(rtnu);

	return TRUE;
}

BOOL CLevelRtnus::Add_FromData(LPSRetinueData *data,LevelPos &pos)
{
	CLevelRtnu *rtnu=Class_New2(CLevelRtnu);
	rtnu->CreateFromData(data,_owner,pos);
	_rtnus.push_back(rtnu);

	return TRUE;
}

BOOL CLevelRtnus::Add_Teleport(CLevelRtnu *rtnuOrg,LevelPos &pos)
{
	CLevelRtnu *rtnu=Class_New2(CLevelRtnu);
	rtnu->CreateTeleport(rtnuOrg,_owner,pos);
	_rtnus.push_back(rtnu);

	return TRUE;
}

void CLevelRtnus::Remove(CLevelObj *lo)
{
	if (!lo->IsRetinue())
		return;
	if (lo->GetPlayerID()!=_owner->GetPlayerID())
		return;
	for (int i=0;i<_rtnus.size();i++)
	{
		if (_rtnus[i]->GetLo()==lo)
		{
			CLevelObj *loRtnu=_rtnus[i]->Dismiss();
			SAFE_RELEASE(loRtnu);
			Safe_Class_Delete(_rtnus[i]);
			Swap(_rtnus[i],_rtnus[_rtnus.size()-1]);//交换到末尾
			_rtnus.pop_back();
			return;
		}
	}
}

void CLevelRtnus::RemoveAll()
{
	for (int i=0;i<_rtnus.size();i++)
	{
		CLevelObj *loRtnu=_rtnus[i]->Dismiss();
		SAFE_RELEASE(loRtnu);
		Safe_Class_Delete(_rtnus[i]);
	}
	_rtnus.clear();
}


void CLevelRtnus::_UpdateRetinuesGC()
{
	DWORD c=0;
	for (int i=0;i<_rtnus.size();i++)
	{
		CLevelObj *lo=_rtnus[i]->GetLo();
		BOOL bDiscard=FALSE;
		if (!lo->IsAlive())
			bDiscard=TRUE;
		else
		{
			extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
			if (LevelUtil_CheckDead(lo))
				bDiscard=TRUE;
			else
			{
				if (lo->GetPlayerID()!=_owner->GetPlayerID())
				{
					assert(FALSE);
					bDiscard=TRUE;
				}
			}
		}

		if (bDiscard)
		{
			RetinueUID uid=_rtnus[i]->GetUID();
			if (uid!=RetinueUID_Invalid)
				LPS_EraseRetinue(_owner->GetLPS(),uid);
			_rtnus[i]->Discard();
			_rtnus[i]->Destroy();
			Safe_Class_Delete(_rtnus[i]);

			continue;
		}
		_rtnus[c]=_rtnus[i];
		c++;
	}
	_rtnus.resize(c);
}

CLevelRtnu**CLevelRtnus::GetRetinues(DWORD &c)
{
	c=_rtnus.size();
	return _rtnus.data();
}

CLevelRtnu**CLevelRtnus::GetValidRetinues(DWORD &c)
{
	_UpdateRetinuesGC();
	return GetRetinues(c);
}

DWORD CLevelRtnus::GetRetinueCount(RecordID idUnit)
{
	_UpdateRetinuesGC();

	DWORD c=0;
	for (int i=0;i<_rtnus.size();i++)
	{
		CLoUnit *lo=_rtnus[i]->GetLo();
		if (lo->GetRecID()==idUnit)
			c++;
	}
	return c;
}


void CLevelRtnus::Update()
{
	const float distPursueOutter=20.0f;
	const float distPursueInner=10.0f;
	const float distCloseThreat=8.0f;
	const float distMeleeThreat=0.5f;

	CLoUnit *loOwner=_owner->GetLoUnit();
	if (loOwner)
	{
		BOOL bOwnerReaching=FALSE;
		if (TRUE)
		{
			LevelMoveStep step;
			_owner->GetMove().GetRecentMoveStep(loOwner->GetT(),LEVEL_FRAME_TICK,step);
			if (step.bReaching)
				bOwnerReaching=TRUE;
		}

		LevelPos posOwner=loOwner->GetFramePos();
		for (int i=0;i<_rtnus.size();i++)
		{
			CLevelRtnu *rtnu=_rtnus[i];
			CLevelObj *loRtnu=rtnu->GetLo();
			if (!loRtnu)
				continue;

			LevelPos posRtnu=loRtnu->GetFramePos();

			BOOL bCloseThreat=FALSE;
			BOOL bMeleeThreat=FALSE;
			if (TRUE)
			{
				CLevelObj *loThreat=NULL;
				if (TRUE)
				{
					CLevelSensor *sensor=loRtnu->GetSensor();
					if (sensor)
					{
						loThreat=sensor->GetThreat();
						if (loThreat)
						{
							if (loThreat->GetFramePos().getDistanceSQFrom(posRtnu)<distCloseThreat*distCloseThreat)
								bCloseThreat=TRUE;
							float dist=distMeleeThreat+loThreat->GetRadius_()+loRtnu->GetRadius_();
							if (loThreat->GetFramePos().getDistanceSQFrom(posRtnu)<dist*dist)
								bMeleeThreat=TRUE;
						}
					}
				}
			}
			BOOL bInPursueOutter=FALSE;
			BOOL bInPursueInner=FALSE;
			if (posRtnu.getDistanceSQFrom(posOwner)>distPursueOutter*distPursueOutter)
				bInPursueOutter=TRUE;
			else
			{
				if (posRtnu.getDistanceSQFrom(posOwner)<distPursueInner*distPursueInner)
					bInPursueInner=TRUE;
			}

			LevelRtnuBehavior bhvOld=rtnu->GetBhv();

			LevelRtnuBehavior bhv=LevelRtnuBehavior_Accompany;
// 			if (!_bGuardModes[rtnu->GetRank()])
// 			{
// 				switch(bhvOld)
// 				{
// 					case LevelRtnuBehavior_Pursue:
// 					{
// 						if (!bInPursueInner)
// 							bhv=LevelRtnuBehavior_Pursue;
// 						else
// 						{
// 							if (!bCloseThreat)
// 								bhv=LevelRtnuBehavior_Accompany;
// 							else
// 								bhv=LevelRtnuBehavior_FreeAttack;
// 						}
// 						break;
// 					}
// 					case LevelRtnuBehavior_FreeAttack:
// 					case LevelRtnuBehavior_GuardAttack:
// 					case LevelRtnuBehavior_Accompany:
// 					case LevelRtnuBehavior_None:
// 					{
// 						if (bInPursueOutter)
// 							bhv=LevelRtnuBehavior_Pursue;
// 						else
// 						{
// 							if (!bCloseThreat)
// 								bhv=LevelRtnuBehavior_Accompany;
// 							else
// 								bhv=LevelRtnuBehavior_FreeAttack;
// 						}
// 						break;
// 					}
// 				}
// 			}
// 			else
// 			{
// 				BOOL bAccompanyReaching=FALSE;
// 				if (TRUE)
// 				{
// 					if (bOwnerReaching)
// 					{
// 						CRtnuUnit *unitRtnu=loRtnu->GetRtnuUnit();
// 						if (unitRtnu)
// 							bAccompanyReaching=!unitRtnu->IsMoving();
// 					}
// 				}
// 
// 				switch(bhvOld)
// 				{
// 					case LevelRtnuBehavior_Pursue:
// 					{
// 						if (!bInPursueInner)
// 							bhv=LevelRtnuBehavior_Pursue;
// 						else
// 						{
// 							if (bMeleeThreat&&bAccompanyReaching)
// 								bhv=LevelRtnuBehavior_GuardAttack;
// 							else
// 								bhv=LevelRtnuBehavior_Accompany;
// 						}
// 						break;
// 					}
// 					case LevelRtnuBehavior_FreeAttack:
// 					{
// 						if (bInPursueOutter)
// 							bhv=LevelRtnuBehavior_Pursue;
// 						else
// 							bhv=LevelRtnuBehavior_Accompany;
// 						break;
// 					}
// 					case LevelRtnuBehavior_None:
// 					case LevelRtnuBehavior_GuardAttack:
// 					case LevelRtnuBehavior_Accompany:
// 					{
// 						if (bInPursueOutter)
// 							bhv=LevelRtnuBehavior_Pursue;
// 						else
// 						{
// 							if (bMeleeThreat&&bAccompanyReaching)
// 								bhv=LevelRtnuBehavior_GuardAttack;
// 							else
// 								bhv=LevelRtnuBehavior_Accompany;
// 						}
// 						break;
// 					}
// 				}
// 			}

			rtnu->SetBhv(bhv);
		}
	}

	_idxRtnusGC++;
	if (_idxRtnusGC%20==0)
		_UpdateRetinuesGC();
}

void CLevelRtnus::AddCoSkillCharge(LevelRecordSkill *rec,LevelSkillGrade grd,LevelSkillTarget &target)
{
	_UpdateRetinuesGC();
	for (int i=0;i<_rtnus.size();i++)
	{
		CLevelRtnu *rtnu=_rtnus[i];
		CLoUnit *lo=rtnu->GetLo();
		if (lo)
		{
			extern BOOL LevelUtil_AddCoSkillCharge(CLevelObj *lo,LevelRecordSkill *recSkill,LevelSkillGrade grd,LevelSkillTarget &target);
			if (LevelUtil_AddCoSkillCharge(lo,rec,grd,target))
				lo->UpdateAI(TRUE);
		}
	}

}

void CLevelRtnus::UpdateAI()
{
	//让所有的随从立即更新一下AI
	_UpdateRetinuesGC();
	for (int i=0;i<_rtnus.size();i++)
	{
		CLevelRtnu *rtnu=_rtnus[i];
		CLoUnit *lo=rtnu->GetLo();
		if (lo)
			lo->UpdateAI(TRUE);
	}
}

