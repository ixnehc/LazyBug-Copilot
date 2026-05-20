/********************************************************************
	created:	2021/01/25
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"


#include "Bgn_SnailP1舌虫_寻找毒爆点.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoSnailP1.h"
#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgn_SnailP1舌虫_寻找毒爆点
BIND_BGN_CLASS(CBgn_SnailP1舌虫_寻找毒爆点,CBgp_SnailP1舌虫_寻找毒爆点);


void CBgn_SnailP1舌虫_寻找毒爆点::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SnailP1舌虫_寻找毒爆点*pad=_GetPad<CBgp_SnailP1舌虫_寻找毒爆点>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	extern CLevelObj *LevelUtil_GetThreat(CLevelObj *lo);

	CLevelObj *loThreat=LevelUtil_GetThreat(lo);
	LevelPos posThreat;
	if (loThreat)
		posThreat=loThreat->GetFramePos();
	const float distMinFromThreat=15.0f;

	LevelPos posCur=lo->GetFramePos();
	const float distMinFromCur=8.0f;

	if (level)
	{
		CLevelObj *lo=level->GetUniqueObj(LevelUniqueObj_SnailP1);
		if (lo)
		{
			CLoSnailP1 *loSnailP1=(CLoSnailP1 *)lo;

			LopSnailP1 *lop=loSnailP1->GetLop();
			if (lop)
			{
				std::vector<i_math::spheref> &locs= lop->locs舌虫毒爆点;
				if (TRUE)
				{
					for (int i=0;i<10;i++)
					{
						int idx=CSysRandom::RandRangeInt<int>(0,locs.size());
						LevelPos pos=locs[idx].center.getXZ();

						if (loThreat)
						{
							if (pos.getDistanceSQFrom(posThreat)<=distMinFromThreat*distMinFromThreat)
								continue;
						}
						if (pos.getDistanceSQFrom(posCur)<=distMinFromCur*distMinFromCur)
							continue;

						_SetPos(pad->varPos,pos);
						_OutputOk(outputs,1,"成功");
						return;
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
	return;
}
