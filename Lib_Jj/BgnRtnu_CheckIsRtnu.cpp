/********************************************************************
	created:	2018/07/08 
	author:		cxi
	
	purpose:	 CheckIsRtnu
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelRtnus.h"


#include "BgnRtnu_CheckIsRtnu.h"


////////////////////////////////////////////////////////////////////////
//CBgnRtnu_CheckIsRtnu
BIND_BGN_CLASS(CBgnRtnu_CheckIsRtnu,CBgpRtnu_CheckIsRtnu);

void CBgnRtnu_CheckIsRtnu::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpRtnu_CheckIsRtnu*pad=_GetPad<CBgpRtnu_CheckIsRtnu>();

	CLevelObj *lo=_GetLo();
	if (lo->IsRetinue())
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}
