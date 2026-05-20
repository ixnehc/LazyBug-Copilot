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


struct AIParam_RockScorpion:public LevelAIParam
{
	DEFINE_AIPARAM_CLASS(AIParam_RockScorpion);

	BEGIN_GOBJ_PURE(AIParam_RockScorpion,1);

		GELEM_OBJ(ActParam_Stroll,paramStroll);
			GELEM_EDITOBJ("闲逛参数","闲逛参数");

		GELEM_OBJ(ActParam_Attack,paramAttack);
			GELEM_EDITOBJ("攻击参数","攻击参数");

		GELEM_OBJ(ActParam_Flee,paramFlee);
			GELEM_EDITOBJ("逃跑参数","逃跑参数");

		GELEM_OBJ(ActParam_Reside,paramReside);
			GELEM_EDITOBJ("进洞参数","进洞参数");

		GELEM_VAR_INIT(float,rangeSight,6.0f);
			GELEM_EDITVAR("警戒视野范围",GVT_F,GSem(GSem_Float,"0.1,20,0.1"),"多大范围内发现有敌人,开始进入战斗状态");

		GELEM_VAR_INIT(RecordID,enterhole,RecordID_Invalid);
			GELEM_EDITVAR("进洞技能",GVT_U,GSem(GSem_RecordID,"skills"),"进洞技能");

	END_GOBJ();

	ActParam_Stroll paramStroll;
	ActParam_Attack paramAttack;
	ActParam_Flee paramFlee;
	ActParam_Reside paramReside;

	float rangeSight;

	RecordID enterhole;

};


class AI_RockScorpion:public CLevelAI
{
public:
	DEFINE_CLASS(AI_RockScorpion);

	enum State
	{
		Aggressive=CustomStart,//攻击型
		Flee,//逃跑型
		Residing,//撤退型
		Resided,//已撤退
	};

	AI_RockScorpion()
	{
	}

protected:
	virtual void _OnCreate();
	virtual void _OnDestroy();

	virtual void _OnUpdate(AnimTick t);

	void _DecideBattleState();//决定单位战斗时采取什么状态


	float _mirale;




};