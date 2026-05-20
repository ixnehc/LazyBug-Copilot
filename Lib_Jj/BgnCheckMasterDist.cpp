/********************************************************************
	created:	2013/5/25 
	author:		cxi
	
	purpose:	检测与主人间的距离
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"

#include "BgnCheckMasterDist.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckMasterDist
BIND_BGN_CLASS(CBgn_CheckMasterDist,CBgp_CheckMasterDist);
void CBgn_CheckMasterDist::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckMasterDist*pad=_GetPad<CBgp_CheckMasterDist>();

	CLevelObj *lo=_GetLo();
	extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
	CLevelObj *loOwner=LevelUtil_GetOwnerLo(lo);

	if (lo&&loOwner)
	{
		if  (lo->GetFramePos().getDistanceSQFrom(loOwner->GetFramePos())<pad->radius*pad->radius)
		{
			_OutputOk(outputs,1,"范围内");
			return;
		}
	}

	_OutputFail(outputs,2,"范围外");
}
