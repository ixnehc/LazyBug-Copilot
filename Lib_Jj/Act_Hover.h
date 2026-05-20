#pragma once

#include "ActBase.h"



struct ActParam_Hover:public ActParam
{
	DEFINE_CLASS(ActParam_Hover);
	BEGIN_GOBJ_PURE(ActParam_Hover,1);

		GELEM_VAR_INIT(AnimTick,gap,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("间隔时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"每次更新中心点的间隔时间");
		GELEM_VAR_INIT(AnimTick,gapVary,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("间隔时间上下浮动",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"间隔时间的浮动值");
		GELEM_VAR_INIT(float,range,10.0f);
			GELEM_EDITVAR("移动范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"盘旋的中心点变动的范围");
		GELEM_VAR_INIT(float,rangeHover,10.0f);
			GELEM_EDITVAR("盘旋范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"盘旋时的范围");
		GELEM_VAR_INIT(float,twist,0.5f);
			GELEM_EDITVAR("水平扭动值",GVT_F,GSem(GSem_Float,"0,1,0.05"),"这个值越大,飞行时可以以越大的角度变换飞行方向");
		GELEM_VAR_INIT(float,twistVary,0.2f);
			GELEM_EDITVAR("水平扭动值浮动",GVT_F,GSem(GSem_Float,"0,1,0.05"),"水平扭动值的变化范围");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续盘旋多久,这个时间到后将停止盘旋");
		GELEM_VAR_INIT(AnimTick,durVary,ANIMTICK_FROM_SECOND(5.0f));
			GELEM_EDITVAR("持续时间浮动",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间的浮动值");

	END_GOBJ();


	AnimTick gap;
	AnimTick gapVary;
	float range;
	float rangeHover;
	float twist;
	float twistVary;
	AnimTick dur;
	AnimTick durVary;

};



//闲逛
class Act_Hover:public ActBase
{
public:
	DEFINE_ACT_CLASS(Act_Hover);

	Act_Hover()
	{
		_tNextMove=ANIMTICK_INFINITE;
		_bTimeUp=FALSE;
	}

	void Start(AnimTick t);
	virtual void Finish()	
	{	
	}
	void Update(AnimTick t);

	BOOL IsTimeUp()	{		return _bTimeUp;	}

protected:
	void _GenNewPos(AnimTick t);
	AnimTick _tNextMove;
	LevelPos _posCur;
	float _twist;//偏离航线的角度

	AnimTick _dur;
	AnimTick _tStart;
	BOOL _bTimeUp;

};


