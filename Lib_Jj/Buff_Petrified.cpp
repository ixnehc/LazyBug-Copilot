
#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Petrified.h"

#include "datapacket/BitPacket.h"

#include "LevelObjPauser.h"

BIND_BUFFPARAM(Buff_Petrified,BuffParam_Petrified,BuffArg_Petrified);


void Buff_Petrified::_OnCreate(LevelBuffArg *param)
{
	CLevelObjPauser *pauser=_GetOwner()->GetPauser();
	if (pauser)
		_idBroken=pauser->Pause();

	_dur=ANIMTICK_INFINITE;
}

void Buff_Petrified::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimple(_idBroken);
}
