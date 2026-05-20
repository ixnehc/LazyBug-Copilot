
#include "stdh.h"

#include "LevelObjSrc.h"
#include "LevelObj.h"

#include "LevelOp.h"
#include "LevelOps.h"

#include "Level.h"
#include "LevelHooks.h"

#include "LevelObjMove.h"
#include "LevelSkillCDs.h"
#include "LevelTalks.h"
#include "LevelAttrs.h" 

//////////////////////////////////////////////////////////////////////////
//CLevelObj
void CLevelObj::OnRelease()
{
	_Destroy();
	Class_Delete(this);
}

BOOL CLevelObj::Create()
{
	if (!_Create())
		return FALSE;
	AddRef();
	return TRUE;
}

void CLevelObj::Destroy()
{
	_Destroy();
	Release();
}

void CLevelObj::_Destroy()
{
	if (!IsAlive())
		return;

	Deactivate();

	OnDestroy();

	_bAlive=0;
	_bActive=0;
	_level->GetIDs()->UnRegister(this);
}

BOOL CLevelObj::_Create()
{	
	if (FALSE==OnCreate())
		return FALSE;
	_bAlive=1;
	_level->GetIDs()->Register(this);

	return TRUE;
}


void CLevelObj::_RegisterLevelHook(int tp,DWORD priority)
{
	if (_level)
		_level->GetHooks()->RegisterHook((LevelHookType)tp,this,priority);
}


BOOL CLevelObj::Activate()
{
	if (IsActive())
		return TRUE;
	if (FALSE==OnActivate())
		return FALSE;
	_bActive=1;

	_level->GetObjMap()->AddLo(this);

	return TRUE;
}

void CLevelObj::Deactivate()
{
	if (!IsActive())
		return;
	_level->GetObjMap()->RemoveLo(this);
	OnDeactivate();
	_bActive=0;
}

void CLevelObj::HandleEvent(LevelEvent &e)
{
	OnEvent(e);
	CLevelBuffs *buffs=GetBuffs();
	if (buffs)
		buffs->HandleEvent(e);
}


LevelPos3D CLevelObj::GetFramePos3D()
{
	LevelPos pos=GetFramePos();

	extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
	return LevelUtil_GetGroundHeight(_level,pos.x,pos.y,FALSE);
}


LevelTick CLevelObj::GetT()	
{		
	return _level->GetT_();	
}

ServerSecond CLevelObj::GetServerSecond()
{
	return _level->GetServerSecond();
}

void CLevelObj::AddOp(CLevelOp *op)
{
	CLevelOps *ops=GetOps();
	if (ops)
		ops->AddOp(op);
}


void CLevelObj::SetCollide_Ghost(BOOL bGhost)
{
	CUnit *unit=GetUnit();
	if (unit)
	{
		extern void UnitCollide_SetGhost(CUnit *unit,BOOL bGhost);
		UnitCollide_SetGhost(unit,bGhost);
	}
}


LevelMoveSession CLevelObj::MoveCmd_RequestTarget(LevelPos &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange)
{
	CLevelObjMove *move=GetMove();
	if (move)
		return move->RequestTarget(pos,range,bClosestFollow,bNoStopMoveWhenInRange);
	return LevelMoveSession_Invalid;
}

LevelMoveSession CLevelObj::MoveCmd_RequestTarget(LevelPos3D &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange)
{
	CLevelObjMove *move=GetMove();
	if (move)
		return move->RequestTarget(pos,range,bClosestFollow,bNoStopMoveWhenInRange);
	return LevelMoveSession_Invalid;
}


LevelMoveSession CLevelObj::MoveCmd_RequestTarget(CLevelObj*lo,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange,BOOL b3DFollow)
{
	CLevelObjMove *move=GetMove();
	if (move)
		return move->RequestTarget(lo,range,bClosestFollow,bNoStopMoveWhenInRange,b3DFollow);
	return LevelMoveSession_Invalid;
}

LevelMoveSession CLevelObj::MoveCmd_RequestFacing(float range,float rad)
{
	CLevelObjMove *move=GetMove();
	if (move)
		return move->RequestFacing(range,rad);
	return LevelMoveSession_Invalid;
}

LevelMoveSession CLevelObj::MoveCmd_RequestNoTarget()
{
	CLevelObjMove *move=GetMove();
	if (move)
		return move->RequestNoTarget();
	return LevelMoveSession_Invalid;
}



void CLevelObj::MoveCmd_ResetIdle()
{
	CLevelObjMove *move=GetMove();
	if (move)
		move->ResetIdle();
}

void CLevelObj::BreakTalk(LevelPlayerID idPlayer)
{
	CLevelTalks *talks=GetTalks();
	if (talks)
		talks->ClearActive(idPlayer);
}

