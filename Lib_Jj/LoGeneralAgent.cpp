
#include "stdh.h"

#include "Level.h"

#include "LevelObjResidable.h"

#include "behaviorgraph/BehaviorMem.h"

#include "LoGeneralAgent.h"

#include "Random/Random.h"

#include "LevelRecords.h"

#include "LevelSkillDriver.h"
#include "LevelTalks.h"
#include "LevelBehavior.h"
#include "LevelAttrs.h"
#include "LevelTroops.h"
#include "LevelEventSrc.h"

#include "LevelRecordAgent.h"

#include "Log/LogDump.h"

//////////////////////////////////////////////////////////////////////////
//LosGeneralAgent
std::vector<i_math::spheref>*LosGeneralAgent::FindShape(StringID nm)
{
	if (nm!=StringID_Invalid)
	{
		for (int i=0;i<shapesEx.size();i++)
		{
			if (shapesEx[i].nm==nm)
				return &shapesEx[i].shape;
		}
	}

	return &shape;
}


//////////////////////////////////////////////////////////////////////////
//LoGeneralAgentShape
void GeneralAgentShape::Clear()
{
	for (int i=0;i<units.size();i++)
		SAFE_DESTROY(units[i]);
	units.clear();
	circles.clear();
}


//////////////////////////////////////////////////////////////////////////
//CLoGeneralAgent
LevelObjCircle *CLoGeneralAgent::GetShapeCircles(DWORD &count)
{
	count=0;
	if (_shape)
	{
		count=_shape->circles.size();
		return &_shape->circles[0];
	}
	return NULL;
}

void CLoGeneralAgent::_BuildShape()
{
	extern void BuildShapeCircles(std::vector<LevelObjCircle>&circles,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat);

	if (!_shape)
	{
		LopGeneralAgent *lop=GetLop<LopGeneralAgent>();
		if (lop)
		{
			if (lop->shape.size()>0)
			{
				_shape=Class_New2(GeneralAgentShape);
				_BuildShapeUnits(_shape->units,lop->shape,*i_math::matrix43f::identity());
				BuildShapeCircles(_shape->circles,lop->shape,i_math::matrix43f::identity());
			}
		}
	}

	if (!_shape)
	{
		LosGeneralAgent *los=GetLos<LosGeneralAgent>();
		if (los)
		{
			if (_nmShape!=StringID_Invalid)
			{
				std::vector<i_math::spheref>*shape=los->FindShape(_nmShape);
				if (shape)
				{
					_shape=Class_New2(GeneralAgentShape);
					i_math::matrix43f *mat=_GetMat();
					if (mat)
					{
						_BuildShapeUnits(_shape->units,*shape,*mat);
						BuildShapeCircles(_shape->circles,*shape,mat);
					}
				}
			}

			if (!_shape)
			{
				if (los->shape.size()>0)
				{
					_shape=Class_New2(GeneralAgentShape);
					i_math::matrix43f *mat=_GetMat();
					if (mat)
					{
						_BuildShapeUnits(_shape->units,los->shape,*mat);
						BuildShapeCircles(_shape->circles,los->shape,mat);
					}
				}
			}
		}
	}
}



