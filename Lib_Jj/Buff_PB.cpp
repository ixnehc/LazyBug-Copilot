/********************************************************************
	created:	2012/01/11
	file base:	Buff_PB
	author:		cxi
	
	purpose:	击退的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LevelRecordBuff.h"


#include "LoUnit.h"

#include "LevelObjPauser.h"


#include "Buff_PB.h"
#include "Buff_KB.h"
#include "Buff_KD.h"
#include "Buff_Dead.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_PB

BIND_BUFFPARAM(Buff_PB,BuffParam_PB,BuffArg_PB);

LevelBuffMask Buff_PB::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_PB)->GetUID());
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KB)->GetUID());
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KD)->GetUID());
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Dead)->GetUID());

	return mask;
}

void Buff_PB::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_PB *arg=(BuffArg_PB *)arg0;

	CLevelObj *owner=_GetOwner();

	//计算击退到何处
	if (TRUE)
	{
		LevelPos src=owner->GetFramePos();

		LevelPos target=arg->posTarget;

		LevelPos hit;
		if (owner->GetLevel()->GetUnitMgr()->StaticRayCast(UnitFindPath_Walkable,src,target,hit))
			target=hit;

		_target=target;
		_face=arg->face;
	}

	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
	{
		_dur+=pauser->GetDelay();
		_idTeleport=owner->GenTeleportID();
		_idBroken=pauser->Teleport(_target,_face,_idTeleport);
	}

}

void Buff_PB::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_idBroken);
	bp->Data_WriteSimple(_idTeleport);
	bp->Data_WriteSimple(_target);
	bp->Data_WriteSimple(_face);
}
