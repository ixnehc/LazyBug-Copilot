/********************************************************************
	created:	2015/08/30
	author:		cxi
	
	purpose:	 Troop的Detect
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelTroops.h"
#include "BgnTroop_Detect.h"

#include "LevelSkillDriver.h"
#include "LoUnit.h"



#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnTroop_Detect
BIND_BGN_CLASS(CBgnTroop_Detect,CBgpTroop_Detect);

extern BOOL LevelUtil_CheckDead(CLevelObj *lo);


void CBgnTroop_Detect::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_Detect*pad=_GetPad<CBgpTroop_Detect>();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	AnimTick t=_GetT();

	BOOL bDetected=FALSE;

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		const float radiusCluster=4.0f;
		static std::vector<LevelPos> centers;
		centers.clear();


		float rangeMin,rangeMax;
		rangeMin=0.0f;
		rangeMax=pad->_range+radiusCluster;

		DWORD nFrames=troop->GetFrameCount();
		for (int i=0;i<nFrames;i++)
		{
			LevelTroopFrame *frm=troop->GetFrame(i);
			CLevelObj *lo=level->GetIDs()->LoFromID(frm->idUnit);
			if (lo)
			{
				if (lo->IsAlive())
				{
					if (!LevelUtil_CheckDead(lo))
					{
						LevelPos pos=lo->GetFramePos();

						int j=0;
						for (j=0;j<centers.size();j++)
						{
							if (pos.getDistanceSQFrom(centers[j])<radiusCluster*radiusCluster)
							{
								break;
							}
						}

						if (j>=centers.size())
						{
							centers.push_back(pos);

							//侦测一下
							if (TRUE)
							{
								extern CLevelObj *DetectFirstTarget(CLevelObj *lo,
									float rangeMin,float rangeMax,
									std::vector<LevelDetectTargetFlag>&flags,
									std::vector<LevelObjRequire>*requires,
									LevelObjMapEnumCallBack dlgt=NULL,CLevelObj **loIgnore=NULL,DWORD nIgnores=0);
								if (DetectFirstTarget(lo,rangeMin,rangeMax,pad->_flags,&pad->_requires))
								{
									bDetected=TRUE;
									break;
								}
							}
						}
					}
				}
			}
		}

		centers.clear();
	}

	if (bDetected)
		_OutputOk(outputs,1,"侦测到");
	else
		_OutputFail(outputs,2,"未侦测到");

}
