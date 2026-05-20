/********************************************************************
	created:	2013/12/10 
	author:		cxi
	
	purpose:	Commit Tile
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LevelBehavior.h"
#include "LevelBGs.h"

#include "LoMagicBoard.h"
#include "MagicBoardAI.h"
#include "Protocal.h"

#include "BgnMB_CommitTile.h"



#include "Log/LogDump.h"
#include "random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnMB_CommitTile
BIND_BGN_CLASS(CBgnMB_CommitTile,CBgpMB_CommitTile);

void CBgnMB_CommitTile::Destroy()
{
}

void CBgnMB_CommitTile::Break(BGNOutputs &outputs)
{
}



void CBgnMB_CommitTile::_Update(MagicBoardAIContext *ctx,BGNOutputs &outputs)
{
	CBgpMB_CommitTile*pad=_GetPad<CBgpMB_CommitTile>();

	BOOL bFail=TRUE;
	if (pad->idTile!=RecordID_Invalid)
	{
		DWORD n;
		MagicTileInfo **tiles=MBUtil_EnumUnseals(ctx,pad->idTile,n);
		if (n>0)
		{
			bFail=FALSE;
			MagicTileInfo *ti=tiles[CSysRandom::RandRangeInt<int>(0,n)];
			if (MBUtil_CheckTileReady(ctx->level,ctx->idPlayer,ti))
			{
				MagicBoardInvoke invoke;
				invoke.id=ctx->lo->GetID();
				invoke.x=ti->x;
				invoke.y=ti->y;
				ctx->lo->Invoke(ctx->idPlayer,invoke);

				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}

	if (bFail)
		_OutputFail(outputs,2,"失败");

}

void CBgnMB_CommitTile::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpMB_CommitTile*pad=_GetPad<CBgpMB_CommitTile>();

	MagicBoardAIContext *ctx=_GetCtxMB();

	_Update(ctx,outputs);
}


void CBgnMB_CommitTile::Update(BGNOutputs &outputs)
{
	CBgpMB_CommitTile*pad=_GetPad<CBgpMB_CommitTile>();

	MagicBoardAIContext *ctx=_GetCtxMB();
	_Update(ctx,outputs);
}


