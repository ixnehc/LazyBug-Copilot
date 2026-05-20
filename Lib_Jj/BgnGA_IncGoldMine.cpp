/********************************************************************
	created:	2022/04/27 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelOSB.h"

#include "BgnGA_IncGoldMine.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_IncGoldMine
BIND_BGN_CLASS(CBgnGA_IncGoldMine,CBgpGA_IncGoldMine);

void CBgnGA_IncGoldMine::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_IncGoldMine*pad=_GetPad<CBgpGA_IncGoldMine>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	LevelPlayerStates *lps=NULL;
	CLoUnit *loUnit=NULL;
	CLevelPlayer *player=_GetTalkPlayer();
	if (player)
	{
		lps=player->GetLPS();
		loUnit=player->GetLoUnit();
	}

	if (lps)
	{
		lps->misc.nGoldMines++;
		lps->misc.SetDirtyDB_High();
	}
	_OutputOk(outputs,1,"结束");
}
