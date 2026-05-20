
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_Deal.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_Deal,BuffParam_Deal,BuffArg_Deal);

static LevelPos3D GetTargetPos(CLevelObj *lo,BuffParam_Deal*param)
{
	LevelPos3D pos=lo->GetFramePos3D();
	if (param->tpTargetPos==1)
		pos.y+=lo->GetAimHeight();
	return pos;
}


void Buff_Deal::_OnUpdate(AnimTick dt)
{
	BuffParam_Deal *param=(BuffParam_Deal*)_param;

	if (!param->bContinuous)
	{
		if (!_bDealed)
		{
			if (_tAge>=param->delay)
			{
				CLevelObj *lo=_GetOwner();
				if (lo)
				{
					DealArg arg;
					arg.link.id=_GetLevel()->GenOpLinkID();
					arg.link.t=_tAge;
					if (param->modeTarget==0||param->modeTarget==2)
					{
						if (param->deal)
							param->deal->Make(LevelOSB(this),GetTargetPos(lo,param),arg,NULL);
						_MakeDeals(lo,arg);
					}
					if (param->modeTarget==1||param->modeTarget==2)
						_MakeDeals(GetTargetPos(lo,param),arg);
				}
				_bDealed=TRUE;
			}
		}
	}
	else
	{
		int nToDeal=(int)(ANIMTICK_TO_SECOND(_tAge)*param->dps+1.0f);
		if (param->deal)
		{
			while(nToDeal>_nDealed)
			{
				CLevelObj *lo=_GetOwner();
				if (lo)
				{
					DealArg arg;
					arg.link.id=_GetLevel()->GenOpLinkID();
					arg.link.t=_tAge;
					if (param->modeTarget==0||param->modeTarget==2)
						_MakeDeals(lo,arg);
					if (param->modeTarget==1||param->modeTarget==2)
						_MakeDeals(GetTargetPos(lo,param),arg);
				}
				_nDealed++;
			}
		}
	}
}
