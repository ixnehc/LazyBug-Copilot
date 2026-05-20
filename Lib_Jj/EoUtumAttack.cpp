
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoUtumAttack.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"
#include "timer/timer.h"



//////////////////////////////////////////////////////////////////////////
//EoUtumAttack
BIND_EOPARAM(EoUtumAttack,EoParamUtumAttack);

BOOL EoUtumAttack::_GetTakeOffTargetPos(LevelFace face,LevelPos &posTarget)
{
	EoParamUtumAttack *param=GetParam<EoParamUtumAttack>();

	LevelPos dir=LevelFaceToDir(face);
	float distClearance=1.5f;

	LevelPos posMe=_GetInitialPos();

	CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();

	float dist=param->radiusTakeOff+param->radiusTakeOffVary+distClearance;

	LevelPos posHit;
	if (unitmgr->StaticRayCast(UnitFindPath_Walkable,posMe,posMe+dir*dist,posHit))
		dist=posMe.getDistanceFrom(posHit);

	dist-=distClearance;

	if (dist<=param->radiusTakeOff-param->radiusTakeOffVary)
		return FALSE;

	dist=CSysRandom::RandRange(param->radiusTakeOff-param->radiusTakeOffVary,dist);

	posTarget=posMe+dir*dist;

	if (!unitmgr->CheckWalkableArea(UnitFindPath_Walkable,posTarget,UTUMATTACK_RADIUS_CHASE_ANCHOR))
		return FALSE;
	return TRUE;
}


BOOL EoUtumAttack::_TakeOff()
{
	EoParamUtumAttack *param=GetParam<EoParamUtumAttack>();

	if (_state.stage!=State::None)
		return FALSE;

	CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();
	if (!unitmgr)
		return FALSE;

	CRandom_Gauss rand;
	rand.srand((DWORD)GetTSC());

	LevelPos posMe=_GetInitialPos();

	BOOL bTarget=FALSE;
	LevelPos posTarget;
	if (TRUE)
	{
		CLevelObj *loTarget=_DetectFirstInRange(_GetInitialPos(),param->radiusTakeOff+param->radiusTakeOffVary+param->radiusChase);
		LevelFace faceTarget=0.0f;
		if (loTarget)
		{
			LevelPos dirTarget=loTarget->GetFramePos()-posMe;
			faceTarget=LevelFaceFromDir(dirTarget);
		}

		for (int i=0;i<5;i++)
		{
			LevelFace faceTest;
			if (loTarget)
				faceTest=CSysRandom::RandRange(faceTarget-i_math::Pi/3.0f,faceTarget+i_math::Pi/3.0f);
			else
				faceTest=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);

			if (_GetTakeOffTargetPos(faceTest,posTarget))
			{
				bTarget=TRUE;
				break;				
			}
		}

		if (!bTarget)
		{
			for (int i=0;i<5;i++)
			{
				LevelFace faceTest;
				faceTest=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);

				if (_GetTakeOffTargetPos(faceTest,posTarget))
				{
					bTarget=TRUE;
					break;				
				}
			}
		}

	}

	if (!bTarget)
		return FALSE;

	_state.stage=State::TakeOff;
	_state.tStageStart=_level->GetT_();
	_state.posTakeOffTarget.setXZ(posTarget);
	_state.posTakeOffTarget.y=CSysRandom::RandVary(param->htTakeOff,param->htTakeOffVary);
	_state.posTakeOffSrc=_GetInitialPos3D();
	_state.posTakeOffSrc.y+=1.0f;

	if (TRUE)
	{
		float dist=_state.posTakeOffSrc.getDistanceFrom(_state.posTakeOffTarget);
		_state.durTakeOff=dist/((param->speedTakeOff+param->speedChase)/2.0f);
	}

	_bSyncDirty=TRUE;

	return TRUE;
}


void EoUtumAttack::_OnPostCreate()
{
	EoParamUtumAttack *param=GetParam<EoParamUtumAttack>();

	if (!_TakeOff())
		DeferDestroy();

}

void EoUtumAttack::OnDestroy()
{
	_spline.Reset();
}


LevelPos EoUtumAttack::GetFramePos()
{
	EoParamUtumAttack *param=GetParam<EoParamUtumAttack>();

	if (_state.stage==State::TakeOff)
	{
		float a=(param->speedChase-param->speedTakeOff)/_state.durTakeOff;
		float t=ANIMTICK_TO_SECOND(_GetAge());
		float dist=param->speedTakeOff*t+0.5f*a*t*t;
		LevelPos3D dir=_state.posTakeOffTarget-_state.posTakeOffSrc;
		dir.normalize();
		LevelPos3D pos=_state.posTakeOffSrc+dir*dist;
		return pos.getXZ();
	}

	if (_state.stage==State::Chase)
	{
		CLevelObj *loTarget=LevelUtil_GetAliveLo(_level,_state.idChaseTarget);
		if (loTarget)
			_state.posChaseTargetCache=loTarget->GetFramePos();

		_spline.Build(_level->GetUnitMgr(),_state.posChaseTargetCache);

		float dist=_CalcChaseDist();

		LevelPos3D pos;
		_spline.Sample(dist,pos);

		return pos.getXZ();
	}

	return _GetInitialPos();

}


