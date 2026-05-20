/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"


#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnModCounter.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "LevelSkillDriver.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgn_ModCounter
BIND_BGN_CLASS(CBgn_ModCounter,CBgp_ModCounter);


void CBgn_ModCounter::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ModCounter*pad=_GetPad<CBgp_ModCounter>();

	CBehavior *bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		BehaviorCounter *counter=bhv->FindCounter(pad->nm);
		if (counter)
		{
			switch(pad->mode)
			{
				case 0:
					counter->v+=pad->vRef;
					break;
				case 1:
					counter->v-=pad->vRef;
					break;
				case 2:
					counter->v=pad->vRef;
					break;
			}
		}
		else
		{
			LOG_DUMP_1P("CBgn_ModCounter",Log_Error,"无法找到计时器(%s)!",StrLib_GetStr(pad->nm));
		}
	}

	_OutputOk(outputs,1,"结束");
}

