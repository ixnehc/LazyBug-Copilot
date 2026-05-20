/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查记时器的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "BgnResetTimer.h"
#include "BgnCheckTimer.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckTimer_Obsolete

BIND_BGN_CLASS(CBgn_CheckTimer_Obsolete,CBgp_CheckTimer_Obsolete);

void CBgn_CheckTimer_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckTimer_Obsolete*pad=_GetPad<CBgp_CheckTimer_Obsolete>();

	CBehavior *bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		BehaviorTimer *timer=bhv->FindTimer(pad->nm);
		if (timer)
		{
			if (bhv->GetT()>=timer->tExpect)
			{
				_OutputOk(outputs,1,"时间到");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"时间没到");
}


//////////////////////////////////////////////////////////////////////////
//CBgn_CheckTimer
BIND_BGN_CLASS(CBgn_CheckTimer,CBgp_CheckTimer);

void CBgn_CheckTimer::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckTimer*pad=_GetPad<CBgp_CheckTimer>();

	CBehavior *bhv=_bhv;

	if (pad->_timer!=StringID_Invalid)
	{
		BMO_Timer *timer=_GetMem()->GetObj<BMO_Timer>(pad->_timer);
		if (timer)
		{
			if (_GetT()>=timer->tTimeUp+ANIMTICK_FROM_SECOND(pad->_durAdd))
			{
				_OutputOk(outputs,1,"时间到");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"时间没到");
}

