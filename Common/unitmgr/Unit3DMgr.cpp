
#include "stdh.h"

#include "Unit3DMgr.h"
#include "UnitMgr.h"

#include "commondefines/general_stl.h"

#include "timer/profiler.h"

#include "gamedata/GameTileMap.h"

#define VERIFY_ALIVE(p)	if (p) {if (!(p)->IsAlive())	SAFE_RELEASE(p);}


void CUnit3D::Init(i_math::vector3df &pos,float face,i_math::vector3df &vel,GameTileMap *gtm,UnitFly *fly)
{
	_pos=pos;
	_face=face;
	_vel=vel;
	_gtm=gtm;
	_fly=fly;

}



void CUnit3D::_ClearTarget()
{
	SAFE_RELEASE(_target);
	SAFE_RELEASE(_target3D);
	_tpTarget=Target_None;
}

void CUnit3D::Clear()
{
	if (_bAlive)
	{
		_ClearTarget();
		extern void DestroyPathLink(CUnitPath *pathes);
		DestroyPathLink(_toMe);

		_infoGesture.Clear();

		Zero();
	}
}

void CUnit3D::ResetIdle()
{
	_ClearTarget();
	_state=Idle;
}

void CUnit3D::Reset(i_math::vector3df &pos,float face)
{
	_pos=pos;
	_face=face;
	_vel.set(0,0,0);
	ResetIdle();
}



void CUnit3D::SetTarget_Unit(CUnit *target,float range)
{
	ResetIdle();

	VERIFY_ALIVE(target);
	if (!target)
		return;

	SAFE_REPLACE(_target,target);
	_rangeFollow=range;
	_tpTarget=Target_Unit;
	_state=PreFollow;
}

void CUnit3D::SetTarget_Unit3D(CUnit3D *target,float range)
{
	ResetIdle();

	VERIFY_ALIVE(target);
	if (!target)
		return;

	SAFE_REPLACE(_target3D,target);
	_rangeFollow=range;
	_tpTarget=Target_Unit3D;
	_state=PreFollow;
}

void CUnit3D::SetTarget_GroundUnit(CUnit *target,float htCenter,float range)
{
	ResetIdle();

	VERIFY_ALIVE(target);
	if (!target)
		return;

	SAFE_REPLACE(_target,target);
	_rangeFollow=range;

	_htTargetCenter=htCenter;
	_tpTarget=Target_GroundUnit;
	_state=PreFollow;
}


void CUnit3D::SetTarget_Pos3D(i_math::vector3df &pos,float range)
{
	ResetIdle();

	_targetPos3D=pos;
	_rangeFollow=range;
	_tpTarget=Target_Pos3D;
	_state=PreFollow;
}

void CUnit3D::SetTarget_Pos(i_math::vector2df &pos,float range)
{
	ResetIdle();

	_targetPos=pos;
	_rangeFollow=range;
	_tpTarget=Target_Pos;
	_state=PreFollow;
}

void CUnit3D::SetSpeed(float speed)
{
	if (_fly)
		_speedScale=(_fly->speed!=0.0f)?speed/_fly->speed:0.0f;
	else
		_speedScale=0.0f;
}


void CUnit3D::_ClampGround(i_math::vector3df &pos)
{
	if (_gtm)
		_gtm->Clamp(pos);
}


inline float LevelFaceLerp(float faceFrom,float faceTo,float r)
{
	float gap=i_math::get_radian_dist(faceFrom,faceTo);
	gap*=r;
	float face=faceFrom;
	i_math::rotate_limited(face,faceTo,gap);
	return face;
}

