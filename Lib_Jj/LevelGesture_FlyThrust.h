#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "anim/KeySet.h"

#include "LevelDefines.h"

#include "LevelGesture.h"

#include "valueset/valueset.h"


struct LevelGestureParam_FlyThrust:public LevelGestureParam
{
	DEFINE_GESTUREPARAM_CLASS(LevelGestureParam_FlyThrust);

	LevelGestureParam_FlyThrust()
	{
		GConstructor();
		ver.ResetFloat(1.0f);
		hor.ResetFloat(0.0f);
	}

	~LevelGestureParam_FlyThrust()
	{
		GDestructor();
	}

	BEGIN_GOBJ(LevelGestureParam_FlyThrust,1);

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间");
		GELEM_VAR_INIT(float ,radiusStopFollow,1.0f);
			GELEM_EDITVAR("目标在多大范围内后停止跟踪",GVT_F,GSem(GSem_Float,"0,100,0.1"),"目标在多大范围内后停止跟踪");
		GELEM_OBJVAR( ValueSet, ver);
			GELEM_EDITOBJ_EX("竖直方向曲线","竖直方向曲线",GSem( GSem_Unknown, "0,0,10,2" ));
		GELEM_OBJVAR( ValueSet, hor);
			GELEM_EDITOBJ_EX("水平方向曲线","水平方向曲线",GSem( GSem_Unknown, "0,0,10,1" ));
		GELEM_VAR_INIT(float,speedRot,180.0f);
			GELEM_EDITVAR("转身速度",GVT_F,GSem(GSem_Float,"0.1,3600.0,0.05"),"转身朝向目标的速度(度/秒)");
	END_GOBJ();

	ValueSet ver;
	ValueSet hor;
	AnimTick dur;
	float radiusStopFollow;
	float speedRot;
};


class CLevelGesture_FlyThrust:public CLevelGesture
{
public:
	DEFINE_GESTURE_CLASS(CLevelGesture_FlyThrust,1);

	CLevelGesture_FlyThrust()
	{
		_bStopFollow=FALSE;
	}

	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt);

	virtual void WriteFirstSync(CBitPacket *bp);
	virtual BOOL WriteSync(CBitPacket *bp);

	static void CalcXfm(LevelGestureCore &core,float dt,LevelPos3D &posTarget,LevelPos3D &posCur,LevelFace &faceCur);
	static BOOL CheckFinished(LevelGestureCore &core);


protected:
	virtual void _OnCreate();
	virtual void _OnDestroy();

	void _UpdateTargetPos(LevelPos &pos);

	BOOL _bStopFollow;

	LevelPos3D _pos3DTargetLastSync;
	LevelPos3D _pos3DTarget;

};