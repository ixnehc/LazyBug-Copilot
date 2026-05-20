#pragma once

#include "LevelSkill.h"


struct SkillParam_Summon:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Summon);

	enum Mode
	{
		Mode_AroundMe,
		Mode_AroundTarget,

		ForceDword=0xffffffff,
	};

	BEGIN_GOBJ_PURE(SkillParam_Summon,1);
		GELEM_VAR_INIT(Mode,mode,Mode_AroundMe);
			GELEM_EDITVAR("模式",GVT_U,GSem(GSem_Interger,"在自己身边召唤,在目标身边召唤"),"以何种模式召唤");
		GELEM_VAR_INIT(int,count,1);
			GELEM_EDITVAR("个数",GVT_S,GSem_Interger,"召唤几个");
		GELEM_VAR_INIT(float,dur,0.0f);
			GELEM_EDITVAR("召唤冷却时间",GVT_F,GSem(GSem_Float,"0,100,0.2"),"一次召唤冷却时间");
		GELEM_VAR_INIT(float,range,1.0f);
			GELEM_EDITVAR("范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"在离自己多远的距离处召唤");
		GELEM_VAR_INIT(float,fov,60.0f);
			GELEM_EDITVAR("FOV",GVT_F,GSem(GSem_Float,"0.0,360.0,1"),"召唤的FOV");

	END_GOBJ();

	Mode mode;
	int count;
	float dur;

	float range;
	float fov;

};



class Skill_Summon:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Summon,19)

	Skill_Summon()
	{
		_nSummoned=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Pos)|(1<<LevelSkillTarget::Target_ObjPos)|(1<<LevelSkillTarget::Target_None);
	}

protected:

	virtual void _OnStart();
	virtual void _OnFinish();
	virtual void _OnUpdate(AnimTick dt);


	void _Update(AnimTick dt);

	void _BuildSummonSites();
	std::vector<LevelPos> &_GetSites();
	std::vector<LevelPos> &_GetDirs()	{		return _dirs;	}

	CLevelSkillCasting _casting;

	std::vector<LevelPos> _sites;
	std::vector<LevelPos> _dirs;

	std::vector<int> _indicesRandom;
	DWORD _nSummoned;

};

