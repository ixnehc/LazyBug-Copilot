/********************************************************************
	created:	2019/11/01 
	author:		cxi
	
	purpose:	FliesµÄBuff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_Flies.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_Flies
BIND_BUFFPARAM(Buff_Flies,BuffParam_Flies,BuffArg_Flies);

LevelBuffMask Buff_Flies::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Flies)->GetUID());

	return mask;
}

void Buff_Flies::_OnCreate(LevelBuffArg *arg)
{
	BuffParam_Flies *param=(BuffParam_Flies*)_param;
	_count=param->count;

	CLevelObj *owner=_GetOwner();
	_pos=owner->GetFramePos();

	_bEnchanted=TRUE;
}

void Buff_Flies::_OnUpdate(AnimTick dt)
{
	BuffParam_Flies *param=(BuffParam_Flies*)_param;

	LevelPos pos=_GetOwner()->GetFramePos();

	float dist=pos.getDistanceFrom(_pos);

	if (_buffs->TestFlag(BuffFlag_Invisible))
		_bEnchanted=FALSE;
	else
	{
		if (_bEnchanted)
		{
			if (dist>param->radiusUnenchant)
				_bEnchanted=FALSE;
		}
		else
		{
			if (dist<param->radiusEnchant)
				_bEnchanted=TRUE;
		}
	}

	LevelPos vel;
	if (TRUE)
	{
		vel=pos-_pos;
		vel.safe_normalize();

		float spd=param->speed;
		if (spd>dist/ANIMTICK_TO_SECOND(dt))
			spd=dist/ANIMTICK_TO_SECOND(dt);
		vel*=spd;
	}

	float r=0.4f;
	_vel=_vel*(1.0f-r)+vel*r;

	_pos=_pos+_vel*ANIMTICK_TO_SECOND(dt);

	_AddSyncDataOp();

}

void Buff_Flies::_OnDestroy()
{

}


void Buff_Flies::_WriteData(CBitPacket *dp)
{
	dp->Data_NextWord()=(WORD)_count;
	if (IsEnchanted())
		dp->Bit_Write_1();
	else
		dp->Bit_Write_0();

	dp->Bits_Write(_form,2);

	dp->Data_WriteSimpleR(_pos);

}

void Buff_Flies::OverrideEnchanted(BOOL bEnchanted)
{
	_enchantedOverriden=bEnchanted?1:2;
}
