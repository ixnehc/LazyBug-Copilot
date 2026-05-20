#pragma once

#include "ActBase.h"



extern const char *GetAgentsSemConstraint();

struct ActParam_FlyUp:public ActParam
{
	DEFINE_CLASS(ActParam_FlyUp);
	RecordID idSkill;
	AnimTick dur;

	BEGIN_GOBJ_PURE(ActParam_FlyUp,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("起飞使用的技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("起飞时间",GVT_U,GSem(GSem_AnimTick,"0.0f,100,0.1"),"起飞过程持续多久");
	END_GOBJ();
};




//攻击
class Act_FlyUp:public ActBase
{
public:
	DEFINE_ACT_CLASS(Act_FlyUp);

	Act_FlyUp()
	{
		_result=A_Pending;
		_tStart=0;
		_tStartFly=ANIMTICK_INFINITE;
	}

	void Start(AnimTick t);
	virtual void Finish();
	void Update(AnimTick t);

	AResult GetResult()	{		return _result;	}

protected:

	AResult _result;
	AnimTick _tStart;
	AnimTick _tStartFly;



};

