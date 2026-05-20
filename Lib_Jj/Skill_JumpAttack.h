#pragma once

#include "LevelSkill.h"

struct SkillParam_JumpAttack:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_JumpAttack);

	BEGIN_GOBJ_PURE(SkillParam_JumpAttack,1);
		GELEM_VAR_INIT(AnimTick,durJump,ANIMTICK_FROM_SECOND(0.7f));
			GELEM_EDITVAR("跳跃时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"跳跃时间");
		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果Proto",GVT_Bx8,GSem_ProtoPath,"冲刺效果的Proto");
		GELEM_VAR_INIT(float,rangeDmg,1.0f);
			GELEM_EDITVAR("伤害范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"伤害范围");
	END_GOBJ();

	AnimTick durJump;
	unsigned __int64 idEffect;
	float rangeDmg;
};


class Skill_JumpAttack:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_JumpAttack,27);

	Skill_JumpAttack()
	{
		_tCasting=0;
		_bJumpReach=FALSE;
		_bDealed=FALSE;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Pos)|(1<<LevelSkillTarget::Target_DefObj);
	}

	virtual BOOL IsImmune()	{		return FALSE;}


protected:
	virtual void _OnStart();
	virtual void _OnBreak()		{		_Finish();	}
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}

	float _GetSpeed();


	void _Finish();

	BOOL _bJumpReach;
	AnimTick _tCasting;
	BOOL _bDealed;



};

