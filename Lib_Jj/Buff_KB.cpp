/********************************************************************
	created:	2012/01/11
	file base:	Buff_KB
	author:		cxi
	
	purpose:	击退的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LevelRecordBuff.h"


#include "LoUnit.h"

#include "LevelObjPauser.h"


#include "Buff_KB.h"
#include "Buff_KD.h"
#include "Buff_Dead.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_KB

BIND_BUFFPARAM(Buff_KB,BuffParam_KB,BuffArg_KB);

LevelBuffMask Buff_KB::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KD)->GetUID());
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Dead)->GetUID());

	return mask;

}


LevelBuffMask Buff_KB::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KB)->GetUID());

	return mask;
}

void Buff_KB::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_KB *arg=(BuffArg_KB *)arg0;

	CLevelObj *owner=_GetOwner();

	//计算击退到何处
	if (TRUE)
	{
		LevelPos src=owner->GetFramePos();

		LevelPos dir=arg->strike.GetDir();
		LevelPos target=src+dir*1.0f;//击退两米距离


		LevelPos hit;
		if (owner->GetLevel()->GetUnitMgr()->StaticRayCast(UnitFindPath_Walkable,src,target,hit))
			target=hit;

		_target=target;
		_face=atan2f(-dir.y,-dir.x);
	}

	extern BOOL LevelUtil_AddStunSrc(CLevelBuff *buff);
	LevelUtil_AddStunSrc(this);


	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
	{
		_dur+=pauser->GetDelay();
		_idTeleport=owner->GenTeleportID();
		_idBroken=pauser->Teleport(_target,_face,_idTeleport);
	}

	_strike=arg->strike;

	//继承旧的弱点
	extern void LevelUtil_TakeOverWeaksOverride(CLevelBuff *buff);
	LevelUtil_TakeOverWeaksOverride(this);

}

void Buff_KB::_OnDestroy()
{
	extern BOOL LevelUtil_NotififyStunSrc_Finish(CLevelBuff *buff);
	LevelUtil_NotififyStunSrc_Finish(this);

	extern void LevelUtil_ClearWeaksOverride(CLevelBuff *buff);
	LevelUtil_ClearWeaksOverride(this);

}


void Buff_KB::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_idBroken);
	bp->Data_WriteSimple(_idTeleport);
	bp->Data_WriteSimple(_target);
	bp->Data_WriteSimple(_face);
	_strike.Save(bp);

}
