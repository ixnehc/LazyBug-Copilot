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

#include "BgnMB_CheckUnsealCount.h"



#include "Log/LogDump.h"
#include "random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnMB_CheckUnsealCount
BIND_BGN_CLASS(CBgnMB_CheckUnsealCount,CBgpMB_CheckUnsealCount);

void CBgnMB_CheckUnsealCount::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpMB_CheckUnsealCount*pad=_GetPad<CBgpMB_CheckUnsealCount>();

	MagicBoardAIContext *ctx=_GetCtxMB();

	if (pad->idTile==RecordID_Invalid)
	{
		_OutputFail(outputs,2,"否");
		return;
	}

	DWORD n;
	MagicTileInfo **tiles=MBUtil_EnumUnseals(ctx,pad->idTile,n);
	if (n>=pad->cMin)
		_OutputOk(outputs,1,"是");
	else
		_OutputFail(outputs,2,"否");
}

