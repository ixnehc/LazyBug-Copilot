
#include "stdh.h"

#include "Level.h"

#include "LoMagicCircuit.h"

#include "Random/Random.h"

#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "LevelUtil.h"
#include "LoUnit.h"
#include "LoBelly.h"

#include "LevelTroops.h"

#include "LevelOSB.h"

#include "Buff_Teleport.h"

//////////////////////////////////////////////////////////////////////////
//CMagicCircuitTail
void CMagicCircuitTail::Clear()
{
	_nodes.clear();
}

void CMagicCircuitTail::UpdateHead(i_math::vector3df &pos)
{
	const float step=0.1f;
	const float distMax=10.0f;

	if (_nodes.size()<=0)
	{
		Node node;
		node.pos=pos;
		node.length=0.0f;
		_nodes.push_back(node);
		return;
	}

	float dist=_nodes[0].pos.getDistanceFrom(pos);
	if (dist<step)
		return;

	if (TRUE)
	{
		Node node;
		node.pos=pos;
		node.length=0.0f;
		node.dir.set(0.0f,0.0f,0.0f);

		_nodes[0].length=_nodes[0].pos.getDistanceFrom(pos);
		_nodes[0].dir=(_nodes[0].pos-pos);
		_nodes[0].dir.normalize();
		_nodes.push_front(node);

	}

	float distTotal=0.0f;
	for (int i=1;i<_nodes.size();i++)
	{
		distTotal+=_nodes[i].length;
		if (distTotal>distMax)
		{
			_nodes.resize(i+1);
			break;
		}
	}
}

BOOL CMagicCircuitTail::Sample(float dist, i_math::vector3df &pos,i_math::vector3df *normal)
{
	if (_nodes.size()<=1)
		return FALSE;
	float distTotal=0.0f;
	for (int i=1;i<_nodes.size();i++)
	{
		distTotal+=_nodes[i].length;
		if (distTotal>=dist)
		{
			float distLocal=dist-(distTotal-_nodes[i].length);
			float ratio=_nodes[i].length>0.0001f?distLocal/_nodes[i].length:0.0f;
			pos=_nodes[i].pos.getInterpolated(_nodes[i-1].pos,ratio);
			if (normal)
				*normal=_nodes[i].dir;
			return TRUE;
		}
	}

	pos=_nodes[_nodes.size()-1].pos;
	if (normal)
	{
		if (_nodes.size()>=2)
			*normal=_nodes[_nodes.size()-2].dir;
		else
			normal->set(1.0f,0.0f,0.0f);
	}

	return TRUE;
}

BOOL CMagicCircuitTail::Sample(int idx, i_math::vector3df &pos,i_math::vector3df *normal)
{
	return Sample(idx*0.6f+1.0f,pos,normal);
}




//////////////////////////////////////////////////////////////////////////
//CLoMagicCircuit



MagicCircuitSetting &CLoMagicCircuit::_GetSetting()
{
	LevelRecordGlobal *recGlobal=_level->GetRecords()->GetGlobal();
	if (recGlobal)
		return recGlobal->magiccircuitsetting;

	static MagicCircuitSetting setting;
	return setting;
}


void CLoMagicCircuit::Clear()
{
	if (_bhv)
	{
		_bhv->Clear();
		Safe_Class_Delete(_bhv);
	}

	_infosRelay.clear();
	_infosBollard.clear();

	_networkEel.Reset();

	_tail.Clear();
	_tailorbs.clear();

	for (int i = 0;i < _idsRailGuards.size();i++)
		LevelUtil_DestroyLo(_level, _idsRailGuards[i]);
	_idsRailGuards.clear();

	Zero();
}


void CLoMagicCircuit::OnDestroy()
{
	Clear();

	_level->UnregisterUniqueObj(LevelUniqueObj_MagicCircuit,this);
}

