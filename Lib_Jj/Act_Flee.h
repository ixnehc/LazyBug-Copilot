#pragma once

#include "ActBase.h"


struct ActParam_Flee:public ActParam
{
	DEFINE_CLASS(ActParam_Flee);
	float radius;
	float dist;
	AnimTick dur;

	BEGIN_GOBJ_PURE(ActParam_Flee,1);
		GELEM_VAR_INIT(float,radius,5.0f);
			GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0,20,0.1"),"敌人进入多远范围内开始逃跑");
		GELEM_VAR_INIT(float,dist,3.0f);
			GELEM_EDITVAR("逃跑距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"每次逃多远的距离");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续逃跑多长时间,0表示永久持续");
	END_GOBJ();
};


//逃跑
class Act_Flee:public ActBase
{
public:
	DEFINE_ACT_CLASS(Act_Flee);

	Act_Flee()
	{
		_tFlee=ANIMTICK_INFINITE;
		_tStart=0;
		_bTimeUp=FALSE;
	}

	void Start(AnimTick t);
	virtual void Finish()		{		}
	void Update(AnimTick t);

	BOOL IsTimeUp()	{		return _bTimeUp;	}

	AnimTick GetIdleTime(AnimTick t)//返回有多长时间没有找到逃避的对象了
	{
		return ANIMTICK_SAFE_MINUS(t,_tFlee);
	}

protected: 
	void _UpdateFlee(AnimTick t);
	AnimTick _tFlee;//上一次丢失攻击对象的时间
	AnimTick _tStart;
	BOOL _bTimeUp;

};

