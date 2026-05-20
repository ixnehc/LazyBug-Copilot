/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBGs.h"
#include "LevelBehavior.h"

 
#include "BgnDetectSignal.h"

BIND_BGN_CLASS(CBgn_DetectSignal,CBgp_DetectSignal);

BOOL CBgn_DetectSignal::_DoDetect(BGNOutputs &outputs)
{
	CBgp_DetectSignal*pad=_GetPad<CBgp_DetectSignal>();

	if (pad->nm!=StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			LevelSignals::Signal *signal=_GetLevel()->GetEventMap()->FindSignal(pad->nm,lo->GetID(),lo->GetFramePos());
			if (signal)
			{
				if (pad->varSender!=StringID_Invalid)
					_SetID(pad->varSender,BehaviorMemType_ObjID,signal->idSender);
				_OutputOk(outputs,1,"侦测到");
				return TRUE;
			}
		}
	}
	return FALSE;
}

void CBgn_DetectSignal::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_DetectSignal*pad=_GetPad<CBgp_DetectSignal>();

	if (_DoDetect(outputs))
		return;

	if (!pad->bKeepDetect)
		_OutputFail(outputs,2,"未侦测到");
}


void CBgn_DetectSignal::Update(BGNOutputs &outputs)
{
	_DoDetect(outputs);
}
