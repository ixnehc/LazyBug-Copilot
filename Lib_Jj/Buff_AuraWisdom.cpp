/********************************************************************
	created:	2022/08/22
	author:		cxi
	
	purpose:	Buff_AuraWisdom
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_AuraWisdom.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_AuraWisdom,BuffParam_AuraWisdom,BuffArg_AuraWisdom);

BOOL Buff_AuraWisdom::Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)
{
	if (rec->GetParam<BuffParam_AuraWisdom>())
	{
		_nRepeat++;
		return TRUE;
	}
	return FALSE;
}


void Buff_AuraWisdom::_OnUpdate(AnimTick dt)
{
}

void Buff_AuraWisdom::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_ModDamageAttr)
	{
// 		BuffParam_AuraWisdom *param=_rec->GetParam<BuffParam_AuraWisdom>();
// 
// 		LeModDamageAttr &e=(LeModDamageAttr &)e0;
// 
// 		if (e.modsAttack)
// 		{
// 			e.bAttackMods=TRUE;
// 			e.modsAttack->modsDamage[DamageAttrType_Pierce].atkRate+=param->rateDamage;
// 			e.modsAttack->modsDamage[DamageAttrType_Smash].atkRate+=param->rateDamage;
// 		}
	}

}
