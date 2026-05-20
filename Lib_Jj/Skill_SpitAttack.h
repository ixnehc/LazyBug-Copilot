#pragma once

#include "LevelSkill.h"

struct SkillParam_SpitAttack:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_SpitAttack);

		BEGIN_GOBJ_PURE(SkillParam_SpitAttack,1);

			GELEM_VAR_INIT(float,spd,10.0f);
				GELEM_EDITVAR("喷射液体飞行速度",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"喷射液体飞行速度,单位为米/秒");

		END_GOBJ();

	float spd;

};


class Skill_SpitAttack:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_SpitAttack,17);

	Skill_SpitAttack()
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
	virtual void _OnUpdate(AnimTick dt);

	void _UpdateDamage(AnimTick dt);

	virtual void _OnStart();
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}

	CLevelSkillCasting _casting;


	AnimTick _tHit;

};