void CLoMagicCircuit::Zero()
{
	_tStart=ANIMTICK_INFINITE;

	_bhv=NULL;

	_bRelayActiveSyncDirty=FALSE;
	_activesRelay=0;

	_bRelayHitSyncDirty = FALSE;
	_hitsRelay = 0;

	_bSpotsSyncDirty=FALSE;
	_statesSpot.Zero();

	_bFocusSyncDirty=FALSE;
	_stateFocus=Focus_None;
	_tFocusStateStart=0;
	_idxFocus=-1;
	_idTailOrbOfFocus=MagicCuicuitTailOrbID_Invalid;

	_bTailOrbsSyncDirty=FALSE;
	_tTailOrbsReachingStart=ANIMTICK_INFINITE;

}


BOOL CLoMagicCircuit::OnActivate()
{
	MagicCircuitSetting &setting=_GetSetting();
	LopMagicCircuit* lop = (LopMagicCircuit*)_param;

	_level->RegisterUniqueObj(LevelUniqueObj_MagicCircuit,this);

	_tStart=GetT();

	if(setting.idBG!=StringID_Invalid)
	{
		LevelBehaviorContext ctx;
		ctx.lo=this;
		_bhv=_level->CreateBehavior(setting.idBG,ctx);
	}

	return TRUE;
}

void CLoMagicCircuit::_BuildRelayInfo()
{
	if (_infosRelay.size()>0)
		return;

	LosMagicCircuit *los=(LosMagicCircuit *)_src;
	MagicCircuitSetting &setting=_GetSetting();
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	_infosRelay.resize(lop->relays.size());
	for(int i=0;i<lop->relays.size();i++)
	{
		LevelPos pos=lop->relays[i].getXZ();

		if (TRUE)
		{
			CLevelObj *lo=LevelUtil_DetectClosestAgent(_level,pos,2.0f,NULL,setting.idAgentRelay);
			if (lo)
				_infosRelay[i].idRelay=lo->GetID();
		}

	}

}

void CLoMagicCircuit::_BuildBollardInfo()
{
	if (_infosBollard.size()>0)
		return;

	LosMagicCircuit *los=(LosMagicCircuit *)_src;
	MagicCircuitSetting &setting=_GetSetting();
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	_infosBollard.resize(lop->bollards.size());
	for(int i=0;i<lop->bollards.size();i++)
	{
		LevelPos pos=lop->bollards[i].getXZ();

		if (TRUE)
		{
			CLevelObj *lo=LevelUtil_DetectClosestAgent(_level,pos,2.0f,NULL,setting.idAgentBollard);
			if (lo)
				_infosBollard[i]=lo->GetID();
		}
	}
}

LevelObjID CLoMagicCircuit::FindEelNetworkNodeID(i_math::vector2df &pos)
{
	const float range=1.0f;
	for (int i=0;i<_infosRelay.size();i++)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,_infosRelay[i].idRelay);
		if (lo->GetFramePos().getDistanceSQFrom(pos)<range*range)
			return _infosRelay[i].idRelay;
	}
	for (int i=0;i<_infosBollard.size();i++)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,_infosBollard[i]);
		if (lo->GetFramePos().getDistanceSQFrom(pos)<range*range)
			return _infosBollard[i];
	}
	return LevelObjID_Invalid;
}

CEelRoadNetwork& CLoMagicCircuit::GetEelRoadNetwork()
{
	_BuildEelNetwork();
	return _networkEel;
}


void CLoMagicCircuit::_BuildEelNetwork()
{
	if (!_networkEel.IsEmpty())
		return;

	_BuildRelayInfo();
	_BuildBollardInfo();


	for (int i=0;i<_infosRelay.size();i++)
		_networkEel.AddNode(_infosRelay[i].idRelay);
	for (int i=0;i<_infosBollard.size();i++)
		_networkEel.AddNode(_infosBollard[i]);

	LopMagicCircuit *lop=(LopMagicCircuit *)_param;
	for(int i=0;i<lop->networksEel.size()-1;i++)
	{
		LevelObjID idA=FindEelNetworkNodeID(lop->networksEel[i].getXZ());
		LevelObjID idB=FindEelNetworkNodeID(lop->networksEel[i+1].getXZ());

		_networkEel.AddRoad(idA,idB);
	}
}


