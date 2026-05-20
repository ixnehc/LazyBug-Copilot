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

#include "BgnCheckCounter.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckCounter,CBgp_CheckCounter);

void CBgn_CheckCounter::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckCounter*pad=_GetPad<CBgp_CheckCounter>();

	CBehavior*bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		BehaviorCounter *counter=bhv->FindCounter(pad->nm);
		if (counter)
		{
			int v=counter->v;
			BOOL b=FALSE;
			switch(pad->op)
			{
				case CBgp_CheckCounter::EQ:
					b=(v==pad->vRef);break;
				case CBgp_CheckCounter::NE:
					b=(v!=pad->vRef);break;
				case CBgp_CheckCounter::GE:
					b=(v>=pad->vRef);break;
				case CBgp_CheckCounter::GT:
					b=(v>pad->vRef);break;
				case CBgp_CheckCounter::LE:
					b=(v<=pad->vRef);break;
				case CBgp_CheckCounter::LT:
					b=(v<pad->vRef);break;
			}
			if (b)
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"否");
}

