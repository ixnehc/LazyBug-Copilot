#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObj.h"
#include "LevelAI.h"

#include "Act_Attack.h"
#include "Act_Flee.h"
#include "Act_Stroll.h"
#include "Act_FlyUp.h"
#include "Act_Hover.h"
#include "Act_FlyDown.h"
#include "Act_AttackOnce3D.h"



struct AIParam_FlyingRat:public LevelAIParam
{
	DEFINE_AIPARAM_CLASS(AIParam_FlyingRat);


	BEGIN_GOBJ_PURE(AIParam_FlyingRat,1);

		GELEM_OBJ(ActParam_Stroll,stroll);
			GELEM_EDITOBJ("闲逛参数","闲逛参数");

		GELEM_OBJ(ActParam_Attack,attackGround);
			GELEM_EDITOBJ("地面攻击参数","AIParam_Goblin");

		GELEM_OBJ(ActParam_Flee,flee);
			GELEM_EDITOBJ("逃跑参数","逃跑参数");

		GELEM_OBJ(ActParam_FlyUp,flyup);
			GELEM_EDITOBJ("起飞参数","起飞参数");

		GELEM_OBJ(ActParam_Hover,hover);
			GELEM_EDITOBJ("飞行闲逛参数","飞行闲逛参数");

		GELEM_OBJ(ActParam_AttackOnce3D,attackFlying);
			GELEM_EDITOBJ("俯冲攻击参数","俯冲攻击参数");

		GELEM_OBJ(ActParam_FlyDown,flydown);
			GELEM_EDITOBJ("降落参数","降落参数");

		GELEM_VAR_INIT(float,rangeSight,6.0f);
			GELEM_EDITVAR("警戒视野范围",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"多大范围内发现有敌人,开始进入战斗状态");

		GELEM_VAR_INIT(float,rangeFlyingSight,15.0f);
			GELEM_EDITVAR("警戒视野范围(飞行时)",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"(飞行时)多大范围内发现有敌人,开始进入战斗状态");

	END_GOBJ();

	ActParam_Stroll stroll;
	ActParam_Attack attackGround;
	ActParam_Flee flee;
	ActParam_FlyUp flyup;
	ActParam_Hover hover;
	ActParam_AttackOnce3D attackFlying;
	ActParam_FlyDown flydown;

	float rangeSight;
	float rangeFlyingSight;

};


class AI_FlyingRat:public CLevelAI
{
public:
	DEFINE_CLASS(AI_FlyingRat);

	enum State
	{
		Aggressive=CustomStart,//攻击型
		Flee,//逃跑型
		FlyUp,
		FlyingIdle,
		FlyDown,
		FlyingAttack,
	};


	AI_FlyingRat()
	{
	}

protected:

	virtual void _OnCreate();
	virtual void _OnDestroy();
	virtual void _OnInitialUpdate();

	virtual void _OnUpdate(AnimTick t);

	virtual void OnEvent(LevelEvent &e);


};