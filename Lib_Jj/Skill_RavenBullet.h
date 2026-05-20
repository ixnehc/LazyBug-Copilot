#pragma once

#include "LevelSkill.h"
#include "LevelGesture.h"


struct SkillParam_RavenBullet;
class CLevelGesture_RavenThrust:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CLevelGesture_RavenThrust);

	CLevelGesture_RavenThrust()
	{
		Zero();
	}

	void Zero()
	{
		_t=0.0f;
		_gtm=NULL;
		_bFinished=FALSE;
		_param=FALSE;

		_bPassBy=FALSE;

		_loTarget=NULL;

		_htInitial=0.0f;

		_gtm=NULL;
	}

	void Create(CLevelObj *loSrc,CLevelObj *loTarget,SkillParam_RavenBullet *param);

	virtual void Destroy();
	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{		return;}//不支持	}
	virtual BOOL IsFinished()	{		return _bFinished;	}

	BOOL IsPassBy()	{		return _bPassBy;	}

protected:

	float _t;

	BOOL _bPassBy;//是否已经通过了loTarget
	LevelPos _dirPassBy;//通过时的飞行方向

	GameTileMap *_gtm;

	float _htInitial;

	LevelPos _dirInitial;
	CLevelObj *_loTarget;

	BOOL _bFinished;

	SkillParam_RavenBullet *_param;
};


struct SkillGradeInfo_RavenBullet
{

	BEGIN_GOBJ_PURE(SkillGradeInfo_RavenBullet,1);
	END_GOBJ();

};

struct SkillParam_RavenBullet:public LevelSkillParam
{
	DEFINE_SKILLPARAM_CLASS(SkillParam_RavenBullet);

	BEGIN_GOBJ_PURE(SkillParam_RavenBullet,1);

		GELEM_OBJVECTOR(SkillGradeInfo_RavenBullet,grdinfos)
			GELEM_EDITOBJ("等级参数","等级参数");

		GELEM_VAR_INIT(float,htThrust,2.0f);
			GELEM_EDITVAR("冲刺高度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"冲刺时保持的高度(相对于冲刺目标)");

		GELEM_VAR_INIT(float,spdThrust,8.0f);
			GELEM_EDITVAR("冲刺速度",GVT_F,GSem(GSem_Float,"0.1f,100,0.1"),"冲刺时飞行的速度");

		GELEM_VAR_INIT(float,rangeFire,3.0f);
			GELEM_EDITVAR("发射范围",GVT_F,GSem(GSem_Float,"0.1f,100,0.1"),"进入多近范围内开始发射");

		GELEM_VAR_INIT(float,spdBullet,8.0f);
			GELEM_EDITVAR("子弹速度",GVT_F,GSem(GSem_Float,"0.1f,100,0.1"),"子弹飞行的速度");

		GELEM_VAR_INIT(float,rangeBullet,12.0f);
			GELEM_EDITVAR("子弹作用范围",GVT_F,GSem(GSem_Float,"0.1f,100,0.1"),"子弹作用的范围");

		GELEM_VAR_INIT(float,radiusBullet,0.5f);
			GELEM_EDITVAR("子弹的半径",GVT_F,GSem(GSem_Float,"0.1f,100,0.1"),"子弹的半径(子弹有多大)");

		GELEM_VAR_INIT(unsigned __int64,idBullet,0);
			GELEM_EDITVAR("子弹效果Proto",GVT_Bx8,GSem_ProtoPath,"子弹效果的Proto");

	END_GOBJ();
 
	SkillGradeInfo_RavenBullet *GetGrdInfo(LevelGrade grd)
	{
		SkillGradeInfo_RavenBullet *info=NULL;
		if (grd!=LevelSkillGrade_Invalid)
		{
			int idx=grd-1;
			if (grdinfos.size()>0)
			{
				if (idx>=grdinfos.size())
					idx=grdinfos.size()-1;
				info=&grdinfos[idx];
			}
		}
		if (!info)
		{
			static SkillGradeInfo_RavenBullet t;
			info=&t;
		}
		return info;
	}

	std::vector<SkillGradeInfo_RavenBullet> grdinfos;

	float htThrust;//冲刺时保持的高度
	float spdThrust;//冲刺时的速度
	float rangeFire;//
	float spdBullet;//子弹速度
	float rangeBullet;//子弹的作用范围
	float radiusBullet;//子弹的半径
	unsigned __int64 idBullet;//子弹的飞行效果


};

class CThrowBullet;
class Skill_RavenBullet:public CLevelSkill
{
public:
	DEFINE_SKILL_CLASS(Skill_RavenBullet,16);

	Skill_RavenBullet()
	{
		_ges=NULL;

		_tCasting=0;

// 		_bullet=NULL;

	}


	LevelSkillTarget::TypeMask GetTargetTypes()
	{
		return 1<<LevelSkillTarget::Target_DefObj;
	}

	virtual CastMoving GetCastMoving()	{		return CastMoving_Move;	}



protected:

	virtual void _OnStart();
	virtual void _OnBreak()	{		_SetState(SkillState_Finished);	}
	virtual void _OnFinish();

	virtual void _OnUpdate(AnimTick dt);

	void _UpdateCast(AnimTick dt);

	void _UpdateBullet(AnimTick dt);

	AnimTick _tCasting;

	CLevelGesture_RavenThrust *_ges;

// 	CThrowBullet *_bullet;

};

