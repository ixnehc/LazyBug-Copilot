/********************************************************************
	created:	2017/11/08 
	author:		cxi
	
	purpose:	InSlatesµÄBuff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_InSlates.h"


////////////////////////////////////////////////////////////////////////
//BuffData_InSlates
void BuffData_InSlates::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(idSlates);
}

void BuffData_InSlates::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(idSlates);
}


//////////////////////////////////////////////////////////////////////////
//CBuff_InSlates
BIND_BUFFPARAM(Buff_InSlates,BuffParam_InSlates,BuffArg_InSlates);

LevelBuffMask Buff_InSlates::GetReplaceBuffs()
{
	LevelBuffMask mask=0;

	return mask;
}

void Buff_InSlates::_OnCreate(LevelBuffArg *arg0)
{
	CLevelObj *owner=_GetOwner();

	BuffArg_InSlates *arg=arg0->ToPtr<BuffArg_InSlates>();
	if (arg)
		_data.idSlates=arg->idSlates;
}

void Buff_InSlates::_WriteData(CBitPacket *bp)
{
	_data.Save(bp);
}