BOOL CLevelObj::TestBuffFlag(DWORD flags)
{
	CLevelBuffs *buffs=GetBuffs();
	if (!buffs)
		return FALSE;
	return buffs->TestFlag(flags);
}



void CLevelObj::StartCD(LevelRecordSkill *rec)
{
	CLevelSkillCDs *cds=GetSkillCDs();
	if (cds)
		cds->StartCD(rec);
}


CLevelOp *CLevelObj::NewOp(CClass *clssOp,LevelOpLink&link)
{
	extern CLevelOp*NewLevelOp(ClassUID uid);
	CLevelOp *op=NewLevelOp(clssOp->GetUID());
	if (!op)
		return NULL;

	LevelOpDesc &desc=op->GetDesc();
	FillOpDesc(desc,clssOp,link);

	return op;

}

void CLevelObj::AddEvent(LevelEvent *e)
{
// 	LevelEventQueue *eq=GetEventQueue();
// 	if (!eq)
// 	{
// 		Safe_Class_Delete(e);
// 	}
// 	else
// 	{
// 		e->t=_level->GetT_();
// 		e->iSerial=_level->GetEventMap()->AllocSerial();
// 		//注意不需要设位置信息
// 
// 		eq->Add(e);
// 		eq->DiscardOlds(e->t);
// 	}
}


void CLevelObj::_WriteSync_PlayerID(CBitPacket *bp,BOOL &bContent)
{
	if(_bSyncPlayerID)
	{
		bp->Bit_Write_1();
		bp->Data_WriteSimple(_idPlayer);
		bContent=TRUE;
	}
	else
		bp->Bit_Write_0();
}

void CLevelObj::_PostWriteSync_PlayerID()
{
	_bSyncPlayerID=0;
}

// 
// LevelAttack CLevelObj::GetAttack(LevelAttackType tp)
// {
// 	LevelAttr_Battle *attrBattle=GetAttr_Battle();
// 	if (!attrBattle)
// 		return LevelAttack();
// 
// 	LevelAttack atk;
// 	switch (tp)
// 	{
// 		case Attack_MeleePhys:
// 		case Attack_RangePhys:
// 		{
// 			atk.tp=tp;
// 			atk.atkLo=attrBattle->atkLo;
// 			atk.atkHi=attrBattle->atkHi;
// 			atk.accu=attrBattle->accu;
// 			atk.stun=attrBattle->stun;
// 			break;
// 		}
// 		case Attack_Fire:
// 		{
// 			atk.tp=tp;
// 			atk.atkLo=atk.atkHi=attrBattle->atkFire;
// 			break;
// 		}
// 		case Attack_Electricity:
// 		{
// 			atk.tp=tp;
// 			atk.atkLo=atk.atkHi=attrBattle->atkElec;
// 			break;
// 		}
// 		case Attack_Cold:
// 		{
// 			atk.tp=tp;
// 			atk.atkLo=atk.atkHi=attrBattle->atkCold;
// 			break;
// 		}
// 		case Attack_Poison:
// 		{
// 			atk.tp=tp;
// 			atk.atkLo=atk.atkHi=attrBattle->atkPoison;
// 			break;
// 		}
// 	}
// 	atk.grdOwner=GetGrade();
// 
// 	return atk;
// }
// 
// LevelDefence CLevelObj::GetDefence(LevelAttackType tp)
// {
// 	LevelAttr_Battle *attrBattle=GetAttr_Battle();
// 	if (!attrBattle)
// 		return LevelDefence();
// 
// 	LevelDefence def;
// 	def.stunresist=attrBattle->stunresist;
// 	def.evade=attrBattle->evade;
// 	def.bCanStun=attrBattle->bCanStun;
// 	switch (tp)
// 	{
// 		case Attack_MeleePhys:
// 		case Attack_RangePhys:
// 		{
// 			def.tp=tp;
// 			def.def=attrBattle->def;
// 			break;
// 		}
// 		case Attack_Fire:
// 		{
// 			def.tp=tp;
// 			def.def=attrBattle->resistFire;
// 			break;
// 		}
// 		case Attack_Electricity:
// 		{
// 			def.tp=tp;
// 			def.def=attrBattle->resistElec;
// 			break;
// 		}
// 		case Attack_Cold:
// 		{
// 			def.tp=tp;
// 			def.def=attrBattle->resistCold;
// 			break;
// 		}
// 		case Attack_Poison:
// 		{
// 			def.tp=tp;
// 			def.def=attrBattle->resistPoison;
// 			break;
// 		}
// 	}
// 
// 	def.grdOwner=GetGrade();
// 	return def;
// }
