#pragma once

#include "EoBulletBase.h"

#include "EoUtumAttack.h"


#define CLASSUID_SplineBullet 69

struct EoParamSplineBullet:public EoParamBulletBase
{
	DEFINE_EOPARAM_CLASS(EoParamSplineBullet);

	BEGIN_GOBJ_PURE(EoParamSplineBullet,1);

		GELEM_EOPARAMBULLETBASE;
		GELEM_VAR_INIT(float,distDamping,2.0f);
			GELEM_EDITVAR("缓冲距离",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"缓冲距离");

	END_GOBJ();

	float distDamping;

};

class EoSplineBullet;
class CSplineBullet:public CBulletBase
{
public:
	DEFINE_CLASS(CSplineBullet);

	CSplineBullet()
	{
		_param=NULL;
		_owner=NULL;

		_t=0.0f;
		_yInitial=0.0f;
	}

	~CSplineBullet()
	{
	}

	void Init(EoSplineBullet *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamSplineBullet *param);

protected:

	BOOL _NeedStop()override;
	void _UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist)override;
	BOOL _CanHit(CLevelObj *lo) override
	{
		return TRUE;
	}

	LevelObjID _DetectHit(i_math::line3df &line)override;
	void _DetectHits(i_math::line3df &line,LevelObjHits &hits,CLevelObjHistory &history)override;

	EoParamSplineBullet *_param;
	EoSplineBullet *_owner;
	float _t;

	float _yInitial;
	LevelPos3D _posLast;

};


class EoSplineBullet:public EoBulletBase
{
public:
	EoSplineBullet()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoSplineBullet,CLASSUID_SplineBullet);

	virtual const char *GetShowName()	{		return "沿Spline飞行的子弹";	}

	void BuildSpline(LevelPos &posSrc,LevelPos &posGuide,LevelPos &posTarget);
	CCubicSpline &GetSpline()	{		return _spline._spline;	}

protected:
	virtual CBulletBase *_CreateBullet();
	virtual void _DestroyBullet(CBulletBase *bullet);

	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	CPathSpline _spline;

};