void CUnit3D::UpdateState(float dt)
{
	if (_infoGesture.IsValid())
	{
		_infoGesture.UpdateState(this,dt);
		return;
	}

	if (_state==Idle)
	{
		_vel.setZero();
		return;
	}

	if ((_tpTarget==Target_Unit)||(_tpTarget==Target_GroundUnit))
	{
		VERIFY_ALIVE(_target);
		if (!_target)
		{
			_state=Idle;
			return;
		}
	}
	if (_tpTarget==Target_Unit3D)
	{
		VERIFY_ALIVE(_target3D);
		if (!_target3D)
		{
			_state=Idle;
			return;
		}
	}

	i_math::vector3df posTarget;
	switch(_tpTarget)
	{
		case Target_Pos3D:
			posTarget=_targetPos3D;
			break;
		case Target_Pos:
		{
			posTarget.x=_targetPos.x;
			posTarget.z=_targetPos.y;
			posTarget.y=_pos.y;
			float ht=_gtm->GetHeight(posTarget.x,posTarget.z);
			posTarget.y=i_math::clamp_f(posTarget.y,ht+_fly->GetHangLow(),ht+_fly->GetHangHi());

			break;
		}
		case Target_Unit:
		{
			posTarget.x=_target->GetPos().x;
			posTarget.z=_target->GetPos().y;

			posTarget.y=_pos.y;
			float ht=_gtm->GetHeight(posTarget.x,posTarget.z);
			posTarget.y=i_math::clamp_f(posTarget.y,ht+_fly->GetHangLow(),ht+_fly->GetHangHi());
			break;
		}
		case Target_Unit3D:
		{
			posTarget=_target3D->GetPos();
			break;
		}
		case Target_GroundUnit:
		{
			posTarget.x=_target->GetPos().x;
			posTarget.z=_target->GetPos().y;

			posTarget.y=_gtm->GetHeight(posTarget.x,posTarget.z);
			posTarget.y+=_htTargetCenter;

			break;
		}
	}

	float speed=GetSpeed();

	i_math::vector3df dir=posTarget-_pos;
	float dist=(float)dir.getLength();
	if (_state==PostFollow)
	{
		if (dist<=_rangeFollow)
			return;
	}
	if (dist<=_rangeFollow)
	{
		_state=PostFollow;
	}
	else
	{
		if (dist<speed*dt)
		{
			_pos=posTarget;
			_state=PostFollow;
			return;
		}

		_state=Follow;
		i_math::vector3df vel;
		vel=dir;
		vel.normalize();
		vel*=speed;

		float blend=_fly->blendSpeed;

		if (FALSE)
		{
			_vel=_vel*(1-blend)+vel*blend;
			float len=(float)_vel.getLength();
	//		_vel=vel;
			if (_vel.getLengthSQ()*dt*dt>dist*dist)
				_vel.setLength(dist/dt);

			_pos+=dt*_vel;

			//更新face
			if (TRUE)
			{
				float face=atan2f(_vel.z,_vel.x);
				float delta=_fly->speedFace*i_math::GRAD_PI2*dt;
				i_math::rotate_limited(_face,face,delta);
			}
		}
		else
		{
			float dist=vel.getLength();
			i_math::vector3df euler=vel;
			euler.normalize();
			euler.toEuler();

			float distOld=_vel.getLength();
			i_math::vector3df eulerOld;
			if (distOld>0.1f)
			{
				eulerOld=_vel;
				eulerOld.normalize();
				eulerOld.toEuler();
			}
			else
			{
				eulerOld.setZero();
				eulerOld.x=i_math::Pi/2.0f-_face;
			}

			eulerOld.x=LevelFaceLerp(eulerOld.x,euler.x,blend);
			eulerOld.y=LevelFaceLerp(eulerOld.y,euler.y,blend);
			eulerOld.z=LevelFaceLerp(eulerOld.z,euler.z,blend);

			_vel=eulerOld;
			_vel.eulerToDir();
			_vel*=(i_math::lerp(distOld,dist,blend));

			_pos+=dt*_vel;

			//更新face
			if (TRUE)
			{
				float face=atan2f(_vel.z,_vel.x);
				float delta=_fly->speedFace*i_math::GRAD_PI2*dt;
				i_math::rotate_limited(_face,face,delta);
			}
		}

		_ClampGround(_pos);
		
	}
}

