#pragma once

#include "LevelSkill.h"

struct SkillParam_MeleeAttack:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_MeleeAttack);

		BEGIN_GOBJ_PURE(SkillParam_MeleeAttack,1);
			GELEM_VARVECTOR_INIT(AnimTick,hitsEx,ANIMTICK_FROM_SECOND(0.1f));
				GELEM_EDITVAR("额外的命中延迟",GVT_U,GSem(GSem_AnimTick,"0,100,0.01"),
						"上次命中隔多久后发生,以秒为单位,这个数值由Server使用");
			GELEM_VAR_INIT(float,rangeDmg,1.0f);
				GELEM_EDITVAR("伤害范围",GVT_F,GSem(GSem_Float,"0.1,100000.0,0.05"),"伤害范围");

		END_GOBJ();

	std::vector<AnimTick> hitsEx;//额外的Hit
	float rangeDmg;//伤害范围

};


class Skill_MeleeAttack:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_MeleeAttack,1);

	Skill_MeleeAttack()
	{
		_nDamages=0;
		_nToDamages=0;
		_tCasting=0;
		_tLastHit=0;
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

	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}

	void _UpdateDamage(AnimTick dt);

	AnimTick _tCasting;

	BYTE _nDamages;
	BYTE _nToDamages;
	AnimTick _tLastHit;

};

