/********************************************************************
	created:	2020/02/02
	file base:	Buff_CentipedeNode_Move
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_CentipedeNode_Move.h"

#include "LoCentipede.h"


//////////////////////////////////////////////////////////////////////////
//CLevelGesture_CentipedeNode_Move
void CLevelGesture_CentipedeNode_Move::Create(Buff_CentipedeNode_Move *owner)
{
	_owner=owner;
}

void CLevelGesture_CentipedeNode_Move::Update(CUnit *unit,float dt)
{
	if (_owner)
	{
		LevelPos pos;
		LevelFace face;
		if (_owner->GetLoc(pos,face))
		{
			unit->_pos=pos;
			unit->_face=face;
		}
	}
}




//////////////////////////////////////////////////////////////////////////
//CBuff_RavenBirth

BIND_BUFFPARAM(Buff_CentipedeNode_Move,BuffParam_CentipedeNode_Move,BuffArg_CentipedeNode_Move);

BuffFlag Buff_CentipedeNode_Move::GetFlags()
{
	BuffParam_CentipedeNode_Move*param=_rec->GetParam<BuffParam_CentipedeNode_Move>();
	if (!param->bCyst)
		return BuffFlag_NotAttackable;
	else
		return 0;
}


LevelBuffMask Buff_CentipedeNode_Move::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_CentipedeNode_Move)->GetUID());

	return mask;
}

void Buff_CentipedeNode_Move::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_CentipedeNode_Move *arg=(BuffArg_CentipedeNode_Move *)arg0;
	CLevelObj *owner=_GetOwner();

	BuffParam_CentipedeNode_Move*param=_rec->GetParam<BuffParam_CentipedeNode_Move>();

	if (param)
	{
		extern LevelObjID LevelUtil_GetLevelObjIDFromVar(CLevelObj *owner,StringID nm);
		_idAgent=LevelUtil_GetLevelObjIDFromVar(owner,param->varCentipedeAgent);

		//启动一个Gesture
		CUnit *unit=owner->GetUnit();
		if (unit)
		{
			CLevelGesture_CentipedeNode_Move *ges=Class_New2(CLevelGesture_CentipedeNode_Move);
			ges->Create(this);
			unit->SetGesture(ges);

			SAFE_REPLACE(_ges,ges);
		}
	}
}

void Buff_CentipedeNode_Move::_OnDestroy()
{
	CLevelObj *owner=_GetOwner();


	if (_ges)
		_ges->Finish();
	SAFE_RELEASE(_ges);
}



void Buff_CentipedeNode_Move::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_idAgent);
}

CLoCentipede *Buff_CentipedeNode_Move::GetLoCentipede()
{
	extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
	CLevelObj *lo=LevelUtil_GetAliveLo(_GetLevel(),_idAgent);
	if (lo)
	{
		if (lo->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
			return (CLoCentipede *)lo;
	}
	return NULL;
}


BOOL Buff_CentipedeNode_Move::GetLoc(LevelPos &pos,LevelFace &face)
{
	BuffParam_CentipedeNode_Move*param=_rec->GetParam<BuffParam_CentipedeNode_Move>();

	CLoCentipede *loCentipede=GetLoCentipede();
	if (loCentipede)
	{
		if (!param->bCyst)
			return loCentipede->GetNodeLoc(_GetOwner()->GetID(),pos,face);
		else
			return loCentipede->GetCystLoc(_GetOwner()->GetID(),pos,face);
	}
	return FALSE;
}
