
#include "stdh.h"

#include "Level.h"

#include "LevelSensor.h"

#include "LevelRecordBuff.h"

#include "Buff_Shoot.h"

#include "datapacket/BitPacket.h"

#include "Deal_CreateEo.h"

BIND_BUFFPARAM(Buff_Shoot,BuffParam_Shoot,BuffArg_Shoot);


void Buff_Shoot::_OnUpdate(AnimTick dt)
{
	BuffParam_Shoot *param=(BuffParam_Shoot*)_param;

	if (!_bShooted)
	{
		if (_tAge>=param->delay)
		{
			CLevelObj *lo=_GetOwner();
			if (lo)
			{
				CLevelSensor *sensor=lo->GetSensor();
				if (sensor)
				{
					CLevelObj *loThreat=sensor->GetThreat();
					if (loThreat)
					{
						if (param->deal)
						{
							DealArg arg;
							LevelPos3D posSrc;
							posSrc=lo->GetFramePos3D();
							posSrc.y+=lo->GetCastHeight();
							LevelPos3D posTarget=loThreat->GetFramePos3D();
							posTarget.y+=loThreat->GetAimHeight();
							arg.dir=posTarget-posSrc;
							arg.dir.normalize();

							arg.link.id=_GetLevel()->GenOpLinkID();

							CLoEffectObj *eo=((Deal_CreateEo*)param->deal)->CreateEo(LevelOSB(this),posSrc,arg,loThreat->GetID());
							SAFE_RELEASE(eo);
						}
					}
				}
			}
			_bShooted=TRUE;
		}
	}
}
