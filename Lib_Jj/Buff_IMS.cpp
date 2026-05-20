
#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_IMS.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_IMS,BuffParam_IMS,BuffArg_IMS);

float Buff_IMS::GetIMS()
{
	BuffParam_IMS *param=(BuffParam_IMS *)_param;

	return param->ims;
}

