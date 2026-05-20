#pragma once

#include "LevelSkill.h"

#include "LevelGesture.h"

#include "valueset/valueset.h"



struct SkillParam_FlyUp:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_FlyUp);

	SkillParam_FlyUp()
	{
		GConstructor();
		vsLift.ResetFloat(0.0f);
		vsLift.AddFloat(1.0f,1.0f);
	}
	~SkillParam_FlyUp()
	{
		GDestructor();
	}

	BEGIN_GOBJ(SkillParam_FlyUp,1);

		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"起飞的动作");

		GELEM_VAR_INIT(float,tDelay,0.2f);
			GELEM_EDITVAR("升起延迟",GVT_F,GSem(GSem_Float,"0,100,0.1"),"升起前延迟多久");

		GELEM_VAR_INIT(float,dur,2.0f);
			GELEM_EDITVAR("升起时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"升起时间");

		GELEM_VAR_INIT(float,lift,10.0f);
			GELEM_EDITVAR("升高距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"起飞时升高多少距离");
		GELEM_OBJVAR( ValueSet, vsLift);
			GELEM_EDITOBJ_EX("升高曲线","升高曲线",GSem( GSem_Unknown, "0,0,1,1" ));
		GELEM_VAR_INIT(float,shift,10.0f);
			GELEM_EDITVAR("水平距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"起飞时偏离多少距离");

	END_GOBJ();

	unsigned __int64 idEffect;

	float tDelay;
	float dur;
	float lift;
	ValueSet vsLift;
	float shift;
};

class CSkillGesture_FlyUp:public CLevelGesture_BuildIn
{
public:
	DEFINE_CLASS(CSkillGesture_FlyUp);
	IMPLEMENT_REFCOUNT_C;

	CSkillGesture_FlyUp()
	{
		Zero();
	}

	void Zero()
	{
		_bFinished=FALSE;
		_t=0.0f;

		_cfg=NULL;

	}

	void Create(SkillParam_FlyUp *cfg,LevelPos3D &posInitial,LevelFace faceInitial);
	virtual void Destroy()	{		Zero();	Release();}


	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{	}

	virtual BOOL IsFinished();

protected:

	BOOL _bFinished;

	float _t;
	LevelPos3D _posInitial;

	i_math::vector3df _step;

	SkillParam_FlyUp *_cfg;

	GameTileMap *_gtm;


};


class Skill_FlyUp:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_FlyUp,11);

	Skill_FlyUp()
	{
	}

	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_None);
	}

	virtual CastMoving GetCastMoving()	{		return CastMoving_Move;	}



protected:

	virtual void _OnStart();
	virtual void _OnUpdate(AnimTick dt);
	virtual void _OnFinish()	{		_Finish();	}


	void _Finish();

};

