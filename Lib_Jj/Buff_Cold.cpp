
#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Cold.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Cold,BuffParam_Cold,BuffArg_Cold);

void Buff_Cold::_WriteData(CBitPacket *dp)
{
	BYTE v=(BYTE)(_str*100.0f);
	dp->Data_WriteSimple(v);
}

void Buff_Cold::_OnCreate(LevelBuffArg *arg)
{
	BuffParam_Cold *param=_rec->GetParam<BuffParam_Cold>();
	if (param)
		_str=param->str;
}



BOOL Buff_Cold::Merge(LevelRecordBuff *recNew,LevelBuffArg *arg0,AnimTick dur)
{
	if (recNew)
	{
		BuffParam_Cold *param=recNew->GetParam<BuffParam_Cold>();
		if(param)
		{
			_str=param->str;
			if (_dur!=ANIMTICK_INFINITE)
			{
				if (dur>_dur)
					_dur=dur;
			}
			_AddSyncDataOp();
			return TRUE;
		}
	}

	return FALSE;
}