BOOL CLoGeneralAgent::OnActivate()
{
	//Shape
	_BuildShape();

	LevelRecordAgent *rec=_GetRec();
	assert(rec);
	if (!rec)
		return TRUE;

	if (rec->bAttackable)
	{
		if (!_attrAttackable)
		{
			_attrAttackable=Class_New2(LevelAttr_Attackable);
			_attrAttackable->attrBase.Init(rec);
			_attrAttackable->attrResists=rec->GetResists();
			_attrAttackable->attrEvade=rec->GetEvade();
		}

		if (!_dmgsrc)
			_dmgsrc=Class_New2(CLevelEventSrc);
	}

	if (rec->bBuffs)
	{
		if (!_buffs)
			_buffs=Class_New2(LevelBuffsWrapper);
		_buffs->buffs.Init(this,_level->GetBuffIDPool());
	}


	//MagicBoard资源
	if (TRUE)
	{
		BOOL bExistRes=FALSE;
		for (int i=0;i<MBRes_ActualMax;i++)
		{
			if (rec->resMB[i]!=0)
			{
				bExistRes=TRUE;
				break;
			}
		}

		if (bExistRes)
		{
			_attrMagicBoard=Class_New2(LevelAttr_MagicBoard);
			for (int i=0;i<MBRes_ActualMax;i++)
			{
				if (rec->resMB[i]==0)
					continue;

				_attrMagicBoard->res[i].SetCur_Int((int)rec->resMB[i]);
				_attrMagicBoard->res[i].SetMax_Int((int)rec->resMB[i]);
			}
		}
	}

	_tUpdate=_level->GetT_();


	if (rec->bSkillDriver)
	{
		_driver=Class_New2(CLevelSkillDriver);
		_driver->Init(this);
	}
	if (rec->modeTalk!=TalkMode_None)
	{
		_talks=Class_New2(CLevelTalks);
		_talks->Create(this);
		_talks->SetMode(rec->modeTalk);
	}

	if (TRUE)
	{
		i_math::matrix43f *mat=_GetMat();
		if (mat)
		{
			switch(rec->modeResidable)
			{
				case AgentResidable_SingleSeat:
				{
					_residable=Class_New2(CLevelObjResidable_Single);
					((CLevelObjResidable_Single*)_residable)->Init(*mat,LevelPos3D(0,0,0));
					break;
				}
				case AgentResidable_InfiniteSeat:
				{
					_residable=Class_New2(CLevelObjResidable_Infinite);
					((CLevelObjResidable_Infinite*)_residable)->Init(*mat,LevelPos3D(0,0,0));
					break;
				}
			}
		}
	}

 	_RegisterLevelHook(LH_PlayerEnterLevel,10);
 	_RegisterLevelHook(LH_PlayerLeaveWorld,10);

	_VerifyBehavior();


	return TRUE;
}

void CLoGeneralAgent::OnDestroy()
{
	for (int i=0;i<ARRAY_SIZE(_bhvs);i++)
	{
		if (_bhvs[i])
		{
			if (_bhvs[i])
				_bhvs[i]->Clear();
			Safe_Class_Delete(_bhvs[i]);
		}
	}

	if (_talks)
	{
		_talks->Destroy();
		Safe_Class_Delete(_talks);
	}

	if (_residable)
	{
		_residable->Clear();
		Safe_Class_Delete(_residable);
	}

	if (_driver)
	{
		_driver->Clear();
		Safe_Class_Delete(_driver);
	}

	if (_buffs)
	{
		_buffs->buffs.Clear();
		Safe_Class_Delete(_buffs);
	}

	Safe_Class_Delete(_attrAttackable);
	Safe_Class_Delete(_attrMagicBoard);

	if (_troops)
	{
		_troops->Clear();
		Safe_Class_Delete(_troops);
	}

	_ClearShape();

	Safe_Class_Delete(_dmgsrc);
	__super::OnDestroy();
}

int CLoGeneralAgent::_GetBehaviorIdx(LevelPlayerID idPlayer)
{
	LevelRecordAgent *rec=_GetRec();
	assert(rec);
	if (rec->modeBehavior==AgentBehavior_Single)
		return 0;

	if (rec->modeBehavior==AgentBehavior_EachPlayer)
	{
		if ((int)(idPlayer)<LEVEL_MAX_PLAYER)
			return (int)(idPlayer);
	}
	return -1;
}

int CLoGeneralAgent::_GetBehaviorCount()
{
	LevelRecordAgent *rec=_GetRec();
	assert(rec);
	if (rec->modeBehavior==AgentBehavior_Single)
		return 1;
	if (rec->modeBehavior==AgentBehavior_EachPlayer)
		return LEVEL_MAX_PLAYER;
	return 0;
}


CLevelBehavior* CLoGeneralAgent::GetBehaviorAI()
{
	return _bhvs[0];
}


CLevelBehavior*CLoGeneralAgent::GetBehavior(LevelPlayerID idPlayer)
{
	int idx=_GetBehaviorIdx(idPlayer);
	if (idx>=0)
		return _bhvs[idx];
	return NULL;
}


