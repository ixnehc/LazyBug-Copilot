/********************************************************************
	created:	2016/12/31 
	author:		cxi
	
	purpose:	 发送信号
*********************************************************************/
#include "stdh.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBGs.h"
#include "LevelBehavior.h"


#include "BgnSignal.h"

BIND_BGN_CLASS(CBgn_Signal,CBgp_Signal);
void CBgn_Signal::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Signal*pad=_GetPad<CBgp_Signal>();

	if (pad->nm!=StringID_Invalid)
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			if (pad->varTarget==StringID_Invalid)
			{
				if (pad->area.sphereset.empty())
					_GetLevel()->GetEventMap()->AddSignal(pad->nm,lo->GetFramePos(),pad->radius,lo->GetID());
				else
				{
					for (int i=0;i<pad->area.sphereset.size();i++)
					{
						float radius=pad->area.sphereset[i].radius;
						LevelPos pos=pad->area.sphereset[i].center.getXZ();
						_GetLevel()->GetEventMap()->AddSignal(pad->nm,pos,radius,lo->GetID());
					}
				}
			}
			else
			{
				LevelObjID idTarget=LevelObjID_Invalid;
				if (_GetID(pad->varTarget,BehaviorMemType_ObjID,idTarget))
				{
					if (idTarget!=LevelObjID_Invalid)
						_GetLevel()->GetEventMap()->AddSignal(pad->nm,idTarget,lo->GetID());
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}

