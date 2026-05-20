#pragma once

#include "LevelSkill.h"



struct SkillParam_Sweep:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Sweep);

	BEGIN_GOBJ_PURE(SkillParam_Sweep,1);

		GELEM_VAR_INIT(float,fov,120.0f);
			GELEM_EDITVAR("扇形角度范围",GVT_F,GSem(GSem_Float,"0.0,360.0,1.0"),"扇形角度范围");

	END_GOBJ();

	float fov;
};


class Skill_Sweep:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Sweep,5);

	Skill_Sweep()
	{
		_bDamage=FALSE;

		_tCasting=0;
	}

	enum OpCode
	{
	};

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim)|(1<<LevelSkillTarget::Target_DefObj);
	}



protected:
	virtual void _OnStart();
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}
	virtual void _OnUpdate(AnimTick dt);

	void _UpdateDamage(AnimTick dt);

	AnimTick _tCasting;

	BOOL _bDamage;



};

