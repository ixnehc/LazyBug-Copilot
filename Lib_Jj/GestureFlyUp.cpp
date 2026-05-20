/********************************************************************
	created:	2012/11/10 
	author:		cxi
	
	purpose:	起飞的Gesture
*********************************************************************/
#include "stdh.h"

#include "GestureFlyUp.h"

#include "Random/Random.h"

#include "LevelObj.h"
#include "Level.h"


BIND_GESTUREPARAM(Gesture_FlyUp,GestureParam_FlyUp);


void Gesture_FlyUp::_OnCreate()
{
	GestureParam_FlyUp *param=(GestureParam_FlyUp *)_core.param;
	float angle=CSysRandom::RandRange(0.0f,2.0f*(float)i_math::Pi);
	i_math::vector2df dir;
	dir.x=cosf(angle);
	dir.y=sinf(angle);

	_step.x=param->shift*dir.x;
	_step.z=param->shift*dir.y;
	_step.y=param->lift;

}

void Gesture_FlyUp::_OnDestroy()
{

}


void Gesture_FlyUp::Update(CUnit3D *unit,float dt)
{
	GestureParam_FlyUp *param=(GestureParam_FlyUp *)_core.param;
	if (_t<param->tDelay)
	{
		_t+=dt;
		return;
	}
	_t+=dt;
	float t=_t-param->tDelay;
	i_math::vector3df pos=unit->_pos;
	pos+=_step*(dt/param->dur);

	unit->_ClampGround(pos);

	unit->_vel=(pos-unit->_pos)/dt;
	unit->_pos=pos;
	if (_t>=param->tDelay+param->dur)
		_bFinished=TRUE;
}

