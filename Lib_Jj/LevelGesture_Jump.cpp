/********************************************************************
	created:	2016/12/10 
	author:		cxi
	
	purpose:	闪避跳跃
*********************************************************************/
#include "stdh.h"

#include "LevelGesture_Jump.h"

#include "LevelObj.h"

#include "unitmgr/UnitMgr.h"

#include "datapacket/BitPacket.h"


BIND_GESTUREPARAM(CLevelGesture_Jump,LevelGestureParam_Jump);


void CLevelGesture_Jump::_OnCreate()
{
	if (_core.target.tp==LevelSkillTarget::Target_FixPosAndObj)
	{
		_posJump=_core.target.Pos();
		_idTarget=_core.target.ObjID();
		_posTarget=_posJump;
	}
	else
	{
		assert(FALSE);
		_bFinished=TRUE;
		return;
	}

	_UpdateTargetPos();
}

void CLevelGesture_Jump::_UpdateTargetPos()
{
	extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
	CLevelObj *lo=LevelUtil_GetAliveLo(_owner->GetLevel(),_idTarget);
	if (lo)
		_posTarget=lo->GetFramePos();
}


void CLevelGesture_Jump::_OnDestroy()
{
}

void CLevelGesture_Jump::CalcXfm(LevelGestureCore &core,float dt,LevelPos &posJump,LevelPos &posTarget,LevelPos &posCur,LevelFace &faceCur)
{
	LevelGestureParam_Jump *param=(LevelGestureParam_Jump *)core.param;

	if (core.t<param->durJumpStart)
		posCur=core.pos3DInitial.getXZ();
	else
	{
		if (core.t<param->durJumpStart+param->durJump)
		{
			float r=((float)(core.t-param->durJumpStart))/(float)param->durJump;
			posCur=core.pos3DInitial.getXZ().getInterpolated(posJump,1.0f-r);

			//转向目标
			LevelPos dir=posTarget-posCur;
			if (dir.getLengthSQ()>0.01f)
			{
				LevelFace face=LevelFaceFromDir(dir);

				LevelFace rot=param->speedRot*dt;
				i_math::rotate_limited(faceCur,face,rot);
			}
		}
		else
			posCur=posJump;
	}

}

BOOL CLevelGesture_Jump::CheckFinished(LevelGestureCore &core)
{
	LevelGestureParam_Jump *param=(LevelGestureParam_Jump *)core.param;
	if (core.t>=param->durJump+param->durJumpEnd+param->durJumpStart)
		return TRUE;
	return FALSE;
}

void CLevelGesture_Jump::Update(CUnit3D *unit,float dt)
{

}


void CLevelGesture_Jump::Update(CUnit*unit,float dt)
{
	LevelGestureParam_Jump *param=(LevelGestureParam_Jump *)_core.param;

	_ApplySpeedRate(dt);

	_core.t+=ANIMTICK_FROM_SECOND(dt);

	_UpdateTargetPos();

	LevelPos posCur;
	posCur=unit->_pos;
	LevelFace faceCur=unit->_face;
	CalcXfm(_core,dt,_posJump,_posTarget,posCur,faceCur);
	unit->_pos=posCur;
	unit->_face=faceCur;

	if (CheckFinished(_core))
		_bFinished=TRUE;

}


void CLevelGesture_Jump::WriteFirstSync(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(_posJump);
	bp->Data_WriteSimpleR(_posTarget);

	_posTargetLastSync=_posTarget;

}

BOOL CLevelGesture_Jump::WriteSync(CBitPacket *bp)
{
	if (_posTarget.getDistanceSQFrom(_posTargetLastSync)>0.02f*0.02f)
	{
		bp->Data_WriteSimpleR(_posTarget);
		_posTargetLastSync=_posTarget;
		return TRUE;
	}

	return FALSE;
}
