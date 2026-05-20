#pragma once

#include "LevelSkill.h"

struct SkillParam_MeleeAbsorb:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_MeleeAbsorb);

		BEGIN_GOBJ_PURE(SkillParam_MeleeAbsorb,1);

		END_GOBJ();


};


class Skill_MeleeAbsorb:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_MeleeAbsorb,39);

	Skill_MeleeAbsorb()
	{
		_bDealed=FALSE;
	}

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_DefObj;
	}




protected:
	virtual void _OnStart();

	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}

	void _UpdateDamage(AnimTick dt);

	AnimTick _tCasting;
	BOOL _bDealed;


};

