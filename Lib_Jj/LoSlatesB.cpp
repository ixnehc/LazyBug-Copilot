
#include "stdh.h"

#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelHooks.h"

#include "LevelUtil.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"

#include "LoSlate.h"
#include "LoSlatesB.h" 

#include "SlatesBuilderB.h"

#include "LevelOSB.h"

#include "LevelRecordSlateType.h"
#include "LevelRecordGlobal.h"

#include "Random/Random.h"
#include "commondefines/general_stl.h"


//////////////////////////////////////////////////////////////////////////
//CLevelSlateB

void CLevelSlateB::Init(LevelSlateInfo *info)
{
	_info=info;

	_status.tp=info->tpDef;

	_pos=info->src->GetPos();
}

BOOL CLevelSlateB::CheckIn(LevelPos &pos)
{
	const float radiusDetect=0.5f;

	if (pos.getDistanceSQFrom(_pos)<radiusDetect*radiusDetect)
		return TRUE;
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//CLoSlatesB

void CLoSlatesB::PostCreate()
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

	}

	_state=LevelSlatesState_NotInSlates;

	_tUpdate=_level->GetT_();

	_skilldriver.Init(this);

}

void CLoSlatesB::Clear()
{

	_skilldriver.Clear();
	_sensor.Destroy();
	_bufSlate.clear();
	_reachables.clear();
	if (_bhvProcess)
	{
		_bhvProcess->Clear();
		Safe_Class_Delete(_bhvProcess);
	}
	Zero();
}


void CLoSlatesB::OnDestroy()
{
	Clear();
}

void CLoSlatesB::HandleHook(LevelHook &hk0)
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


void CLoSlatesB::_WriteInitials(CBitPacket *bp)
{
	bp->Data_WriteSimple(_info->iStartSlate);

	bp->Data_WriteSimple(_info->dirH);
	bp->Data_WriteSimple(_info->dirV);

	bp->Data_NextWord()=_bufSlate.size();

	CLevelRecords *records=_level->GetRecords();

	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateB &slate=_bufSlate[i];
		LevelGUID uid=slate._info->GetUID();
		bp->Data_WriteSimple(uid);
	}

}


void CLoSlatesB::_WriteStates(CBitPacket *bp)
{
	WORD &count=bp->Data_NextWord();
	count=0;
	for (int i=0;i<_bufSlate.size();i++)
	{
		CLevelSlateB *slate=&_bufSlate[i];

		if (!slate->_bSyncDirty)
			continue;

		bp->Data_WriteSimple(slate->_info->idxMe);

		if (slate->_status.nRevealed>0)
		{
			bp->Bit_Write_1();
			bp->Data_WriteSimpleR(slate->_status);
		}
		else
			bp->Bit_Write_0();

		count++;
	}

	DP_WriteVector(*bp,_reachables);
	bp->Data_WriteSimple(_reached);
	bp->Data_WriteSimple(_state);

}



void CLoSlatesB::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteInitials(bp);
	_WriteStates(bp);
	bContent=TRUE;
}

void CLoSlatesB::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_verSync==_ver)
	{
		bp->Bit_Write(FALSE);
		return;
	}

	bp->Bit_Write(TRUE);
	_WriteStates(bp);

	bContent=TRUE;
}

void CLoSlatesB::_OnPostWriteSync()
{
	_verSync=_ver;

	for (int i=0;i<_bufSlate.size();i++)
		_bufSlate[i].ResetSyncDirty();
}

CLevelPlayer *CLoSlatesB::_GetCurPlayer()
{
	extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
	return LevelUtil_GetFirstPlayer(_level);
}


void CLoSlatesB::_RefreshSensor()
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


void CLoSlatesB::Update()
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

}


LevelSlatesGrpHandle CLoSlatesB::_FindGrp(StringID nmGrp)
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

void CLoSlatesB::Setup_SetExit(StringID nmGrp)
{
	LevelSlatesGrpHandle hGrp=_FindGrp(nmGrp);
	if (LevelSlatesGrpHandle_IsValid(hGrp))
	{
		for (int i=0;i<LevelSlatesGrpHandle_GetCount(hGrp);i++)
		{
			CLevelSlateB &slate=_GetGrpSlate(hGrp,i);
			slate._status.bExit=1;

			slate.SetSyncDirty();
		}
	}
}

