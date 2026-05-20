/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查HP的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelAttrs.h"

#include "BgnCheckHP.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckHP,CBgp_CheckHP);

void CBgn_CheckHP::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckHP*pad=_GetPad<CBgp_CheckHP>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
	{
		float rate=attr->hp.GetRatio();

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
	}
	_OutputFail(outputs,2,"否");
}