void EoUtumAttack::_WriteState(CBitPacket *bp)
{
	bp->Bits_Write(_state.stage,3);
	if (_state.stage==State::TakeOff)
		bp->Data_WriteSimpleR(_state.posTakeOffTarget);
	if (_state.stage==State::Chase)
	{
		bp->Data_WriteSimpleR(_state.idChaseTarget);
		bp->Data_WriteSimple(_state.faceChaseAnchor);
	}
	if (_state.stage==State::Return)
		bp->Bit_Write(_state.bDamaged);
}


void EoUtumAttack::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteState(bp);
	bContent=TRUE;
}

void EoUtumAttack::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Bit_Write(_bSyncDirty);
	if (_bSyncDirty)
	{
		_WriteState(bp);
		bContent=TRUE;
	}
}

void EoUtumAttack::_OnPostWriteSync()
{
	_bSyncDirty=FALSE;
}


void EoUtumAttack::_Return(BOOL bDamaged)
{
	_state.stage=State::Return;
	_state.tStageStart=_level->GetT_();
	_state.bDamaged=bDamaged;

	_bSyncDirty=TRUE;
}

void EoUtumAttack::_Chase(LevelPos &posSrc,LevelPos &dirSrc,CLevelObj *loTarget)
{
	_state.stage=State::Chase;
	_state.tStageStart=_level->GetT_();
	_state.idChaseTarget=loTarget->GetID();

	_state.posChaseSrc=posSrc;
	_state.posChaseTargetCache=loTarget->GetFramePos();
	_state.faceChaseSrc=LevelFaceFromDir(dirSrc);

	if (TRUE)
	{
		LevelPos dirTarget=loTarget->GetFramePos()-_state.posChaseSrc;

		LevelFace faceTarget=LevelFaceFromDir(dirTarget);

		LevelFaceYaw yaw=LevelFaceCalcYaw(_state.faceChaseSrc,faceTarget);

		yaw/=2.0f;

		LevelFace faceAverage=_state.faceChaseSrc;
		LevelFaceApplyYaw(faceAverage,yaw);
		_state.faceChaseAnchor=_state.faceChaseSrc;
		i_math::rotate_limited(_state.faceChaseAnchor,faceAverage,i_math::Pi/4.0f);
	}

	if (TRUE)
	{
		_spline.AddInitialPos(_state.posChaseSrc);

		LevelPos posAnchor=_state.posChaseSrc+LevelFaceToDir(_state.faceChaseAnchor)*(UTUMATTACK_RADIUS_CHASE_ANCHOR);
		_spline.AddInitialPos(posAnchor);
	}


	_bSyncDirty=TRUE;

}

float EoUtumAttack::_CalcChaseDist()
{
	EoParamUtumAttack *param=GetParam<EoParamUtumAttack>();
	float t=ANIMTICK_TO_SECOND(_level->GetT_()-_state.tStageStart);
	return param->CalcChaseDist(t);
}


void EoUtumAttack::_OnUpdate()
{
	EoParamUtumAttack *param=GetParam<EoParamUtumAttack>();

	if (_state.stage==State::Return)
	{
		if (_level->GetT_()>_state.tStageStart+ANIMTICK_FROM_SECOND(8.0f))
			DeferDestroy();
	}

	if (_state.stage==State::Chase)
	{
		_spline.Confirm(_CalcChaseDist());

		if (_state.tStageStart+ANIMTICK_FROM_SECOND(param->durChase)<_level->GetT_())
			_Return(FALSE);
		else
		{
			BOOL bTargetValid=FALSE;
			CLevelObj *loTarget=LevelUtil_GetAliveLo(_level,_state.idChaseTarget);
			if (loTarget)
			{
				if (!LevelUtil_CheckDead(loTarget))
				{
					if (!LevelUtil_CheckInvisible(loTarget))
					{
						LevelPos posMe=GetFramePos();
						LevelPos posTarget=loTarget->GetFramePos();
						if (posMe.getDistanceFrom(posTarget)<param->rangeAttack)
						{
							CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();
							if (unitmgr)
							{
								if (!unitmgr->StaticObstacleTest(UnitFindPath_Walkable,posMe,posTarget))
								{
									DealArg arg;
									arg.link.id=_level->GenOpLinkID();
									_MakeDeals(LevelOSB(this),loTarget,arg);

									_Return(TRUE);
								}
							}
						}
						bTargetValid=TRUE;
					}
				}
			}

			if (!bTargetValid)
				_Return(FALSE);
		}
	}

	if (_state.stage==State::TakeOff)
	{
		if (_GetAge()>=ANIMTICK_FROM_SECOND(_state.durTakeOff))
		{
			CLevelObj *loTarget=_DetectFirstInRange(_state.posTakeOffTarget.getXZ(),param->radiusChase);
			if (!loTarget)
				_Return(FALSE);
			else
			{
				LevelPos dir=(_state.posTakeOffTarget-_state.posTakeOffSrc).getXZ();
				dir.normalize();
				_Chase(_state.posTakeOffTarget.getXZ(),dir,loTarget);
			}
		}
	}

}

