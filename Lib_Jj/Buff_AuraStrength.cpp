/********************************************************************
	created:	2016/11/16
	author:		cxi
	
	purpose:	Buff_AuraStrength
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_AuraStrength.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_AuraStrength,BuffParam_AuraStrength,BuffArg_AuraStrength);

BOOL Buff_AuraStrength::Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)
{
	if (rec->GetParam<BuffParam_AuraStrength>())
	{
		_nRepeat++;
		return TRUE;
	}
	return FALSE;
}


void Buff_AuraStrength::_OnUpdate(AnimTick dt)
{
}

void Buff_AuraStrength::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_ModDamageAttr)
	{
		BuffParam_AuraStrength *param=_rec->GetParam<BuffParam_AuraStrength>();

		LeModDamageAttr &e=(LeModDamageAttr &)e0;

		if (e.modsAttack)
		{
			e.bAttackMods=TRUE;
			e.modsAttack->modsDamage[DamageAttrType_Pierce].atkRate+=param->rateDamage*(float)_nRepeat;
			e.modsAttack->modsDamage[DamageAttrType_Smash].atkRate+=param->rateDamage*(float)_nRepeat;
		}
	}

}
