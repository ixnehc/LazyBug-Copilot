
#include "stdh.h"

#include "Level.h"
#include "LevelIDs.h"
#include "LevelRecords.h"
#include "LevelRecordMap.h"
#include "LevelTroops.h"

#include "behaviorgraph/BehaviorGraphs.h"
#include "LevelBehavior.h"

#include "behaviorgraph/BehaviorMem.h"

#include "LevelAIs.h"

#include "Log/LogDump.h"

//////////////////////////////////////////////////////////////////////////
//CLevelAI

void CLevelAI::Clear()
{
	for (int j=0;j<LEVEL_MAX_PLAYER;j++)
	{
		if (_bhvs[j])
		{
			_bhvs[j]->Clear();
			Safe_Class_Delete(_bhvs[j]);
		}
	}

	if (_troops)
	{
		_troops->Clear();
		Safe_Class_Delete(_troops);
	}

	Zero();
}

void CLevelAI::_LoadPersist(LevelPlayerStates *lps,CLevelBehavior *bhv)
{
	if (!bhv)
		return;

	CBehaviorGraph *bg=bhv->GetBg();
	if (!bg)
		return;

	StringID nmBg=bg->GetName();
	if (nmBg!=StringID_Invalid)
		return;

	extern LevelPersistEntry_LevelAI*LPS_FindPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI);
	LevelPersistEntry_LevelAI * entry=LPS_FindPersistEntry_LevelAI(lps,_level->GetMapID(),nmBg);
	if (entry)
	{
		CDataPacket dp;
		dp.SetDataBufferPointer(entry->data);

		CBehaviorMem *mem=bhv->GetMem(0);
		if (mem)
			mem->LoadPersist(&dp);
	}
}

void CLevelAI::_SavePersist(LevelPlayerStates *lps,CLevelBehavior *bhv)
{
	if (!bhv)
		return;

	CBehaviorGraph *bg=bhv->GetBg();
	if (!bg)
		return;

	StringID nmBg=bg->GetName();
	if (nmBg!=StringID_Invalid)
		return;


	CBehaviorMem *mem=bhv->GetMem(0);
	if (!mem)
	{//没有内容可以保存
		extern void LPS_ErasePersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI);
		LPS_ErasePersistEntry_LevelAI(lps,_level->GetMapID(),nmBg);
	}
	else
	{
		if (mem->IsPersistDirty())
		{
			extern LevelPersistEntry_LevelAI *LPS_QueryPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI);
			LevelPersistEntry_LevelAI *entry=LPS_QueryPersistEntry_LevelAI(lps,_level->GetMapID(),nmBg);
			if (!entry)
				return;

			CDataPacket dp;

			mem->SavePersist(&dp);

			if (dp.GetDataSize()>MAX_PERSISTENTRY_LEVELAI_SIZE)
			{
				LOG_DUMP_1P("LevelAI",Log_Error,"LevelAI(%s)的保存数据量过大!",StrLib_GetStr(nmBg));
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
}


void CLevelAI::VerifyPlayerAIs()
{
	if (_nmBg==StringID_Invalid)
		return;

	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		LevelPlayerID idPlayer=(LevelPlayerID)i;
		LevelPlayerStates *lps=_level->GetWorld()->GetLPS(idPlayer);
		if (lps)
		{
			if (!_bhvs[i])
			{
				LevelBehaviorContext ctx;
				ctx.aiLvl=this;
				CLevelBehavior *bhv=_level->CreateBehavior(_nmBg,ctx);
				if (bhv)
				{
					_LoadPersist(lps,bhv);

					bhv->Start();

					_SavePersist(lps,bhv);

					_bhvs[i]=bhv;
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

void CLevelAI::Update()
{
	for (int j=0;j<LEVEL_MAX_PLAYER;j++)
	{
		LevelPlayerID idPlayer=(LevelPlayerID)j;
		LevelPlayerStates *lps=_level->GetWorld()->GetLPS(idPlayer);
		if (lps)
		{
			CLevelBehavior *bhv=_bhvs[j];
			if (bhv)
			{
				bhv->Update();
				if (bhv->GetMem(0))
					_SavePersist(lps,bhv);
			}
		}
	}
}

CLevelTroops *CLevelAI::ObtainTroops()
{
	if (!_troops)
	{
		_troops=Class_New2(CLevelTroops);
		_troops->Init(_level);
	}
	return _troops;
}



//////////////////////////////////////////////////////////////////////////
//CLevelAIs

void CLevelAIs::Init(CLevel *level)
{
	_level=level;

	LevelRecordMap *recMap=NULL;
	CLevelRecords *records=_level->GetRecords();
	if (records)
	{
		RecordID idMap=_level->GetMapID();
		if (idMap!=RecordID_Invalid)
		{
			LevelRecordMap *recMap=records->GetMap(idMap);
			if (recMap)
			{
				for (int i=0;i<recMap->ais.size();i++)
				{
					if (recMap->ais[i]!=StringID_Invalid)
					{
						CLevelAI *ai=Class_New2(CLevelAI);
						ai->Init(_level,recMap->ais[i]);
						_ais.push_back(ai);
					}
				}
			}
		}
	}
}

void CLevelAIs::Clear()
{
	for (int i=0;i<_ais.size();i++)
	{
		if (_ais[i])
		{
			_ais[i]->Clear();
			Safe_Class_Delete(_ais[i]);
		}
	}

	_ais.clear();
}


void CLevelAIs::Update()
{
	for (int i=0;i<_ais.size();i++)
	{
		if (_ais[i])
			_ais[i]->Update();
	}

}


void CLevelAIs::_VerifyPlayerAIs()
{
	for (int i=0;i<_ais.size();i++)
	{
		if (_ais[i])
			_ais[i]->VerifyPlayerAIs();
	}
}

void CLevelAIs::OnPlayerEnter()
{
	_VerifyPlayerAIs();
}

void CLevelAIs::OnPlayerLeave()
{
	_VerifyPlayerAIs();
}
