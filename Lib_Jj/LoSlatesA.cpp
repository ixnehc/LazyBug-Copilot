
#include "stdh.h"

#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelHooks.h"

#include "LevelUtil.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"

#include "LoSlate.h"
#include "LoSlatesA.h" 

#include "LevelOSB.h"

#include "LevelRecordSlateType.h"
#include "LevelRecordGlobal.h"

#include "Random/Random.h"
#include "commondefines/general_stl.h"


//////////////////////////////////////////////////////////////////////////
//CLevelSlateA

void CLevelSlateA::Init(LevelSlateInfo *info)
{
	_info=info;

	_status.tp=info->tpDef;

	_pos=info->src->GetPos();
}

BOOL CLevelSlateA::CheckIn(LevelPos &pos)
{
	const float radiusDetect=0.5f;

	if (pos.getDistanceSQFrom(_pos)<radiusDetect*radiusDetect)
		return TRUE;
	return FALSE;
}

//XXXXX:与CGameSlate::CheckCoverLocked()要保持一致
BOOL CLevelSlateA::CheckCoverLocked()
{
	return _statusCover.CheckLocked();
}



//////////////////////////////////////////////////////////////////////////
//CLoSlatesA

void CLoSlatesA::PostCreate()
{
	CLoAgent::PostCreate();

	_RegisterLevelHook(LH_PlayerLeaveLevel,40);

	if (_level)
	{
		CLevelData *data=_level->GetData();
		if (data)
		{
			if (data->GetBasis())
				_basis=&data->GetBasis()->GetSlatesBasis();
		}
	}

	if (_basis)
	{
		LevelGUID uid=_src->GetGUID();
		LevelSlatesInfo *info=_basis->FindSlatesInfo(uid);
		if (info)
		{
			_bufSlate.resize(info->countSlates);

			for (int i=0;i<info->countSlates;i++)
			{
				LevelSlateInfo *infoSlate=_basis->GetSlateInfo(info->iStartSlate+i);
				assert(infoSlate);
				_bufSlate[i].Init(infoSlate);
			}
		}

		_info=info;

		//设置迷宫
		LopSlates *lop=(LopSlates *)_param;
		if (lop)
		{
			if (lop->bhvSetup!=StringID_Invalid)
			{
				LevelBehaviorContext ctx;
				ctx.lo=this;
				CLevelBehavior *bhvAI=_level->CreateBehavior(lop->bhvSetup,ctx);
				bhvAI->Start();
				bhvAI->Clear();
				Safe_Class_Delete(bhvAI);
			}
		}

		//校验一下每个Slate的EdgeLocks
		for (int i=0;i<_bufSlate.size();i++)
			_ValidateEdgeLocks(_bufSlate[i]);
	}

	_RefreshSwitchPointer();

	_state=LevelSlatesState_NotInSlates;

	_tUpdate=_level->GetT_();

	_skilldriver.Init(this);

}

void CLoSlatesA::Clear()
{
	_DestroyEmbeds();

	_skilldriver.Clear();
	_sensor.Destroy();
	_bufSlate.clear();
	_reachables.clear();
	_linksTeleport.clear();
	if (_bhvProcess)
	{
		_bhvProcess->Clear();
		Safe_Class_Delete(_bhvProcess);
	}
	Zero();
}


void CLoSlatesA::OnDestroy()
{
	Clear();
}

void CLoSlatesA::HandleHook(LevelHook &hk0)
{
	if (hk0.GetType()==LH_PlayerLeaveLevel)
	{
		LevelHook_PlayerLeaveLevel &hk=(LevelHook_PlayerLeaveLevel &)hk0;
		CLevelPlayer *player=_level->GetPlayer(hk.idPlayer);

		if (player)
		{
			_RefreshSensor();
			if (_sensor.GetThreat()==player->GetLoUnit())
			{
				//将位置调整为进入Slate前的位置

				if (_enter!=LevelSlateIdx_Invalid)
				{
					LevelPos pos=_GetSlate(_enter).GetPos();
					if (_level->GetUnitMgr()->ToClosestWalkable(UnitFindPath_Walkable,pos))
					{
						LevelPlayerStates *lps=player->GetLPS();
						if (lps)
						{
							lps->base.pos=pos;
							lps->base.SetDirtyDB_Urgent();
						}
					}
				}
			}
		}

	}
}


void CLoSlatesA::_WriteInitials(CBitPacket *bp)
{
	bp->Data_WriteSimple(_info->iStartSlate);

	bp->Data_WriteSimple(_info->dirH);
	bp->Data_WriteSimple(_info->dirV);

	bp->Data_NextWord()=_bufSlate.size();

	CLevelRecords *records=_level->GetRecords();

	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA &slate=_bufSlate[i];
		LevelGUID uid=slate._info->GetUID();
		bp->Data_WriteSimple(uid);

		if (TRUE)
		{
			LevelSlateA_Cover cover=LevelSlateA_Cover_None;
			LevelRecordSlateType *recSlateType=records->GetSlateType(slate._status.tp);
			if (recSlateType)
				cover=recSlateType->Cover;

			bp->Data_WriteSimple(cover);
		}

		if(slate._statusCover.nEdgeLocks>0)
		{
			bp->Bit_Write_1();
			bp->Data_NextByte()=(BYTE)slate._info->nLinks;
			bp->Data_WriteData(slate._info->links,slate._info->nLinks*sizeof(slate._info->links[0]));
		}
		else
			bp->Bit_Write_0();
	}

}


