/********************************************************************
	created:	2018/07/08 
	author:		cxi
	
	purpose:	 CheckBehavior
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelRtnus.h"


#include "BgnRtnu_CheckBehavior.h"


////////////////////////////////////////////////////////////////////////
//CBgnRtnu_CheckBehavior
BIND_BGN_CLASS(CBgnRtnu_CheckBehavior,CBgpRtnu_CheckBehavior);

void CBgnRtnu_CheckBehavior::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpRtnu_CheckBehavior*pad=_GetPad<CBgpRtnu_CheckBehavior>();

	CLevelObj *lo=_GetLo();
	extern CLevelRtnu *LevelUtil_RtnuFromLo(CLevelObj *lo);
	CLevelRtnu *rtnu=LevelUtil_RtnuFromLo(lo);
	if (rtnu)
	{
		if (rtnu->GetBhv()==pad->_bhv)
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}

	_OutputFail(outputs,2,"否");
}
