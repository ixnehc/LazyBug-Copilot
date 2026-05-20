/********************************************************************
	created:	2012/10/22 
	author:		cxi
	
	purpose:	进洞的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"

#include "LevelEvents.h"


#include "Buff_ResideHole.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_Dead
BIND_BUFFPARAM(Buff_ResideHole,BuffParam_ResideHole,BuffArg_Reside);

LevelBuffMask Buff_ResideHole::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_ResideHole)->GetUID());
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KB)->GetUID());
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KD)->GetUID());

	return mask;
}

void Buff_ResideHole::_OnCreate(LevelBuffArg *param0)
{
	BuffArg_Reside *param=(BuffArg_Reside*)param0;

	CLevelObj *owner=_GetOwner();

	_data.idTarget=param->idTarget;

	CLevelObj *obj=owner->GetLevel()->GetIDs()->LoFromID(_data.idTarget);
	if (obj)
	{
		LevelPos3D pos;

		//跳转到目的位点上去
		pos=obj->GetFramePos3D();

		CLevelObjPauser *pauser=owner->GetPauser();
		if (pauser)
		{
			_dur=ANIMTICK_INFINITE;

			_data.idTeleport=owner->GenTeleportID();
			_data.idBroken=pauser->TeleportResided(pos,_data.idTeleport);//Reside到target的位点上
		}
	}

}


void Buff_ResideHole::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimpleR(_data);
}

