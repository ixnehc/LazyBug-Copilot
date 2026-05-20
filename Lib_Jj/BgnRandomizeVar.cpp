/********************************************************************
	created:	2016/12/20 
	author:		cxi
	
	purpose:	 产生随机数
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "behaviorgraph/BehaviorGraphs.h"


#include "BgnRandomizeVar.h"

#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgn_RandomizeVar
BIND_BGN_CLASS(CBgn_RandomizeVar,CBgp_RandomizeVar);


void CBgn_RandomizeVar::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_RandomizeVar*pad=_GetPad<CBgp_RandomizeVar>();

	CBehavior *bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		float v=0.0f;
		if (!pad->bFloat)
			v=CSysRandom::RandRange((float)pad->low,(float)pad->hi);
		else
			v=CSysRandom::RandRange((float)pad->fLow,(float)pad->fHi);
		if (pad->nm<LevelSimpleVarName_Max)
			_SetNumber(pad->nm,(short)FloatToNearestInt(v));
		else
			_SetFloat(pad->nm,v);
	}

	_OutputOk(outputs,1,"结束");
}