void CLoMagicCircuit::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (!_bRelayActiveSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();

		bp->Data_WriteSimple(_activesRelay);
	}

	if (!_bRelayHitSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent = TRUE;
		bp->Bit_Write_1();

		bp->Data_WriteSimple(_hitsRelay);
	}

	if (!_bSpotsSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();

		bp->Data_WriteSimpleR(_statesSpot);
	}


	if (!_bFocusSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();

		bp->Bits_Write(_stateFocus,3);
		if (_stateFocus==Focus_Activated)
			bp->Data_WriteSimple(_idxFocus);
	}

	if (!_bTailOrbsSyncDirty)
		bp->Bit_Write_0();
	else
	{
		bContent=TRUE;
		bp->Bit_Write_1();

		bp->Data_EncodeDword(_tailorbs.size());
		for (int i=0;i<_tailorbs.size();i++)
			bp->Data_WriteSimple(_tailorbs[i].id);

		if (_tTailOrbsReachingStart!=ANIMTICK_INFINITE)
			bp->Bit_Write_1();
		else
			bp->Bit_Write_0();
	}

}

void CLoMagicCircuit::_OnPostWriteSync()
{
	_bRelayActiveSyncDirty=FALSE;
	_bRelayHitSyncDirty = FALSE;
	_bSpotsSyncDirty=FALSE;
	_bFocusSyncDirty=FALSE;
	_bTailOrbsSyncDirty=FALSE;
}


void CLoMagicCircuit::Update()
{
	LosMagicCircuit *los=(LosMagicCircuit *)_src;
	MagicCircuitSetting &setting=_GetSetting();
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	if (_bhv)
		_bhv->Update();

	_UpdateRelayHits();
	_UpdateTeleport();

	_UpdateSpotsState();
	_UpdateTail();


}

void CLoMagicCircuit::ActivateRelay(LevelObjID idRelay,BOOL bActivate)
{
	_BuildRelayInfo();

	int iRelay;
	VEC_FIND_BY_ELEMENT(_infosRelay,idRelay,idRelay,iRelay);
	if (iRelay>=0)
	{
		WORD activesRelayOld=_activesRelay;
		if (bActivate)
			_activesRelay|=(1<<iRelay);
		else
			_activesRelay&=~(1<<iRelay);
		if (activesRelayOld!=_activesRelay)
			_bRelayActiveSyncDirty=1;
	}
}

BOOL CLoMagicCircuit::_CheckRelayConnected(int idxRelay)
{
	if (idxRelay>=0)
	{
		for (int i=0;i<idxRelay;i++)
		{
			if (!(_activesRelay&(1<<i)))
				return FALSE;
		}
		return TRUE;
	}

	return FALSE;
}

void CLoMagicCircuit::_UpdateRelayHits()
{
	_BuildRelayInfo();

	MagicCircuitSetting& setting = _GetSetting();
	if (setting.idBuffRelayHit != RecordID_Invalid)
	{
		WORD hitsRelay = _hitsRelay;
		_hitsRelay = 0;
		for (int i = 0;i < _infosRelay.size();i++)
		{
			if (LevelUtil_FindBuffByRecordID(_level, _infosRelay[i].idRelay, setting.idBuffRelayHit))
				_hitsRelay|= (1 << i);
		}
		if (hitsRelay != _hitsRelay)
			_bRelayHitSyncDirty = TRUE;
	}

}


LevelObjID CLoMagicCircuit::SpawnCrystal()
{
	LosMagicCircuit *los=(LosMagicCircuit *)_src;
	MagicCircuitSetting &setting=_GetSetting();
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	int idx=CSysRandom::RandRangeInt<int>(0,lop->crystals.size());
	i_math::vector3df pos=lop->crystals[idx];

	DealArg arg;
	DealResult result;
	MakeDeals(setting.dealsSummonCrystal,LevelOSB(this),pos,arg,&result);

	return result.idSummoned;
}

LevelObjID CLoMagicCircuit::GetCrystalTarget()
{
	_BuildRelayInfo();

	return _infosRelay[IDX_CRYSTALTARGET].idRelay;
}

BOOL CLoMagicCircuit::CanSpawnCrystal()
{
	return _CheckRelayConnected(IDX_CRYSTALTARGET);
}


