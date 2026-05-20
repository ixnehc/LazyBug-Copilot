/********************************************************************
	created:	2012/11/26 
	author:		CXI
	
	purpose:	标准NPC(站桩型)
*********************************************************************/
#include "stdh.h"

#include "resdata/BehaviorGraphPads.h"

#include "Level.h"
#include "LoUnit.h"

#include "LevelNPCs.h"

#include "NPC_Laentia.h"

#include "LevelBasis.h"
#include "LevelBehavior.h"
#include "LevelBGs.h"

#include "LevelPlayerStates.h"

#include "LoNPCLoc.h"

#include "LevelRecordNPC.h"


#include "Random/Random.h"

BIND_NPCPARAM(NPC_Laentia,NPCParam_Laentia);

BOOL CreateNPCs(NPCParam_Laentia *param,CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute)
{
	int idx=CSysRandom::RandRangeInt<DWORD>(0,distribute.locs.size());
	LosNPCLoc *loc=distribute.locs[idx];

	NPC_Laentia *npc=Class_New2(NPC_Laentia);
	if (npc->Create(npcs->GetLevel(),param,rec,loc->GetPos()))
	{
		npcs->AddNPC(npc);
	}
	else
	{
		Safe_Class_Delete(npc);
	}

	return TRUE;

}

void NPC_Laentia::_OnCreate()
{

}

void NPC_Laentia::_OnDestroy()
{
	for (int i=0;i<ARRAY_SIZE(_entries);i++)
	{
		Entry *entry=&_entries[i];
		_ClearEntry(entry);
	}


	Zero();
}



extern LPSNpcSet *LPS_FindNpcSet(LevelPlayerStates *lps,RecordID idRec);

void NPC_Laentia::_SaveEntry(Entry *entry,LevelPlayerStates *lps)
{
	if (!entry->persist->IsDirty())
		return;

	LPSNpcData *p=NULL;
	LPSNpcSet *npcset=NULL;
	if (TRUE)
	{
		npcset=LPS_FindNpcSet(lps,_rec->GetID());
		if (npcset)
			p=npcset->ObtainData(_rec->GetID());
	}

	if (p)
	{
		CDataPacket dp;
		DP_BeginSave(dp,p->data);
		entry->persist->Save(&dp);
		DP_EndSave();

		entry->persist->ClearDirty();

		npcset->SetDirtyDB_Urgent();
	}

}

void NPC_Laentia::_LoadEntry(Entry *entry,LevelPlayerStates *lps)
{
	LPSNpcData *p=NULL;
	if (TRUE)
	{
		LPSNpcSet *npcset=LPS_FindNpcSet(lps,_rec->GetID());
		if (npcset)
			p=npcset->FindData(_rec->GetID());
	}
	if (p)
	{
		CDataPacket dp;
		dp.SetDataBufferPointer(&p->data[0]);
		entry->persist->Load(&dp);
	}
	else
	{
		NPCParam_Laentia *param=(NPCParam_Laentia *)_param;
		extern void LevelUtil_BuildDefBehaviorPersist(CLevel *level,StringID idBg,CLevelBehaviorPersist *persist);
		LevelUtil_BuildDefBehaviorPersist(_level,param->nmBG,entry->persist);
	}

}

void NPC_Laentia::_ClearEntry(Entry *entry)
{
	if (entry->bhv)
	{
		entry->bhv->Clear();
		Safe_Class_Delete(entry->bhv);
	}

	if (entry->loUnit)
	{
		entry->loUnit->DeferDestroy();
		SAFE_RELEASE(entry->loUnit);
	}

	if (entry->persist)
	{
		entry->persist->Clear();
		Safe_Class_Delete(entry->persist);
	}

}



void NPC_Laentia::_OnAddPlayer(LevelPlayerID idPlayer)
{
	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (!player)
		return;

	Entry *entry=&_entries[idPlayer];

	NPCParam_Laentia *param=(NPCParam_Laentia *)_param;

	//创建单位
	if (TRUE)
	{
		CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));
		lo->PostCreate(LevelPlayerID_Wild,NULL,param->idUnit,_pos);
		lo->SetOnlyVisible(idPlayer);
		_level->AddToActives(lo);

		entry->loUnit=lo;
	}

	entry->persist=Class_New2(CLevelBehaviorPersist);

	//读入
	_LoadEntry(entry,player->GetLPS());

	//创建BG
	if (TRUE)
	{
		LevelBehaviorContext ctx;
		ctx.idPlayerLock=idPlayer;
		ctx.lo=entry->loUnit;
		entry->bhv=_level->CreateBehavior(param->nmBG,ctx,entry->persist);
		if (entry->bhv)
			entry->bhv->Start();
	}

	_SaveEntry(entry,player->GetLPS());

}

void NPC_Laentia::_OnRemovePlayer(LevelPlayerID idPlayer)
{
	Entry *entry=&_entries[idPlayer];

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (player)
		_SaveEntry(entry,player->GetLPS());

	_ClearEntry(entry);



}



void NPC_Laentia::_OnUpdate(AnimTick t)
{
	for (int i=0;i<ARRAY_SIZE(_entries);i++)
	{
		Entry *entry=&_entries[i];
		if (!entry->loUnit)
			continue;

		if (entry->bhv)
			entry->bhv->Update();

		CLevelPlayer *player=_level->GetPlayer(i);
		if (player)
			_SaveEntry(entry,player->GetLPS());
	}
}

