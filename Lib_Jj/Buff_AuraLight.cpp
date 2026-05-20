/********************************************************************
	created:	2022/08/22
	author:		cxi
	
	purpose:	Buff_AuraLight
*********************************************************************/


#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"
#include "LevelAttrs.h"

#include "Buff_AuraLight.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_AuraLight,BuffParam_AuraLight,BuffArg_AuraLight);

BOOL Buff_AuraLight::Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)
{
	if (rec->GetParam<BuffParam_AuraLight>())
	{
		_nRepeat++;
		return TRUE;
	}
	return FALSE;
}


void Buff_AuraLight::_OnUpdate(AnimTick dt)
{
}

void Buff_AuraLight::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_ModBaseAttrs)
	{
// 		BuffParam_AuraLight *param=_rec->GetParam<BuffParam_AuraLight>();
// 
 		LeModBaseAttrs &e=(LeModBaseAttrs&)e0;
		if (e.mod)
		{
			e.mod->hnrAdd=5;
			e.bMod=TRUE;
		}
	}

	if (e0.GetType()==LET_PreKill)
	{
		LePreKill &e=(LePreKill&)e0;

		if (e.loTarget==_GetOwner())
		{
			_GetLevel()->GetDecider()->MakeCure(LevelOSB(this),_GetOwner(),100000,LevelStrike(),e.link);
			SetDur(0);

			e.bAbandon=TRUE;
		}
	}

}
