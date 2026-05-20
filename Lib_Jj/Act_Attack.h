#pragma once

#include "ActBase.h"

struct ActParam_Attack:public ActParam
{
	DEFINE_CLASS(ActParam_Attack);
	RecordID idSkill;
	float radius;
	float distKeep;
	AnimTick durCheckEscape;

	BEGIN_GOBJ_PURE(ActParam_Attack,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("使用技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(float,radius,5.0f);
			GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0,20,0.1"),"搜索敌人时的侦测半径");
		GELEM_VAR_INIT(float,distKeep,0.0f);
			GELEM_EDITVAR("保持距离",GVT_F,GSem(GSem_Float,"0,20,0.1"),"在攻击敌人时,保持多远的距离");
		GELEM_VAR_INIT(AnimTick,durCheckEscape,ANIMTICK_FROM_SECOND(5.0f));
			GELEM_EDITVAR("保持距离检测间隔",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"多长时间检测一次保持距离,单位为秒");	
	END_GOBJ();
};



//攻击
class Act_Attack:public ActBase
{
public:
	DEFINE_ACT_CLASS(Act_Attack);

	Act_Attack()
	{
		_target=NULL;
		_tLoseTarget=ANIMTICK_INFINITE;
		_tLastCheckEscape=0;
		_bEscaping=FALSE;
		_methods=LevelMoveMethodMask_None;
	}

	void Start(AnimTick t,float radius);
	virtual void Finish();
	void Update(AnimTick t);

	AnimTick GetIdleTime(AnimTick t)//返回有多长时间没有找到对象攻击了
	{
		return ANIMTICK_SAFE_MINUS(t,_tLoseTarget);
	}


protected:

	BOOL _bEscaping;

	CLevelObj *_target;
	AnimTick _tLoseTarget;//上一次丢失攻击对象的时间
	AnimTick _tLastCheckEscape;//上一次检查要不要escape的时间

	LevelMoveMethodMask _methods;

};

