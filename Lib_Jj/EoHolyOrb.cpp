
#include "stdh.h"

#include "Level.h"

#include "LevelResources.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoHolyOrb.h"

#include "LevelRecords.h"


#include "Random/Random.h"
#include "timer/timer.h"



//////////////////////////////////////////////////////////////////////////
//EoHolyOrb
BIND_EOPARAM(EoHolyOrb,EoParamHolyOrb);


void EoHolyOrb::_OnPostCreate()
{
	EoParamHolyOrb *param=GetParam<EoParamHolyOrb>();


}

void EoHolyOrb::OnDestroy()
{
}

void EoHolyOrb::SetPath(RecordID idPathRes)
{
	_idPathRes=idPathRes;
	_durPath=ANIMTICK_FROM_SECOND(2.0f);
	LevelPath *path=_level->GetResources()->FindPath(idPathRes);
	if (path)
	{
		_durPath=path->dur;
	}
}



void EoHolyOrb::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(_idPathRes);
	bContent=TRUE;
}

void EoHolyOrb::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Bit_Write(_bSyncDirty);
	if (_bSyncDirty)
	{
		bContent=TRUE;
	}
}

void EoHolyOrb::_OnPostWriteSync()
{
	_bSyncDirty=FALSE;
}

void EoHolyOrb::_OnUpdate()
{
	EoParamHolyOrb *param=GetParam<EoParamHolyOrb>();

	if (!_bReached)
	{
		if (_GetT()>=GetCreateTime()+_durPath)
		{
			LevelPath *path=_level->GetResources()->FindPath(_idPathRes);
			if (path)
			{
				Key_2f k;
				if (path->ksPos.CalcKey(_GetT()-GetCreateTime(),&k))
				{
					if (param->nmSignal1!=StringID_Invalid)
						_level->GetEventMap()->AddSignal(param->nmSignal1,k.v,1.0f,GetID());
					if (param->nmSignal2!=StringID_Invalid)
					_level->GetEventMap()->AddSignal(param->nmSignal2,k.v,10.0f,GetID());
				}
			}

			_bReached=TRUE;
		}
	}

	if (_bReached)
	{
		if (_GetT()>=GetCreateTime()+_durPath+ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
	}
	return;


}

