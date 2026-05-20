#pragma once

#include "LevelSkill.h"

struct SkillParam_CastSignal:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_CastSignal);

	BEGIN_GOBJ_PURE(SkillParam_CastSignal,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "信号名称", GVT_U, GSem(GSem_StringID,"信号名称"), "施放的信号名称" );

		GELEM_VAR_INIT(float,radius,10.0f);
			GELEM_EDITVAR("施放范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"施放范围");

	END_GOBJ();

	StringID nm;
	float radius;


};


class Skill_CastSignal:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_CastSignal,20);

	Skill_CastSignal()
	{
	}

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_None|1<<LevelSkillTarget::Target_DefObj;
	}




protected:
	virtual void _OnStart();

	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}

	void _UpdateDamage(AnimTick dt);

	CLevelSkillCasting _casting;
	

};

