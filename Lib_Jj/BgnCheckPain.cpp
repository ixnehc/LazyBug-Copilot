/********************************************************************
	created:	2023/04/07 
	author:		cxi
	
	purpose:	 检查Pain的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelAttrs.h"

#include "BgnCheckPain.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckPain,CBgp_CheckPain);

void CBgn_CheckPain::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckPain*pad=_GetPad<CBgp_CheckPain>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	extern float LevelUtil_CalcCurPainRatio(CLevelObj *lo);

	float rate=LevelUtil_CalcCurPainRatio(lo);

	float rateMin,rateMax;
	rateMin=pad->rateMin;
	rateMax=pad->rateMax;
	if (rateMin>rateMax)
		Swap(rateMin,rateMax);
	if ((rate<=rateMax)&&(rate>=rateMin))
	{
		_OutputOk(outputs,1,"是");
		return;
	}
	_OutputFail(outputs,2,"否");
}

