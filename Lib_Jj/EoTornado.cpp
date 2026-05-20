
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoTornado.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "Random/Random.h"


//////////////////////////////////////////////////////////////////////////
//EoTornado
BIND_EOPARAM(EoTornado,EoParamTornado);

void EoTornado::_OnPostCreate()
{
	EoParamTornado *param=GetParam<EoParamTornado>();

	_state.t=_level->GetT_();
	_state.pos=_GetInitialPos();
	_state.face=LevelFaceFromDir(_GetInitialDir());
	_state.deflectMax=CSysRandom::RandRange(param->deflectMin,param->deflectMax);
	_state.deflectMax*=i_math::GRAD_PI2;
	if (CSysRandom::Roll(0.5f))
		_state.signDeflect=1.0f;
	else
		_state.signDeflect=-1.0f;
	_state.speed=CSysRandom::RandRange(param->speedMin,param->speedMax);

}

void EoTornado::_WriteState(CBitPacket *bp)
{
	bp->Data_WriteSimple(_state.t);
	bp->Data_WriteSimpleR(_state.pos);
	bp->Data_WriteSimple(_state.face);
}


void EoTornado::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteState(bp);
	bContent=TRUE;
}

void EoTornado::_OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteState(bp);
	bContent=TRUE;
}



void EoTornado::_OnUpdate()
{
	EoParamTornado *param=GetParam<EoParamTornado>();

	AnimTick durStep=LEVEL_FRAME_TICK;

	while (_level->GetT_()>=_state.t+durStep)
	{
		_state.t+=durStep;

		float dt=ANIMTICK_TO_SECOND(durStep);

		_state.deflect+=param->accDeflect*i_math::GRAD_PI2*dt;
		if (_state.deflect>_state.deflectMax)
			_state.deflect=_state.deflectMax;

		_state.face+=_state.deflect*dt*_state.signDeflect;

		extern BOOL LevelUtil_DetectPos(CLevelObj *lo,float &euler,float &dist,float fov,int nSideSteps,LevelPos &pos);

		float eulerX=LevelFaceToEuler(_state.face);
		float dist=_state.speed*dt*1.2f;
		LevelPos posNext;
		LevelUtil_DetectPos(this,eulerX,dist,360.0f*i_math::GRAD_PI2,8,posNext);
		_state.face=LevelFaceFromEuler(eulerX);
		_state.pos=posNext;
	}

	if (_GetT()<_tCreate+param->dur)
	{
		int nToDamage=_GetAge()/param->cycleDmg;

		while(nToDamage>_nDamaged)
		{
			_MakeRangeDeal(LevelOSB(this),param->radiusDmg);
			_nDamaged++;
		}
	}

	if (_GetT()>_tCreate+param->dur+ANIMTICK_FROM_SECOND(2.0f))//确保效果全部淡出后再删除
	{
		DeferDestroy();
	}

}

