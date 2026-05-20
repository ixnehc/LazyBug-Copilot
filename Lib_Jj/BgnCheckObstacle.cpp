/********************************************************************
	created:	2016/06/25
	author:		cxi
	
	purpose:	 检测障碍
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelBGs.h"

#include "LevelObj.h"

#include "BgnCheckObstacle.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckObstacle
BIND_BGN_CLASS(CBgn_CheckObstacle,CBgp_CheckObstacle);
void CBgn_CheckObstacle::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckObstacle*pad=_GetPad<CBgp_CheckObstacle>();

	CLevelObj *lo=_GetLo();
	CLevelObj *loTarget=NULL;

	LevelObjID id=LevelObjID_Invalid;
	if (pad->tpTarget==CBgp_CheckObstacle::Target_Custom)
	{
		_GetID(pad->nmVar,BehaviorMemType_ObjID,id);
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		loTarget=LevelUtil_GetAliveLo(_GetLevel(),id);
	}
	if (pad->tpTarget==CBgp_CheckObstacle::Target_TalkPlayer)
		loTarget=_GetTalkLo();

	if (lo&&loTarget)
	{
		CLevel *level=_GetLevel();
		if (level)
		{
			CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
			if (unitmgr)
			{
				if(unitmgr->StaticObstacleTest(UnitFindPath_Walkable,lo->GetFramePos(),loTarget->GetFramePos()))
				{
					_OutputOk(outputs,1,"有障碍");
				}
			}
		}
	}

	_OutputFail(outputs,2,"没有障碍");
}