void CLoSlatesA::_WriteStates(CBitPacket *bp)
{
	WORD &count=bp->Data_NextWord();
	count=0;
	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA *slate=&_bufSlate[i];

		if (!slate->_bSyncDirty)
			continue;

		bp->Data_WriteSimple(slate->_info->idxMe);
		bp->Data_WriteSimpleR(slate->_statusCover);

		if ((slate->_status.bRevealed||slate->_status.bFlipped||SlateA_IsKnownState(slate->_status.tp)||slate->_status.bEntrance) )
		{
			bp->Bit_Write_1();
			bp->Data_WriteSimpleR(slate->_status);
		}
		else
			bp->Bit_Write_0();

		count++;
	}

	DP_WriteVector(*bp,_reachables);
	DP_WriteVector(*bp,_linksTeleport);
	bp->Data_WriteSimple(_reached);
	bp->Data_WriteSimple(_state);
	bp->Data_NextShort()=(short)_nStars;

}

void CLoSlatesA::_WriteEmbeds(CBitPacket *bp)
{
	bp->Data_NextWord()=_embeds.size();
	std::unordered_map<LevelSlateIdx,LevelObjID>::iterator it;
	for (it=_embeds.begin();it!=_embeds.end();it++)
	{
		bp->Data_WriteSimple((*it).first);
		bp->Data_WriteSimple((*it).second);
	}
}


void CLoSlatesA::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteInitials(bp);
	_WriteStates(bp);
	_WriteEmbeds(bp);
	bContent=TRUE;
}

void CLoSlatesA::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_verSync==_ver)
	{
		bp->Bit_Write(FALSE);
		return;
	}

	bp->Bit_Write(TRUE);
	_WriteStates(bp);
	_WriteEmbeds(bp);

	bContent=TRUE;
}

void CLoSlatesA::_OnPostWriteSync()
{
	_verSync=_ver;

	for (int i=0;i<_bufSlate.size();i++)
		_bufSlate[i].ResetSyncDirty();
}

CLevelPlayer *CLoSlatesA::_GetCurPlayer()
{
	extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
	return LevelUtil_GetFirstPlayer(_level);
}


void CLoSlatesA::_RefreshSensor()
{
	_sensor.SetThreat(NULL);
	if (_state!=LevelSlatesState_NotInSlates)
	{
		CLevelPlayer *player=_GetCurPlayer();
		if (player)
		{
			CLoUnit *loPlayer=player->GetLoUnit();
			if (loPlayer)
				_sensor.SetThreat(loPlayer);
		}
	}
}

void CLoSlatesA::_FlushStar()
{		
	if (_nStars!=0)
	{
		if (_level)
		{
			if (_GetCurPlayer())
			{
				CLoUnit *loUnit=_GetCurPlayer()->GetLoUnit();
				if (loUnit)
					_level->GetDecider()->MakeResModify(loUnit,_GetResType(),_nStars);
			}
		}
	}

	_nStars=0;

}


void CLoSlatesA::Update()
{
	AnimTick dt=0;
	if (TRUE)
	{
		LevelTick t=_level->GetT_();
		dt=ANIMTICK_SAFE_MINUS(t,_tUpdate);
		_tUpdate=t;
	}
	_RefreshSensor();
	_UpdateState();

	_skilldriver.Update(dt);

//	_FlushStar();

}


LevelSlatesGrpHandle CLoSlatesA::_FindGrp(StringID nmGrp)
{
	LevelSlatesGrpHandle hGrp;
	hGrp.iStart=hGrp.count=0;
	if (_info)
	{
		std::unordered_map<StringID,LevelSlatesInfo::Grp>::iterator it=_info->grps.find(nmGrp);
		if (it!=_info->grps.end())
			hGrp=(*it).second;
	}

	return hGrp;
}


void CLoSlatesA::Setup_SetEntrance(StringID nmGrp)
{
	LevelSlatesGrpHandle hGrp=_FindGrp(nmGrp);
	if (LevelSlatesGrpHandle_IsValid(hGrp))
	{
		for (int i=0;i<LevelSlatesGrpHandle_GetCount(hGrp);i++)
		{
			CLevelSlateA &slate=_GetGrpSlate(hGrp,i);
			slate._status.bEntrance=1;
//			slate._status.bRevealed=1;

			slate.SetSyncDirty();
		}
	}
}

void CLoSlatesA::Setup_SetExit(StringID nmGrp)
{
	LevelSlatesGrpHandle hGrp=_FindGrp(nmGrp);
	if (LevelSlatesGrpHandle_IsValid(hGrp))
	{
		for (int i=0;i<LevelSlatesGrpHandle_GetCount(hGrp);i++)
		{
			CLevelSlateA &slate=_GetGrpSlate(hGrp,i);
			slate._status.bExit=1;

			slate.SetSyncDirty();
		}
	}
}


