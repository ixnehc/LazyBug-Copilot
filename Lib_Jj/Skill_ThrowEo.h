#pragma once

#include "LevelSkill.h"

#define MAX_THROWS 32

struct EoThrow
{
	LevelPos pos;
	AnimTick dur;//多久能扔到
	BOOL bReached;
};



struct SkillParam_ThrowEo:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_ThrowEo);

	BEGIN_GOBJ_PURE(SkillParam_ThrowEo,1);

		GELEM_VAR_INIT(float,Speed,10.0f);
			GELEM_EDITVAR("投掷物飞行速度",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"投掷物飞行速度,单位为米/秒");
		GELEM_VAR_INIT(float,Range,15.0f);
			GELEM_EDITVAR("投掷物作用距离",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"投掷物飞出多远后会无效,单位为米");
		GELEM_VAR_INIT(int,Count,1);
			GELEM_EDITVAR("投掷物的个数",GVT_S,GSem(GSem_Interger,"缺省,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"投掷物个数");
		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果Proto",GVT_Bx8,GSem_ProtoPath,"投掷物效果的Proto");

	END_GOBJ();

	float Speed;
	float Range;
	int Count;
	unsigned __int64 idEffect;
};



class Skill_ThrowEo:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_ThrowEo,18)

	Skill_ThrowEo()
	{

		_nThrow=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Pos)|(1<<LevelSkillTarget::Target_DefObj);
	}

protected:

	void _ClearThrows();

	virtual void _OnStart();
	virtual void _OnFinish();
	virtual void _OnUpdate(AnimTick dt);


	void _UpdateThrows(AnimTick dt);

	EoThrow _throws[MAX_THROWS];
	DWORD _nThrow;

	CLevelSkillCasting _casting;



};

