/********************************************************************
	created:	2022/05/01 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnCheckResPile.h"

#include "LevelObj.h"

#include "LoUnit.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckResPile
BIND_BGN_CLASS(CBgn_CheckResPile,CBgp_CheckResPile);

void CBgn_CheckResPile::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckResPile*pad=_GetPad<CBgp_CheckResPile>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CLevelResPiles &piles=level->GetResPiles();

		if (pad->tpRes!=LevelResource_None)
		{
			if (piles.GetAmount(lo->GetID(),pad->tpRes)>0)
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	_OutputOk(outputs,2,"否");
	return;

}
