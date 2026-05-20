/********************************************************************
	created:	2015/10/05 
	author:		cxi
	
	purpose:	 检测Troop距离
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelTroops.h"
#include "BgnTroop_CheckDist.h"

#include "LevelSkillDriver.h"
#include "LoUnit.h"



#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnTroop_CheckDist
BIND_BGN_CLASS(CBgnTroop_CheckDist,CBgpTroop_CheckDist);

extern BOOL LevelUtil_CheckDead(CLevelObj *lo);

static BOOL CheckDist(float dist,float distRef,float radiusCluster,CBgpTroop_CheckDist::Op op)
{
	switch(op)
	{
		case CBgpTroop_CheckDist::LessThan:
			return dist<distRef-radiusCluster;
		case CBgpTroop_CheckDist::GreatThan:
			return dist>distRef+radiusCluster;
	}
	return FALSE;
}


void CBgnTroop_CheckDist::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_CheckDist*pad=_GetPad<CBgpTroop_CheckDist>();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	AnimTick t=_GetT();

	BccRoute *bcc=&pad->_route;
	if (bcc)
	{
		CLevelTroop *troop=_GetTroop(pad->_troop);
		if (troop)
		{
			const float radiusCluster=4.0f;
			static std::vector<LevelPos> centers;
			centers.clear();

			float distRef=pad->_distRef;

			DWORD nFrames=troop->GetFrameCount();
			for (int i=0;i<nFrames;i++)
			{
				LevelTroopFrame *frm=troop->GetFrame(i);
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				CLevelObj *lo=LevelUtil_GetAliveLo(level,frm->idUnit);
				if (lo)
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

							//比较一下
							float distMin=10000000.0;
							if (bcc->sphereset.size()>0)
							{
								if (bcc->sphereset.size()>1)
								{
									for (int k=0;k<bcc->sphereset.size()-1;k++)
									{
										i_math::line2df line(bcc->sphereset[k].center.x,bcc->sphereset[k].center.z,
												bcc->sphereset[k+1].center.x,bcc->sphereset[k+1].center.z);
										float dist=line.getClosestDistTo(pos);
										if (dist<distMin)
											distMin=dist;
									}
								}
								else
								{
									LevelPos posCenter(bcc->sphereset[0].center.x,bcc->sphereset[0].center.z);
									distMin=posCenter.getDistanceFrom(pos);
								}
							}
							else
							{
								LevelPos posCenter=ctx->lo->GetFramePos();
								distMin=posCenter.getDistanceFrom(pos);
							}

							if (pad->_op==CBgpTroop_CheckDist::LessThan)
							{
								if(distMin>distRef-radiusCluster)
								{
									//任何一个cluster距离过远
									_OutputFail(outputs,2,"否");
									return;
								}
							}
							else
							{
								if(distMin<distRef-radiusCluster)
								{
									//任何一个cluster距离过近
									_OutputFail(outputs,2,"否");
									return;
								}
							}
						}
					}
				}
			}
			centers.clear();

			_OutputOk(outputs,1,"是");
			return;
		}
	}

	_OutputFail(outputs,2,"否");
}