void CUnit3D::AccumVelPos(i_math::vector3df &velOrg,float blend,float dt)
{
	i_math::vector3df vel;
	vel=_vel*(1.0f-blend)+velOrg*blend;
	float length=vel.getLength();
	i_math::vector3df pos=_pos+vel*dt;
	_ClampGround(pos);
	vel=(pos-_pos);
	vel.setLength(length);
	_pos+=vel*dt;
	_vel=vel;

	//更新face
	if (TRUE)
	{
		float face=atan2f(_vel.z,_vel.x);
		float delta=_fly->speedFace*i_math::GRAD_PI2*dt;
		i_math::rotate_limited(_face,face,delta);
	}

	_ClampGround(_pos);
}

void CUnit3D::AccumVelPosWithFace(i_math::vector3df &velOrg,float blend,float face,float dt)
{
	i_math::vector3df vel;
	vel=_vel*(1.0f-blend)+velOrg*blend;
	float length=vel.getLength();
	i_math::vector3df pos=_pos+vel*dt;
	_ClampGround(pos);
	vel=(pos-_pos);
	vel.setLength(length);
	_pos+=vel*dt;
	_vel=vel;

	//更新face
	if (TRUE)
	{
		float delta=_fly->speedFace*i_math::GRAD_PI2*dt;
		i_math::rotate_limited(_face,face,delta);
	}

	_ClampGround(_pos);
}



////////////////////////////////////////////////////////////////////////
//CUnit3DMgr
void CUnit3DMgr::Init(GameTileMap *gtm)
{
	_gtm=gtm;
}

void CUnit3DMgr::Clear()
{
	for (int i=0;i<_units.size();i++)
	{
		_units[i]->Destroy();
	}
	_units.clear();
	Zero();
}


void CUnit3DMgr::Update(float dt)
{
	DWORD cDead=0;
	for (int i=0;i<_units.size();i++)
	{
		CUnit3D *unit=_units[i];
		if (!unit->IsAlive())
		{
			cDead++;
			continue;
		}

		unit->UpdateState(dt);
	}

	if (cDead>_units.size()/4)
		_bNeedFlushDead=TRUE;
	
}

void CUnit3DMgr::_GarbageCollect()
{
	if (_bNeedFlushDead)
	{
		DWORD c=0;
		for (int i=0;i<_units.size();i++)
		{
			if (_units[i]->IsAlive())
				_units[c++]=_units[i];
			else
				SAFE_RELEASE(_units[i]);
		}
		_units.resize(c);

		_bNeedFlushDead=FALSE;
	}

	if (_units.size()<=0)
		return;

	DWORD nStep=_units.size()/4+1;//至少有一个step

	if (nStep>_units.size())
		nStep=_units.size();

	for (int i=0;i<nStep;i++)
	{
		int idx=(_idxGC+i)%_units.size();
		CUnit3D *unit=_units[idx];
		if (!unit->IsAlive())
			continue;

		//清除unit的_toMe中没有被用到的路径
		if (TRUE)
		{
			CUnitPath **p=&unit->_toMe;
			while(*p)
			{
				if ((*p)->GetRef()==1)
				{//只有一个引用计数,没有别人引用它了
					CUnitPath *next=(*p)->_next;
					CUnitPath *path=(*p);
					SAFE_DESTROY(path);

					(*p)=next;
					continue;
				}
				p=&(*p)->_next;
			}
		}
	}

	_idxGC=(_idxGC+nStep)%_units.size();

}


CUnit3D *CUnit3DMgr::CreateUnit3D(i_math::vector3df &pos,float face,UnitFly *fly)
{
	CUnit3D *unit=Class_New2(CUnit3D);
	unit->Init(pos,face,i_math::vector3df(0,0,0),_gtm,fly);
	unit->_bAlive=1;
	unit->AddRef();
	_units.push_back(unit);
	unit->AddRef();
	return unit;
}



