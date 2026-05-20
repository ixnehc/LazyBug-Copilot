
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoSplineBullet.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "LevelSensor.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//CSplineBullet
void CSplineBullet::Init(EoSplineBullet *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamSplineBullet *param)
{
	_owner=owner;

	CBulletBase::Init(owner,param,src,param->radius,param->fall,param->distIgnoreStatic,param->bMH);
	_param=param;

	_yInitial=src.y;
	_posLast=src;
}


LevelObjID CSplineBullet::_DetectHit(i_math::line3df &line)
{
	if (_owner)
	{
		LevelObjID idIgnore=LevelObjID_Invalid;

		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=idIgnore;

		CLevelObj *hit=_owner->DetectHit(line,argHit);
		if (hit)
			return hit->GetID();
	}
	return LevelObjID_Invalid;
}


void CSplineBullet::_DetectHits(i_math::line3df &line,LevelObjHits &hits,CLevelObjHistory &history)
{
	hits.Zero();
	if (_owner)
	{
		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;

		_owner->DetectHits(line,argHit,hits,history);
	}
}


LevelPos3D CalcSplineBulletPos(LevelPos3D &posGround,float ht,float ySrc,CCubicSpline &spline,float distOnSpline)
{
	LevelPos3D posCur=posGround;
	posCur.y+=ht;

	//Blend初始y
	if (TRUE)
	{
		float distHeightBlend=2.0f;
		if (distHeightBlend>spline.GetDistance())
			distHeightBlend=spline.GetDistance();

		float ratioBlend=i_math::clamp_f(distOnSpline/distHeightBlend,0.0f,1.0f);

		posCur.y=i_math::lerp(ySrc,posCur.y,ratioBlend);
	}
	return posCur;
}

float CalcSplineBulletDist(CCubicSpline &spline,float t,float speed,float distDamping)
{
	if (distDamping>spline.GetDistance())
		distDamping=spline.GetDistance();

	float distCruise=spline.GetDistance()-distDamping;
	if (distCruise>t*speed)
		return t*speed;

	float tDamping=t-distCruise/speed;

	float durDamping=distDamping/(0.5f*speed);
	if (tDamping>durDamping)
		tDamping=durDamping;
	float a=-speed/durDamping;

	return distCruise+speed*tDamping+0.5f*a*tDamping*tDamping;
}

float CalcSplineBulletDur(CCubicSpline &spline,float speed,float distDamping)
{
	if (distDamping>spline.GetDistance())
		distDamping=spline.GetDistance();

	float distCruise=spline.GetDistance()-distDamping;
	return distCruise/speed+distDamping/(0.5f*speed);
}

void CSplineBullet::_UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist)
{
	_t+=_param->speed*(float)dt;

	float dist=CalcSplineBulletDist(_owner->GetSpline(),_t,_param->speed,_param->distDamping);

	LevelPos3D posCur=_owner->GetSpline().GetPositionByDist(dist);
	posCur=LevelUtil_GetWalkableGroundHeight(_owner->GetLevel(),posCur.x,posCur.z,TRUE);
	posCur=CalcSplineBulletPos(posCur,0.5f,_yInitial,_owner->GetSpline(),dist);

	dir=posCur-_posLast;
	dDist=dir.getLength();
	dir.normalize();

	_posLast=posCur;
}

BOOL CSplineBullet::_NeedStop()
{
	if (_t>=CalcSplineBulletDur(_owner->GetSpline(),_param->speed,_param->distDamping))
		return TRUE;
	return FALSE;
}




//////////////////////////////////////////////////////////////////////////
//EoSplineBullet
BIND_EOPARAM(EoSplineBullet,EoParamSplineBullet);

CBulletBase *EoSplineBullet::_CreateBullet()
{
	CSplineBullet *bullet=Class_New2(CSplineBullet);
	EoParamSplineBullet*param=GetParam<EoParamSplineBullet>();
	bullet->Init(this,_GetInitialPos3D(),_GetInitialDir3D(),param);
	bullet->SetHiResoStaticCheck(TRUE);

	return bullet;
}
void EoSplineBullet::_DestroyBullet(CBulletBase *bullet0)
{
	CSplineBullet *bullet=(CSplineBullet *)bullet0;
	Safe_Class_Delete(bullet);
}


void EoSplineBullet::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	__super::_OnWriteFirstSync(bp,bContent,idPlayer);

	DWORD nNodes=_spline._spline.GetNodeCount();
	bp->Data_NextWord()=(WORD)nNodes;

	for (int i=0;i<nNodes;i++)
	{
		LevelPos pos=_spline._spline.GetNode(i)->position.getXZ();
		bp->Data_WriteSimpleR(pos);
	}

	bContent=TRUE;
}




void EoSplineBullet::BuildSpline(LevelPos &posSrc,LevelPos &posGuide,LevelPos &posTarget)
{
	_spline.AddInitialPos(posSrc);
	_spline.AddInitialPos(posGuide);

	_spline.Build(_level->GetUnitMgr(),posTarget);
}