CBehaviorMem *CLoGeneralAgent::GetBehaviorMem(LevelPlayerID idPlayer)
{
	CLevelBehavior *bhv=GetBehavior(idPlayer);
	if (bhv)
		return bhv->GetMem(0);
	return NULL;
}

LevelSimpleMem*CLoGeneralAgent::GetSimpleMem(LevelPlayerID idPlayer)
{
	int idx=_GetBehaviorIdx(idPlayer);
	if (idx>=0)
		return &_memsSimple[idx];

	return NULL;
}

void CLoGeneralAgent::_SetLastBehavior(LevelPlayerID idPlayer,BOOL bExist)
{
	int idx=_GetBehaviorIdx(idPlayer);
	if (idx>=0)
		_bhvsLast[idx]=(BYTE)bExist;
}

BOOL CLoGeneralAgent::_CheckLastBehavior(LevelPlayerID idPlayer)
{
	int idx=_GetBehaviorIdx(idPlayer);
	if (idx>=0)
		return _bhvsLast[idx]!=0;
	return FALSE;
}


void CLoGeneralAgent::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_attrAttackable)
	{
		bp->Bit_Write_1();
		_attrAttackable->attrBase.WriteFirstSync(bp,FALSE);
	}
	else
		bp->Bit_Write_0();

	if (_attrMagicBoard)
	{
		bp->Bit_Write_1();
		_attrMagicBoard->WriteFirstSync(bp);
	}
	else
		bp->Bit_Write_0();

	if (_buffs)
	{
		bp->Bit_Write_1();
		_buffs->buffs.WriteFirstSync(bp);
	}
	else
		bp->Bit_Write_0();

	if (_talks)
	{
		_talks->Write(bp,idPlayer,bContent);
		bContent=TRUE;
	}

	if (_shape)
		bp->Bit_Write_1();
	else
		bp->Bit_Write_0();

	bp->Bit_Write(_bBodyEnabled);

	if (TRUE)
	{
		CBehaviorMem *mem=GetBehaviorMem(idPlayer);
		if (mem)
		{
			bp->Bit_Write_1();
			mem->SetSyncDirty();
			mem->SaveSync(bp->GetDP());
			mem->ClearSyncDirty();
			_SetLastBehavior(idPlayer,TRUE);
		}
		else
		{
			bp->Bit_Write_0();
			_SetLastBehavior(idPlayer,FALSE);
		}
	}

	if (TRUE)
	{
		LevelSimpleMem *memSimple=GetSimpleMem(idPlayer);
		assert(memSimple);
		memSimple->Save(*bp->GetDP());
	}

}

void CLoGeneralAgent::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_talks)
	{
		_talks->Write(bp,idPlayer,bContent);
	}

	if (TRUE)
	{
		CBehaviorMem *mem=GetBehaviorMem(idPlayer);
		if (mem)
		{//现在有内容
			if ((mem->IsSyncDirty())||(!_CheckLastBehavior(idPlayer)))
			{//内容有变化或者之前没有内容
				bContent=1;
				bp->Bit_Write_1();//有变化
				bp->Bit_Write_1();//有内容
				mem->SaveSync(bp->GetDP());

				mem->ClearSyncDirty();
				_SetLastBehavior(idPlayer,TRUE);
			}
			else
			{
				bp->Bit_Write_0();//没有变化
			}
		}
		else
		{//当前没有内容
			if (_CheckLastBehavior(idPlayer))
			{//之前有内容
				bContent=1;
				bp->Bit_Write_1();//有变化
				bp->Bit_Write_0();//没有内容
				_SetLastBehavior(idPlayer,FALSE);
			}
			else
				bp->Bit_Write_0();//没有变化
		}
	}

	LevelSimpleMem*memSimple=GetSimpleMem(idPlayer);
	assert(memSimple);
	if (memSimple->bSyncDirty)
	{
		bp->Bit_Write_1();//有变化
		memSimple->Save(*bp->GetDP());
	}
	else
		bp->Bit_Write_0();//没有变化

	if (_buffs)
		_buffs->buffs.WriteSyncL(bp,bContent);

}

void CLoGeneralAgent::_OnPostWriteSync()
{
	if (_talks)
		_talks->PostWrite();

	if (_buffs)
		_buffs->buffs.PostWriteSync();

}


