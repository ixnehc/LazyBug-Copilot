/********************************************************************
	created:	2016/05/15 
	author:		cxi
	
	purpose:	NotAttackableµÄBuff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "LevelRecordBuff.h"

#include "Buff_NotAttackable.h"


////////////////////////////////////////////////////////////////////////
//BuffData_NotAttackable
void BuffData_NotAttackable::Save(CBitPacket *bp)
{
}

void BuffData_NotAttackable::Load(CBitPacket *bp)
{
}


//////////////////////////////////////////////////////////////////////////
//CBuff_NotAttackable
BIND_BUFFPARAM(Buff_NotAttackable,BuffParam_NotAttackable,BuffArg_NotAttackable);

LevelBuffMask Buff_NotAttackable::GetReplaceBuffs()
{
	LevelBuffMask mask=0;

	return mask;
}

LevelBuffMask Buff_NotAttackable::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_NotAttackable)->GetUID());
	return mask;
}


void Buff_NotAttackable::_OnCreate(LevelBuffArg *arg0)
{
	CLevelObj *owner=_GetOwner();


}

void Buff_NotAttackable::_WriteData(CBitPacket *bp)
{
	_data.Save(bp);
}