void CLoSlatesA::Setup_SetType(StringID grp,LevelSlateType tp,std::vector<LevelSlateIdx> *result)
{
	if (grp==StringID_Invalid)
	{
		for (int i=0;i<_bufSlate.size();i++)
		{
			_bufSlate[i]._status.tp=tp;

			_bufSlate[i].SetSyncDirty();

			if (result)
				result->push_back(_bufSlate[i]._info->idxMe);
		}
	}
	else
	{
		LevelSlatesGrpHandle hGrp=_FindGrp(grp);
		if (LevelSlatesGrpHandle_IsValid(hGrp))
		{
			for (int i=0;i<LevelSlatesGrpHandle_GetCount(hGrp);i++)
			{
				CLevelSlateA &slate=_GetGrpSlate(hGrp,i);
				slate._status.tp=tp;

				slate.SetSyncDirty();

				if (result)
					result->push_back(slate._info->idxMe);
			}
		}
	}
}

void CLoSlatesA::Setup_SetType_RandomPick(StringID grp,SlatesRandomPickEntryA *entries,DWORD c,std::vector<LevelSlateIdx> *result)
{
	if (result)
		result->clear();

	LevelSlatesGrpHandle hGrp;
	DWORD nTotal=0;
	if (grp==StringID_Invalid)
		nTotal=_bufSlate.size();
	else
	{
		hGrp=_FindGrp(grp);
		if (LevelSlatesGrpHandle_IsValid(hGrp))
			nTotal=LevelSlatesGrpHandle_GetCount(hGrp);
	}
	if (nTotal<=0)
		return;

	if (!entries)
		return;
	if (c<=0)
		return;

	CSysRandom::GenRandomIndices(_indicesTemp,nTotal);

	for (int i=0;i<c;i++)
	{
		SlatesRandomPickEntryA &e=entries[i];
		int idx=0;

		int j=0;
		while(j<e.count)
		{
			if (idx>=_indicesTemp.size())
				break;

			CLevelSlateA *slate=NULL;
			if (!LevelSlatesGrpHandle_IsValid(hGrp))
				slate=&_bufSlate[_indicesTemp[idx]];
			else
				slate=&_GetGrpSlate(hGrp,_indicesTemp[idx]);

			assert(slate);

			if (e.tpsTarget.size()>0)
			{
				BOOL bFiltered=TRUE;
				for (int k=0;k<e.tpsTarget.size();k++)
				{
					if (e.tpsTarget[k]==slate->_status.tp)
					{
						bFiltered=FALSE;
						break;
					}
				}
				if (bFiltered)
				{
					idx++;
					continue;
				}
			}

			if (e.tp!=LevelSlateType_None)
			{
				slate->_status.tp=e.tp;
				slate->SetSyncDirty();
			}
			if (result)
				result->push_back(slate->_info->idxMe);
			_indicesTemp[idx]=_indicesTemp[_indicesTemp.size()-1];
			_indicesTemp.pop_back();

			j++;
		}
	}
}

void CLoSlatesA::Setup_SetMatchKey(std::vector<LevelSlateIdx> &indicesSlates,int matchkey)
{
	for (int i=0;i<indicesSlates.size();i++)
	{
		if (!_CheckValidSlateIdx(indicesSlates[i]))
			continue;

		CLevelSlateA &slate=_GetSlate(indicesSlates[i]);
		slate._status.matchkey=matchkey;

		slate.SetSyncDirty();
	}
}

void CLoSlatesA::Setup_SetSwitch(std::vector<LevelSlateIdx> &indicesLocks,std::vector<LevelSlateIdx> &indicesSwitches,LevelSlateA_SwitchChannel channel)
{
	CSysRandom::GenRandomIndices(_indicesTemp,indicesSwitches.size());

	DWORD nSwitches=0;
	for (int i=0;i<_indicesTemp.size();i++)
	{
		if (!_CheckValidSlateIdx(indicesSwitches[_indicesTemp[i]]))
			continue;

		CLevelSlateA &slate=_GetSlate(indicesSwitches[_indicesTemp[i]]);
		slate._status.channelSwitch=channel;
		slate._status.serialSwitch=i;

		nSwitches++;

		slate.SetSyncDirty();
	}

	for (int i=0;i<indicesLocks.size();i++)
	{
		if (!_CheckValidSlateIdx(indicesLocks[i]))
			continue;

		CLevelSlateA &slate=_GetSlate(indicesLocks[i]);
		slate._statusCover.channelSwitch=channel;
		slate._statusCover.nSwitches=nSwitches;

		slate.SetSyncDirty();
	}
}

void CLoSlatesA::Setup_SetSwitchPointer(std::vector<LevelSlateIdx> &indicesSwitchPointers,LevelSlateA_SwitchChannel channel)
{
	for (int i=0;i<indicesSwitchPointers.size();i++)
	{
		if (!_CheckValidSlateIdx(indicesSwitchPointers[i]))
			continue;

		CLevelSlateA &slate=_GetSlate(indicesSwitchPointers[i]);

		slate._status.channelSwitch=channel;

		slate.SetSyncDirty();
	}
}

