#pragma once

#include "EoBulletBase.h"


#define CLASSUID_Bullet 34

struct EoParamBullet:public EoParamBulletBase
{
	DEFINE_EOPARAM_CLASS(EoParamBullet);

	BEGIN_GOBJ_PURE_UID(EoParamBullet,1);

		GELEM_EOPARAMBULLETBASE;
		GELEM_VAR_INIT(float,speedAdj,2.0f); GELEM_UID(6);
			GELEM_EDITVAR("速度可调范围",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"速度可调范围(用于抛物线准确命中目标)");
		GELEM_VAR_INIT(int,modeThrowAim,FALSE); GELEM_UID(7);
			GELEM_EDITVAR("抛物线修正目标",GVT_S,GSem(GSem_Interger,"不修正:2,目标脚下:0,瞄准点:1"),"抛物线修正目标");
		GELEM_VAR_INIT(float,g,0.0f); GELEM_UID(8);
			GELEM_EDITVAR("重力加速度",GVT_F,GSem(GSem_Float,"0.0,100.0,0.05"),"重力加速度");
		GELEM_VAR_INIT(float,range,10.0f);GELEM_UID(9);
			GELEM_EDITVAR("飞行范围",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"子弹最远飞多远");

	END_GOBJ();

	float speedAdj;
	float range;
	float g;
	int modeThrowAim;
};

class EoBullet;
class CFlyingBullet:public CBulletBase
{
public:
	DEFINE_CLASS(CFlyingBullet);

	CFlyingBullet()
	{
		_param=NULL;
		_t=0.0f;
		_speed=0.0f;
	}

	~CFlyingBullet()
	{
	}

	void Init(EoBullet *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamBullet *param);

	virtual i_math::vector3df&GetDir()	override{		return _dir;	}
	float GetSpeed()	{		return _speed;	}

	LevelObjID _DetectHit_ShieldAmulet(i_math::line3df &line) override;

	LevelObjID _DetectHit(i_math::line3df &line) override;
	void _DetectHits(i_math::line3df &line,LevelObjHits &hits,CLevelObjHistory &history) override;


protected:

	virtual BOOL _NeedStop()
	{
		if (_dist>=_param->range)
			return TRUE;
		return FALSE;
	}
	virtual void _UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist);
	virtual BOOL _CanHit(CLevelObj *lo)
	{
		return TRUE;
	}

	i_math::vector3df _dir;

	EoParamBullet *_param;

	float _speed;

	LevelPos3D _pos;
	float _t;


};

class EoBullet:public EoBulletBase
{
public:
	EoBullet()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoBullet,CLASSUID_Bullet);

	virtual const char *GetShowName()	{		return "子弹";	}

protected:


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	virtual CBulletBase *_CreateBullet();
	virtual void _DestroyBullet(CBulletBase *bullet);
};
