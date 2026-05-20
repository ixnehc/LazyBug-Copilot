/********************************************************************
	created:	2012/01/17
	file base:	Buff_Teleport
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


#include "Buff_Teleport.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_Teleport
BIND_BUFFPARAM(Buff_Teleport,BuffParam_Teleport,BuffArg_Teleport);

LevelBuffMask Buff_Teleport::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Teleport)->GetUID());

	return mask;
}

void Buff_Teleport::_OnCreate(LevelBuffArg *param0)
{
	BuffArg_Teleport *param=(BuffArg_Teleport *)param0;

	CLevelObj *owner=_GetOwner();

	if (TRUE)
	{
		_data.target=param->pos;
		_data.face=param->face;
	}

	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
	{

		_data.idTeleport=owner->GenTeleportID();
		_data.idBroken=pauser->Teleport(_data.target,_data.face,_data.idTeleport);
	}

}

void Buff_Teleport::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimpleR(_data);
}