LevelObjID CLoMagicCircuit::SpawnRelayBird()
{
	LosMagicCircuit *los=(LosMagicCircuit *)_src;
	MagicCircuitSetting &setting=_GetSetting();
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	i_math::vector3df pos=lop->perches[0];

	DealArg arg;
	DealResult result;
	MakeDeals(setting.dealsSummonRelayBird,LevelOSB(this),pos,arg,&result);

	return result.idSummoned;
}

BOOL CLoMagicCircuit::GetRelayBirdTargetPos(LevelPos& posRelayBird,LevelPos &posTarget)
{
	LosMagicCircuit *los=(LosMagicCircuit *)_src;
	MagicCircuitSetting &setting=_GetSetting();
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	int idxCur=-1;
	for (int i=0;i<lop->perches.size();i++)
	{
		LevelPos pos=lop->perches[i].getXZ();
		if (pos.getDistanceFrom(posRelayBird)<2.0f)
		{
			idxCur=i;
			break;
		}
	}

	int idxTarget=-1;
	if(idxCur!=IDX_RELAYBIRDHOME)
	{
		if (CSysRandom::Roll(0.8f))
			idxTarget=IDX_RELAYBIRDHOME;
	}
	if (idxTarget<0)
	{
		for (int i=0;i<10;i++)
		{
			int idx=CSysRandom::RandRangeInt<int>(0,lop->perches.size());
			if (idx!=idxCur)
			{
				idxTarget=idx;
				break;
			}
		}
	}

	if ((idxTarget<0)&&(lop->perches.size()>1))
		idxTarget=(idxCur+1)%lop->perches.size();

	if (idxTarget>=0)
	{
		posTarget=lop->perches[idxTarget].getXZ();
		return TRUE;
	}
	return FALSE;
}

BOOL CLoMagicCircuit::CheckRelayBirdAtHome(LevelPos& posRelayBird)
{
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	const int idxRelayBirdHome=IDX_RELAYBIRDHOME;
	if (lop->perches.size()<=idxRelayBirdHome)
		return FALSE;

	if (lop->perches[idxRelayBirdHome].getXZ().getDistanceFrom(posRelayBird)<1.0f)
		return TRUE;
	return FALSE;
}

BOOL CLoMagicCircuit::CheckRelayBirdAtRest(LevelPos& posRelayBird)
{
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	for (int i=0;i<lop->perches.size();i++)
	{
		if (lop->perches[i].getXZ().getDistanceFrom(posRelayBird)<0.5f)
			return TRUE;
	}
	return FALSE;
}


BOOL CLoMagicCircuit::CanActivateFocus()
{
	return _CheckRelayConnected(IDX_TAILORBSHOME);
}


void CLoMagicCircuit::ActivateFocus()
{
	LopMagicCircuit *lop=(LopMagicCircuit *)_param;

	if (lop->focuses.size()>0)
	{
		MagicCuicuitTailOrbID idTailOrb=_GenTailOrbID();
		if (idTailOrb!=MagicCuicuitTailOrbID_Invalid)
		{
			_idxFocus=CSysRandom::RandRangeInt<int>(0,lop->focuses.size());
			_tFocusStateStart=_level->GetT_();
			_stateFocus=Focus_Activated;
			_idTailOrbOfFocus=idTailOrb;
			_bFocusSyncDirty=TRUE;
		}
	}
}

void CLoMagicCircuit::CommitFocus()
{
	if (_idxFocus>=0)
	{
		LopMagicCircuit *lop=(LopMagicCircuit *)_param;
		i_math::vector3df posFocus=lop->focuses[_idxFocus];

		const float range=2.f;
		CLoUnit *loPlayer=LevelUtil_GetFirstPlayerLoUnit(_level);
		if (loPlayer->GetFramePos().getDistanceFrom(posFocus.getXZ())<range)
			_stateFocus=Focus_Hit;
		else
			_stateFocus=Focus_None;

		_tFocusStateStart=_level->GetT_();
		_idxFocus=-1;
		_bFocusSyncDirty=TRUE;

		if (_stateFocus==Focus_Hit)
			_SpawnTailOrb(_idTailOrbOfFocus);

		_idTailOrbOfFocus=MagicCuicuitTailOrbID_Invalid;
	}
}

