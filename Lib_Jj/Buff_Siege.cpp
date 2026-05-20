
#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Siege.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Siege,BuffParam_Siege,BuffArg_Siege);

void Buff_Siege::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_idTarget);
}

void Buff_Siege::Stop()
{
	_idTarget=LevelObjID_Invalid;
	_dur=0;
	_AddSyncDataOp();
}
