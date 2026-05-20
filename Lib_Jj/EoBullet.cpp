
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoBullet.h"

#include "LevelRecords.h"

#include "LevelOSB.h"



//////////////////////////////////////////////////////////////////////////
//CFlyingBullet
void CFlyingBullet::Init(EoBullet *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamBullet *param)
{
	_owner=owner;

	float distIgnoreStatic=param->distIgnoreStatic;
	if (param->distIgnoreStatic>0.0f)
	{//换算成3D空间的距离
		i_math::vector3df dirVer;
		dirVer.set(dir.x,0.0f,dir.z);
		if (dirVer.getLengthSQ()>0.001f)
		{//足够斜
			dirVer.normalize();
			float rate=dirVer.dotProduct(dir);
			distIgnoreStatic/=rate;
		}
	}

	CBulletBase::Init(_owner,param,src,param->radius,param->fall,distIgnoreStatic,param->bMH);
	_param=param;
	_dir=dir;
	_pos.set(0,0,0);
	_t=0.0f;
	_speed=param->speed;

	//调整速度以保证抛物线命中目标
	if (param->g>0.0f)
		_speed=_AdjustThrowSpeed(owner,src,dir,_param->speed,_param->speedAdj,_param->g,_param->modeThrowAim);
}

LevelObjID CFlyingBullet::_DetectHit_ShieldAmulet(i_math::line3df &line)
{
	if (_owner)
	{
		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=_param->bIgnoreHost?((EoBullet*)_owner)->GetHost():LevelObjID_Invalid;

		CLevelObj *hit=((EoBullet*)_owner)->DetectHit_ShieldAmulet(line,argHit);
		if (hit)
			return hit->GetID();
	}
	return LevelObjID_Invalid;
}


LevelObjID CFlyingBullet::_DetectHit(i_math::line3df &line)
{
	if (_owner)
	{
		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=_param->bIgnoreHost?((EoBullet*)_owner)->GetHost():LevelObjID_Invalid;

		CLevelObj *hit=((EoBullet*)_owner)->DetectHit(line,argHit);
		if (hit)
			return hit->GetID();
	}
	return LevelObjID_Invalid;
}

void CFlyingBullet::_DetectHits(i_math::line3df &line,LevelObjHits &hits,CLevelObjHistory &history)
{
	hits.Zero();
	if (_owner)
	{
		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=_param->bIgnoreHost?((EoBullet*)_owner)->GetHost():LevelObjID_Invalid;

		((EoBullet*)_owner)->DetectHits(line,argHit,hits,history);
	}
}


LevelPos3D CalcBulletPos(LevelPos3D &velInitial,float t,float g)
{
	LevelPos3D pos;
	pos.x=velInitial.x*t;
	pos.z=velInitial.z*t;
	pos.y=velInitial.y*t-0.5f*g*t*t;
	return pos;
}

void CFlyingBullet::_UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist)
{
	if (_param->g<=0.0f)
	{
		dir=_dir;
		dDist=((float)dt)*_speed;
	}
	else
	{
		_t+=(float)dt;
		LevelPos3D pos=CalcBulletPos(_dir*_speed,_t,_param->g);
		dir=pos-_pos;
		dDist=dir.getLength();
		if (dDist>0.0f)
			dir/=dDist;
		_pos=pos;
	}
}



//////////////////////////////////////////////////////////////////////////
//EoBullet
BIND_EOPARAM(EoBullet,EoParamBullet);
CBulletBase *EoBullet::_CreateBullet()
{
	CFlyingBullet *bullet=Class_New2(CFlyingBullet);
	EoParamBullet*param=GetParam<EoParamBullet>();
	bullet->Init(this,_GetInitialPos3D(),_GetInitialDir3D(),param);
	if (param->dealsStaticHit.size()>0)
		bullet->SetHiResoStaticCheck(TRUE);
	return bullet;
}

void EoBullet::_DestroyBullet(CBulletBase *bullet0)
{
	CFlyingBullet *bullet=(CFlyingBullet *)bullet0;
	Safe_Class_Delete(bullet);
}


void EoBullet::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	__super::_OnWriteFirstSync(bp,bContent,idPlayer);

	LevelPos3D dirInitial=_GetInitialDir3D();
	bp->Data_WriteSimpleR(dirInitial);
	bp->Data_WriteSimple(((CFlyingBullet*)_core)->GetSpeed());
	bContent=TRUE;
}