void CLoGeneralAgent::Update()
{
	AnimTick dt,t;
	t=_level->GetT_();
	dt=t-_tUpdate;
	_tUpdate=t;

	if (_driver)
		_driver->Update(dt);

	if (_buffs)
		_buffs->buffs.Update(_level->GetT_());

	extern void LevelUtil_VerifyTalksPlayer(CLevel *level,CLevelTalks *talks);
	LevelUtil_VerifyTalksPlayer(GetLevel(),_talks);

	_UpdateBehavior();

}

void CLoGeneralAgent::_LoadPersist(LevelPlayerID idPlayer)
{
	LevelGUID guid=_GetGUID();
	if (guid==LevelGUID_Invalid)
		return;

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		extern LevelPersistEntry_Agent *LPS_FindPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
		LevelPersistEntry_Agent *entry=LPS_FindPersistEntry_Agent(lps,_level->GetMapID(),guid);
		if (entry)
		{
			if (entry->szData>0)
			{
				CDataPacket dp;
				dp.SetDataBufferPointer(entry->data);

				CBehaviorMem *mem=GetBehaviorMem(idPlayer);

				if (mem)
					mem->LoadPersist(&dp);
			}
		}
	}
}

void CLoGeneralAgent::_SavePersist(LevelPlayerID idPlayer)
{
	LevelGUID guid=_GetGUID();
	if (guid==LevelGUID_Invalid)
		return;

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (!player)
		return;

	LevelPlayerStates *lps=player->GetLPS();

	CBehaviorMem *mem=GetBehaviorMem(idPlayer);
	if (TRUE)
	{
		if (!mem)
		{//没有内容可以保存
			extern void LPS_ErasePersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
			LPS_ErasePersistEntry_Agent(lps,_level->GetMapID(),guid);
			return;
		}
		if (!mem->IsPersistDirty())
			return;//没有变化
	}

	if (TRUE)
	{
		LevelPlayerStates *lps=player->GetLPS();
		extern LevelPersistEntry_Agent *LPS_QueryPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
		LevelPersistEntry_Agent *entry=LPS_QueryPersistEntry_Agent(lps,_level->GetMapID(),guid);
		if (!entry)
			return;

		CDataPacket dp;

		mem->SavePersist(&dp);

		if (dp.GetDataSize()>MAX_PERSISTENTRY_AGENT_SIZE)
		{
			LevelRecordAgent *rec=_GetRec();
			assert(rec);
			LOG_DUMP_1P("CLoGeneralAgent",Log_Error,"通用Agent(%s)的保存数据量过大!",rec->Name.c_str());
		}
		else
		{
			dp.SetDataBufferPointer(entry->data);
			mem->SavePersist(&dp);
			mem->ClearPersistDirty();
			entry->szData=dp.GetDataSize();
			entry->ver=0;
		}
	}
}

void CLoGeneralAgent::_LoadPersistS(LevelPlayerID idPlayer)
{
	LevelGUID guid=_GetGUID();
	if (guid==LevelGUID_Invalid)
		return;

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (player)
	{
		LevelPlayerStates *lps=player->GetLPS();
		extern LevelPersistEntry_AgentS *LPS_FindPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
		LevelPersistEntry_AgentS *entry=LPS_FindPersistEntry_AgentS(lps,_level->GetMapID(),guid);
		if (entry)
		{
			LevelSimpleMem*memSimple=GetSimpleMem(idPlayer);
			if (memSimple)
				memSimple->CopyContent(entry->mem_);
		}
	}

}

void CLoGeneralAgent::_SavePersistS(LevelPlayerID idPlayer)
{
	LevelGUID guid=_GetGUID();
	if (guid==LevelGUID_Invalid)
		return;

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (player)
	{
		if (_memsSimple[idPlayer].bPersistDirty)
		{
			LevelPlayerStates *lps=player->GetLPS();
			extern LevelPersistEntry_AgentS *LPS_QueryPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
			LevelPersistEntry_AgentS *entry=LPS_QueryPersistEntry_AgentS(lps,_level->GetMapID(),guid);
			if (entry)
			{
				LevelSimpleMem*memSimple=GetSimpleMem(idPlayer);
				assert(memSimple);

				entry->mem_.CopyContent(*memSimple);
				memSimple->bPersistDirty=0;
			}
		}
	}
}



