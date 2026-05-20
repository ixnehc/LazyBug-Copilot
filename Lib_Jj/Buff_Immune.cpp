/********************************************************************
	created:	2016/05/15 
	author:		cxi
	
	purpose:	ImmuneµÄBuff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_Immune.h"


////////////////////////////////////////////////////////////////////////
//BuffData_Immune
void BuffData_Immune::Save(CBitPacket *bp)
{
}

void BuffData_Immune::Load(CBitPacket *bp)
{
}


//////////////////////////////////////////////////////////////////////////
//CBuff_Immune
BIND_BUFFPARAM(Buff_Immune,BuffParam_Immune,BuffArg_Immune);

LevelBuffMask Buff_Immune::GetReplaceBuffs()
{
	LevelBuffMask mask=0;

	return mask;
}

void Buff_Immune::_OnCreate(LevelBuffArg *arg0)
{
	CLevelObj *owner=_GetOwner();


}

void Buff_Immune::_WriteData(CBitPacket *bp)
{
	_data.Save(bp);
}
