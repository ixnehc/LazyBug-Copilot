
#include "stdh.h"

#include "BulletBase.h"

#include "EoBulletBase.h"


#include "LoUnit.h"
#include "LoEffectObj.h"
#include "Level.h"


#include "timer/timer.h"


//根据起点和瞄准点,产生若干个散射的方向
void ScatterThrowTarget(LevelPos3D &src,LevelPos3D &target,LevelPos3D*targets,DWORD c)
{
	if (c==1)
	{
		targets[0]=target;
		return;
	}

	float MaxRange=90.0f*(float)i_math::GRAD_PI2;
	float MinRange=10.0f*(float)i_math::GRAD_PI2;
	float MaxDist=12.0f;

	LevelPos dir;
	dir.x=target.x-src.x;
	dir.y=target.z-src.z;
	float dist=dir.getLength();
	if (dist>MaxDist)
		dist=MaxDist;
	float range=MaxRange-(MaxRange-MinRange)*dist/MaxDist;

	float rad=atan2f(dir.y,dir.x);

	float step=range*2.0f/(float)(c-1);

	float radFrom=rad-range;

	for (int i=0;i<c;i++)
	{
		float r=radFrom+step*(float)i;
		targets[i].x=src.x+cosf(r)*dist;
		targets[i].z=src.y+sinf(r)*dist;
		targets[i].y=target.y;
	}

}


//根据起点和瞄准点,产生若干个散射的方向
void ScatterBulletDirs(LevelPos3D &src,LevelPos3D &aim,LevelPos *dirs,DWORD c)
{
	if (c==1)
	{
		dirs[0].x=aim.x-src.x;
		dirs[0].y=aim.z-src.z;
		dirs[0].normalize();
		return;
	}

	float MaxDist=12.0f;

	LevelPos dir;
	dir.x=aim.x-src.x;
	dir.y=aim.z-src.z;

	float dist=dir.getLength();
	if (dist>MaxDist)
		dist=MaxDist;

	float rad=atan2f(dir.y,dir.x);


	float step=atan2f(0.5f,dist);
	float radFrom=rad-step*(c-1)/2;

	for (int i=0;i<c;i++)
	{
		float r=radFrom+step*(float)i;
		dirs[i].x=cosf(r);
		dirs[i].y=sinf(r);
	}
}

void ScatterBulletDirs(LevelPos3D &src,LevelPos3D &aim,LevelPos3D *dirs3D,DWORD c)
{
	if (c==1)
	{
		dirs3D[0]=aim-src;
		dirs3D[0].normalize();
		return;
	}

	float MaxDist=12.0f;

	LevelPos dir;
	dir.x=aim.x-src.x;
	dir.y=aim.z-src.z;

	float dist=dir.getLength();
	if (dist>MaxDist)
		dist=MaxDist;

	float rad=atan2f(dir.y,dir.x);

	float step=atan2f(0.5f,dist);
	float radFrom=rad-step*(c-1)/2;

	float length=(float)(src-aim).getLengthXZ();

	if (length<0.005f)
		length=0.005f;

	for (int i=0;i<c;i++)
	{
		float r=radFrom+step*(float)i;
		dirs3D[i].x=cosf(r)*length;
		dirs3D[i].z=sinf(r)*length;
		dirs3D[i].y=aim.y-src.y;
		dirs3D[i].normalize();
	}
}


void ScatterBulletDirs_Uniform(LevelPos &dir,LevelPos *dirs,DWORD c,float fov)
{
	if (c==1)
	{
		dirs[0].x=dir.x;
		dirs[0].y=dir.y;
		dirs[0].normalize();
		return;
	}

	float rad=atan2f(dir.y,dir.x);

	float step=fov/(float)(c+1);
	float radFrom=rad-fov/2.0f;

	for (int i=0;i<c;i++)
	{
		float r=radFrom+step*(float)(i+1);
		dirs[i].x=cosf(r);
		dirs[i].y=sinf(r);
	}
}



