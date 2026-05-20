#pragma once

#include "LevelSkill.h"

#include "spline/CubicSpline.h"

#include "LevelGesture.h"

struct SkillParam_Plunge:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Plunge);

	BEGIN_GOBJ_PURE(SkillParam_Plunge,1);

		GELEM_VAR_INIT(unsigned __int64,idEffect,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"俯冲攻击的动作");

		GELEM_VAR_INIT(float,speed,10.0f);
			GELEM_EDITVAR("速度",GVT_S,GSem(GSem_Float,"1.0,20,0.1"),"速度");
	END_GOBJ();

	unsigned __int64 idEffect;
	float speed;

};


class GameTileMap;
class CSkillGesture_Plunge:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CSkillGesture_Plunge);

	CSkillGesture_Plunge()
	{
		Zero();
	}

	void Zero()
	{
		_level=NULL;
		_idTarget=LevelObjID_Invalid;
		_nReachCount=0;
		_bFinished=FALSE;
		_speed=0.0f;
	}

	void Create(LevelObjID idTarget,float speed,CLevel *level);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{		return;}//不支持	}
	virtual BOOL IsFinished()	{		return _bFinished;	}

	BOOL IsAlive()	{		return _level!=NULL;	}

protected:
	CLevel *_level;

	LevelObjID _idTarget;

	BOOL _bReached;
	BOOL _bFinished;
	int _nReachCount;
	float _speed;


};


class Skill_Plunge:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Plunge,26);

	Skill_Plunge()
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

	CSkillGesture_Plunge *_ges;

};

