/********************************************************************
	created:	2012/10/24 
	author:		cxi
	
	purpose:	逃跑的Act
*********************************************************************/
#include "stdh.h"

#include "Act_Flee.h"
#include "LevelRecords.h"

#include "Random/Random.h"

#include "Level.h"

BIND_ACT_PARAM(Act_Flee,ActParam_Flee);

void Act_Flee::_UpdateFlee(AnimTick t)
{
	if (_bTimeUp)
		return;
	ActParam_Flee *param=(ActParam_Flee*)_param;
	if (param->dur!=0)
	{
		if (t>_tStart+param->dur)
		{
			_bTimeUp=TRUE;
			return;
		}
	}
	extern CLevelObj *AIUtil_DetectClosestPlayerUnit(CLevelObj *lo,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods);
	CLevelObj *lo=AIUtil_DetectClosestPlayerUnit(_owner,param->radius,NULL,LevelMoveMethodMask_Ground);
	if (lo)
	{
		extern BOOL AIUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep);
		AIUtil_Flee(_owner,lo,param->radius+1.0f);
		_tFlee=t;
	}

}


void Act_Flee::Start(AnimTick t)
{
	_tStart=t;
	_UpdateFlee(t);
}

void Act_Flee::Update(AnimTick t)
{
	_UpdateFlee(t);
}

