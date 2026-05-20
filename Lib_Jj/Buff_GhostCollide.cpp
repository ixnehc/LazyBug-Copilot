/********************************************************************
	created:	2020/02/21 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_GhostCollide.h"

////////////////////////////////////////////////////////////////////////
//BuffData_GhostCollide
void BuffData_GhostCollide::Save(CBitPacket *bp)
{
}

void BuffData_GhostCollide::Load(CBitPacket *bp)
{
}


//////////////////////////////////////////////////////////////////////////
//CBuff_GhostCollide
BIND_BUFFPARAM(Buff_GhostCollide,BuffParam_GhostCollide,BuffArg_GhostCollide);

LevelBuffMask Buff_GhostCollide::GetReplaceBuffs()
{
	LevelBuffMask mask=0;

	return mask;
}

LevelBuffMask Buff_GhostCollide::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_GhostCollide)->GetUID());
	return mask;
}



void Buff_GhostCollide::_OnCreate(LevelBuffArg *arg0)
{
	CLevelObj *owner=_GetOwner();


}

void Buff_GhostCollide::_WriteData(CBitPacket *bp)
{
	_data.Save(bp);
}