void CLoSlatesA::Setup_SetButton(std::vector<LevelSlateIdx> &indicesLocks,std::vector<LevelSlateIdx> &indicesButtons,LevelSlateA_ButtonChannel channel)
{
	CSysRandom::GenRandomIndices(_indicesTemp,indicesButtons.size());

	DWORD nButtons=0;
	for (int i=0;i<_indicesTemp.size();i++)
	{
		if (!_CheckValidSlateIdx(indicesButtons[_indicesTemp[i]]))
			continue;

		CLevelSlateA &slate=_GetSlate(indicesButtons[_indicesTemp[i]]);
		slate._status.channelButton=channel;
		slate._status.countButtonChips=0;
		slate._status.serialButton=i;

		nButtons++;

		slate.SetSyncDirty();
	}

	for (int i=0;i<indicesLocks.size();i++)
	{
		if (!_CheckValidSlateIdx(indicesLocks[i]))
			continue;

		CLevelSlateA &slate=_GetSlate(indicesLocks[i]);
		slate._statusCover.channelButton=channel;
		slate._statusCover.nButtons=nButtons;
		slate._statusCover.locksButton=(1<<nButtons)-1;

		slate.SetSyncDirty();
	}
}



void CLoSlatesA::Setup_SetEdgeLock(std::vector<LevelSlateIdx> &indicesSlates)
{
	for (int i=0;i<indicesSlates.size();i++)
	{
		if (!_CheckValidSlateIdx(indicesSlates[i]))
			continue;

		CLevelSlateA &slate=_GetSlate(indicesSlates[i]);

		slate._statusCover.nEdgeLocks=slate._info->nLinks;
		slate._statusCover.locksEdge=(1<<slate._info->nLinks)-1;

		slate.SetSyncDirty();
	}

}

void CLoSlatesA::_TriggerSwitch(CLevelSlateA &slateSwitch)
{
	if (!slateSwitch._status.IsSwitch())
		return;

	BOOL bTriggered=FALSE;
	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA *slate=&_bufSlate[i];
		if (slate->_statusCover.channelSwitch!=slateSwitch._status.channelSwitch)
			continue;

		if (slate->_statusCover.nUnlockedSwitches!=slateSwitch._status.serialSwitch)
			continue;

		slate->_statusCover.nUnlockedSwitches++;
		slate->SetSyncDirty();
		bTriggered=TRUE;
	}

	if (bTriggered)
	{
		slateSwitch._status.bSwitchActivated=1;
		slateSwitch.SetSyncDirty();

		_RefreshSwitchPointer();
		_IncVer();
	}
}


void CLoSlatesA::_ApplyReach(CLevelSlateA &slateReach,CLevelPlayer *player)
{
	_reached=slateReach._info->idxMe;
	_RevealSlate(slateReach);
	_TriggerSwitch(slateReach);

	_reachables.clear();
	_revealsTemp.clear();

	extern BOOL LevelUtil_CheckSlatesAutoReveal(CLevelPlayer *player);
	BOOL bAutoReveal=	LevelUtil_CheckSlatesAutoReveal(player);

	for (int i=0;i<slateReach._info->nLinks;i++)
	{
		LevelSlateIdx idxLink=slateReach._info->links[i];
		if (!_CheckValidSlateIdx(idxLink))
			continue;

		CLevelSlateA &slate=_GetSlate(idxLink);

		if ((!slate._statusCover.IsButtonLock())&&(!slate._statusCover.IsSwitchLock()))
		{
			if (bAutoReveal)
				_revealsTemp.push_back(idxLink);
			else
			{
				if (SlateA_IsEmbeddingSlate(slate._status.tp))//如果有embed的slate,要强制Reveal
					_revealsTemp.push_back(idxLink);
			}
		}

		//如果是一个打开的Fence,要把Fence另一边的Slate设为Reachable的
		if (slate._status.tp==LevelSlateTypeA_Fence)
		{
			if (slate._status.bOpened)
			{
				for (int i=0;i<slate._info->nLinks;i++)
				{
					if (slate._info->links[i]==slateReach._info->idxMe)
						continue;
					UNIQUE_VEC_ADD(_reachables,slate._info->links[i]);
					if (bAutoReveal)
						_revealsTemp.push_back(slate._info->links[i]);
				}
			}
		}
		else
			_reachables.push_back(idxLink);
	}


	LevelSlateIdx idxTeleportTarget=FindTeleportTarget(_reached);
	if (idxTeleportTarget!=LevelSlateIdx_Invalid)
	{
		UNIQUE_VEC_ADD(_reachables,idxTeleportTarget);
		if (bAutoReveal)
		{
			if (!CheckInTeleportCD())
			{
				UNIQUE_VEC_ADD(_revealsTemp,idxTeleportTarget);
			}
		}
	}

	//Reveal
	if (TRUE)
	{
		CLevelRecords *records=NULL;
		if (_level)
			records=_level->GetRecords();

		for (int i=0;i<_revealsTemp.size();i++)
		{
			CLevelSlateA &slate=_GetSlate(_revealsTemp[i]);
			_RevealSlate(slate);
		}
	}

	_revealsTemp.clear();

	_IncVer();
}