void CLoMagicCircuit::_SpawnTailOrb(MagicCuicuitTailOrbID id)
{
	TailOrb orb;
	orb.id=id;
	orb.tBirth=_level->GetT_();

	_tailorbs.push_back(orb);

	_bTailOrbsSyncDirty=TRUE;
}

int CLoMagicCircuit::_GetMaxTailOrbsCount()
{
	return MAGICCIRCUIT_MAX_TAILORB_COUNT;
}


MagicCuicuitTailOrbID CLoMagicCircuit::_GenTailOrbID()
{
	if (_tTailOrbsReachingStart!=ANIMTICK_INFINITE)
		return MagicCuicuitTailOrbID_Invalid;

	const int countTailOrbMax=_GetMaxTailOrbsCount();

	BOOL flag[32];//big enough
	memset(flag,0,sizeof(flag));

	for (int i=0;i<_tailorbs.size();i++)
	{
		if (_tailorbs[i].id!=MagicCuicuitTailOrbID_Invalid)
			flag[_tailorbs[i].id]=TRUE;
	}
	if (_idTailOrbOfFocus!=MagicCuicuitTailOrbID_Invalid)
		flag[_idTailOrbOfFocus]=TRUE;

	for (int i=0;i<countTailOrbMax;i++)
	{
		if (!flag[i])
			return (MagicCuicuitTailOrbID)i;
	}
	return MagicCuicuitTailOrbID_Invalid;
}

LevelObjID CLoMagicCircuit::GetTailOrbsHome()
{
	_BuildRelayInfo();
	return _infosRelay[IDX_TAILORBSHOME].idRelay;
}


BOOL CLoMagicCircuit::CheckTailOrbsReached()
{
	LevelTick t=_level->GetT_();

	if (_tTailOrbsReachingStart!=ANIMTICK_INFINITE)
	{
		if (t>_tTailOrbsReachingStart+MAGICCIRCUIT_TAILORBS_REACHING_DURATION)
			return TRUE;
	}
	return FALSE;
}

BOOL CLoMagicCircuit::CanTailOrbsReach()
{
	if(_tTailOrbsReachingStart!=ANIMTICK_INFINITE)
		return FALSE;

	if (_GetMaxTailOrbsCount()<=0)
		return FALSE;

	if (_tailorbs.size()<_GetMaxTailOrbsCount())
		return FALSE;

	for (int i=0;i<_tailorbs.size();i++)
	{
		if (_tailorbs[i].tBirth+MAGICCIRCUIT_TAILORB_MAX_FALL_DURATION+ANIMTICK_FROM_SECOND(2.0f)>_level->GetT_())
			return FALSE;
	}

	LevelPos posHome;
	if(!LevelUtil_GetFramePos(_level,GetTailOrbsHome(),posHome))
		return FALSE;

	const int nCheckCount=4;
	const float radius=2.0f;

	for (int i=0;i<nCheckCount;i++)
	{
		i_math::vector3df pos;
		if (!_tail.Sample(i,pos,NULL))
			return FALSE;

		if (pos.getXZ().getDistanceFrom(posHome)>radius)
			return FALSE;
	}
	return TRUE;
}

void CLoMagicCircuit::StartTailOrbsReach()
{
	if (!CanTailOrbsReach())
		return;

	_tTailOrbsReachingStart=_level->GetT_();
	_bTailOrbsSyncDirty=1;
}

void CLoMagicCircuit::_UpdateTail()
{
	CLoUnit *loPlayer=LevelUtil_GetFirstPlayerLoUnit(_level);
	if (loPlayer)
		_tail.UpdateHead(loPlayer->GetFramePos3D());

}

