/********************************************************************
	created:	2022/08/22
	author:		cxi
	
	purpose:	Buff_AuraNature
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_AuraNature.h"

#include "LevelAttrs.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_AuraNature,BuffParam_AuraNature,BuffArg_AuraNature);

BOOL Buff_AuraNature::Merge(LevelRecordBuff *rec,LevelBuffArg *arg,AnimTick dur)
{
	if (rec->GetParam<BuffParam_AuraNature>())
	{
		_nRepeat++;
		return TRUE;
	}
	return FALSE;
}


void Buff_AuraNature::_OnUpdate(AnimTick dt)
{
}

void Buff_AuraNature::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_ModBaseAttrs)
	{
// 		BuffParam_AuraNature *param=_rec->GetParam<BuffParam_AuraNature>();
// 
		LeModBaseAttrs &e=(LeModBaseAttrs &)e0;

		if (e.mod)
		{
			e.bMod=TRUE;
			e.mod->hpRecoverAdd+=_nRepeat;
		}
	}

	if (e0.GetType()==LET_ModSPCost)
	{
		LeModSPCost &e=(LeModSPCost &)e0;
		e.rateReduce+=0.2f*(float)_nRepeat;
	}

}
