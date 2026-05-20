/********************************************************************
	created:	2018/03/09
	
	purpose:	Dizzy
*********************************************************************/


#include "stdh.h"

#include "LevelSkillDriver.h"
#include "LevelRecordBuff.h"


#include "Buff_Dizzy.h"
#include "Buff_Dead.h"
#include "LevelObjPauser.h"

#include "datapacket/BitPacket.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_Dizzy
BIND_BUFFPARAM(Buff_Dizzy,BuffParam_Dizzy,BuffArg_Dizzy);

void Buff_Dizzy::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_Dizzy *arg=(BuffArg_Dizzy *)arg0;
	CLevelObjPauser *pauser=_GetOwner()->GetPauser();
	if (pauser)
	{
		_dur+=pauser->GetDelay();
	}
}

LevelBuffMask Buff_Dizzy::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Dead)->GetUID());

	return mask;
}


void Buff_Dizzy::_WriteData(CBitPacket *dp)
{
}

BOOL Buff_Dizzy::Merge(LevelRecordBuff *recNew,LevelBuffArg *arg,AnimTick dur)
{
	if (recNew)
	{
		BuffParam_Dizzy *param=recNew->GetParam<BuffParam_Dizzy>();
		if(param)
		{
			if (_dur!=ANIMTICK_INFINITE)
				_dur+=dur;

			return TRUE;
		}
	}

	return FALSE;
}
