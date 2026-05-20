/********************************************************************
	created:	2017/02/13 
	author:		cxi
	
	purpose:	 ResPile
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnResPile.h"

#include "LevelObj.h"

#include "LoUnit.h"

////////////////////////////////////////////////////////////////////////
//CBgn_ResPile
BIND_BGN_CLASS(CBgn_ResPile,CBgp_ResPile);

void CBgn_ResPile::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ResPile*pad=_GetPad<CBgp_ResPile>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CLevelResPiles &piles=level->GetResPiles();

		if (pad->tpRes!=LevelResource_None)
			piles.Deposit(LevelObjID_Invalid,pad->tpRes,pad->amount);
	}
	
	_OutputOk(outputs,1,"结束");
}