void CLoSlatesB::_LoadFromData(SlatesBData &data)
{
	for (int i=0;i<_info->wMatrix;i++)
	for (int j=0;j<_info->hMatrix;j++)
	{
		_GetSlate(i,j)._status.tp=(LevelSlateType)data.buf[j*data.w+i].tp;
		_GetSlate(i,j)._status.bPath=(LevelSlateType)data.buf[j*data.w+i].bPath;
	}
}


void CLoSlatesB::Setup_Generate(SlatesB_GenerateParam &param)
{
	SlatesBData *data=_level->GetWorld()->GetData()->PickUpSlatesB(param.lvlDifficulty==0);
	if (data)
		_LoadFromData(*data);
	else
	{
		CSlatesBuilderB builder;
		builder.Init(8,8,8);

		builder.AddLock(1,1);
		builder.AddLock(6,1);
		builder.AddLock(1,6);
		builder.AddLock(6,6);

		builder.Build();

		SlatesBData dataTemp;
		builder.Dump(dataTemp);
		_LoadFromData(dataTemp);
	}

	_indicesTemp.clear();
	for (int i=0;i<_info->wMatrix;i++)
	{
		if (_GetSlate(i,0)._status.IsPath())
			_indicesTemp.push_back(i);
	}
	if (_indicesTemp.size()>0)
	{
		int idx=CSysRandom::RandRangeInt(0,(int)_indicesTemp.size());
		_GetSlate(_indicesTemp[idx],0)._status.bEntrance=1;
		_RevealSlate(_GetSlate(_indicesTemp[idx],0),_GetSlate(_indicesTemp[idx],0)._info->idxMe);
	}
	else
	{
		for (int i=0;i<_info->wMatrix;i++)
		{
			if (_GetSlate(i,1)._status.IsPath()&&(!_GetSlate(i,0)._status.IsLock()))
				_indicesTemp.push_back(i);
		}
		if (_indicesTemp.size()>0)
		{
			int idx=CSysRandom::RandRangeInt(0,(int)_indicesTemp.size());
			_GetSlate(_indicesTemp[idx],0)._status.bEntrance=1;
			_RevealSlate(_GetSlate(_indicesTemp[idx],0),_GetSlate(_indicesTemp[idx],0)._info->idxMe);
		}
	}

}


void CLoSlatesB::_RevealNearbySlates(CLevelSlateB &slate,i_math::pos2di *offsets,DWORD nOffsets)
{
	for (int i=0;i<nOffsets;i++)
	{
		i_math::pos2di pt=slate._info->pt+offsets[i];
		LevelSlateIdx idx=_info->GetSlateAt(pt.x,pt.y);
		if (idx!=LevelSlateIdx_Invalid)
			_revealsTemp.insert(&_GetSlate(idx));
	}
}

void CLoSlatesB::_AddNearbyReachables(CLevelSlateB &slate)
{
	static i_math::pos2di offsets[]=
	{
		i_math::pos2di(-1,0),i_math::pos2di(1,0),i_math::pos2di(0,1),i_math::pos2di(0,-1),
	};
	for (int i=0;i<ARRAYSIZE(offsets);i++)
	{
		i_math::pos2di pt=slate._info->pt+offsets[i];
		LevelSlateIdx idx=_info->GetSlateAt(pt.x,pt.y);
		if (idx!=LevelSlateIdx_Invalid)
		{
			if (!_GetSlate(idx).IsLock())
			{
				UNIQUE_VEC_ADD(_reachables,idx);
			}
		}
	}
}

void CLoSlatesB::_RevealTouchingSlates(CLevelSlateB &slate)
{
	static i_math::pos2di offsets[]=
	{
		i_math::pos2di(-1,0),i_math::pos2di(1,0),i_math::pos2di(0,1),i_math::pos2di(0,-1)
	};
	_RevealNearbySlates(slate,offsets,ARRAYSIZE(offsets));
}



