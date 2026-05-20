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

#include "BgnFindNearPos_Special.h"

#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgn_FindNearPos_Special

BIND_BGN_CLASS(CBgn_FindNearPos_Special,CBgp_FindNearPos_Special);

void CBgn_FindNearPos_Special::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FindNearPos_Special*pad=_GetPad<CBgp_FindNearPos_Special>();

	CBehavior*bhv=_bhv;

	CLevelObj *lo=_GetLo();
	if (lo)
	{
		LevelPos posCenter;
		posCenter=lo->GetFramePos();
		CLevel *level=_GetLevel();
		if (level)
		{
			CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
			if (unitmgr)
			{
				int nTry=pad->nTry;
				LevelPos pos;
				BOOL bFound=FALSE;

				if (pad->mode==CBgp_FindNearPos_Special::DodgeAttackThreat)
				{
					CLevelObj *threat=_GetThreat();
					if (threat)
					{
						LevelPos posThreat=threat->GetFramePos();
						float distMin,distMax;
						distMin=pad->radiusToThreat-pad->radiusToThreatVary;
						distMax=pad->radiusToThreat+pad->radiusToThreatVary;
						float dist2Min=distMin*distMin;
						float dist2Max=distMax*distMax;

						for (int i=0;i<nTry;i++)
						{
							float radius=pad->radius+CSysRandom::RandRange(-pad->radiusVary,pad->radiusVary);
							if (radius<0.0f)
								radius=0.0f;
							float radian=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
							pos.x=posCenter.x+radius*cosf(radian);
							pos.y=posCenter.y+radius*sinf(radian);
							float dist2=pos.getDistanceSQFrom(posThreat);
							if (dist2>dist2Max)
								continue;
							if (dist2<dist2Min)
								continue;

							if (unitmgr->StaticObstacleTest(UnitFindPath_Walkable,posThreat,pos))
								continue;
							if (unitmgr->StaticObstacleTest(UnitFindPath_Walkable,posThreat,posCenter))
								continue;

							bFound=TRUE;
							break;
						}
					}
				}

				if (bFound)
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


	_OutputOk(outputs,2,"未找到");
}