void CLoMagicCircuit::_UpdateSpotsState()
{
	MagicCircuitSetting &setting=_GetSetting();

	SpotsState stateOld;
	stateOld=_statesSpot;

	if (_statesSpot.IsAllOff())
	{
		if (_activesRelay&(1<<(IDX_SPOTHOME-1)))
		{
			//Activate spots
//			_statesSpot.states[CSysRandom::RandRangeInt(0,4)]=2;
			for (int i=0;i<ARRAY_SIZE(_statesSpot.states);i++)
			{
				if(_statesSpot.states[i]==0)
					_statesSpot.states[i]=1;
			}

			//为所有Flash的spot对应的Relay添加FuseReady buff
			for (int i=0;i<ARRAY_SIZE(_statesSpot.states);i++)
			{
				if(_statesSpot.states[i]==1)
				{
					int idxRelay=i;
					if (idxRelay>=IDX_SPOTHOME)
						idxRelay++;

					LevelObjID idRelay=_infosRelay[idxRelay].idRelay;

					CLevelObj *lo=LevelUtil_GetAliveLo(_level,idRelay);

					if (lo)
					{
						if (i<setting.idsFuseReady.size())
							_level->GetDecider()->MakeBuff(LevelOSB(lo),lo,setting.idsFuseReady[i],ANIMTICK_INFINITE,NULL,LevelOpLink());
					}
				}
			}
		}
	}
	else
	{
		if (!(_activesRelay&(1<<(IDX_SPOTHOME-1))))
		{
			//Deactivate spots
			_statesSpot.Zero();

			//清除所有relay上的FuseReady和FuseBurning buff
			for (int i=0;i<ARRAY_SIZE(_statesSpot.states);i++)
			{
				int idxRelay=i;
				if (idxRelay>=IDX_SPOTHOME)
					idxRelay++;

				CLevelObj *lo=LevelUtil_GetAliveLo(_level,_infosRelay[idxRelay].idRelay);
				if (lo)
				{
					if (i<setting.idsFuseReady.size())
						LevelUtil_RemoveBuffByRecordID(lo,setting.idsFuseReady[i]);
					LevelUtil_RemoveBuffByRecordID(lo,setting.idFuseBurn);
				}
			}
		}
	}

	//检查每一个Spot对应的Relay,如果上面既没有FuseReady,也没有FuseBurn,说明Fuse已经燃尽,可以把Spot置成On了
	if (!_statesSpot.IsAllOff())
	{
		for (int i=0;i<ARRAY_SIZE(_statesSpot.states);i++)
		{
			if(_statesSpot.states[i]==1)
			{
				int idxRelay=i;
				if (idxRelay>=IDX_SPOTHOME)
					idxRelay++;

				LevelObjID idRelay=_infosRelay[idxRelay].idRelay;
				CLevelObj *lo=LevelUtil_GetAliveLo(_level,idRelay);
				BOOL bFuseBurn=LevelUtil_FindBuffByRecordID(lo,setting.idFuseBurn)?TRUE:FALSE;
				BOOL bFuseReady=FALSE;
				if (i<setting.idsFuseReady.size())
					bFuseReady=LevelUtil_FindBuffByRecordID(lo,setting.idsFuseReady[i])?TRUE:FALSE;
				if ((!bFuseBurn)&&(!bFuseReady))
					_statesSpot.states[i]=2;//设为On
			}
		}
	}

	if (_statesSpot.IsAllOn()&&(!stateOld.IsAllOn()))
	{
		ActivateRelay(_infosRelay[IDX_SPOTHOME].idRelay,TRUE);
	}
	else
	{
		if (stateOld.IsAllOn()&&(!_statesSpot.IsAllOn()))
			ActivateRelay(_infosRelay[IDX_SPOTHOME].idRelay,FALSE);
	}

	if (!_statesSpot.Equals(stateOld))
		_bSpotsSyncDirty=TRUE;

}

