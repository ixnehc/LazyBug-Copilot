
#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Petrify.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Petrify,BuffParam_Petrify,BuffArg_Petrify);

void Buff_Petrify::_WriteData(CBitPacket *bp)
{
	float str=_str;
	if (str>1.0f)
		str=1.0f;
	BYTE v=(BYTE)(str*100.0f);
	bp->Data_WriteSimple(v);
}


void Buff_Petrify::_OnUpdate(AnimTick dt)
{
	//衰减
	if ((!_bInc)&&(_bDmp))
	{
		BuffParam_Petrify *param=(BuffParam_Petrify *)_param;
		_str-=param->dmp*ANIMTICK_TO_SECOND(dt);
		CLevelObj *owner=_GetOwner();

		if (_str>0.0f)
		{
			_AddSyncDataOp();
		}
		else
		{
			_dur=0;//结束自己
		}
	}

	_bInc=FALSE;
}

void Buff_Petrify::IncStr(float str)
{
	_str+=str;
	if (_str>=2.0f)
		_str=2.0f;
	_bInc=TRUE;
	_AddSyncDataOp();
}
