/********************************************************************
	created:	2019/09/14
	file base:	Buff_Blocking
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LevelRecordBuff.h"


#include "LoUnit.h"

#include "LevelObjPauser.h"


#include "Buff_Blocking.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_Blocking

BIND_BUFFPARAM(Buff_Blocking,BuffParam_Blocking,BuffArg_Blocking);

LevelBuffMask Buff_Blocking::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;

	return mask;

}


LevelBuffMask Buff_Blocking::GetReplaceBuffs()
{
	LevelBuffMask mask=0;

	return mask;
}

void Buff_Blocking::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_Blocking *arg=(BuffArg_Blocking *)arg0;

	CLevelObj *owner=_GetOwner();

	//计算击退到何处
	if (TRUE)
	{
		LevelPos dir=arg->strike.GetDir();
		_face=atan2f(-dir.y,-dir.x);
	}

	_strike=arg->strike;
}

void Buff_Blocking::_OnDestroy()
{
}


void Buff_Blocking::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_face);
	_strike.Save(bp);

}
