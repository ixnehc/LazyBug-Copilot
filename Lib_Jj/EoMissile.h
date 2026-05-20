#pragma once

#include "EoBulletBase.h"


#define CLASSUID_Missile 37

struct EoParamMissile:public EoParamBulletBase
{
	DEFINE_EOPARAM_CLASS(EoParamMissile);

	enum LockTargetMode
	{
		LockTargetMode_None,
		LockTargetMode_Default,
		LockTargetMode_Host,
		LockTargetMode_Moth,
		LockTargetMode_Special_MagicCircuitCrystalTarget,

		LockTargetMode_ForceDword,
	};

	BEGIN_GOBJ_PURE(EoParamMissile,1);

		GELEM_EOPARAMBULLETBASE;
		GELEM_VAR_INIT(LockTargetMode,modeLockTarget,LockTargetMode_Default);
			GELEM_EDITVAR("锁定目标方式",GVT_U,GSem(GSem_Interger,
				"缺省(技能目标/Threat):1"		"|搜索半径&加速度&游荡速度,"
				"锁定Host:2"				"|搜索半径&加速度&游荡速度,"
				"飞蛾模式:3"				","
				"魔法回路CrystalTarget:4"				"|搜索半径&加速度&游荡速度"
				),"锁定目标方式");

		GELEM_VAR_INIT(float,radiusDetect,3.0f);
			GELEM_EDITVAR("搜索半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"搜索半径");
		GELEM_VAR_INIT(float,accel,10.0f);
			GELEM_EDITVAR("加速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"加速度");
		GELEM_VAR_INIT(float,speedWonder,1.0f);
			GELEM_EDITVAR("游荡速度",GVT_F,GSem(GSem_Float,"0.0,100,0.1"),"游荡速度");

		GELEM_VAR_INIT(float,speedRot,180.0f);
			GELEM_EDITVAR("转速",GVT_F,GSem(GSem_Float,"0.01,1080,0.1"),"转向目标的速度(度/秒");
		GELEM_VAR_INIT(float,dur,3.0f);
			GELEM_EDITVAR("飞行时长",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"子弹飞多久");
		GELEM_VAR_INIT(float,durBlendIn,0.5f);
			GELEM_EDITVAR("飞行位置BlendIn时间(客户端)",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"飞行位置BlendIn时间(从初始绑定位置Blend到真实的飞行位置)");

	END_GOBJ();

	LockTargetMode modeLockTarget;

	float radiusDetect;
	float accel;
	float speedWonder;

	float speedRot;
	float dur;

	float durBlendIn;

};

class EoMissile;
class CMissile:public CBulletBase
{
public:
	DEFINE_CLASS(CMissile);

	CMissile()
	{
		_param=NULL;
		_owner=NULL;
		_t=0.0f;
		_speed=0.0f;
		_idLockTarget=LevelObjID_Invalid;
		_bTracing=FALSE;
		_speedMin=0.0f;
		_speedMax=0.0f;
	}

	~CMissile()
	{
	}

	void Init(EoMissile *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamMissile *param);

	i_math::vector3df&GetEuler()	{		return _euler;	}
	float GetSpeed()	{		return _speed;	}


protected:

	LevelObjID _DetectHit_ShieldAmulet(i_math::line3df &line) override;
	LevelObjID _DetectHit(i_math::line3df &line) override;


	virtual BOOL _NeedStop();
	virtual void _UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist);
	virtual BOOL _CanHit(CLevelObj *lo)
	{
		return TRUE;
	}


	i_math::vector3df _euler;

	EoParamMissile *_param;
	EoMissile *_owner;

	BOOL _bTracing;

	LevelObjID _idLockTarget;

	float _speed;
	float _speedMax;
	float _speedMin;

	float _t;
};


class EoMissile:public EoBulletBase
{
public:
	EoMissile()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoMissile,CLASSUID_Missile);

	virtual const char *GetShowName()	{		return "导弹";	}

protected:
	virtual CBulletBase *_CreateBullet();
	virtual void _DestroyBullet(CBulletBase *bullet);


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	void _OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _WritePos(CBitPacket *bp);

};
