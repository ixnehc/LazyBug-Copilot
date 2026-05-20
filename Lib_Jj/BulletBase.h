#pragma once
#include "LevelObj.h"

#include "LevelObjHistory.h"

#include "spline/CubicSpline.h"

struct BulletStaticHit
{
	BulletStaticHit()
	{
		Zero();
	}
	enum Type
	{
		None=0,
		Default,
		ShieldAmulet,
	};
	void Zero()
	{
		tp=None;
		id=LevelObjID_Invalid;
	}
	BOOL IsEmpty()
	{
		return tp==None;
	}
	Type tp;
	LevelObjID id;
};

struct BulletAbsorbSetting;
struct BulletAbsorb
{
	DEFINE_CLASS(BulletAbsorb);
	BulletAbsorb()
	{
		idTarget=LevelObjID_Invalid;
		distStart=0.0f;
		setting=NULL;
	}
	LevelObjID idTarget;
	CCubicSpline spline;
	float distStart;
	BulletAbsorbSetting *setting;

};

class CLevelObj;
class CLevel;
struct EoParamBulletBase;
class CBulletBase
{
public:

	CBulletBase()
	{
		_level=NULL;
		_owner=NULL;
		_radius=0.0f;
		_distIgnoreStatic=0.0f;
		_dist=0.0f;
		_bHiResoStaticCheck=FALSE;
		_bPenetrate=FALSE;
		_paramBase=NULL;
		_absorb=NULL;
	}

	void Init(CLevelObj *lo,EoParamBulletBase *paramBase,LevelPos3D pos,float radius,float fall,float distIgnoreStatic,BOOL bPenetrate)
	{
		_owner=lo;
		_level=lo->GetLevel();
		_paramBase=paramBase;
		_radius=radius;
		_fall=fall;
		_distIgnoreStatic=distIgnoreStatic;
		_pos=pos;
		_bPenetrate=bPenetrate;
	}
	void SetHiResoStaticCheck(BOOL bHiReso)	{		_bHiResoStaticCheck=bHiReso;	}

	BOOL Update(AnimSecond dt,LevelObjHits &hits,BulletStaticHit &hitStatic,LevelObjID &hitAbsorb);//返回FALSE表示这个bullet已经结束了

	BOOL IsStop()	{		return _level==NULL;	}

	virtual i_math::vector3df &GetDir()	{		return _dir;	}
	virtual i_math::vector3df &GetPos()	{		return _pos;	}


protected:

	virtual BOOL _NeedStop()	{		return TRUE;	}
	virtual void _UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist)	{	}//根据dt计算要往什么方向上移动多少距离,dir必须是normalized的
	virtual BOOL _CanHit(CLevelObj *lo){return FALSE;}

	virtual LevelObjID _DetectHit_ShieldAmulet(i_math::line3df &line)	{	return LevelObjID_Invalid;}

	virtual LevelObjID _DetectHit(i_math::line3df &line)=0;
	virtual void _DetectHits(i_math::line3df &line,LevelObjHits &hits,CLevelObjHistory &history)	{	}//目前不支持

	float _AdjustThrowSpeed(CLoEffectObj *owner,LevelPos3D &posInitial,i_math::vector3df &dirInitial,float speedInitial,float speedVary,float g,int modeThrowAim);

	BOOL _bHiResoStaticCheck;

	CLevel *_level;//这个指针表示这个Bullet是否有效
	CLevelObj *_owner;
	EoParamBulletBase *_paramBase;

	float _radius;//Bullet的半径
	float _fall;//Bullet的下探
	float _distIgnoreStatic;//开始多少距离内忽略静态物体的碰撞

	float _dist;//飞行轨迹的长度(到当前时间)
	LevelPos3D _pos;
	LevelPos3D _dir;

	BOOL _bPenetrate;

	BulletAbsorb *_absorb;

	CLevelObjHistory _history;

	friend class EoBulletBase;

};

