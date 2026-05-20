/********************************************************************
	created:	2012/11/26 
	author:		cxi
	
	purpose:	NPC
*********************************************************************/
#include "stdh.h"

#include "LevelNPC.h"

#include "LevelPlayerStates.h"

BOOL CLevelNPC::Init(CLevel *level,LevelRecordNPC *rec,NPCDistribute *distrib)
{
	_level=level;
	_rec=rec;
	_distrib=distrib;

	return TRUE;
}

void CLevelNPC::Clear()
{

}

void CLevelNPC::Update(AnimTick t)
{

}



void CLevelNPC::AddPlayer(LevelPlayerID idPlayer)
{
	if (!_distrib)
		return;

	CLevelPlayer *player=_level->GetPlayer(idPlayer);
	if (!player)
		return;

	LPSNpcData *p=NULL;
	if (TRUE)
	{
		LPSNpcSet *npcset=LPS_FindNpcSet(player->GetLPS(),_rec->GetID());
		if (npcset)
			p=npcset->FindData(_rec->GetID());
	}


	Entry *entry=&_entries[idPlayer];

	entry->states=p->states;

	LevelPos posUnit;//在哪里创建单位
	if (TRUE)
	{
		if (!entry->state.bRetinue)
		{
			//不是Retinue,在分布的位置中随机挑选一个
			int idx=CSysRandom::RandRangeInt<DWORD>(0,_distrib->locs.size());
			LosNPCLoc *loc=_distrib->locs[idx];
			posUnit=loc->GetPos();
		}
		else
		{

		}
	}


	//创建单位
	if (!entry->bDead)
	{
		if (TRUE)
		{
			CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));
			lo->PostCreate(LevelPlayerID_NeutralWild,NULL,_rec->idUnit,_rec->grdBase,loc->GetPos());
			lo->EnableAI(FALSE);//AI由外部管理
			lo->SetOnlyVisible(idPlayer);
			_level->AddToActives(lo);

			entry->loUnit=lo;
		}

		//创建Behavior
		if (_rec->nmBG!=StringID_Invalid)
		{
			LevelBehaviorContext ctx;
			ctx.idPlayerLock=idPlayer;
			ctx.lo=entry->loUnit;
			entry->bhv=_level->CreateBehavior(param->nmBG,ctx,entry->persist);
		}
	}

	if (entry->bhv)
		entry->bhv->Start();


	_SaveEntry(entry,player->GetLPS());

	
}
void CLevelNPC::RemovePlayer(LevelPlayerID idPlayer)
{

}
