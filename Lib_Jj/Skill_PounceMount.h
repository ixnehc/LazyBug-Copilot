#pragma once

#include "LevelSkill.h"

struct SkillParam_PounceMount:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_PounceMount);

	BEGIN_GOBJ_PURE(SkillParam_PounceMount,1);

		GELEM_VAR_INIT(RecordID,idStun,RecordID_Invalid);
			GELEM_EDITVAR("硬直Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"对猛扑对象加的硬直Buff");

		GELEM_VAR_INIT(float,rangeMin,2.0f);
			GELEM_EDITVAR("施放最小距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"技能的施放最小距离");

		GELEM_VAR_INIT(float,rangeMax,8.0f);
			GELEM_EDITVAR("施放最大距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"技能的施放最大距离");

		GELEM_VAR_INIT(AnimTick,durWait,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("蓄力时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"蓄力时间,单位为秒");
		GELEM_VAR_INIT(AnimTick,durBite,ANIMTICK_FROM_SECOND(0.8f));
			GELEM_EDITVAR("撕咬时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"撕咬时间,单位为秒");
		GELEM_VAR_INIT(AnimTick,tDmgDelay,ANIMTICK_FROM_SECOND(0.6f));
			GELEM_EDITVAR("撕咬伤害延迟时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"撕咬伤害延迟时间,单位为秒");
		GELEM_VAR_INIT(AnimTick,durBack,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("翻回时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"翻回时间,单位为秒");
		GELEM_VAR_INIT(AnimTick,durJump,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("Miss后跳跃时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Miss后跳跃时间,单位为秒");


	END_GOBJ();

	RecordID idStun;

	AnimTick durWait;//蓄力时间
	AnimTick durBite;//撕咬时间
	AnimTick tDmgDelay;//撕咬伤害的延迟时间
	AnimTick durJump;//Miss后跳跃时间
	AnimTick durBack;//翻回时间

	float rangeMin;
	float rangeMax;

};


class Skill_PounceMount:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_PounceMount,25);

	Skill_PounceMount()
	{
		_tCasting=0;
		_stage=None;
		_bBiteDealed=FALSE;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_DefObj);
	}

	enum Stage
	{
		None=0,
		Wait,
		WaitCanceled,
		Bite,
		Back,
		Jump,
		Finish,
	};

protected:

	virtual void _OnStart();
	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnBreak();
	virtual BOOL _WriteSyncData(CBitPacket *bp);

	virtual BOOL IsImmune()
	{
		if ((_stage==Bite)||(_stage==Back)||(_stage==Jump))
			return TRUE;
		return FALSE;
	}

	LevelPos _posLock;
	LevelPos _posOrg;

	CLevelObj *_GetLoTarget();

	AnimTick _tCasting;

	Stage _stage;
	BOOL _bBiteDealed;


};