LevelObjID CLoMagicCircuit::FindTargetRelayForEel(LevelPos posEel)
{
	_BuildRelayInfo();

	CLoBelly *loBelly=(CLoBelly *)_level->GetUniqueObj(LevelUniqueObj_Belly);
	if (!loBelly)
		return LevelObjID_Invalid;

	float distMin=4.0f;
	float distMax=30.0f;
	int offset=CSysRandom::RandRangeInt<int>(0,_infosRelay.size());
	LevelObjID idClosest=LevelObjID_Invalid;
	float distClosest=100000.0f;
	for (int i=0;i<_infosRelay.size();i++)
	{
		CLevelObj *loRelay=LevelUtil_GetAliveLo(_level,_infosRelay[i+offset].idRelay);
		if (!loRelay)
			continue;

		float dist=loRelay->GetFramePos().getDistanceFrom(posEel);
		if (dist<distMin)
			continue;

		LevelPos posRelay=loRelay->GetFramePos();
		if (!loBelly->ValidateEelTargetPos(posEel,posRelay))
			continue;

		if (dist<distMax)
			return loRelay->GetID();

		if (dist<distClosest)
		{
			distClosest=dist;
			idClosest=loRelay->GetID();
		}
	}
	return idClosest;
}

void CLoMagicCircuit::_UpdateTeleport()
{
	LopMagicCircuit* lop = (LopMagicCircuit*)_param;
	MagicCircuitSetting& setting = _GetSetting();

	if (setting.idBuffTeleport == RecordID_Invalid)
		return;

	CLoUnit* loPlayer = LevelUtil_GetFirstPlayerLoUnit(_level);
	if (!loPlayer)
		return;

	if (lop->startsTeleport.size() < _infosRelay.size())
		return;
	if (lop->endsTeleport.size() < _infosRelay.size())
		return;

	LevelPos posPlayer = loPlayer->GetFramePos();

	for (int i = 0;i < _infosRelay.size();i++)
	{
		BOOL bTeleportActive = _hitsRelay & (1 << i);
		if (bTeleportActive)
		{
			if (lop->startsTeleport[i].isPointIn(posPlayer))
			{
				if (!LevelUtil_FindBuff(loPlayer, Class_Ptr2(Buff_Teleport)))
				{
					BuffArg_Teleport argTeleport;
					argTeleport.pos = lop->endsTeleport[i].getXZ();
					argTeleport.face = LevelUtil_GenRandomFace();
					_level->GetDecider()->MakeBuff(LevelOSB(this), loPlayer, setting.idBuffTeleport, 0, &argTeleport, LevelOpLink());
					break;
				}
			}
		}
	}
}

void CLoMagicCircuit::SpawnRailGuards()
{
	LopMagicCircuit* lop = (LopMagicCircuit*)_param;
	MagicCircuitSetting& setting = _GetSetting();

	if (lop->railguardsSpawn.size() <= 0)
		return;

	CRandomPositionsGenerator<64> generator;
	generator.Init(lop->railguardsSpawn, 2.0f);

	CRandomPositionsGenerator<64>::PositionContainer positions;
	CRandomPositionsGenerator<64>::PositionContainer empty;

	const int nGuards = 1;

	CLoUnit* loPlayer = LevelUtil_GetFirstPlayerLoUnit(_level);

	generator.Gen(nGuards, empty, positions,
		[loPlayer](const i_math::vector2df& position) 
		{
			const float minDist = 3.0f;
			if (loPlayer->GetFramePos().getDistanceFrom(position) > minDist)
				return true;
			return false;
		}
	);

	_idsRailGuards.reserve(positions.size());
	for (int i = 0;i < positions.size();i++)
	{
		DealArg arg;
		DealResult result;
		LevelPos3D pos = LevelUtil_GetWalkableGroundHeight(_level, positions[i].x, positions[i].y, TRUE);
		MakeDeals(setting.dealsSummonRailGuard, LevelOSB(this), pos, arg, &result);
		if (result.idSummoned != LevelObjID_Invalid)
			_idsRailGuards.push_back(result.idSummoned);
	}

}

void CLoMagicCircuit::DespawnRailGuards()
{

}

LevelPos3D CLoMagicCircuit::FindClosestRailPoint(i_math::vector3df& pos)
{
	LopMagicCircuit* lop = (LopMagicCircuit*)_param;

	float distMin = 10000000.0f;
	LevelPos3D posClosest;
	for (int i = 0;i < lop->rail.size();i++)
	{
		float dist = lop->rail[i].getDistanceXZFrom(pos);
		if (dist < distMin)
		{
			distMin = dist;
			posClosest = lop->rail[i];
		}
	}
	return posClosest;
}
