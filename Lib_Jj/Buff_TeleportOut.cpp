/********************************************************************
	created:	2012/01/17
	file base:	Buff_TeleportOut
	author:		cxi
	
	purpose:	死亡的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_TeleportOut.h"

#include "Buff_KB.h"
#include "Buff_KD.h"

//////////////////////////////////////////////////////////////////////////
//CBuff_TeleportOut
BIND_BUFFPARAM(Buff_TeleportOut,BuffParam_TeleportOut,BuffArg_TeleportOut);

LevelBuffMask Buff_TeleportOut::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_TeleportOut)->GetUID());

	return mask;
}

void Buff_TeleportOut::_OnCreate(LevelBuffArg *param0)
{
	BuffArg_TeleportOut *param=(BuffArg_TeleportOut *)param0;

	CLevelObj *owner=_GetOwner();

	if (TRUE)
	{
		LevelPos src=owner->GetFramePos();

		_data.target=owner->GetFramePos();
		_data.face=owner->GetFrameFace();
	}

	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
	{
		_dur=ANIMTICK_INFINITE;

		_data.idTeleport=owner->GenTeleportID();
		_data.idBroken=pauser->Teleport(_data.target,_data.face,_data.idTeleport);
	}

}

void Buff_TeleportOut::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimpleR(_data);
}
