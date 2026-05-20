/********************************************************************
	created:	2020/8/21 
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"


#include "Bgn_地狱触手_DetectThreat.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelUnitArg.h"
#include "LoUnit.h"
#include "LevelSensor.h"




////////////////////////////////////////////////////////////////////////
//CBgn_地狱触手_DetectThreat
BIND_BGN_CLASS(CBgn_地狱触手_DetectThreat,CBgp_地狱触手_DetectThreat);


void CBgn_地狱触手_DetectThreat::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_地狱触手_DetectThreat*pad=_GetPad<CBgp_地狱触手_DetectThreat>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (lo)
	{
		if (lo->GetType()==LevelObjType_Unit)
		{
			CLoUnit *loUnit=(CLoUnit *)lo;
			UnitArg_地狱触手 *arg=loUnit->GetArg<UnitArg_地狱触手>();
			if (arg)
			{
				if (lo->GetSensor())
				{
					CLevelObj *loThreat=lo->GetSensor()->GetThreat();
					if (loThreat)
					{
						LevelPos pos=loThreat->GetFramePos();
						for (int i=0;i<arg->locsSense.size();i++)
						{
							if (arg->locsSense[i].isPointIn(pos))
							{
								_OutputOk(outputs,1,"成功");
								return;
							}
						}
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
	return;
}
