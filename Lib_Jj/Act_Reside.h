#pragma once

#include "ActBase.h"



extern const char *GetAgentsSemConstraint();

struct ActParam_Reside:public ActParam
{
	DEFINE_CLASS(ActParam_Reside);
	ClassUID uidAgent;
	RecordID idSkill;
	float radius;
	AnimTick tSearching;

	BEGIN_GOBJ_PURE(ActParam_Reside,1);
		GELEM_VAR_INIT(ClassUID,uidAgent,0);
			GELEM_EDITVAR("驻留对象类型",GVT_U,GSem(GSem_Interger,GetAgentsSemConstraint()),"搜寻那种驻留对象");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("驻留使用的技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(float,radius,12.0f);
			GELEM_EDITVAR("侦测半径",GVT_F,GSem(GSem_Float,"0,20,0.1"),"搜索驻留对象时的侦测半径");
		GELEM_VAR_INIT(AnimTick,tSearching,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("搜索时间",GVT_U,GSem(GSem_AnimTick,"1,100,0.1"),"搜索多长时间找不到算失败,单位为秒");
	END_GOBJ();
};




//攻击
class Act_Reside:public ActBase
{
public:
	DEFINE_ACT_CLASS(Act_Reside);

	Act_Reside()
	{
		_result=A_Pending;
		_tStart=0;
		_token=LevelObjSeatToken_Invalid;
		_target=NULL;
	}

	void Start(AnimTick t,CLevelObj *target,LevelObjSeatToken token);
	virtual void Finish();
	void Update(AnimTick t);

	AResult GetResult()	{		return _result;	}

protected:

	void _ClearTarget();
	CLevelObj *_target;
	LevelObjSeatToken _token;
	AResult _result;
	AnimTick _tStart;



};