void CLoSlatesA::_UpdateState()
{
	CLevelPlayer *player=_GetCurPlayer();
	if (!player)
		return;
	CLoUnit *loPlayer=player->GetLoUnit();
	if (!loPlayer)
		return;

	LevelPos posPlayer=loPlayer->GetFramePos();

	if (_state==LevelSlatesState_NotInSlates)
	{
		for (int i=0;i<_bufSlate.size();i++)
		{
			CLevelSlateA *slate=&_bufSlate[i];
			if (!slate->_status.bEntrance)
				continue;

			if (slate->CheckIn(posPlayer))
			{
				_enter=slate->_info->idxMe;
				extern void LevelUtil_ChangeInSlatesBuff(CLevelPlayer *player,LevelObjID idSlates,BOOL bAddOrRemove);
				LevelUtil_ChangeInSlatesBuff(player,GetID(),TRUE);
				_ApplyReach(*slate,player);
				_SwitchState_Process(*slate);
				break;
			}
		}
	}
	if (_state==LevelSlatesState_Process)
	{
		_RefreshSensor();
		if (_bInProcess)
		{
			if (_bhvProcess)
				_bhvProcess->Update();
		}
		else
		{
			if (_bhvProcess)
				_bhvProcess->Clear();
			Safe_Class_Delete(_bhvProcess);

			_GetSlate(_reached)._status.bProcessed=1;
			_GetSlate(_reached).SetSyncDirty();

			_SwitchState_Idle(_GetSlate(_reached));
		}
	}
	if (_state==LevelSlatesState_Idle)
	{
		for (int i=0;i<_reachables.size();i++)
		{
			CLevelSlateA &slate=_GetSlate(_reachables[i]);
			if (slate.CheckIn(posPlayer))
			{
				_ApplyReach(slate,player);
				_SwitchState_Process(slate);
				break;
			}
		}

		if (_state==LevelSlatesState_Idle)
		{
			if (_level)
			{
				if (_level->GetUnitMgr())
				{
					if (_level->GetUnitMgr()->IsWalkable(UnitFindPath_Walkable,posPlayer))
					{
						extern void LevelUtil_ChangeInSlatesBuff(CLevelPlayer *player,LevelObjID idSlates,BOOL bAddOrRemove);
						LevelUtil_ChangeInSlatesBuff(player,LevelObjID_Invalid,FALSE);

						_SwitchState_NotInSlate();
					}
				}
			}
		}

	}


}

void CLoSlatesA::_SwitchState_Process(CLevelSlateA &slate)
{
	_state=LevelSlatesState_Process;
	_RefreshSensor();

	if (_countTeleportCD>0)
		_countTeleportCD--;

	if (_level)
	{
		CLevelRecords *records=_level->GetRecords();
		if (records)
		{
			LevelRecordSlateType *rec=records->GetSlateType(slate._status.tp);
			if (rec)
			{
				if (rec->idProcessBG!=StringID_Invalid)
				{
					LevelBehaviorContext ctx;
					ctx.lo=this;
					ctx.idxSlate=slate._info->idxMe;
					_bhvProcess=_level->CreateBehavior(rec->idProcessBG,ctx);
					_bhvProcess->Start();
					_bInProcess=TRUE;
				}
			}
		}
	}

	if (!CheckInTeleportCD())
		_AnnealButtons();

	_IncVer();
}

void CLoSlatesA::_SwitchState_Idle(CLevelSlateA &slate)
{
	_state=LevelSlatesState_Idle;

	_IncVer();
}


void CLoSlatesA::_SwitchState_NotInSlate()
{
	_state=LevelSlatesState_NotInSlates;
	_reachables.clear();
	_reached=LevelSlateIdx_Invalid;
	_enter=LevelSlateIdx_Invalid;
	_RefreshSensor();
	_FlushStar();

	_IncVer();
}

CLevelObj *CLoSlatesA::GetThreat()
{
	return _sensor.GetThreat();
}


BOOL CLoSlatesA::CheckProcessed(LevelSlateIdx idxSlate)
{
	if (!_CheckValidSlateIdx(idxSlate))
		return FALSE;
	return _GetSlate(idxSlate)._status.bProcessed?TRUE:FALSE;
}

void CLoSlatesA::_RevealSlate(CLevelSlateA &slate)
{
	if (!_FlipSlate(slate))
		return;

	if (slate._status.bRevealed)
		return;

	slate._status.bRevealed=1;

	slate.SetSyncDirty();

	CLevelRecords *records=_level->GetRecords();
	if (records)
	{
		LevelRecordSlateType *rec=records->GetSlateType(slate._status.tp);
		if (rec)
		{
			//产生Gem
			if (TRUE)
			{
				if (rec->nMaxGem>=rec->nMinGem)
				{
					slate._status.nGems=CSysRandom::RandRangeInt(rec->nMinGem,rec->nMaxGem+1);
					_nStars+=slate._status.nGems;
					_IncVer();
				}
				else
					slate._status.nGems=0;
			}
			if (rec->idRevealBG!=StringID_Invalid)
			{
				LevelBehaviorContext ctx;
				ctx.lo=this;
				ctx.idxSlate=slate._info->idxMe;
				CLevelBehavior *bhv=_level->CreateBehavior(rec->idRevealBG,ctx);
				bhv->Start();
				bhv->Clear();
				Safe_Class_Delete(bhv);
			}
		}
	}

	if (slate._status.tp==LevelSlateTypeA_Door_Entry)
		_RefreshTeleportLink();
}

