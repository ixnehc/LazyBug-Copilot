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

#include "BgnMB_CheckResCount.h"



#include "Log/LogDump.h"
#include "random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnMB_CheckResCount
BIND_BGN_CLASS(CBgnMB_CheckResCount,CBgpMB_CheckResCount);

void CBgnMB_CheckResCount::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpMB_CheckResCount*pad=_GetPad<CBgpMB_CheckResCount>();

	MagicBoardAIContext *ctx=_GetCtxMB(); 

	BOOL bMeet=TRUE;
	for (int i=1;i<MBRes_ActualMax;i++)
	{
		if (ctx->attr.res[i].GetCur_Int()<pad->cMin[i])
		{
			bMeet=FALSE;
			break;
		}
	}

	if (bMeet)
		_OutputOk(outputs,1,"是");
	else
		_OutputFail(outputs,2,"否");

}

