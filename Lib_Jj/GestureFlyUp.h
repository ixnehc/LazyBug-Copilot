#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelGesture.h"


struct GestureParam_FlyUp:public LevelGestureParam
{
	DEFINE_GESTUREPARAM_CLASS(GestureParam_FlyUp);

	BEGIN_GOBJ_PURE(GestureParam_FlyUp,1);

		GELEM_VAR_INIT(float,tDelay,0.2f);
			GELEM_EDITVAR("升起延迟",GVT_F,GSem(GSem_Float,"0,100,0.1"),"升起前延迟多久");

		GELEM_VAR_INIT(float,dur,2.0f);
			GELEM_EDITVAR("升起时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"升起时间");

		GELEM_VAR_INIT(float,lift,10.0f);
			GELEM_EDITVAR("升高距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"起飞时升高多少距离");
		GELEM_VAR_INIT(float,shift,10.0f);
			GELEM_EDITVAR("水平距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"起飞时偏离多少距离");
	END_GOBJ();

	float tDelay;
	float dur;
	float lift;
	float shift;

};


class Gesture_FlyUp:public CLevelGesture
{
public:
	DEFINE_CLASS(Gesture_FlyUp);

	Gesture_FlyUp()
	{
		_lift=0.0f;
		_t=0.0f;
	}

	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{	}


protected:
	virtual void _OnCreate();
	virtual void _OnDestroy();

	float _t;
	float _lift;

	i_math::vector3df _step;


};