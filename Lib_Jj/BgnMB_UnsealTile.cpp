/********************************************************************
	created:	2013/12/10 
	author:		cxi
	
	purpose:	Unseal Tile
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LevelBehavior.h"
#include "LevelBGs.h"

#include "LoMagicBoard.h"
#include "MagicBoardAI.h"
#include "Protocal.h"

#include "BgnMB_UnsealTile.h"



#include "Log/LogDump.h"
#include "random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnMB_UnsealTile
BIND_BGN_CLASS(CBgnMB_UnsealTile,CBgpMB_UnsealTile);

void CBgnMB_UnsealTile::Destroy()
{
}

void CBgnMB_UnsealTile::Break(BGNOutputs &outputs)
{
}



void CBgnMB_UnsealTile::_Update(MagicBoardAIContext *ctx,BGNOutputs &outputs)
{

	if (ctx->seals.size()<=0)
	{
		_SetResult(A_Fail);
		return;
	}
	if (TRUE)
	{
		int idx=CSysRandom::RandRangeInt<int>(0,ctx->seals.size());

		MagicTileInfo *ti=ctx->lo->GetTileInfo(ctx->seals[idx]);
		if (ti)
		{
			if (MBUtil_CheckTileReady(ctx->level,ctx->idPlayer,ti))
			{
				MagicBoardInvoke invoke;
				invoke.id=ctx->lo->GetID();
				invoke.x=ti->x;
				invoke.y=ti->y;
				ctx->lo->Invoke(ctx->idPlayer,invoke);

				_OutputOk(outputs,1,"结束");
				return;
			}
		}
	}

}

void CBgnMB_UnsealTile::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpMB_UnsealTile*pad=_GetPad<CBgpMB_UnsealTile>();

	MagicBoardAIContext *ctx=_GetCtxMB();

	_Update(ctx,outputs);
}


void CBgnMB_UnsealTile::Update(BGNOutputs &outputs)
{
	CBgpMB_UnsealTile*pad=_GetPad<CBgpMB_UnsealTile>();

	MagicBoardAIContext *ctx=_GetCtxMB();
	_Update(ctx,outputs);
}


