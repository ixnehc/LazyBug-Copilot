
#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Slow.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Slow,BuffParam_Slow,BuffArg_Slow);

void Buff_Slow::_WriteData(CDataPacket *dp)
{
	BYTE v=(BYTE)(_str*100.0f);
	dp->Data_WriteSimple(v);
}
