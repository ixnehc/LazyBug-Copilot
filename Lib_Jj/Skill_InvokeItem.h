#pragma once

#include "LevelSkill.h"

struct SkillParam_InvokeItem:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_InvokeItem);

	BEGIN_GOBJ_PURE(SkillParam_InvokeItem,1);

	END_GOBJ();

};


class Skill_InvokeItem:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_InvokeItem,9);

	Skill_InvokeItem()
	{
	}

	enum OpCode
	{
	};

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_DefObj;
	}



protected:

	virtual void _OnStart();
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}

};

