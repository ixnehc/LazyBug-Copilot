#pragma once

#include "LevelSkill.h"

#include "spline/CubicSpline.h"

#include "LevelGesture.h"

struct SkillParam_Dive:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Dive);

	BEGIN_GOBJ_PURE(SkillParam_Dive,1);

		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"俯冲攻击的动作");

		GELEM_VAR_INIT(float,distThrust,8.0f);
			GELEM_EDITVAR("冲刺距离(水平方向)",GVT_F,GSem(GSem_Float,"0,100,0.1"),"冲刺距离");
		GELEM_VAR_INIT(float,distDive,4.0f);
			GELEM_EDITVAR("冲刺距离(竖直方向)",GVT_F,GSem(GSem_Float,"0,100,0.1"),"冲刺距离");
		GELEM_VAR_INIT(float,distEscape,8.0f);
			GELEM_EDITVAR("惯性滑动距离(水平方向)",GVT_F,GSem(GSem_Float,"0,100,0.1"),"攻击后向前滑动的距离");
		GELEM_VAR_INIT(float,lift,1.0f);
			GELEM_EDITVAR("抬升高度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"攻击后的抬升高度");
		GELEM_VAR_INIT(float,slide,3.0f);
			GELEM_EDITVAR("滑行距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"攻击后的滑行距离");

		GELEM_VAR_INIT(float,radiusDamage,1.5f);
			GELEM_EDITVAR("伤害半径",GVT_F,GSem(GSem_Float,"0,100,0.1"),"伤害半径");

	END_GOBJ();

	unsigned __int64 idEffect;

	float distThrust;
	float distEscape;
	float distDive;
	float lift;
	float slide;

	float radiusDamage;

};


class GameTileMap;
class CSkillGesture_Dive:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CSkillGesture_Dive);

	CSkillGesture_Dive()
	{
		Zero();
	}

	void Zero()
	{
		_t=0.0f;
		_gtm=NULL;
		_bFinished=FALSE;
		_bPassBy=FALSE;
	}

	void Create(i_math::vector3df &posCur,i_math::vector3df &posTarget,float dur,float lift,float slide,GameTileMap *gtm);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{		return;}//不支持	}
	virtual BOOL IsFinished()	{		return _bFinished;	}

	BOOL IsPassBy()	{		return _bPassBy;	}
	i_math::vector3df GetPassByPos()	{		return _pos[1];	}
	i_math::vector3df GetPassByVel() {	 return _vel[1];}

	BOOL IsAlive()	{		return _gtm!=NULL;	}

protected:
	void _UpdateFormula(i_math::vector3df &posSrc,i_math::vector3df posTarget,float lift,float slide);

	i_math::vector3df _pos[3];
	i_math::vector3df _vel[3];

	float _t;
	float _dur;

	GameTileMap * _gtm;
	BOOL _bFinished;
	BOOL _bPassBy;


};


class Skill_Dive:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Dive,12);

	Skill_Dive()
	{
		_ges=NULL;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_DefObj);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Move;	}



protected:
	virtual void _OnStart();
	virtual void _OnBreak()		{		_Finish();	}
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}

	float _GetSpeed();


	void _Finish();

	CSkillGesture_Dive *_ges;

};

