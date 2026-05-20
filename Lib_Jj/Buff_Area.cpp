/********************************************************************
	created:	2016/11/16
	author:		cxi
	
	purpose:	Buff_Area
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_Area.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Area,BuffParam_Area,BuffArg_Area);


void Buff_Area::_OnUpdate(AnimTick dt)
{
	BuffParam_Area *param=_rec->GetParam<BuffParam_Area>();

	if (param)
	{
		int nToDeal=(int)(ANIMTICK_TO_SECOND(_tAge)*param->speed)+1;
		while(nToDeal>_nDealed)
		{
 			_MakeRangeDeals(param->radius);
			_nDealed++;
		}
	}

	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	if (LevelUtil_CheckDead(_GetOwner()))
		_dur=0;
}
