#pragma once

#include "ActBase.h"



struct ActParam_Stroll:public ActParam
{
	DEFINE_CLASS(ActParam_Stroll);
	BEGIN_GOBJ_PURE(ActParam_Stroll,1);

		GELEM_VAR_INIT(AnimTick,gap,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("间隔时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"每次移动的间隔时间");
		GELEM_VAR_INIT(AnimTick,gapVary,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("间隔时间上下浮动",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"每次移动的间隔时间的浮动值");
		GELEM_VAR_INIT(float,range,2.0f);
			GELEM_EDITVAR("移动距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"每次移动的距离的范围");
	END_GOBJ();


	AnimTick gap;
	AnimTick gapVary;
	float range;

};



//闲逛
class Act_Stroll:public ActBase
{
public:
	DEFINE_ACT_CLASS(Act_Stroll);

	Act_Stroll()
	{
		_tNextMove=ANIMTICK_INFINITE;
	}

	void Start(AnimTick t);
	virtual void Finish()	
	{	
	}
	void Update(AnimTick t);

protected:
	AnimTick _tNextMove;

};

// class Act_Attack
// {
// public:
// 	DEFINE_CLASS(Act_Attack);
// 
// 	void Start(CLevelObj *owner,RecordID idSkill,CLevelObj *lo);
// 	void Finish();
// 	void Update();
// 
// };