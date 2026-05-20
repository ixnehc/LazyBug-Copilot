/********************************************************************
	created:	2012/12/6 
	author:		cxi
	
	purpose:	
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LoUnit.h"

#include "LevelNPCs.h"

#include "NPC_Pygmy.h"

#include "LevelBasis.h"
#include "LevelBehavior.h"
#include "LevelBGs.h"

#include "LevelTalks.h"

#include "LevelPlayerStates.h"

#include "LoNPCLoc.h"

#include "LevelRecordNPC.h"


#include "Random/Random.h"
#include "BgnEvent.h"

BIND_NPCPARAM(NPC_Pygmy,NPCParam_Pygmy);

BOOL CreateNPCs(NPCParam_Pygmy *param,CLevelNPCs *npcs,LevelRecordNPC *rec,NPCDistribute &distribute)
{
	int idx=CSysRandom::RandRangeInt<DWORD>(0,distribute.locs.size());
	LosNPCLoc *loc=distribute.locs[idx];

	NPC_Pygmy *npc=Class_New2(NPC_Pygmy);
	if (npc->Create(npcs->GetLevel(),param,rec,loc->GetPos()))
		npcs->AddNPC(npc);
	else
	{
		Safe_Class_Delete(npc);
	}
	return TRUE;
}

void NPC_Pygmy::_CreateLo()
{
	NPCParam_Pygmy *param=(NPCParam_Pygmy *)_param;

	CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));
	if (TRUE)
	{
		lo->PostCreate(LevelPlayerID_NeutralWild,NULL,param->idUnit,0,_pos);
		CLevelTalks *talks=lo->GetTalks();
		if (talks)
			talks->SetMode(TalkMode_Exclusive);
	}
	_level->AddToActives(lo);

	_loUnit=lo;
}

void NPC_Pygmy::_OnCreate()
{
	NPCParam_Pygmy *param=(NPCParam_Pygmy *)_param;
	_CreateLo();

	//创建BG
	LevelBehaviorContext ctx;
	ctx.idPlayerLock=LevelPlayerID_Invalid;
	ctx.lo=_loUnit;
	_bhv=_level->CreateBehavior(param->nmBG,ctx);
	if (_bhv)
	{
		_bhv->Start();
	}
}

BOOL NPC_Pygmy::SwitchRetinue(LevelPlayerID idPlayer,CLevelObj *lo)
{
	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if(!player)
		return FALSE;
	if (lo!=_loUnit)
		return FALSE;

	_ClearBehavior();

	if (player)
		player->AddRetinue_New(_loUnit,StringID_Invalid,FALSE);
	SAFE_RELEASE(_loUnit);//这个lo由Player的Retinue控制了

	return TRUE;
}


void NPC_Pygmy::_ClearBehavior()
{
	if (_bhv)
		_bhv->Clear();
	Safe_Class_Delete(_bhv);

}


void NPC_Pygmy::_OnDestroy()
{
	_ClearBehavior();

	if (_loUnit)
		_loUnit->DeferDestroy();
	SAFE_RELEASE(_loUnit);

	Zero();
}

void NPC_Pygmy::_OnAddPlayer(LevelPlayerID idPlayer)
{

}

void NPC_Pygmy::_OnRemovePlayer(LevelPlayerID idPlayer)
{
	if (_loUnit)
		_loUnit->BreakTalk(idPlayer);
}

void NPC_Pygmy::_OnUpdate(AnimTick t)
{
	if (_bhv)
	{
		_bhv->Update();
	}
}


