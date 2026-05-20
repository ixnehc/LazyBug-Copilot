/********************************************************************
	created:	2012/01/17
	file base:	Buff_KD
	author:		cxi
	
	purpose:	击倒的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"
#include "LevelRecordBuff.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordUnit.h"


#include "Buff_KD.h"
#include "Buff_KB.h"
#include "Buff_Dead.h"

//////////////////////////////////////////////////////////////////////////
//CBuff_KD
BIND_BUFFPARAM(Buff_KD,BuffParam_KD,BuffArg_KD);

LevelBuffMask Buff_KD::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Dead)->GetUID());

	return mask;

}


LevelBuffMask Buff_KD::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KD)->GetUID());
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KB)->GetUID());

	return mask;
}

void Buff_KD::_OnCreate(LevelBuffArg *arg0)
{
	BuffParam_KD *param=(BuffParam_KD *)_param;
	BuffArg_KD *arg=(BuffArg_KD *)arg0;

	CLevelObj *owner=_GetOwner();

	//计算击倒到何处
	if (TRUE)
	{
		LevelPos src=owner->GetFramePos();
		float face=owner->GetFrameFace();

		if ((arg->dir.x==0.0f)&&(arg->dir.y==0.0f))
		{
			_data.target=src;
			_data.face=face;
			_data.bKB=0;
		}
		else
		{
			LevelPos target=src+arg->dir*param->dist;//击退两米距离
			LevelPos hit;
			if (owner->GetLevel()->GetUnitMgr()->StaticRayCast(UnitFindPath_Walkable,src,target,hit))
				target=hit;
			_data.target=target;
			_data.face=atan2f(-arg->dir.y,-arg->dir.x);
			_data.bKB=1;
		}
	}

	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
	{
 		if (_dur!=ANIMTICK_INFINITE)
		{
			_dur+=pauser->GetDelay();
			_dur+=param->durKnockDown+param->durRaiseUp;
		}

		_data.idTeleport=owner->GenTeleportID();
		_data.idBroken=pauser->Teleport(_data.target,_data.face,_data.idTeleport);
	}
}

void Buff_KD::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimpleR(_data);
}

void Buff_KD::RaiseUp()
{
	BuffParam_KD *param=(BuffParam_KD *)_param;

	if (_dur>param->durRaiseUp)
	{
		_dur=param->durRaiseUp;
		LevelOp_ModBuffDur*op=NewOp<LevelOp_ModBuffDur>();
		op->durNew=_dur;
		_GetOwner()->AddOp(op);
	}
}
