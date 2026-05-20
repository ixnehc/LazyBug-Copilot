/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 ºËÐÄµÄBGN
*********************************************************************/
#include "stdh.h"
#include "BehaviorGraphs.h"
#include "Behavior.h"

#include "BgnRelay.h"

#include "../Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Relay

BIND_BGN_CLASS(CBgn_Relay,CBgp_Relay);
void CBgn_Relay::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Relay*pad=_GetPad<CBgp_Relay>();
	if (pad)
	{
		if (pad->_nm!=StringID_Invalid)
			outputs.Add(0,_thrd);
	}
	_SetResult(A_Ok);
}

////////////////////////////////////////////////////////////////////////
//CBgn_SwitchState
BIND_BGN_CLASS(CBgn_StartRelay,CBgp_StartRelay);
void CBgn_StartRelay::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_StartRelay*pad=_GetPad<CBgp_StartRelay>();
	if (pad)
	{
		if (pad->_nm!=StringID_Invalid)
		{
			CBehaviorGraph *bg=_bhv->GetBg();
			PadID idRelay=bg->PadIDFromRelayName(pad->_nm);
			if (idRelay!=PadID_Null)
			{
				outputs.idRelay=idRelay;
				outputs.thrdRelay=_thrd;
			}
		}
	}
	_SetResult(A_Ok);
}

