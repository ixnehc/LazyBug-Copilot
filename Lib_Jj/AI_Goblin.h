#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObj.h"
#include "LevelAI.h"

#include "Act_Attack.h"
#include "Act_Flee.h"
#include "Act_Stroll.h"
#include "Act_Reside.h"



struct AIParam_Goblin:public LevelAIParam
{
	DEFINE_AIPARAM_CLASS(AIParam_Goblin);


	BEGIN_GOBJ_PURE(AIParam_Goblin,1);

		GELEM_VAR_INIT(RecordID,idRod,RecordID_Invalid);
			GELEM_EDITVAR("棒子的Item",GVT_U,GSem(GSem_RecordID,"items"),"使用什么棒子");

		GELEM_VAR_INIT(RecordID,idSpear,RecordID_Invalid);
			GELEM_EDITVAR("标枪的Item",GVT_U,GSem(GSem_RecordID,"items"),"使用什么标枪");

		GELEM_OBJ(ActParam_Stroll,stroll);
			GELEM_EDITOBJ("闲逛参数","闲逛参数");

		GELEM_OBJ(ActParam_Attack,attackUnarmed);
			GELEM_EDITOBJ("徒手攻击参数","徒手攻击参数");
		GELEM_OBJ(ActParam_Attack,attackRod);
			GELEM_EDITOBJ("棒子攻击参数","棒子攻击参数");
		GELEM_OBJ(ActParam_Attack,attackSpear);
			GELEM_EDITOBJ("标枪攻击参数","标枪攻击参数");
		GELEM_OBJ(ActParam_Attack,attackWT);
			GELEM_EDITOBJ("岗楼标枪攻击参数","岗楼标枪攻击参数");

		GELEM_OBJ(ActParam_Reside,resideWT);
			GELEM_EDITOBJ("爬岗楼参数","爬岗楼参数");

		GELEM_OBJ(ActParam_Flee,flee);
			GELEM_EDITOBJ("逃跑参数","逃跑参数");

		GELEM_VAR_INIT(float,rangeSight,6.0f);
			GELEM_EDITVAR("警戒视野范围",GVT_F,GSem(GSem_Float,"0.1,20,0.1"),"多大范围内发现有敌人,开始进入战斗状态");

	END_GOBJ();

	ActParam_Stroll stroll;
	ActParam_Attack attackUnarmed;
	ActParam_Attack attackRod;
	ActParam_Attack attackSpear;
	ActParam_Attack attackWT;
	ActParam_Flee flee;
	ActParam_Reside resideWT;//WatchTower

	RecordID idRod;
	RecordID idSpear;


	float rangeSight;

};


class AI_Goblin:public CLevelAI
{
public:
	DEFINE_CLASS(AI_Goblin);

	enum State
	{
		Aggressive=CustomStart,//攻击型
		Flee,//逃跑型
		ResidingWT,
		ResidedWT,
	};

	enum Posture
	{
		Unarmed=0,
		Rod,
		Spear,
	};


	AI_Goblin()
	{
		_posture=Unarmed;
	}

protected:

	virtual void _OnCreate();
	virtual void _OnDestroy();
	virtual void _OnInitialUpdate();

	virtual void _OnUpdate(AnimTick t);

	virtual void OnEvent(LevelEvent &e);

	Posture _posture;



};