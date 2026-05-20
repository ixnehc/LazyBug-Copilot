/********************************************************************
	created:	2012/11/26 
	author:		CXI
	
	purpose:	标准NPC(站桩型)
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LoUnit.h"

#include "LevelNPCs.h"

#include "NPC_Standard.h"

#include "LevelBasis.h"
#include "LevelBehavior.h"
#include "LevelBGs.h"

#include "LoNPCLoc.h"

#include "Random/Random.h"

BIND_NPCPARAM(NPC_Standard,NPCParam_Standard);

BOOL CreateNPCs(NPCParam_Standard *param,CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute)
{
	int idx=CSysRandom::RandRangeInt<DWORD>(0,distribute.locs.size());
	LosNPCLoc *loc=distribute.locs[idx];

	NPC_Standard *npc=Class_New2(NPC_Standard);
	if (npc->Create(npcs->GetLevel(),param,rec,loc->GetPos()))
	{
		npcs->AddNPC(npc);
		return TRUE;
	}
	else
	{
		Safe_Class_Delete(npc);
	}

	return TRUE;

}

void NPC_Standard::_OnCreate()
{
	NPCParam_Standard *param=(NPCParam_Standard *)_param;

	CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

	lo->PostCreate(LevelPlayerID_Wild,NULL,param->idUnit,param->grdBase,_pos);
	_level->AddToActives(lo);

	_loUnit=lo;

}

void NPC_Standard::_OnDestroy()
{
	for (int i=0;i<ARRAY_SIZE(_bhv);i++)
	{
		CLevelBehavior *bhv=_bhv[i];
		if (bhv)
		{
			bhv->Clear();
			Safe_Class_Delete(bhv);
		}
	}

	_loUnit->DeferDestroy();
	SAFE_RELEASE(_loUnit);

	Zero();
}

void NPC_Standard::_OnAddPlayer(LevelPlayerID idPlayer)
{
	NPCParam_Standard *param=(NPCParam_Standard *)_param;

	LevelBehaviorContext ctx;
	ctx.idPlayerLock=idPlayer;
	ctx.lo=_loUnit;
	_bhv[idPlayer]=_level->CreateBehavior(param->nmBG,ctx);
	if (_bhv[idPlayer])
		_bhv[idPlayer]->Start();

}

void NPC_Standard::_OnRemovePlayer(LevelPlayerID idPlayer)
{
	if (_loUnit)
		_loUnit->BreakTalk(idPlayer);

	if (_bhv[idPlayer])
	{
		_bhv[idPlayer]->Clear();
		Safe_Class_Delete(_bhv[idPlayer]);
	}

}


void NPC_Standard::_OnUpdate(AnimTick t)
{
	for (int i=0;i<ARRAY_SIZE(_bhv);i++)
	{
		CLevelBehavior *bhv=_bhv[i];
		if (bhv)
			bhv->Update();
	}
}

