#pragma once

#include "LevelSkill.h"

struct SkillParam_TeleportAttack:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_TeleportAttack);

		BEGIN_GOBJ_PURE(SkillParam_TeleportAttack,1);
			GELEM_VAR_INIT(AnimTick,delayTeleport,ANIMTICK_FROM_SECOND(0.5f));
				GELEM_EDITVAR("传送延迟",GVT_U,GSem(GSem_AnimTick,"0,5,0.05"),"传送延迟");
			GELEM_VAR_INIT(float,rangeDmg,1.0f);
				GELEM_EDITVAR("伤害范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"伤害范围");

		END_GOBJ();

	AnimTick delayTeleport;//Teleport之后
	float rangeDmg;//伤害范围

};


class Skill_TeleportAttack:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_TeleportAttack,31);

	Skill_TeleportAttack()
	{
		_tCasting=0;
		_bDamaged=FALSE;
		_bTeleported=FALSE;
	}

	enum OpCode
	{
	};

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_DefObj|1<<LevelSkillTarget::Target_Pos;
	}




protected:
	virtual void _OnStart();

	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}

	void _Update(AnimTick dt);

	BOOL _CalcTeleportPos(LevelPos &pos,LevelFace &face);

	AnimTick _tCasting;
	BOOL _bDamaged;
	BOOL _bTeleported;

};