////////////////////////////////////////////////////////////////////////
//CBulletBase
// LevelObjID CBulletBase::_DetectHit(i_math::line3df &line)
// {
// 	LevelObjID idHit=LevelObjID_Invalid;
// 
// 	GameTileMap *gtm=_level->GetGtm();
// 
// 	//XXXXX:目前只考虑GoUnit的情况
// 	i_math::rectf rc;
// 	rc.set(line.start.x,line.start.z,line.end.x,line.end.z);
// 	rc.repair();
// 	CUnitMap *mp=_level->GetUnitMgr()->GetMap();
// 	mp->Enum(rc);
// 
// 	DWORD c;
// 	CUnitBase **units=(CUnitBase **)mp->GetEnums(c);
// 
// 	//XXXXX:目前只考虑LoUnit的情况
// 	for (int i=0;i<c;i++)
// 	{
// 		CUnit *unit=(CUnit *)units[i];
// 		CLoUnit*loUnit=(CLoUnit*)unit->GetData();
// 
// 		if (!IsClass2(loUnit,CLoUnit))
// 			continue;
// 
// 		if (loUnit==_owner)
// 			continue;
// 
// 		if (_absorb)
// 		{
// 			if (_absorb->idTarget==loUnit->GetID())
// 				continue;
// 		}
// 
// 		if (!_CanHit(loUnit))
// 			continue;
// 
// 		LevelPos pos=loUnit->GetFramePos();
// 		LevelPos3D pos3D;
// 		LevelPos3DFrom2D(pos3D,pos,gtm);
// 
// 		LevelPos3D posHit;
// 		extern BOOL LevelUtil_UnitHitTest(i_math::line3df &line,i_math::vector3df &center,float radius,float fall,float height,i_math::vector3df &vHit);
// 		if (LevelUtil_UnitHitTest(line,pos3D,loUnit->GetRadius_()+_radius,_fall,loUnit->GetHeight()+_radius,posHit))
// 		{
// 			line.end=posHit;
// 			idHit=loUnit->GetID();
// 		}
// 	}
// 
// 	return idHit;
// 
// }