//检查某个Slate是否可以EdgeLock周围的Slate
BOOL CLoSlatesA::_CanLockEdge(CLevelSlateA &slate)
{
	if (slate._statusCover.nEdgeLocks>0)
		return FALSE;
	return TRUE;
}


void CLoSlatesA::_ValidateEdgeLocks(CLevelSlateA &slate)
{
	if (slate._statusCover.nEdgeLocks<=0)
		return;

	SlateA_EdgeLockMask locksOld=slate._statusCover.locksEdge;

	for (int i=0;i<slate._info->nLinks;i++)
	{
		CLevelSlateA &slate2=_GetSlate(slate._info->links[i]);

		if (!_CanLockEdge(slate2))
			slate._statusCover.locksEdge&=(~(1<<i));//清除这个Lock
	}

	if (slate._statusCover.locksEdge!=locksOld)
		slate.SetSyncDirty();
}


BOOL CLoSlatesA::_FlipSlate(CLevelSlateA &slate)
{
	if (slate._status.bFlipped)
		return TRUE;

	if(slate.CheckCoverLocked())
		return FALSE;

	slate._status.bFlipped=1;

	slate.SetSyncDirty();

	//解锁周围Slate的EdgeLock
	for (int i=0;i<slate._info->nLinks;i++)
	{
		CLevelSlateA &slate2=_GetSlate(slate._info->links[i]);

		if (slate2._statusCover.nEdgeLocks>0)
		{
			for (int j=0;j<slate2._info->nLinks;j++)
			{
				if (slate2._info->links[j]==slate._info->idxMe)
				{
					slate2._statusCover.locksEdge&=(~(1<<j));
					slate2.SetSyncDirty();
				}
			}
		}
	}

	return TRUE;
}



void CLoSlatesA::RevealNearBy(LevelSlateIdx idxSlate,int radius)
{
	if (!_CheckValidSlateIdx(idxSlate))
		return;

	CLevelSlateA &slate=_GetSlate(idxSlate);
	if (slate._info->src->links.size()<1)
		return;

	i_math::vector2df &posMe=slate.GetPos();
	float lenHalf=0.0f;
	i_math::vector2df dirH=_info->dirH;
	i_math::vector2df dirV=_info->dirV;
	if (TRUE)
	{
		i_math::vector3df pos3D=slate._info->src->links[0].getTranslation();
		slate._info->src->GetMat().transformVect(pos3D,pos3D);
		i_math::vector2df pos=pos3D.getXZ()-posMe;
		lenHalf=pos.getLength();
// 		dirH=pos;
// 		dirV.x=dirH.y;
// 		dirV.y=-dirH.x;
// 
// 		dirH.normalize();
// 		dirV.normalize();
	}

	float range=lenHalf+lenHalf*(float)radius*2.0f
							+lenHalf*0.2f;//一点冗余

	for (int i=0;i<_bufSlate.size();i++)
	{
		LevelPos &pos=_bufSlate[i].GetPos();
		i_math::vector2df dir=pos-posMe;
		if (fabsf(dir.dotProduct(dirH))>range)
			continue;
		if (fabsf(dir.dotProduct(dirV))>range)
			continue;

		_RevealSlate(_bufSlate[i]);
	}

	_IncVer();
}

void CLoSlatesA::RevealAll(LevelSlateIdx idxSlate)
{
	if (!_CheckValidSlateIdx(idxSlate))
		return;

	for (int i=0;i<_bufSlate.size();i++)
		_RevealSlate(_bufSlate[i]);

	_IncVer();

}

RollAwardParam *CLoSlatesA::GetRollAwardParam()
{
	LopSlatesA *lop=(LopSlatesA *)_param;
	return &lop->paramAwardTB;
}


BOOL CLoSlatesA::GetTeleLeavePos(LevelPos &pos)
{
	LopSlatesA *lop=(LopSlatesA *)_param;
	if (lop->sitesTeleLeave.size()>0)
	{
		pos=lop->sitesTeleLeave[0].getTranslationP()->getXZ();
		return TRUE;
	}
	return FALSE;
}

LevelSlateIdx CLoSlatesA::FindTeleportTarget(LevelSlateIdx idxSlate)
{
	if (!_CheckValidSlateIdx(idxSlate))
		return LevelSlateIdx_Invalid;

	CLevelSlateA &slate=_GetSlate(idxSlate);
	if (slate._status.tp!=LevelSlateTypeA_Door_Entry)
		return LevelSlateIdx_Invalid;

	int iOff=idxSlate-_info->iStartSlate;

	LevelPos pos=slate.GetPos();
	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA &slate2=_bufSlate[(i+iOff)%_bufSlate.size()];
		if (&slate2==&slate)
			continue;

		if (slate2._status.matchkey!=slate._status.matchkey)
			continue;

		if ((slate2._status.tp!=LevelSlateTypeA_Door_Exit)&&(slate2._status.tp!=LevelSlateTypeA_Door_Entry))
			continue;

		return slate2._info->idxMe;
	}

	return LevelSlateIdx_Invalid;
}

