#pragma once

#include "LevelSkill.h"

struct SkillParam_PhantomAttack:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_PhantomAttack);

		BEGIN_GOBJ_PURE(SkillParam_PhantomAttack,1);
			GELEM_VAR_INIT(unsigned __int64,idPhantom,0);
				GELEM_EDITVAR("幻影效果",GVT_Bx8,GSem_ProtoPath,"幻影效果");
			GELEM_VAR_INIT(AnimTick,delayPhantom,ANIMTICK_FROM_SECOND(0.3f));
				GELEM_EDITVAR("等待开始幻影的时间",GVT_U,GSem(GSem_AnimTick,"0.0,5,0.05"),"等待开始幻影的时间");
			GELEM_VAR_INIT(DWORD,nPhantoms,3);
				GELEM_EDITVAR("幻影个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"有几个幻影");
			GELEM_VAR_INIT(AnimTick,durStep,ANIMTICK_FROM_SECOND(0.3f));
				GELEM_EDITVAR("幻影间隔时间",GVT_U,GSem(GSem_AnimTick,"0.1,5,0.05"),"幻影间隔时间");
			GELEM_VAR_INIT(AnimTick,durPhantom,ANIMTICK_FROM_SECOND(0.5f));
				GELEM_EDITVAR("幻影持续时间",GVT_U,GSem(GSem_AnimTick,"0,5,0.05"),"幻影持续时间");
			GELEM_VAR_INIT(AnimTick,delayHit,ANIMTICK_FROM_SECOND(0.1f));
				GELEM_EDITVAR("命中延迟",GVT_U,GSem(GSem_AnimTick,"0,5,0.05"),"命中延迟");
			GELEM_VAR_INIT(float,rangeDmg,1.0f);
				GELEM_EDITVAR("伤害范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"伤害范围");

		END_GOBJ();

	unsigned __int64 idPhantom;

	AnimTick delayPhantom;
	int nPhantoms;
	AnimTick durStep;
	AnimTick durPhantom;
	AnimTick delayHit;

	float rangeDmg;//伤害范围

};


class Skill_PhantomAttack:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_PhantomAttack,32);

	struct Phantom
	{
		Phantom()
		{
			memset(this,0,sizeof(*this));
		}
		LevelObjID idTarget;
		AnimTick tStart;
		LevelPos posSrc;
		BOOL bHit;
	};

	Skill_PhantomAttack()
	{
		_tCasting=0;
		_iCandidateStart=0;
		_faceOrg=0.0f;
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

	BOOL _CreatePhantom();

	void _CollectCandidates();

	AnimTick _tCasting;

	LevelFace _faceOrg;
	LevelPos _posOrg;
	LevelPos _posTarget;
	LevelPos _posLast;
	std::deque<LevelObjID> _candidates;	
	std::deque<WORD> _indicesCandidates;	
	int _iCandidateStart;
	std::deque<Phantom> _phantoms;

};

