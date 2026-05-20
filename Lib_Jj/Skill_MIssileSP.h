#pragma once

#include "LevelSkill.h"

struct SkillParam_MissileSP:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_MissileSP);

	BEGIN_GOBJ_PURE(SkillParam_MissileSP,1);
		GELEM_VAR_INIT(float,Speed,10.0f);
			GELEM_EDITVAR("飞行速度",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"飞行速度,单位为米/秒");

	END_GOBJ();

	float Speed;
};




class Skill_MissileSP:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_MissileSP,7);

	Skill_MissileSP()
	{
		_dur=ANIMTICK_INFINITE;
	}

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_DefObj;
	}


	virtual void Start();


protected:

	virtual void _OnUpdate(AnimTick dt);

	AnimTick _dur;


};

