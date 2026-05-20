/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"
#include "LevelBGs.h"

#include "BgnResetTimer.h"
#include "behaviorgraph/BgnHelper.h"

#include "Random/Random.h"
#include "Log/LogDump.h"





////////////////////////////////////////////////////////////////////////
//CBgn_ResetTimer_Obsolete
BIND_BGN_CLASS(CBgn_ResetTimer_Obsolete,CBgp_ResetTimer_Obsolete);


void CBgn_ResetTimer_Obsolete::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ResetTimer_Obsolete*pad=_GetPad<CBgp_ResetTimer_Obsolete>();

	CBehavior *bhv=_bhv;

	if (pad->nm!=StringID_Invalid)
	{
		BOOL bOk=FALSE;
		BehaviorTimer *timer=bhv->FindTimer(pad->nm);
		if (timer)
		{
			CBgp_Timer *padTimer=bhv->GetBg()->FindTimer(pad->nm);
			if (padTimer)
			{
				timer->tExpect=bhv->GetT()+CSysRandom::RandVaryUInt(padTimer->_init,padTimer->_vary);
				bOk=TRUE;
			}
		}

		if (!bOk)
		{
			LOG_DUMP_1P("CBgn_ResetTimer",Log_Error,"重置计时器失败!",StrLib_GetStr(pad->nm));
		}
	}


	_OutputOk(outputs,1,"结束");
}


//////////////////////////////////////////////////////////////////////////
//BMO_Timer
IMPLEMENT_CLASS(BMO_Timer);


////////////////////////////////////////////////////////////////////////
//CBgn_ResetTimer
BIND_BGN_CLASS(CBgn_ResetTimer,CBgp_ResetTimer);


void CBgn_ResetTimer::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ResetTimer*pad=_GetPad<CBgp_ResetTimer>();

	CBehavior *bhv=_bhv;

	if (pad->_timer!=StringID_Invalid)
	{
		BMO_Timer *bmo=Class_New(BMO_Timer);
		float dur = pad->_fDur;
		if (pad->_fVariance > 0.0f)
			dur = CSysRandom::RandVary(pad->_fDur, pad->_fVariance);

		bmo->dur=ANIMTICK_FROM_SECOND(dur);
		bmo->tTimeUp=_GetT()+bmo->dur;

		_GetMem()->DepositObj(pad->_timer,bmo);
	}


	_OutputOk(outputs,1,"结束");
}

