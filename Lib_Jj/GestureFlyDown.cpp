/********************************************************************
	created:	2012/11/10 
	author:		cxi
	
	purpose:	降落的Gesture
*********************************************************************/
#include "stdh.h"

#include "GestureFlyDown.h"

#include "LevelObj.h"

#include "unitmgr/UnitMgr.h"


BIND_GESTUREPARAM(Gesture_FlyDown,GestureParam_FlyDown);


void Gesture_FlyDown::_OnCreate()
{
	KeySet_Define(&_ks,KT_Pos,0);
	_ks.SetKeyCount(3);
}

void Gesture_FlyDown::_OnDestroy()
{
	_ks.Clean();
}

void Gesture_FlyDown::SetTarget(LevelPos3D posDescend)
{
	GestureParam_FlyDown *param=(GestureParam_FlyDown *)_core.param;

	Key_pos *k0,*k1,*k2;
	k0=(Key_pos *)_ks.GetKey(0);
	k0->v=_owner->GetFramePos3D();
	k0->t=0;

	k1=(Key_pos *)_ks.GetKey(1);
	k1->v=posDescend;
	k1->t=ANIMTICK_FROM_SECOND(param->durDescend);

	if (TRUE)
	{
		i_math::vector3df dir=(k1->v-k0->v);
		i_math::vector2df dir2(dir.x,dir.z);
		dir2.normalize();
		k2=(Key_pos *)_ks.GetKey(2);
		k2->v.x=k1->v.x+dir2.x*param->rangeLand;
		k2->v.z=k1->v.z+dir2.y*param->rangeLand;
		k2->v.y=k1->v.y;
		k2->t=ANIMTICK_FROM_SECOND(param->durDescend+param->durLand);
	}

}



void Gesture_FlyDown::Update(CUnit3D *unit,float dt)
{
	_t+=dt;
	Key_pos k;
	_ks.CalcKey(ANIMTICK_FROM_SECOND(_t),&k);


	i_math::vector3df vel=(k.v-unit->_pos)/dt;
	unit->_pos=k.v;
	unit->_vel=vel;

	unit->_ClampGround(unit->_pos);

	if (_t>=ANIMTICK_TO_SECOND(_ks.GetEndTick())-0.2f)
		_bFinished=TRUE;
	GestureParam_FlyDown *param=(GestureParam_FlyDown *)_core.param;
	if (_t>=param->durDescend)
		_bLanded=TRUE;
}

void Gesture_FlyDown::Update(CUnit*unit,float dt)
{
	_t+=dt;
	Key_pos k;
	_ks.CalcKey(ANIMTICK_FROM_SECOND(_t),&k);

	unit->_pos.set(k.v.x,k.v.z);

	if (_t>=ANIMTICK_TO_SECOND(_ks.GetEndTick())-0.2f)
		_bFinished=TRUE;
	GestureParam_FlyDown *param=(GestureParam_FlyDown *)_core.param;
	if (_t>=param->durDescend)
		_bLanded=TRUE;

}

