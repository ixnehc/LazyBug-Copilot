/********************************************************************
	created:	2016/11/10 
	author:		cxi
	
	purpose:	飞行冲刺
*********************************************************************/
#include "stdh.h"

#include "LevelGesture_FlyThrust.h"

#include "LevelObj.h"

#include "unitmgr/UnitMgr.h"

#include "datapacket/BitPacket.h"


BIND_GESTUREPARAM(CLevelGesture_FlyThrust,LevelGestureParam_FlyThrust);


void CLevelGesture_FlyThrust::_OnCreate()
{
	extern BOOL LevelUtil_CalcTargetPos3D(CLevel *level,LevelSkillTarget &target,LevelPos3D &pos3D);
	if (!LevelUtil_CalcTargetPos3D(_owner->GetLevel(),_core.target,_pos3DTarget))
	{
		_bFinished=TRUE;
		return;
	}
}

void CLevelGesture_FlyThrust::_OnDestroy()
{
}

void CLevelGesture_FlyThrust::CalcXfm(LevelGestureCore &core,float dt,LevelPos3D &posTarget,LevelPos3D &posCur,LevelFace &faceCur)
{
	LevelGestureParam_FlyThrust *param=(LevelGestureParam_FlyThrust *)core.param;

	LevelPos3D dir=posTarget-core.pos3DInitial;

	LevelPos3D off;
	LevelFace faceTarget=faceCur;

	if (TRUE)
	{
		float r=param->ver.GetFloat(core.t);
		off.y=dir.y*(1.0f-r);
	}

	if (TRUE)
	{
		float r=param->hor.GetFloat(core.t);

		LevelPos dir2D=dir.getXZ();
		float dist=dir2D.getLength();
		float distKeep=core.radiusOwner+core.radiusTarget;

		dist-=distKeep;
		if (dist<0.0f)
			dist=0.0f;

		dir2D.safe_normalize();
		dir2D*=dist;

		off.x=dir2D.x*r;
		off.z=dir2D.y*r;

		if (dir2D.getLengthSQ()>0.0001f)
			faceTarget=LevelFaceFromDir(dir2D);
	}

	posCur=core.pos3DInitial+off;

	float delta=dt*param->speedRot*i_math::GRAD_PI2;
	i_math::rotate_limited(faceCur,faceTarget,delta);
}

BOOL CLevelGesture_FlyThrust::CheckFinished(LevelGestureCore &core)
{
	LevelGestureParam_FlyThrust *param=(LevelGestureParam_FlyThrust *)core.param;
	if (core.t>=param->dur)
		return TRUE;
	return FALSE;
}


void CLevelGesture_FlyThrust::_UpdateTargetPos(LevelPos &pos)
{
	LevelGestureParam_FlyThrust *param=(LevelGestureParam_FlyThrust *)_core.param;
	if (!_bStopFollow)
	{
		LevelPos3D pos3DTarget;
		extern BOOL LevelUtil_CalcTargetPos3D(CLevel *level,LevelSkillTarget &target,LevelPos3D &pos3D);
		if (LevelUtil_CalcTargetPos3D(_owner->GetLevel(),_core.target,pos3DTarget))
		{
			float radius=param->radiusStopFollow;
			radius+=_core.radiusOwner+_core.radiusTarget;
			if (pos.getDistanceSQFrom(pos3DTarget.getXZ())<radius*radius)
				_bStopFollow=TRUE;
			else
				_pos3DTarget=pos3DTarget;
		}
	}
}


void CLevelGesture_FlyThrust::Update(CUnit3D *unit,float dt)
{
	LevelGestureParam_FlyThrust *param=(LevelGestureParam_FlyThrust *)_core.param;

	_ApplySpeedRate(dt);
	_core.t+=ANIMTICK_FROM_SECOND(dt);

	_UpdateTargetPos(unit->_pos.getXZ());

	LevelPos3D posCur=unit->_pos;
	LevelFace faceCur=unit->_face;
	CalcXfm(_core,dt,_pos3DTarget,posCur,faceCur);
	i_math::vector3df vel=(posCur-unit->_pos)/dt;
	unit->_pos=posCur;
	unit->_vel=vel;
	unit->_face=faceCur;

	unit->_ClampGround(unit->_pos);

	if (_core.t>=param->dur)
		_bFinished=TRUE;
}

void CLevelGesture_FlyThrust::Update(CUnit*unit,float dt)
{
	LevelGestureParam_FlyThrust *param=(LevelGestureParam_FlyThrust *)_core.param;

	_ApplySpeedRate(dt);

	_core.t+=ANIMTICK_FROM_SECOND(dt);

	_UpdateTargetPos(unit->_pos);

	LevelPos3D posCur;
	posCur.setXZ(unit->_pos);
	LevelFace faceCur=unit->_face;
	CalcXfm(_core,dt,_pos3DTarget,posCur,faceCur);
	unit->_pos=posCur.getXZ();
	unit->_face=faceCur;

	if (CheckFinished(_core))
		_bFinished=TRUE;

}


void CLevelGesture_FlyThrust::WriteFirstSync(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(_pos3DTarget);

	_pos3DTargetLastSync=_pos3DTarget;
}

BOOL CLevelGesture_FlyThrust::WriteSync(CBitPacket *bp)
{
	if (_pos3DTarget.getDistanceFromSQ(_pos3DTargetLastSync)>0.02f*0.02f)
	{
		bp->Data_WriteSimpleR(_pos3DTarget);
		_pos3DTargetLastSync=_pos3DTarget;
		return TRUE;
	}

	return FALSE;
}
