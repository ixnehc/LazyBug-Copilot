
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoMagnetBall.h"

#include "LevelRecords.h" 

#include "LevelOSB.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//CMagnetBall
void CMagnetBall::Init(CLevel *level,EoParamMagnetBall *param,LevelPos &pos,float face,AnimTick t)
{
	_level=level;
	_param=param;

	_state.t=t;
	_state.pos.setXZ(pos);
	_state.pos.y=_level->GetGtr()->GetHeight(pos.x,pos.y);
	_state.pos.y+=CSysRandom::RandVary(param->ht,param->htVary);
	if (param->ht>0.0f)
		_state.bSupport=FALSE;
	else
		_state.bSupport=TRUE;
	_state.face=face;
	_state.spdVer=CSysRandom::RandVary(param->speedVerInitial,param->speedVerInitialVary);
	_state.spdHor=CSysRandom::RandVary(param->speedHor,param->speedHorVary);
	_state.rotMax=param->rotMax;
}

void CMagnetBall::Update(AnimTick t)
{
	AnimTick durStep=LEVEL_FRAME_TICK;

	while (t>=_state.t+durStep)
	{
		_state.t+=durStep;

		float dt=ANIMTICK_TO_SECOND(durStep);

		_state.rotMax-=_param->rotMaxDec*dt;
		if (_state.rotMax<0.0f)
			_state.rotMax=0.0f;

		if (_bTargetPos&&_state.bSupport)//有目标,并且落在地上了
		{
			LevelFace faceTarget=LevelFaceFromDir(_posTarget-_state.pos.getXZ());

			float rotlimit=_state.rotMax*dt*i_math::GRAD_PI2;
			i_math::rotate_limited(_state.face,faceTarget,rotlimit);
		}

		LevelPos3D posNext;
		posNext.setXZ(_state.pos.getXZ()+LevelFaceToDir(_state.face)*_state.spdHor*dt);

		BOOL bInNavmesh=FALSE;//在navmesh内部移动
		if (_level->GetUnitMgr()->IsWalkable(UnitFindPath_Walkable,_state.pos.getXZ()))
		{
			LevelPos posHit2D;
			if (!_level->GetUnitMgr()->StaticRayCast(UnitFindPath_Walkable,_state.pos.getXZ(),posNext.getXZ(),posHit2D))
				bInNavmesh=TRUE;
		}

		BOOL bHitStatic=FALSE;
		BOOL bSupport=FALSE;
		if (!bInNavmesh)
		{
			const float lift=0.4f;
			LevelPos3D posSrc=_state.pos;
			posSrc.y+=lift;
			posNext.y=posSrc.y;

			LevelPos3D posHit;
			if (_level->GetGtr()->RayCheck(posSrc,posNext,posHit))
			{
				posNext=posHit;
				bHitStatic=TRUE;
			}
		}
		if (!bHitStatic)
		{
			posNext.y=_state.pos.y;
			_state.spdVer-=_param->g*dt;
			posNext.y+=_state.spdVer*dt;
			float ht=_level->GetGtr()->GetHeight(posNext.x,posNext.z);
			if (posNext.y<=ht)
			{
				posNext.y=ht;
				bSupport=TRUE;
			}
			else
				bSupport=FALSE;

// 			if (bSupport)
// 				_state.spdVer=0.0f;
			if (bSupport&&(!_state.bSupport))
			{
				_state.nBumps++;
				if (_state.nBumps<=_param->nBumps)
				{
					_state.spdVer=fabsf(_state.spdVer)*_param->rateBump;
					_state.spdHor*=_param->dampHorBump;
				}
				else
				{
					_state.spdVer=0.0f;
				}
			}


			_state.pos=posNext;
			_state.bSupport=bSupport;
		}
		else
			_state.bHitStatic=TRUE;
	}
}

//////////////////////////////////////////////////////////////////////////
//EoMagnetBall
BIND_EOPARAM(EoMagnetBall,EoParamMagnetBall);

void EoMagnetBall::_OnPostCreate()
{
	EoParamMagnetBall *paramEo=GetParam<EoParamMagnetBall>();

	_tStart=_GetT();

	//寻找锁定目标
	_idTarget=LevelObjID_Invalid;
	if (paramEo->bTrace)
	{
		LevelUtilDetectParam param;
		param.loSrc=this;
		param.pos=_GetInitialPos();

		param.rangeMin=0.0f;
		param.rangeMax=100.0f;
		LevelDetectTargetFlag flag=(LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Player|LevelDetectTarget_Ground);
		param.flags=&flag;
		param.nFlags=1;
		param.weights.AddFlag(LevelDetectWeights_Dist);
		param.weights.wtDist=1.0f;
		param.weights.distRef=100.0f;

		CLevelObj *loTarget=LevelUtil_DetectBest(param,NULL);
		if (loTarget)
			_idTarget=loTarget->GetID();
	}

	_core.Init(_level,paramEo,_GetInitialPos(), LevelFaceFromDir(_GetInitialDir()),_level->GetT_());
}

void EoMagnetBall::_WriteState(CBitPacket *bp)
{
	CMagnetBall::State& state=_core.GetState();
	bp->Data_WriteSimple(state.t);
	bp->Data_WriteSimpleR(state.pos);
	bp->Data_WriteSimple(state.face);
}


void EoMagnetBall::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteState(bp);
	bContent=TRUE;
}

void EoMagnetBall::_OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteState(bp);
	bContent=TRUE;
}



void EoMagnetBall::_OnUpdate()
{
	EoParamMagnetBall *param=GetParam<EoParamMagnetBall>();

	if (_bDamage)
	{
		if (_level->GetT_()>_tExplode+param->delayExplodeDmg+4.0f)
			DeferDestroy();
		return;
	}

	if (_bExplode)
	{
		if (_level->GetT_()>=_tExplode+param->delayExplodeDmg)
		{
			_bDamage=TRUE;
			_MakeRangeDeal(param->radiusExplode);
		}
		return;
	}


	if (_idTarget!=LevelObjID_Invalid)
	{
		CLevelObj *loTarget=LevelUtil_GetAliveLo(_level,_idTarget);
		if (loTarget)
			_core.SetTargetPos(loTarget->GetFramePos());
	}

	CMagnetBall::State stateLast=_core.GetState();
	_core.Update(_level->GetT_());
	CMagnetBall::State &stateCur=_core.GetState();

	BOOL bExplode=FALSE;
	if (stateCur.bHitStatic)
		bExplode=TRUE;
	else
	{
		if (stateCur.t>stateLast.t)
		{
			//往前进了一步
			i_math::line3df line;
			line.start=stateLast.pos;
			line.end=stateCur.pos;

			line.start.y+=param->radius;
			line.end.y+=param->radius;

			LevelEoDetectHitArg argHit;
			argHit.radius=param->radius;

			CLevelObj *loHit=_DetectHit(line,argHit);
			if (loHit)
				bExplode=TRUE;
		}
	}

	if (!bExplode)
	{
		if (param->durAutoExplode>0)
		{
			if (_GetT()>=_tStart+param->durAutoExplode)
				bExplode=TRUE;
		}
	}

	if (bExplode)
	{
		LevelOpLink link;
		link.id=_level->GenOpLinkID();
		link.t=_level->GetT_();
		LevelOp_StartFire *op=NewOp<LevelOp_StartFire>(link);
		AddOp(op);

		_bExplode=TRUE;
		_tExplode=_level->GetT_();
	}


}

