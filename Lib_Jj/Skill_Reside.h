#pragma once

#include "LevelSkill.h"

struct SkillParam_Reside:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Reside);

	BEGIN_GOBJ_PURE(SkillParam_Reside,1);
		GELEM_VAR_INIT(RecordID,buffEnter,RecordID_Invalid);
			GELEM_EDITVAR("进入Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"单位进入Agent时的Buff");
	END_GOBJ();

	RecordID buffEnter;
};


class Skill_Reside:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Reside,10);

	Skill_Reside()
	{
	}

	enum OpCode
	{
	};

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_DefObj);
	}

	virtual BOOL PreInitStartCheck(CLevelObj *owner,LevelRecordSkill *rec,LevelSkillTarget &target);


protected:

	virtual void _OnStart();


};

