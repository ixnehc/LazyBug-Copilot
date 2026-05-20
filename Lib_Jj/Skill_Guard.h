#pragma once

#include "LevelSkill.h"


struct SkillParam_Guard:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Guard);

	BEGIN_GOBJ_PURE(SkillParam_Guard,1);

	END_GOBJ();

};


struct AttrNodeBase;
class Skill_Guard:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Guard,35);

	Skill_Guard()
	{
		_tCasting=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_None);
	}

	virtual BOOL NeedLockPick()	{		return FALSE;	}

	virtual void NotifyCasted()
	{
		_Finish();
	}

protected:
	virtual void _OnStart();
	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnBreak()	{		_Finish();	}
	virtual void _OnFinish()	{		_Finish();	}

	void _Finish();

	AnimTick _tCasting;


};