void CLoSlatesB::_ApplyReach(CLevelSlateB &slateReach,CLevelPlayer *player)
{
	_reached=slateReach._info->idxMe;

	_revealsTemp.clear();
	_revealsTemp.insert(&slateReach);

	_reachables.clear();
	_AddNearbyReachables(slateReach);

	i_math::pos2di pt=slateReach._info->pt;

	switch(slateReach._status.tp)
	{
		case LevelSlateTypeB_Cross:
		{
			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Cross_x2:
		{
			static i_math::pos2di offsets[]=
			{
				i_math::pos2di(-1,0),i_math::pos2di(1,0),i_math::pos2di(0,1),i_math::pos2di(0,-1),
				i_math::pos2di(-2,0),i_math::pos2di(2,0),i_math::pos2di(0,2),i_math::pos2di(0,-2),
			};
			_RevealNearbySlates(slateReach,offsets,ARRAYSIZE(offsets));
			break;
		}
		case LevelSlateTypeB_Ring:
		{
			static i_math::pos2di offsets[]=
			{
				i_math::pos2di(-1,0),i_math::pos2di(1,0),i_math::pos2di(0,1),i_math::pos2di(0,-1),
				i_math::pos2di(-1,1),i_math::pos2di(1,1),i_math::pos2di(1,-1),i_math::pos2di(-1,-1),
			};
			_RevealNearbySlates(slateReach,offsets,ARRAYSIZE(offsets));
			break;
		}
		case LevelSlateTypeB_Ring_x2:
		{
			static i_math::pos2di offsets[]=
			{
				i_math::pos2di(-2,-2),i_math::pos2di(-2,-1),i_math::pos2di(-2,0),i_math::pos2di(-2,1),i_math::pos2di(-2,2),
				i_math::pos2di(-1,-2),i_math::pos2di(-1,-1),i_math::pos2di(-1,0),i_math::pos2di(-1,1),i_math::pos2di(-1,2),
				i_math::pos2di(0,-2),i_math::pos2di(0,-1),i_math::pos2di(0,0),i_math::pos2di(-0,1),i_math::pos2di(-0,2),
				i_math::pos2di(1,-2),i_math::pos2di(1,-1),i_math::pos2di(1,0),i_math::pos2di(1,1),i_math::pos2di(1,2),
				i_math::pos2di(2,-2),i_math::pos2di(2,-1),i_math::pos2di(2,0),i_math::pos2di(2,1),i_math::pos2di(2,2),
			};
			_RevealNearbySlates(slateReach,offsets,ARRAYSIZE(offsets));
			break;
		}
		case LevelSlateTypeB_Ver:
		{
			for (int i=0;i<_info->hMatrix;i++)
			{
				LevelSlateIdx idx=_info->GetSlateAt(pt.x,i);
				if (idx!=LevelSlateIdx_Invalid)
					_revealsTemp.insert(&_GetSlate(idx));
			}
			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Hor:
		{
			for (int i=0;i<_info->wMatrix;i++)
			{
				LevelSlateIdx idx=_info->GetSlateAt(i,pt.y);
				if (idx!=LevelSlateIdx_Invalid)
					_revealsTemp.insert(&_GetSlate(idx));
			}
			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Right:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if(_bufSlate[i]._info->pt.x>=pt.x)
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Left:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if(_bufSlate[i]._info->pt.x<=pt.x)
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Down:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if(_bufSlate[i]._info->pt.y<=pt.y)
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Up:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if(_bufSlate[i]._info->pt.y>=pt.y)
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Ascend:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				i_math::pos2di off=_bufSlate[i]._info->pt-pt;
				if ((abs(off.x)==abs(off.y))&&(off.x*off.y>0))
					_revealsTemp.insert(&_bufSlate[i]);
			}
			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_Descend:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				i_math::pos2di off=_bufSlate[i]._info->pt-pt;
				if ((abs(off.x)==abs(off.y))&&(off.x*off.y<0))
					_revealsTemp.insert(&_bufSlate[i]);
			}
			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_LeftUp:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if((_bufSlate[i]._info->pt.y>=pt.y)&&(_bufSlate[i]._info->pt.x<=pt.x))
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_LeftDown:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if((_bufSlate[i]._info->pt.y<=pt.y)&&(_bufSlate[i]._info->pt.x<=pt.x))
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_RightUp:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if((_bufSlate[i]._info->pt.y>=pt.y)&&(_bufSlate[i]._info->pt.x>=pt.x))
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}
		case LevelSlateTypeB_RightDown:
		{
			for (int i=0;i<_bufSlate.size();i++)
			{
				if((_bufSlate[i]._info->pt.y<=pt.y)&&(_bufSlate[i]._info->pt.x>=pt.x))
					_revealsTemp.insert(&_bufSlate[i]);
			}

			_RevealTouchingSlates(slateReach);
			break;
		}

		case LevelSlateTypeB_Full:
		{
			for (int i=0;i<_bufSlate.size();i++)
				_revealsTemp.insert(&_bufSlate[i]);
			break;
		}
		case LevelSlateTypeB_Rune01:
		case LevelSlateTypeB_Rune02:
		case LevelSlateTypeB_Rune03:
		case LevelSlateTypeB_Rune04:
		case LevelSlateTypeB_Rune05:
		case LevelSlateTypeB_Rune06:
		{
			_RevealTouchingSlates(slateReach);
			break;
		}
		//XXXXX:MoreSlateTypeB
	}

	for (int i=0;i<_bufSlate.size();i++)
	{
		if(_revealsTemp.find(&_bufSlate[i])==_revealsTemp.end())
			_UnRevealSlate(_bufSlate[i]);
		else
			_RevealSlate(_bufSlate[i],_reached);
	}

	if (!_bUnlocked)
	{
		_bUnlocked=TRUE;

		for (int i=0;i<_bufSlate.size();i++)
		{
			if(_bufSlate[i].IsLock())
			{
				if (!_bufSlate[i]._status.IsRevealed())
				{
					_bUnlocked=FALSE;
					break;
				}
			}
		}
	}


	_IncVer();
}


