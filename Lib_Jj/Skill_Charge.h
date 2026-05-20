#pragma once

#include "LevelSkill.h"

struct SkillParam_Charge:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Charge);

	BEGIN_GOBJ_PURE(SkillParam_Charge,1);
		GELEM_VAR_INIT(float,Speed,10.0f);
			GELEM_EDITVAR("冲刺速度",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"冲刺速度,单位为米/秒");
		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果Proto",GVT_Bx8,GSem_ProtoPath,"冲刺效果的Proto");

	END_GOBJ();

	float Speed;
	unsigned __int64 idEffect;
};


class Skill_Charge:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Charge,4);

	Skill_Charge()
	{
		_dur=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim)|(1<<LevelSkillTarget::Target_DefObj);
	}



protected:
	virtual void _OnStart();
	virtual void _OnBreak()		{		_Finish();	}
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}

	float _GetSpeed();


	void _Finish();

	AnimTick _dur;



};

