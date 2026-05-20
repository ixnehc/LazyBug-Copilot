#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "anim/KeySet.h"

#include "LevelDefines.h"

#include "LevelGesture.h"

#include "valueset/valueset.h"


struct LevelGestureParam_Jump:public LevelGestureParam
{
	DEFINE_GESTUREPARAM_CLASS(LevelGestureParam_Jump);

	BEGIN_GOBJ_PURE(LevelGestureParam_Jump,1);

		GELEM_VAR_INIT(AnimTick,durJumpStart,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("起跳时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间");
		GELEM_VAR_INIT(AnimTick,durJump,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("跳跃时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间");
		GELEM_VAR_INIT(AnimTick,durJumpEnd,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("落地时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间");
		GELEM_VAR_INIT(float,speedRot,180.0f);
			GELEM_EDITVAR("转身速度",GVT_F,GSem(GSem_Float,"0.1,1080.0,1.0"),"转身速度");
	END_GOBJ();

	AnimTick durJumpStart;
	AnimTick durJump;
	AnimTick durJumpEnd;
	float speedRot;
};


class CLevelGesture_Jump:public CLevelGesture
{
public:
	DEFINE_GESTURE_CLASS(CLevelGesture_Jump,2);

	CLevelGesture_Jump()
	{
		_idTarget=LevelObjID_Invalid;
	}

	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt);

	virtual void WriteFirstSync(CBitPacket *bp);
	virtual BOOL WriteSync(CBitPacket *bp);

	static void CalcXfm(LevelGestureCore &core,float dt,LevelPos &posJump,LevelPos &posTarget,LevelPos &posCur,LevelFace &faceCur);
	static BOOL CheckFinished(LevelGestureCore &core);


protected:
	virtual void _OnCreate();
	virtual void _OnDestroy();

	void _UpdateTargetPos();

	LevelPos _posJump;
	LevelObjID _idTarget;
	LevelPos _posTarget;
	LevelPos _posTargetLastSync;

};