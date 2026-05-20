#pragma once

#include "LevelSkill.h"

#include "LevelGesture.h"

#include "anim/KeySet.h"

struct SkillParam_Roll:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_Roll);

	BEGIN_GOBJ_PURE(SkillParam_Roll,1);

		GELEM_VAR_INIT(float,speed,8.0f);
			GELEM_EDITVAR("滚动速度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"滚动速度");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("滚动时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"滚动多长时间,单位为秒");
		GELEM_VAR_INIT(AnimTick,dur2,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("站定时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"滚动后站起来花多长时间,单位为秒");

		GELEM_VAR_INIT(int,nDeal,1);
			GELEM_EDITVAR("释放次数",GVT_S,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"沿路释放多少次效果");

	END_GOBJ();

	unsigned __int64 idEffect;
	AnimTick dur;
	AnimTick dur2;

	float speed;

	DWORD nDeal;

};


class CUnitMgrNavMesh;
class CSkillGesture_Roll:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CSkillGesture_Roll);

	CSkillGesture_Roll()
	{
		Zero();
	}

	void Zero()
	{
		_tCur=0.0f;
		_dur=0.0f;
		_dur2=0.0f;
		_bFinished=FALSE;
		_bAlive=FALSE;
	}

	void Create(KeySet *ks,float dur,float dur2);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt){		return;}//不支持	}
	virtual void Update(CUnit *unit,float dt);
	virtual BOOL IsFinished()	{		return _bFinished;	}


	BOOL IsAlive()	{		return _bAlive;	}
	void Stop()	{		_bFinished=TRUE;	}

protected:

	KeySet *_ks;

	float _tCur;
	float _dur;
	float _dur2;

	DWORD _bAlive;
	DWORD _bFinished;

	friend class Skill_Roll;

};


class Skill_Roll:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_Roll,24);

	Skill_Roll()
	{
		_ges=NULL;
		_tCur=0;
		_nSpawned=0;
	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return (1<<LevelSkillTarget::Target_Aim);
	}
	virtual CastMoving GetCastMoving()	{		return CastMoving_Control;	}


protected:
	virtual void _OnStart();
	virtual void _OnBreak();
	virtual void _OnUpdate(AnimTick dt);

	virtual void _OnFinish()	{		_Finish();	}


	void _Finish();

	CSkillGesture_Roll *_ges;

	KeySet _ksPath;

	DWORD _nSpawned;

	AnimTick _tCur;

};

