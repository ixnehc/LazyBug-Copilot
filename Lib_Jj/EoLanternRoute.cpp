
#include "stdh.h"

#include "Level.h"

#include "LevelResources.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoLanternRoute.h"

#include "LevelRecords.h"


#include "Random/Random.h"
#include "timer/timer.h"



//////////////////////////////////////////////////////////////////////////
//EoLanternRoute
BIND_EOPARAM(EoLanternRoute,EoParamLanternRoute);


void EoLanternRoute::_OnPostCreate()
{
	EoParamLanternRoute *param=GetParam<EoParamLanternRoute>();

	_SetStage(State::Birth);

}

void EoLanternRoute::OnDestroy()
{
}

void EoLanternRoute::SetPath(RecordID idPathRes)
{
	_idPathRes=idPathRes;
	_distPath=0.0f;
	LevelPath *path=_level->GetResources()->FindPath(idPathRes);
	if (path)
		_distPath=path->length;
}


void EoLanternRoute::_SetStage(State::Stage stage)
{
	if (stage==_state.stage)
		return;

	_state.stage=stage;
	_state.tStageStart=_GetT();

	_bSyncDirty=TRUE;
}


void EoLanternRoute::_WriteState(CBitPacket *bp)
{
	bp->Bits_Write(_state.stage,3);
}

void EoLanternRoute::Stop()
{
	if ((_state.stage==State::Opening)||(_state.stage==State::Birth))
		_SetStage(State::Aborting);
	else
	{
		if (_state.stage==State::Opened)
			_SetStage(State::Closing);
	}
}


void EoLanternRoute::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(_idPathRes);
	bp->Data_WriteSimple(_distPath);
	bp->Data_WriteSimple(_idPortal);

	_WriteState(bp);
	bContent=TRUE;
}

void EoLanternRoute::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Bit_Write(_bSyncDirty);
	if (_bSyncDirty)
	{
		_WriteState(bp);
		bContent=TRUE;
	}
}

void EoLanternRoute::_OnPostWriteSync()
{
	_bSyncDirty=FALSE;
}

void EoLanternRoute::_OnUpdate()
{
	EoParamLanternRoute *param=GetParam<EoParamLanternRoute>();

	if (_state.stage==State::Closed)
	{
		if (_GetT()>=_state.tStageStart+ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
		return;
	}

	if (_state.stage==State::Closing)
	{
		float fDur=_distPath/param->spdClose;
		if (_GetT()>=_state.tStageStart+ANIMTICK_FROM_SECOND(fDur))
		{
			if (param->nmCloseSignal!=StringID_Invalid)
				_level->GetEventMap()->AddSignal(param->nmCloseSignal,_idPortal,GetID());

			_SetStage(State::Closed);
		}
		return;
	}

	if (_state.stage==State::Aborting)
	{
		if (_GetT()>=_state.tStageStart+ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
		return;
	}

	if (_state.stage==State::Opening)
	{
		float fDur=_distPath/param->spdOpen;
		if (_GetT()>=_state.tStageStart+ANIMTICK_FROM_SECOND(fDur))
		{
			if (param->nmOpenSignal!=StringID_Invalid)
				_level->GetEventMap()->AddSignal(param->nmOpenSignal,_idPortal,GetID());
			_SetStage(State::Opened);
		}
		return;
	}

	if (_state.stage==State::Birth)
	{
		if (_GetT()>=_state.tStageStart+param->durBirth)
			_SetStage(State::Opening);
		return;
	}

}

