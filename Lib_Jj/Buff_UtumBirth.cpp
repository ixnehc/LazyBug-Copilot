/********************************************************************
	created:	2016/04/15
	file base:	Buff_UtumBirth
	author:		cxi
	
	purpose:	出生的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_UtumBirth.h"

#include "LevelAttrs.h"

#include "LevelObjPauser.h"


//////////////////////////////////////////////////////////////////////////
//CLevelGesture_UtumBirth
void CLevelGesture_UtumBirth::Create(i_math::vector3df &posStart,i_math::vector3df &posEnd,GameTileMap *gtm,BuffParam_UtumBirth *param)
{
	_posStart=posStart;
	_posEnd=posEnd;
	_gtm=gtm;
	_param=param;
}

void CLevelGesture_UtumBirth::Update(CUnit3D *unit,float dt)
{
	_t+=dt;
	if(dt<=0.0f)
		return;

	if (!_bFinished)
	{
		float rate=_t/_param->dur;

		float r=_param->vsHor.GetFloat(ANIMTICK_FROM_SECOND(rate));

		i_math::vector3df pos;
		pos=_posStart*(1.0f-r)+_posEnd*r;

		i_math::vector3df vel;
		vel=pos-unit->_pos;
		vel/=dt;

		unit->_pos=pos;
		unit->_vel=vel;

		unit->_ClampGround(unit->_pos);

		if (_t>=_param->dur)
			_bFinished=1;
	}
}


//////////////////////////////////////////////////////////////////////////
//BuffParam_UtumBirth

BuffParam_UtumBirth::BuffParam_UtumBirth()
{
	GConstructor();

	vsHor.ResetFloat(0.0f);
}
BuffParam_UtumBirth::~BuffParam_UtumBirth()
{
	GDestructor();
}



//////////////////////////////////////////////////////////////////////////
//CBuff_UtumBirth

BIND_BUFFPARAM(Buff_UtumBirth,BuffParam_UtumBirth,BuffArg_UtumBirth);

LevelBuffMask Buff_UtumBirth::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_UtumBirth)->GetUID());

	return mask;
}

void Buff_UtumBirth::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_UtumBirth *arg=(BuffArg_UtumBirth *)arg0;
	CLevelObj *owner=_GetOwner();

	if (_rec->Dur>0)
		_dur=_rec->Dur;

	if (TRUE)
	{
		i_math::vector3df dir=arg->posEnd-arg->posStart;
		dir.normalize();
		dir.toEuler();
		_eulerX=dir.x;
	}

	//启动一个Gesture
	BuffParam_UtumBirth*param=_rec->GetParam<BuffParam_UtumBirth>();
	if (param)
	{
		_dur=ANIMTICK_FROM_SECOND(param->dur);
		CUnit3D *unit=owner->GetUnit3D();
		if (unit)
		{
			CLevelGesture_UtumBirth *ges=Class_New2(CLevelGesture_UtumBirth);
			ges->Create(arg->posStart,arg->posEnd,owner->GetLevel()->GetGtm(),param);
			unit->SetGesture(ges);
		}
	}
}

void Buff_UtumBirth::_OnDestroy()
{
}



void Buff_UtumBirth::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_eulerX);
}
