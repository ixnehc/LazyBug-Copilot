/********************************************************************
	created:	2022/08/22
	author:		cxi
	
	purpose:	Buff_AuraJoker
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_AuraJoker.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_AuraJoker,BuffParam_AuraJoker,BuffArg_AuraJoker);

BOOL Buff_AuraJoker::Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)
{
	if (rec->GetParam<BuffParam_AuraJoker>())
	{
		_nRepeat++;
		return TRUE;
	}
	return FALSE;
}


void Buff_AuraJoker::_OnUpdate(AnimTick dt)
{
}

void Buff_AuraJoker::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_ModDamageAttr)
	{
// 		BuffParam_AuraJoker *param=_rec->GetParam<BuffParam_AuraJoker>();
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