void CLoSlatesB::_UpdateState()
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
			CLevelSlateB *slate=&_bufSlate[i];
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

			_SwitchState_Idle(_GetSlate(_reached));
		}
	}
	if (_state==LevelSlatesState_Idle)
	{
		for (int i=0;i<_reachables.size();i++)
		{
			CLevelSlateB &slate=_GetSlate(_reachables[i]);
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

void CLoSlatesB::_SwitchState_Process(CLevelSlateB &slate)
{
	_state=LevelSlatesState_Process;

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

	_IncVer();
}

void CLoSlatesB::_SwitchState_Idle(CLevelSlateB &slate)
{
	_state=LevelSlatesState_Idle;

	_IncVer();
}


void CLoSlatesB::_SwitchState_NotInSlate()
{
	_state=LevelSlatesState_NotInSlates;
	_reachables.clear();
	_reached=LevelSlateIdx_Invalid;
	_enter=LevelSlateIdx_Invalid;

	for (int i=0;i<_bufSlate.size();i++)
		_ClearSlateReveal(_bufSlate[i]);

	for (int i=0;i<_bufSlate.size();i++)
	{
		if (_bufSlate[i].IsEntrance())
			_RevealSlate(_bufSlate[i],_bufSlate[i]._info->idxMe);
	}

	_RefreshSensor();

	_IncVer();
}

CLevelObj *CLoSlatesB::GetThreat()
{
	return _sensor.GetThreat();
}


void CLoSlatesB::_RevealSlate(CLevelSlateB &slate,LevelSlateIdx stamp)
{
	if (!slate.IsLock())
	{
		if (slate._status.nRevealed>0)
			return;
		slate._status.nRevealed=1;
		slate.SetSyncDirty();
	}
	else
	{
		if (_bUnlocked)
			return;

		if (slate._status.nRevealed>=SLATESB_MAX_STAMP)
			return;
		for (int i=0;i<slate._status.nRevealed;i++)
		{
			if (stamp==slate._stamps[i])
				return;
		}
		slate._stamps[slate._status.nRevealed]=stamp;
		slate._status.nRevealed++;

		slate.SetSyncDirty();
	}
}

void CLoSlatesB::_UnRevealSlate(CLevelSlateB &slate)
{
	if (slate._status.nRevealed<=0)
		return;

	if (slate.IsLock()&&_bUnlocked)
		return;

	slate._status.nRevealed--;

	slate.SetSyncDirty();
}

void CLoSlatesB::_ClearSlateReveal(CLevelSlateB &slate)
{
	if (slate.IsLock()&&_bUnlocked)
		return;

	if (slate._status.nRevealed<=0)
		return;

	slate._status.nRevealed=0;

	slate.SetSyncDirty();
}



BOOL CLoSlatesB::GetSlatePos(LevelSlateIdx idxSlate,LevelPos &pos)
{
	if (!_CheckValidSlateIdx(idxSlate))
		return FALSE;
	pos=_GetSlate(idxSlate).GetPos();
	return TRUE;

}

