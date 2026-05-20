/********************************************************************
	created:	2012/03/15
	file base:	Buff_Birth
	author:		cxi
	
	purpose:	出生的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_Birth.h"

#include "LevelAttrs.h"

#include "LevelObjPauser.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_Birth

BIND_BUFFPARAM(Buff_Birth,BuffParam_Birth,BuffArg_Birth);

LevelBuffMask Buff_Birth::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Birth)->GetUID());

	return mask;
}

void Buff_Birth::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_Birth *arg=(BuffArg_Birth *)arg0;
	CLevelObj *owner=_GetOwner();

	_eulerX=arg->eulerX;

	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
		pauser->Pause();

	if (_rec->Dur>0)
		_dur=_rec->Dur;

	if (!arg->descOp.IsEmpty())
	{
		extern CLevelOp *NewLevelOp(LevelOpDesc &desc);
		CLevelOp *op=NewLevelOp(arg->descOp);
		if (op)
		{
			if (IsClass2(op,LevelOp_AddBuff))
			{
				_op=(LevelOp_AddBuff*)op;
 				ToData(_op->data);
			}
			else
			{
				Class_Delete(op);
			}
		}
	}
}

void Buff_Birth::_OnDestroy()
{
	CLevelObj *owner=_GetOwner();

	Safe_Class_Delete(_op);

}



void Buff_Birth::_WriteData(CBitPacket *dp)
{
	dp->Data_NextFloat()=_eulerX;
}
