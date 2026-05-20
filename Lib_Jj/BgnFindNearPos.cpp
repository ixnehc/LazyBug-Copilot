/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查HP的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelUtil.h"

#include "LoUnit.h"

#include "BgnFindNearPos.h"

#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgn_FindNearPos

BIND_BGN_CLASS(CBgn_FindNearPos,CBgp_FindNearPos);

void CBgn_FindNearPos::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FindNearPos*pad=_GetPad<CBgp_FindNearPos>();


	CBehavior*bhv=_bhv;

	LevelPos posCenter;

	BOOL bCenter=FALSE;
	if (pad->nmCenter==StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			bCenter=TRUE;
			posCenter=lo->GetFramePos();
		}
	}
	else
	{
		if (_GetPos(pad->nmCenter,posCenter))
			bCenter=TRUE;
	}

	if (bCenter)
	{
		CLevel *level=_GetLevel();

		float radius=CSysRandom::RandVary(pad->radius,pad->radiusVary);

		LevelPos pos;
		BOOL bFound=LevelUtil_FindNearbyPos(level,posCenter,radius,pad->bWalkable,FALSE,pad->nTry,pos);

		if (bFound)
		{
			if (_SetPos(pad->nmOutput,pos))
			{
				_OutputOk(outputs,1,"找到");
				return;
			}
		}
	}


	_OutputOk(outputs,2,"未找到");
}