BOOL CBulletBase::Update(AnimSecond dt,LevelObjHits &hits,BulletStaticHit &hitStatic,LevelObjID &hitAbsorb)
{
	hits.Zero();
	hitStatic.Zero();
	hitAbsorb=LevelObjID_Invalid;

	_history.Update(ANIMTICK_FROM_SECOND(dt));

	if (!_level)
		return FALSE;

	GameTileMap *gtm=_level->GetGtm();
	CGameTrisMap *gtr=_level->GetGtr();

	float dDist;
	LevelPos3D dir;

	if (_absorb)
	{
		dDist=(float)dt*_paramBase->speed;
		float distNew=_dist+dDist;
		if (_dist<=_absorb->distStart+_absorb->spline.GetDistance())
		{
			if (distNew>_absorb->distStart+_absorb->spline.GetDistance())
			{
				hitAbsorb=_absorb->idTarget;
			}
		}
		else
		{
			_level=NULL;
			return FALSE;
		}

		LevelPos3D posNew=_absorb->spline.GetPosition(distNew-_absorb->distStart);
		dir=posNew-_pos;
	}
	else
	{
		if (_NeedStop())
		{
			_level=NULL;
			return FALSE;
		}
		_UpdateStep(dt,dir,dDist);
	}


	BOOL bStaticReached=FALSE;

	//首先进行场景碰撞,如果有碰撞,更新line.end
	if (!_absorb)
	{
		if (_dist+dDist>_distIgnoreStatic)
		{
			if (!_bHiResoStaticCheck)
			{
				float step=gtm->hdr.lenTile;
				float d;
				if (_dist>_distIgnoreStatic)
					d=0.0f;
				else
					d=_distIgnoreStatic-_dist;
				LevelPos3D pos;
				while(d<dDist)
				{
					d+=step;
					pos=_pos+dir*d;
					float ht=gtm->GetHeight(pos.x,pos.z);
					ht-=0.5f;//增加一点宽容度
					if (ht>pos.y)
					{
						dDist=d;
						bStaticReached=TRUE;
						break;
					}
				}
			}
			else
			{
				i_math::vector3df posHit;
				if (gtr->RayCheck(_pos,_pos+dir*dDist,posHit))
				{
					bStaticReached=TRUE;
					dDist=posHit.getDistanceFrom(_pos);
				}
			}
		}
	}

	i_math::line3df line;
	line.start=_pos;
	line.end=_pos+dir*dDist;

	BOOL bShieldAmuletReached=FALSE;
	LevelObjID idShieldAmulet=LevelObjID_Invalid;
	if (!_absorb)
	{
		LevelObjID id=_DetectHit_ShieldAmulet(line);
		if (id!=LevelObjID_Invalid)
		{
			dDist=line.getLength();
			idShieldAmulet=id;
			bShieldAmuletReached=TRUE;
		}
	}

	if (bShieldAmuletReached)
	{
		hitStatic.tp=BulletStaticHit::ShieldAmulet;
		hitStatic.id=idShieldAmulet;
		_level=NULL;
		return FALSE;
	}

	if (!_bPenetrate)
	{
		if (!_absorb)
		{
			LevelObjID idHit=_DetectHit(line);
			if (idHit!=LevelObjID_Invalid)
			{
				_dist+=dDist;
				_pos=line.end;
				_dir=dir;

				hits.Add(idHit);
				_level=NULL;
				return FALSE;
			}
		}
	}
	else
		_DetectHits(line,hits,_history);

	_dist+=dDist;
	_pos=line.end;
	_dir=dir;

	if (bStaticReached)
	{
		_level=NULL;
		hitStatic.tp=BulletStaticHit::Default;
		return FALSE;
	}
	return TRUE;
}


float CBulletBase::_AdjustThrowSpeed(CLoEffectObj *owner,LevelPos3D &posInitial,i_math::vector3df &dirInitial,float speedInitial,float speedAdj,float g,int modeThrowAim)
{
	float speedResult=speedInitial;

	if (modeThrowAim==2)
		return speedResult;

	CLevelSkill *skill=owner->GetRootSkill();
	if (skill)
	{
		LevelPos3D posTarget;
		extern BOOL LevelUtil_CalcTargetPos3D(CLevel *level,LevelSkillTarget &target,LevelPos3D &pos3D);
		if (LevelUtil_CalcTargetPos3D(owner->GetLevel(),skill->GetTarget(),posTarget))
		{
			if (modeThrowAim==1)
			{
				extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
				CLevelObj *lo=LevelUtil_GetTargetObj(_owner->GetLevel(),skill->GetTarget());
				if (lo)
					posTarget.y+=lo->GetAimHeight();
			}
			float t=-1.0f;
			if (TRUE)
			{
				if (dirInitial.getLengthXZ()>0.0f)
				{
					float r=dirInitial.y/dirInitial.getLengthXZ();
					float sH=posTarget.getDistanceXZFrom(posInitial);
					float sV=posTarget.y-posInitial.y;

					float tSQ=(sV-sH*r)/(-0.5f*g);
					if (tSQ>=0.0f)
						t=sqrtf(tSQ);
				}
			}
			float speed=100000.0f;
			if (t>0.0f)
				speed=posTarget.getDistanceXZFrom(posInitial)/t;
			speedResult=i_math::clamp_f(speed,speedInitial-speedAdj,speedInitial+speedAdj);

			LevelPos3D velInitial;
			velInitial=dirInitial*speedInitial;

			if (velInitial.getLengthXZ()>0.0f)
				speedResult*=velInitial.getLength()/velInitial.getLengthXZ();
		}
	}

	return speedResult;
}
