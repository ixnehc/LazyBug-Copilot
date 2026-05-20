/********************************************************************
	created:	2016/12/11 
	author:		cxi
	
	purpose:	 寻找周围的位置
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LoUnit.h"

#include "BgnFindWalkableArea.h"

#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgn_FindWalkableArea

BIND_BGN_CLASS(CBgn_FindWalkableArea,CBgp_FindWalkableArea);

void CBgn_FindWalkableArea::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FindWalkableArea*pad=_GetPad<CBgp_FindWalkableArea>();

	CBehavior*bhv=_bhv;

	CLevelObj *lo=_GetLo();
	if (lo)
	{
		LevelPos posMe;
		posMe=lo->GetFramePos();
		LevelFace face=lo->GetFrameFace();

		LevelPos pos=posMe+LevelFaceToDir(face)*pad->distFwd;

		CLevel *level=_GetLevel();
		if (level)
		{
			CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
			if (unitmgr)
			{
				if (unitmgr->CheckWalkableArea(UnitFindPath_Walkable,pos,pad->radius))
				{
					if (_SetPos(pad->nmOutput,pos))
					{
						_OutputOk(outputs,1,"找到");
						return;
					}
				}
			}
		}
	}


	_OutputFail(outputs,2,"未找到");
}


