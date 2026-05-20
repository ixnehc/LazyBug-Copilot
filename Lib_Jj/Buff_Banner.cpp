/********************************************************************
	created:	2016/11/16
	author:		cxi
	
	purpose:	Buff_Banner
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Banner.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Banner,BuffParam_Banner,BuffArg_Banner);


void Buff_Banner::_OnUpdate(AnimTick dt)
{
	BuffParam_Banner *param=_rec->GetParam<BuffParam_Banner>();
}
