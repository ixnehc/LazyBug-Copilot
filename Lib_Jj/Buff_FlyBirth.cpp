/********************************************************************
	created:	2012/03/15
	file base:	Buff_FlyBirth
	author:		cxi
	
	purpose:	出生的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_FlyBirth.h"

#include "LevelAttrs.h"

#include "LevelObjPauser.h"


//////////////////////////////////////////////////////////////////////////
//CLevelGesture_RavenBirth
void CLevelGesture_FlyBirth::Create(i_math::vector3df &posInit,i_math::vector2df &dir,GameTileMap *gtm,BuffParam_FlyBirth *param)
{
	_posInit=posInit;
	_dir=dir;
	_dir.normalize();
	_gtm=gtm;
	_param=param;
}

void CLevelGesture_FlyBirth::Update(CUnit3D *unit,float dt)
{
	_t+=dt;
	if(dt<=0.0f)
		return;

	if (!_bFinished)
	{
		float rate=_t/_param->dur;

		float ht=_param->vsVer.GetFloat(ANIMTICK_FROM_SECOND(rate));
		float off=_param->vsHor.GetFloat(ANIMTICK_FROM_SECOND(rate));

		i_math::vector3df pos;
		pos.y=_posInit.y+ht;
		pos.x=_posInit.x+_dir.x*off;
		pos.z=_posInit.z+_dir.y*off;

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
//BuffParam_RavenBirth

BuffParam_FlyBirth::BuffParam_FlyBirth()
{
	GConstructor();

	vsVer.ResetFloat(0.0f);
	vsHor.ResetFloat(0.0f);

}
BuffParam_FlyBirth::~BuffParam_FlyBirth()
{
	GDestructor();
}



//////////////////////////////////////////////////////////////////////////
//CBuff_RavenBirth

BIND_BUFFPARAM(Buff_FlyBirth,BuffParam_FlyBirth,BuffArg_FlyBirth);

LevelBuffMask Buff_FlyBirth::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_FlyBirth)->GetUID());

	return mask;
}

void Buff_FlyBirth::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_FlyBirth *arg=(BuffArg_FlyBirth *)arg0;
	CLevelObj *owner=_GetOwner();

	if (_rec->Dur>0)
		_dur=_rec->Dur;

	//启动一个Gesture
	BuffParam_FlyBirth*param=_rec->GetParam<BuffParam_FlyBirth>();
	if (param)
	{
		CUnit3D *unit=owner->GetUnit3D();
		if (unit)
		{
			CLevelGesture_FlyBirth *ges=Class_New2(CLevelGesture_FlyBirth);
			ges->Create(arg->posInit,arg->dir,owner->GetLevel()->GetGtm(),param);
			unit->SetGesture(ges);
		}
	}

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

void Buff_FlyBirth::_OnDestroy()
{
	CLevelObj *owner=_GetOwner();

	Safe_Class_Delete(_op);
}



void Buff_FlyBirth::_WriteData(CBitPacket *dp)
{
}
