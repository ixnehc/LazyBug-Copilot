#pragma once

#include "LevelSkill.h"


struct ChainSummonCategory
{
	BEGIN_GOBJ_PURE(ChainSummonCategory,1);
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("召唤的单位",GVT_U,GSem(GSem_RecordID,"units"),"召唤的单位");
		GELEM_VAR_INIT(RecordID,idBirth,RecordID_Invalid);
			GELEM_EDITVAR("出生的Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"召唤后的单位的出生Buff");

		GELEM_VAR_INIT(int,count,4);
			GELEM_EDITVAR("召唤个数",GVT_S,GSem_Interger,"召唤个数");
		GELEM_VAR_INIT(int,vary,0);
			GELEM_EDITVAR("召唤个数浮动值",GVT_S,GSem_Interger,"召唤个数的上下浮动值");
	END_GOBJ();

	RecordID idUnit;
	RecordID idBirth;//出生Buff
	DWORD count;
	DWORD vary;
};

struct SkillParam_ChainSummon:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_ChainSummon);

	enum Mode
	{
		UsingParam=0,//使用传入的参数

		ForceDword=0xffffffff,
	};

	BEGIN_GOBJ_PURE(SkillParam_ChainSummon,1);

		GELEM_OBJVECTOR(ChainSummonCategory,cats);
			GELEM_EDITOBJ("召唤的单位","召唤的各种单位");
		GELEM_VAR_INIT(Mode,mode,UsingParam);
			GELEM_EDITVAR("召唤位置产生模式",GVT_S,GSem(GSem_Interger,"使用预设的参数"),"用什么方式产生召唤位置");
		GELEM_VAR_INIT(AnimTick,durSummon,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("单次召唤时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"单次召唤时间,单位为秒");
		GELEM_VAR_INIT(unsigned __int64,idChain,0);
			GELEM_EDITVAR("连锁效果",GVT_Bx8,GSem_ProtoPath,"连锁效果");
	END_GOBJ();

	std::vector<ChainSummonCategory>cats;
	Mode mode;
	AnimTick durSummon;//
	unsigned __int64 idChain;


};


struct ChainSummonQueue
{
	void Build(SkillParam_ChainSummon*param,LevelSkillArg *arg);

	DWORD nToSummon;
	WORD cats[32];
	LevelPos sites[32];
	
};

class Skill_ChainSummon:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_ChainSummon,21)

	Skill_ChainSummon()
	{
		_radDir=0.0f;
		_nSummon=0;
		_tSummoning=0;
		_tNextSummon=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_None);
	}

protected:

	virtual void _OnStart();
	virtual void _OnFinish();
	virtual void _OnUpdate(AnimTick dt);


	void _Update(AnimTick dt);

	void _UpdateSummon(AnimTick dt);

	void _DoSummon();

	CLevelSkillCasting _casting;

	float _radDir;

	AnimTick _tSummoning;
	AnimTick _tNextSummon;
	DWORD _nSummon;//已经召唤了几个
	ChainSummonQueue _queue;


};