void CLoGeneralAgent::_VerifyBehavior()
{
	LevelRecordAgent *rec=_GetRec();
	if (!rec)
		return;

	int nBehaviors=_GetBehaviorCount();
	if (TRUE)
	{
		CLevel *level=GetLevel();
		for (int i=0;i<nBehaviors;i++)
		{
			if ((nBehaviors==1)||level->GetWorld()->ExistPlayer((LevelPlayerID)i))
			{
				if (!_bhvs[i])
				{
					LevelBehaviorContext ctx;
					ctx.lo=this;
					ctx.idPlayerTalk=(LevelPlayerID)i;
					ctx.memSimple=GetSimpleMem((LevelPlayerID)i);
					_bhvs[i]=_level->CreateBehavior(rec->idBG,ctx);
					if (_bhvs[i])
					{
						_LoadPersist((LevelPlayerID)i);
						_LoadPersistS((LevelPlayerID)i);
						_bhvs[i]->Start();
						_SavePersist((LevelPlayerID)i);
						_SavePersistS((LevelPlayerID)i);
					}
				}
			}
			else
			{
				if (_bhvs[i])
				{
					_bhvs[i]->Clear();
					Safe_Class_Delete(_bhvs[i]);
				}
			}
		}
	}

}

void CLoGeneralAgent::_UpdateBehavior()
{
	int nBehaviors=_GetBehaviorCount();
	for (int i=0;i<nBehaviors;i++)
	{
		if (_bhvs[i])
		{
			_bhvs[i]->Update();
			if (_bhvs[i]->GetMem(0))
				_SavePersist((LevelPlayerID)i);
			_SavePersistS((LevelPlayerID)i);
		}
	}
}


void CLoGeneralAgent::HandleHook(LevelHook &hk)
{
	//Player变化了,我们要更新一下behavior,如果需要的话
	_VerifyBehavior();
	extern void LevelUtil_VerifyTalksPlayer(CLevel *level,CLevelTalks *talks);
	LevelUtil_VerifyTalksPlayer(GetLevel(),_talks);
}

CLevelTroops *CLoGeneralAgent::ObtainTroops()
{
	if (_troops)
		return _troops;

	_troops=Class_New2(CLevelTroops);
	_troops->Init(_level);
	return _troops;
}

extern void LevelUtil_BuildShapeUnits(std::vector<CUnit *>&units,CUnitMgrNavMesh *unitmgr,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat,CLevelObj*owner);
void CLoGeneralAgent::_BuildShapeUnits(std::vector<CUnit*> &units,std::vector<i_math::spheref>&shape,i_math::matrix43f &mat)
{
	//根据shape创建Static 的Units
	CUnitMgrNavMesh *unitmgr=GetLevel()->GetUnitMgr();

	LevelUtil_BuildShapeUnits(units,unitmgr,shape,&mat,this);
}

void CLoGeneralAgent::_ClearShape()
{
	if (_shape)
	{
		_shape->Clear();
		Safe_Class_Delete(_shape);
	}
}


void CLoGeneralAgent::DisableShape()
{
	_ClearShape();
}

void CLoGeneralAgent::EnableShape()
{
	_ClearShape();
	_BuildShape();
}

BOOL CLoGeneralAgent::SetShapeName(StringID nm)
{
	if (_nmShape==nm)
		return FALSE;

	_nmShape=nm;
	if (IsShapeEnabled())
	{
		_ClearShape();
		_BuildShape();
	}
	return TRUE;
}


LevelPos3D CLoGeneralAgent::GetBriefCenter()
{
	LosGeneralAgent *los=GetLos<LosGeneralAgent>();
	if (los)
	{
		if (los->centerBrief.size()>0)
		{
			i_math::matrix43f *mat=_GetMat();
			if (mat)
			{
				LevelPos3D pos=los->centerBrief[0];
				mat->transformVect(pos,pos);
				return pos;
			}

		}
	}

	return CLoAgent::GetBriefCenter();
}
