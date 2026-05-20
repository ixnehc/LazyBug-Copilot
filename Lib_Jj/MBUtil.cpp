
#include "stdh.h"

#include "commondefines/general_stl.h"
 
#include "Level.h"
#include "LevelRecords.h"
#include "LevelOSB.h"


#include "LoMagicBoard.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"

#include "Random/Random.h"

#include "LevelRecordMagicTile.h"

#include "Buff_Birth.h"

#include "MagicBoardAI.h"

 
#include "Protocal.h"

LevelPlayerID MBUtil_GetAIPlayer()
{
	return LevelPlayerID_Wild;
}

BOOL MBUtil_IsAIPlayer(LevelPlayerID idPlayer)
{
	if (idPlayer==MBUtil_GetAIPlayer())
		return TRUE;
	return FALSE;
}

LevelAttr_MagicBoard *MBUtil_GetAttr(CLevel *level,LevelPlayerID idPlayer)
{
	if (!MBUtil_IsAIPlayer(idPlayer))
	{
		CLevelPlayer *player=level->GetPlayer(idPlayer);
		if (!player)
			return NULL;
		CLevelObj *lo=(CLevelObj*)player->GetLoUnit();
		if (!lo)
			return NULL;
		return lo->GetAttr_MagicBoard();
	}
	CLoMagicBoard *loMB=(CLoMagicBoard *)level->GetUniqueObj(LevelUniqueObj_MagicBoard);
	if (!loMB)
		return NULL;
	return &loMB->GetMagicBoardAIContext().attr;
}


void MBUtil_MakeResMod(CLevel *level,LevelPlayerID idPlayer,MBResourceType tp,int mod,BOOL bInstant,LevelOSB &osb,LevelOpLink &link)
{
	CLoMagicBoard *loMB=(CLoMagicBoard *)level->GetUniqueObj(LevelUniqueObj_MagicBoard);
	if (!loMB)
		return;
	if (!MBUtil_IsAIPlayer(idPlayer))
	{
		CLevelPlayer *player=level->GetPlayer(idPlayer);
		if (player)
		{
			CLoUnit *lo=player->GetLoUnit();
			if (lo)
			{
				LevelAttr_MagicBoard *attr=lo->GetAttr_MagicBoard();

				LevelOp_MBResouceMod*op;
				if (osb.IsEmpty())
					op=lo->NewOp<LevelOp_MBResouceMod>(link);
				else
					op=osb.NewOp<LevelOp_MBResouceMod>(link);
				op->tpRes=tp;
				attr->res[tp].MakeMod((float)mod,bInstant,op->mod);
				lo->AddOp(op);
			}
		}
	}
	else
	{
		LavMod t;
		loMB->GetMagicBoardAIContext().attr.res[tp].MakeMod((float)mod,bInstant,t);
	}
}



MBResCost *MBUtil_GetUnsealCost(CLevel *level,MagicTileInfo *ti)
{
	CLoMagicBoard *loMB=(CLoMagicBoard *)level->GetUniqueObj(LevelUniqueObj_MagicBoard);
	if (!loMB)
		return NULL;

	LosMagicBoard *los=loMB->GetLos<LosMagicBoard>();
	if (!los)
		return NULL;

	LevelRecordMagicBoard *rec=level->GetRecords()->GetMagicBoard(los->idBoard);
	if (!rec)
		return NULL;

	if (ti->tpRgn>=MagicTileRegion_Max)
		return NULL;
	return &rec->costRgns[ti->tpRgn];
}

MBResCost *MBUtil_GetCommitCost(CLevel *level,MagicTileInfo *ti)
{
	LevelRecordMagicTile *rec=level->GetRecords()->GetMagicTile(ti->candi->idTile);
	if (!rec)
		return NULL;

	return &rec->costCommit;
}


BOOL MBUtil_CheckTileReady(CLevel *level,LevelPlayerID idPlayer,MagicTileInfo *ti)
{
	CLoMagicBoard *loMB=(CLoMagicBoard *)level->GetUniqueObj(LevelUniqueObj_MagicBoard);
	if (!loMB)
		return FALSE;

	switch(ti->state)
	{
		case MagicTileState_Sealed:
		{
			MBResCost *cost=MBUtil_GetUnsealCost(level,ti);
			if (!cost)
				return FALSE;
			if (!loMB->CheckMBResCost(idPlayer,*cost))
				return FALSE;
			return TRUE;
		}
		case MagicTileState_UnSealed:
		{
			MBResCost *cost=MBUtil_GetCommitCost(level,ti);
			if (!cost)
				return FALSE;
			if (!loMB->CheckMBResCost(idPlayer,*cost))
				return FALSE;
			return TRUE;
		}
	}
	return FALSE;
}

MagicTileInfo **MBUtil_EnumUnseals(MagicBoardAIContext *ctx,RecordID idRec,DWORD &n)
{
	n=0;

	if (!ctx)
		return NULL;
	static std::vector<MagicTileInfo*>buf;
	buf.clear();
	for (int i=0;i<ctx->unseals.size();i++)
	{
		WORD idxTile=ctx->unseals[i];

		MagicTileInfo *ti=ctx->lo->GetTileInfo(idxTile);
		if (ti)
		{
			if (ti->candi)
			{
				if (ti->candi->idTile==idRec)
					buf.push_back(ti);
			}
		}
	}
	n=buf.size();
	return buf.data();
}

MagicTileInfo **MBUtil_EnumCommits(MagicBoardAIContext *ctx,RecordID idRec,DWORD &n)
{
	n=0;

	if (!ctx)
		return NULL;
	static std::vector<MagicTileInfo*>buf;
	buf.clear();
	for (int i=0;i<ctx->commits.size();i++)
	{
		WORD idxTile=ctx->commits[i];

		MagicTileInfo *ti=ctx->lo->GetTileInfo(idxTile);
		if (ti)
		{
			if (ti->candi)
			{
				if (ti->candi->idTile==idRec)
					buf.push_back(ti);
			}
		}
	}
	n=buf.size();
	return buf.data();
}