/********************************************************************
	created:	2019/12/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_恶魔之眼_Update.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"

#include "Random/Random.h"


IMPLEMENT_CLASS(BMO_DevilEyeStatus);

#define DISABLE_FOCUSED 1


////////////////////////////////////////////////////////////////////////
//CBgn_恶魔之眼_Update
BIND_BGN_CLASS(CBgn_DevilEye_Update,CBgp_DevilEye_Update);

void CBgn_DevilEye_Update::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (TRUE)
	{
		_bFaceLimit=FALSE;
		if (pad->paramFL.bEnable)
		{
			if (pad->paramFL.posTarget.size()>0)
			{
				_faceLimitBase=LevelFaceFromDir(pad->paramFL.posTarget[0].getXZ()-lo->GetFramePos());
				_yawLimit=pad->paramFL.range*i_math::GRAD_PI2/2.0f;
				_bFaceLimit=TRUE;
			}
		}
	}

	LevelFace faceTarget=lo->GetFrameFace();

	if (_bFaceLimit)
	{
		LevelFaceYaw yaw=LevelFaceCalcYaw(_faceLimitBase,faceTarget);
		yaw=i_math::clamp_f(yaw,-_yawLimit,_yawLimit);
		faceTarget=_faceLimitBase;
		LevelFaceApplyYaw(faceTarget,yaw);
	}

	DWORD verStatus=0;
	if (pad->varStatus!=StringID_Invalid)
	{
		BMO_DevilEyeStatus *status=NULL;
		status=_GetMem()->GetObj<BMO_DevilEyeStatus>(pad->varStatus);
		if (!status)
		{
			status=Class_New(BMO_DevilEyeStatus);
			status->tTarget=_GetT();
			status->faceTarget=faceTarget;
			status->idTarget=LevelObjID_Invalid;
			_GetMem()->DepositObj(pad->varStatus,status);
		}

		if (status)
			verStatus=status->ver;
	}

	_facing.Reset(faceTarget,level->GetT_(),verStatus+1);
	_SetState(State_LookAround);

	_tLast=_GetT();

	return;
}

BOOL CBgn_DevilEye_Update::_CheckDetected(CLevelObj *lo,float dist2)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();

	if (dist2<pad->param.radiusSense*pad->param.radiusSense)
		return TRUE;

	CLevelObj *loMe=_GetLo();
	LevelPos dir=lo->GetFramePos()-loMe->GetFramePos();
	LevelFace face=LevelFaceFromDir(dir);
	LevelFaceYaw yaw=LevelFaceCalcYaw(_faceCur,face);
	if(fabsf(yaw)>pad->param.fov*i_math::GRAD_PI2/2.0f)
		return FALSE;

	return TRUE;
}


LevelObjID CBgn_DevilEye_Update::_DetectClosest(BOOL &bOutlook)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
	LevelUtilDetectParam param;
	param.loSrc=lo;
	param.pos=lo->GetFramePos();
	param.toIgnores=NULL;
	param.nIgnores=0;
	param.flags=&pad->flagsDetect[0];
	param.nFlags=pad->flagsDetect.size();
	param.requires=&pad->requires[0];
	param.nRequires=pad->requires.size();
	param.rangeMin=0.0f;
	param.rangeMax=pad->param.radiusOutlook;
	param.bTouching=1;
	param.weights.OverrideFrom(pad->weights);

	LevelObjMapEnumCallBack dlgt;
	dlgt.bind(this,&CBgn_DevilEye_Update::_CheckDetected);

	_faceCur=_facing.GetCur(lo);

	bOutlook=FALSE;
	CLevelObj *loDetected=LevelUtil_DetectBest(param,dlgt);
	if (loDetected)
	{
		float dist=loDetected->GetFramePos().getDistanceFrom(lo->GetFramePos());
		if (dist>pad->param.radiusSight)
			bOutlook=TRUE;
	}

#ifdef DISABLE_FOCUSED
	bOutlook=TRUE;
#endif

	return loDetected?loDetected->GetID():LevelObjID_Invalid;
}

void CBgn_DevilEye_Update::_GenSwitchFacingTime()
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();

	AnimTick dur=CSysRandom::RandRangeInt(pad->param.durMinSwitchFaceCD,pad->param.durMinSwitchFaceCD);
	_tNextSwitchFacing=_GetT()+dur;

}


void CBgn_DevilEye_Update::_SetState(State state)
{
	if (_state==state)
		return;

	_state=state;
	_tStateStart=_GetT();

	if (_state==State_LookAround)
		_GenSwitchFacingTime();
}

BOOL CBgn_DevilEye_Update::_CheckFaceLimit(LevelFace face)
{
	if (!_bFaceLimit)
		return TRUE;

	LevelFaceYaw yaw=LevelFaceCalcYaw(_faceLimitBase,face);
	if ((yaw>=-_yawLimit)&&(yaw<_yawLimit))
		return TRUE;
	return FALSE;
}


BOOL CBgn_DevilEye_Update::_Update_LookAround(AnimTick tCur,AnimTick dt)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	if(_state!=State_LookAround)
		return FALSE;

	BOOL bOutlook;
	LevelObjID idDetected=_DetectClosest(bOutlook);
	if (idDetected!=LevelObjID_Invalid)
	{
		if (!bOutlook)
		{
			_facing.SetTarget(lo,idDetected,360.f*i_math::GRAD_PI2);
			_SetState(State_Focusing);
			return TRUE;
		}

		_facing.SetTarget(lo,idDetected,180.f*i_math::GRAD_PI2);
		_SetState(State_OutlookFocusing);
		return TRUE;
	}

	_DecAlert(dt);

	AnimTick t=_GetT();
	if (!_facing.IsReached(t))
		return TRUE;

	if (t>=_tNextSwitchFacing)
	{
		BOOL bValidFaceTarget=FALSE;
		LevelFace faceTarget;
		for (int k=0;k<5;k++)
		{
			LevelFaceYaw yaw=CSysRandom::RandRange(pad->param.angleMinSwitchFace,pad->param.angleMaxSwitchFace);
			yaw*=i_math::GRAD_PI2;
			faceTarget=_facing.GetCur(lo);
			if (CSysRandom::Roll(0.5f))
				LevelFaceApplyYaw(faceTarget,yaw);
			else
				LevelFaceApplyYaw(faceTarget,-yaw);

			if (_CheckFaceLimit(faceTarget))
			{
				bValidFaceTarget=TRUE;
				break;
			}
		}

		if (bValidFaceTarget)
		{
			_facing.SetTarget(lo,faceTarget,180.f*i_math::GRAD_PI2);
			_GenSwitchFacingTime();
		}
	}

	return TRUE;
}

BOOL CBgn_DevilEye_Update::_CheckValidFocusingTarget(LevelObjID idTarget)
{
	CLevel *level=_GetLevel();

	CLevelObj *lo=LevelUtil_GetAliveLo(level,idTarget);
	if (!lo)
		return FALSE;
	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	if (LevelUtil_CheckDead(lo))
		return FALSE;

	return TRUE;
}


BOOL CBgn_DevilEye_Update::_Update_Focusing(AnimTick tCur,AnimTick dt)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	if(_state!=State_Focusing)
		return FALSE;

	if (!_CheckValidFocusingTarget(_facing._idTarget))
	{
		_SetState(State_LookAround);
		return TRUE;
	}

	if (!_facing.IsReached(_GetT()))
		return TRUE;

	_SetState(State_Focused);
	return TRUE;
}

void CBgn_DevilEye_Update::_IncAlert(AnimTick dt)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	_vAlert+=pad->param.spdIncAlert*ANIMTICK_TO_SECOND(dt);
	if (_vAlert>1.0f)
		_vAlert=1.0f;
}

void CBgn_DevilEye_Update::_DecAlert(AnimTick dt)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	_vAlert-=pad->param.spdDecAlert*ANIMTICK_TO_SECOND(dt);
	if (_vAlert<0.0f)
		_vAlert=0.0f;
}


BOOL CBgn_DevilEye_Update::_Update_OutlookFocusing(AnimTick tCur,AnimTick dt)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	if(_state!=State_OutlookFocusing)
		return FALSE;

	if (!_CheckValidFocusingTarget(_facing._idTarget))
	{
		_SetState(State_LookAround);
		return TRUE;
	}

	if (!_facing.IsReached(_GetT()))
		return TRUE;

	_SetState(State_OutlookFocused);
	return TRUE;
}

BOOL CBgn_DevilEye_Update::_Update_OutlookFocused(AnimTick tCur,AnimTick dt)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	if(_state!=State_OutlookFocused)
		return FALSE;

	if (!_CheckValidFocusingTarget(_facing._idTarget))
	{
		_SetState(State_LookAround);
		return TRUE;
	}

	BOOL bOutlook=FALSE;
	if (TRUE)
	{
		CLevelObj *loFocused=LevelUtil_GetAliveLo(level,_facing._idTarget);
		if (loFocused)
		{
			float dist=loFocused->GetFramePos().getDistanceFrom(lo->GetFramePos());
			if (dist<pad->param.radiusOutlook+1.0f)
				bOutlook=TRUE;
		}
	}

	if (bOutlook)
		_IncAlert(dt);
	else
		_DecAlert(dt);

	if (_vAlert<=0.0f)
	{
		_SetState(State_LookAround);
		return TRUE;
	}

#ifndef DISABLE_FOCUSED
	if(_vAlert>=1.0f)
	{
		_SetState(State_Focused);
		return TRUE;
	}
#endif

	return TRUE;

}



void CBgn_DevilEye_Update::Update(BGNOutputs &outputs)
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	AnimTick tCur,dt;
	if (TRUE)
	{
		AnimTick t=_GetT();
		dt=ANIMTICK_SAFE_MINUS(t,_tLast);
		tCur=t;
		_tLast=t;
	}

	BMO_DevilEyeStatus *status=NULL;
	if (pad->varStatus!=StringID_Invalid)
		status=_GetMem()->GetObj<BMO_DevilEyeStatus>(pad->varStatus);

	while(1)
	{
		if (_Update_LookAround(tCur,dt))
			break;
		if (_Update_Focusing(tCur,dt))
			break;
		if (_Update_OutlookFocusing(tCur,dt))
			break;
		if (_Update_OutlookFocused(tCur,dt))
			break;
		break;
	}

	if (status)
	{
		if (status->ver!=_facing._ver)
		{
			status->ver=_facing._ver;
			status->faceTarget=_facing._faceTarget;
			status->idTarget=_facing._idTarget;
			status->tTarget=_facing._tSrc+_facing._durRotate;
			_GetMem()->DepositObj(pad->varStatus,status);
		}
	}

	if(_state==State_Focused)
	{
		_OutputOk(outputs,1,"结束");
	}

}

void CBgn_DevilEye_Update::Break(BGNOutputs &outputs)
{
	Destroy();
}

void CBgn_DevilEye_Update::Destroy()
{
	CBgp_DevilEye_Update*pad=_GetPad<CBgp_DevilEye_Update>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	BMO_DevilEyeStatus *status=NULL;
	if (pad->varStatus!=StringID_Invalid)
		status=_GetMem()->GetObj<BMO_DevilEyeStatus>(pad->varStatus);

	if (status)
	{
		status->faceTarget=lo->GetFrameFace();
		status->tTarget=lo->GetT()+ANIMTICK_FROM_SECOND(0.5f);
		status->idTarget=LevelObjID_Invalid;
		status->ver++;

		_GetMem()->DepositObj(pad->varStatus,status);
	}

}
