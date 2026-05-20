
#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Frozen.h"

#include "datapacket/BitPacket.h"

#include "LevelObjPauser.h"

BIND_BUFFPARAM(Buff_Frozen,BuffParam_Frozen,BuffArg_Frozen);


void Buff_Frozen::_OnCreate(LevelBuffArg *param)
{
	CLevelObjPauser *pauser=_GetOwner()->GetPauser();
	if (pauser)
	{
		_dur+=pauser->GetDelay();
		_idBroken=pauser->Pause();
	}
}

void Buff_Frozen::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimple(_idBroken);
}
