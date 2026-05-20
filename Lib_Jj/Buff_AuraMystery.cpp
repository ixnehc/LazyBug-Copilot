/********************************************************************
	created:	2022/08/22
	author:		cxi
	
	purpose:	Buff_AuraMystery
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_AuraMystery.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_AuraMystery,BuffParam_AuraMystery,BuffArg_AuraMystery);

BOOL Buff_AuraMystery::Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)
{
	if (rec->GetParam<BuffParam_AuraMystery>())
	{
		_nRepeat++;
		return TRUE;
	}
	return FALSE;
}


void Buff_AuraMystery::_OnUpdate(AnimTick dt)
{
}

void Buff_AuraMystery::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_ModDamageAttr)
	{
// 		BuffParam_AuraMystery *param=_rec->GetParam<BuffParam_AuraMystery>();
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
