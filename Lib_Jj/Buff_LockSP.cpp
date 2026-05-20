/********************************************************************
	created:	2022/07/22
	author:		cxi
	
	purpose:	Buff_LockSP
*********************************************************************/


#include "stdh.h"

#include "LevelRecordBuff.h"

#include "Buff_LockSP.h"

#include "datapacket/BitPacket.h"

BIND_BUFFPARAM(Buff_LockSP,BuffParam_LockSP,BuffArg_LockSP);

BOOL Buff_LockSP::IsInLock()
{
	BuffParam_LockSP *param=_rec->GetParam<BuffParam_LockSP>();
	return _t<param->durLock;
}


void Buff_LockSP::_OnUpdate(AnimTick dt)
{
	BuffParam_LockSP *param=_rec->GetParam<BuffParam_LockSP>();
	_t+=dt;
// 	_AddSyncDataOp();
}

void Buff_LockSP::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_t);
}

void Buff_LockSP::LoadTeleport(CLevelBuff *buffOrg)
{
	_t=((Buff_LockSP*)buffOrg)->_t;
}

void Buff_LockSP::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_ModSPCost)
	{
		if (IsInLock())
		{
			LeModSPCost &e=(LeModSPCost &)e0;
			e.rateMul*=0.0f;
		}
	}
}