void CLoSlatesA::_RefreshTeleportLink()
{
	//首先找到第一个Reveal的传送门
	LevelSlateIdx idxStart=LevelSlateIdx_Invalid;
	for (int i=0;i<_bufSlate.size();i++)
	{
		if (_bufSlate[i]._status.tp==LevelSlateTypeA_Door_Entry)
		{
			idxStart=_bufSlate[i]._info->idxMe;
			break;
		}
	}

	if (idxStart!=LevelSlateIdx_Invalid)
	{
		LevelSlateIdx idx=idxStart;
		while(1)
		{
			LevelSlateIdx idxNext=FindTeleportTarget(idx);
			if (idxNext==LevelSlateIdx_Invalid)
				break;
			if (_GetSlate(idx)._status.bRevealed&&_GetSlate(idxNext)._status.bRevealed)
				_AddTeleportLink(idx,idxNext);

			if (idxNext==idxStart)
				break;
			idx=idxNext;
		}
	}
}


void CLoSlatesA::_AddTeleportLink(LevelSlateIdx idxFrom,LevelSlateIdx idxTo)
{
	for (int i=0;i<_linksTeleport.size();i++)
	{
		if ((_linksTeleport[i].idxFrom==idxFrom)&&(_linksTeleport[i].idxTo==idxTo))
			return;
	}

	LevelSlateA_TeleportLink link;
	link.idxFrom=idxFrom;
	link.idxTo=idxTo;
	_linksTeleport.push_back(link);

	_IncVer();
}


void CLoSlatesA::OpenFenceWithSwitch(LevelSlateIdx idxSwitchSlate)
{
	if (!_CheckValidSlateIdx(idxSwitchSlate))
		return;

	DWORD matchkey=_GetSlate(idxSwitchSlate)._status.matchkey;

	BOOL bAnyChange=FALSE;
	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA &slate=_bufSlate[i];

		if (slate._status.tp!=LevelSlateTypeA_Fence)
			continue;

		if (slate._status.matchkey!=matchkey)
			continue;

		if (!slate._status.bOpened)
		{
			slate._status.bOpened=1;
			slate.SetSyncDirty();
			bAnyChange=TRUE;
		}
	}

	if (bAnyChange)
	{
		if (_CheckValidSlateIdx(_reached))
			_ApplyReach(_GetSlate(_reached),_GetCurPlayer());//刷新一下Reachable
		_IncVer();
	}
}


BOOL CLoSlatesA::GetSlatePos(LevelSlateIdx idxSlate,LevelPos &pos)
{
	if (!_CheckValidSlateIdx(idxSlate))
		return FALSE;
	pos=_GetSlate(idxSlate).GetPos();
	return TRUE;

}

LevelSlateIdx CLoSlatesA::FindEmbedID(LevelObjID id)
{
	std::unordered_map<LevelSlateIdx,LevelObjID>::iterator it;
	for (it=_embeds.begin();it!=_embeds.end();it++)
	{
		if ((*it).second==id)
			return (*it).first;
	}
	return LevelSlateIdx_Invalid;
}

LevelObjID CLoSlatesA::_FindEmbed(LevelSlateIdx idx)
{
	std::unordered_map<LevelSlateIdx,LevelObjID>::iterator it=_embeds.find(idx);
	if (it!=_embeds.end())
		return (*it).second;

	return LevelObjID_Invalid;
}

void CLoSlatesA::_AddEmbed(LevelSlateIdx idx,LevelObjID idLo)
{
	_embeds[idx]=idLo;
	_IncVer();
}

void CLoSlatesA::_FlushEmbeds()
{
	std::unordered_map<LevelSlateIdx,LevelObjID>::iterator it=_embeds.begin();

	while(it!=_embeds.end())
	{
		std::unordered_map<LevelSlateIdx,LevelObjID>::iterator itCur=it;
		it++;
		LevelObjID idLo=(*itCur).second;
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,idLo);
		if (!lo)
			_embeds.erase(itCur);
	}
}

void CLoSlatesA::_DestroyEmbeds()
{
	std::unordered_map<LevelSlateIdx,LevelObjID>::iterator it;
	for (it=_embeds.begin();it!=_embeds.end();it++)
	{
		LevelObjID idLo=(*it).second;
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,idLo);
		if (lo)
			lo->DeferDestroy();
	}
	_embeds.clear();
	_IncVer();
}

BOOL CLoSlatesA::SpawnAgent(LevelSlateIdx idxSlate, RecordID idAgent,BOOL bFaceToReachedSlate)
{
	if (!_CheckValidSlateIdx(idxSlate))
		return FALSE;

	_FlushEmbeds();

	if (_FindEmbed(idxSlate)!=LevelObjID_Invalid)
		return FALSE;//已经有一个embed了

	if  (TRUE)
	{
		CLoGeneralAgent* loAgent=(CLoGeneralAgent*)_level->CreateObj(Class_Ptr2(CLoGeneralAgent));

		LevelPos pos=_GetSlate(idxSlate).GetPos();
		LevelFace face=0;
		if (bFaceToReachedSlate)
		{
			if (_CheckValidSlateIdx(_reached))
			{
				LevelPos posReached=_GetSlate(_reached).GetPos();
				LevelPos dir=posReached-pos;
				face=LevelFaceFromDir(dir);
			}
		}

		i_math::xformf xfm;
		LevelFaceToQuat(face,xfm.rot);
		xfm.pos=LevelUtil_GetGroundHeight(_level,pos.x,pos.y,TRUE);

		loAgent->PostCreate(xfm.getMatrix(),idAgent,LevelPlayerID_NeutralWild);

		_level->AddToActives(loAgent);

		_AddEmbed(idxSlate,loAgent->GetID());

		SAFE_RELEASE(loAgent);
	}

	return TRUE;
}

