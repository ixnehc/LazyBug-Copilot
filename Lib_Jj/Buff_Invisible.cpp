/********************************************************************
	created:	2013/7/6 
	author:		cxi
	
	purpose:	隐身的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_Invisible.h"

#include "Buff_KB.h"
#include "Buff_KD.h"

////////////////////////////////////////////////////////////////////////
//BuffData_Invisible
void BuffData_Invisible::Save(CBitPacket *bp)
{
}

void BuffData_Invisible::Load(CBitPacket *bp)
{
}


//////////////////////////////////////////////////////////////////////////
//CBuff_Invisible
BIND_BUFFPARAM(Buff_Invisible,BuffParam_Invisible,BuffArg_Invisible);

LevelBuffMask Buff_Invisible::GetReplaceBuffs()
{
	LevelBuffMask mask=0;

	return mask;
}

void Buff_Invisible::_OnCreate(LevelBuffArg *arg0)
{
	CLevelObj *owner=_GetOwner();


}

void Buff_Invisible::_OnDestroy()
{


}


void Buff_Invisible::_WriteData(CBitPacket *bp)
{
	_data.Save(bp);
}
