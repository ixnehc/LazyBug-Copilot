#pragma once

#include "LevelSkill.h"


struct SkillParam_Spray:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Spray);

	BEGIN_GOBJ_PURE(SkillParam_Spray,1);
		GELEM_VAR_INIT(float,speed,10.0f);
			GELEM_EDITVAR("喷洒波速度",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"喷洒波的移动速度,单位为米/秒");
		GELEM_VAR_INIT(float,speedWave,3.0f);
			GELEM_EDITVAR("喷洒波产生速度",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"每秒产生几个喷洒波,单位为个/秒");
		GELEM_VAR_INIT(float,range,3.0f);
			GELEM_EDITVAR("喷洒的最远影响距离",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"喷洒的最远影响距离,单位为米");
		GELEM_VAR_INIT(float,spread,60.0f);
			GELEM_EDITVAR("喷洒的角度范围",GVT_F,GSem(GSem_Float,"0.0,180,0.1"),"喷洒的角度范围,单位为角度");
		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果Proto",GVT_Bx8,GSem_ProtoPath,"效果的Proto");

	END_GOBJ();

	float speed;//喷洒物的速率
	float range;//喷洒的距离
	float spread;//喷洒的范围,角度
	unsigned __int64 idEffect;
	float speedWave;
};


struct AttrNodeBase;
class Skill_Spray:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Spray,23);

	Skill_Spray()
	{
		_tCasting=0;
		_tSpraying=ANIMTICK_INFINITE;

		_nWaves=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim)|(1<<LevelSkillTarget::Target_DefObj);
	}

	virtual BOOL NeedLockPick()	{		return FALSE;	}

	virtual BOOL Combine(LevelSkillTarget &target);


	virtual void NotifyCasted()
	{
		_Finish();
	}

	struct Wave
	{
		AnimTick tStart;
		float angleMin,angleMax;
		float distLast;
	};



protected:
	virtual void _OnStart();
	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnBreak()	{		_Finish();	}
	virtual void _OnFinish()	{		_Finish();	}

	void _Finish();

	AnimTick _tSpraying;

	AnimTick _tCasting;

	DWORD _nWaves;

	std::deque<Wave> _waves;






};

