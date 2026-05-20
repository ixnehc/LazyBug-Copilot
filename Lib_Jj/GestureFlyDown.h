#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "anim/KeySet.h"

#include "LevelDefines.h"

#include "LevelGesture.h"


struct GestureParam_FlyDown:public LevelGestureParam
{
	DEFINE_GESTUREPARAM_CLASS(GestureParam_FlyDown);

	BEGIN_GOBJ_PURE(GestureParam_FlyDown,1);

		GELEM_VAR_INIT(float,durDescend,2.0f);
			GELEM_EDITVAR("下落时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"下落的时间");
		GELEM_VAR_INIT(float,durLand,0.4f);
			GELEM_EDITVAR("落地时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"落地后的时间");

		GELEM_VAR_INIT(float,rangeDescend,4.0f);
			GELEM_EDITVAR("下落横向距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"下落的水平方向移动距离");

		GELEM_VAR_INIT(float,htDescend,5.0f);
			GELEM_EDITVAR("下落纵向距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"下落的竖直方向移动距离");

		GELEM_VAR_INIT(float,rangeLand,1.0f);
			GELEM_EDITVAR("落地后移动距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"落地后的移动距离");

	END_GOBJ();

	float durDescend;//多少时间落地
	float durLand;//落地后往前多长时间
	float htDescend;//
	float rangeDescend;//下落的横向距离
	float rangeLand;//落地后往前的距离

};


class Gesture_FlyDown:public CLevelGesture
{
public:
	DEFINE_CLASS(Gesture_FlyDown);

	Gesture_FlyDown()
	{
		_bLanded=FALSE;
		_t=0.0f;
	}
	void SetTarget(LevelPos3D posDescend);

	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt);
	virtual BOOL IsLanded()	{		return _bLanded;	}

protected:
	virtual void _OnCreate();
	virtual void _OnDestroy();

	BOOL _bLanded;

	float _t;

	KeySet _ks;

};