void CLoSlatesA::RequestFlipFromClient(LevelPlayerID idPlayer,LevelSlateIdx idxSlate)
{
	CLevelPlayer *player=_GetCurPlayer();
	if (player->GetPlayerID()==idPlayer)
	{
		//目前不加合法性检查
		if (_CheckValidSlateIdx(idxSlate))
		{
			if(_FlipSlate(_GetSlate(idxSlate)))
			{
				_RevealSlate(_GetSlate(idxSlate));
				_IncVer();
			}
		}
	}
}

void CLoSlatesA::RequestIncSlateButtonChip(LevelPlayerID idPlayer,LevelSlateIdx idxSlate)
{
	CLevelPlayer *player=_GetCurPlayer();
	if (player->GetPlayerID()==idPlayer)	
	{
		//目前不加合法性检查
		if (_CheckValidSlateIdx(idxSlate))
		{
			CLevelSlateA &slate=_GetSlate(idxSlate);
			if (slate._status.IsButton())
			{
				if (slate._status.countButtonChips<LevelSlateA_MaxButtonChips)
				{
					_nStars-=2;
					
					slate._status.countButtonChips++;
					slate.SetSyncDirty();

					_RefreshButtonLocks();
//						_FlushStar();
					_IncVer();
				}
			}
		}
	}
}


void CLoSlatesA::_RefreshSwitchPointer()
{
	CLevelSlateA *targets[LevelSlateA_SwitchChannel_Max];
	memset(targets,0,sizeof(targets));

	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA &slate=_bufSlate[i];
		if (slate._status.IsSwitch())
		{
			if (!slate._status.bSwitchActivated)
			{
				CLevelSlateA *&target=targets[slate._status.channelSwitch];
				if (!target)
					target=&slate;
				else
				{
					if (target->_status.serialSwitch>slate._status.serialSwitch)
						target=&slate;
				}
			}
		}
	}


	BOOL bAnyChange=FALSE;
	for (int k=0;k<ARRAY_SIZE(targets);k++)
	{
		if (!targets[k])
			continue;

		CLevelSlateA *target=targets[k];

		LevelSlateA_SwitchChannel ch=(LevelSlateA_SwitchChannel)k;
		for (int i=0;i<_bufSlate.size();i++)
		{
			CLevelSlateA &slate=_bufSlate[i];

			if (!slate._status.IsSwitchPointer())
				continue;

			if (slate._status.channelSwitch!=ch)
				continue;

			DWORD radoff=0;
			if (TRUE)
			{
				i_math::vector2df dir=target->GetPos()-slate.GetPos();
				i_math::vector3df eulerH;
				eulerH.setXZ(_info->dirH);
				eulerH.toEuler();
				i_math::vector3df euler;
				euler.setXZ(dir);
				euler.toEuler();

				float rad=euler.x-eulerH.x+i_math::Pi/8.0f;
				rad=i_math::wrap_radian(rad);
				rad/=i_math::Pi/4.0f;
				radoff=(DWORD)rad;
				radoff%=8;
			}

			if (radoff!=slate._status.radoffSwitchPointer)
			{
				slate._status.radoffSwitchPointer=radoff;
				slate.SetSyncDirty();

				bAnyChange=TRUE;
			}
		}
	}

	if (bAnyChange)
		_IncVer();
}

void CLoSlatesA::_AnnealButtons()
{
	BOOL bAnyChange=FALSE;
	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA &slate=_bufSlate[i];
		if (slate._status.IsButton())
		{
			if (slate._status.countButtonChips>0)
			{
				slate._status.countButtonChips--;
				slate.SetSyncDirty();
				bAnyChange=TRUE;
			}
		}
	}

	if (bAnyChange)
	{
		_IncVer();
		_RefreshButtonLocks();
	}
}


void CLoSlatesA::_RefreshButtonLocks()
{
	SlateA_ButtonLockMask locks[LevelSlateA_ButtonChannel_Max];
	memset(locks,0xff,sizeof(locks));

	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA &slate=_bufSlate[i];
		if (slate._status.IsButton())
		{
			if (slate._status.countButtonChips>0)
				locks[slate._status.channelButton]&=~(1<<slate._status.serialButton);
		}
	}

	BOOL bAnyChange=FALSE;

	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateA &slate=_bufSlate[i];
		if (slate._statusCover.IsButtonLock())
		{
			SlateA_ButtonLockMask locksNew=locks[slate._statusCover.channelButton];
			locksNew&=(1<<slate._statusCover.nButtons)-1;
			if (slate._statusCover.locksButton!=locksNew)
			{
				slate._statusCover.locksButton=locksNew;
				slate.SetSyncDirty();
				bAnyChange=TRUE;
			}
		}
	}

	if (bAnyChange)
		_IncVer();
}


void CLoSlatesA::ModRes(int mod)
{
	_nStars+=mod;
	_IncVer();
}
