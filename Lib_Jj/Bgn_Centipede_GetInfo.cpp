/********************************************************************
	created:	2020/07/10 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelUtil.h"

#include "Bgn_Centipede_GetInfo.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoCentipede.h"




////////////////////////////////////////////////////////////////////////
//CBgn_Centipede_GetInfo
BIND_BGN_CLASS(CBgn_Centipede_GetInfo,CBgp_Centipede_GetInfo);

void CBgn_Centipede_GetInfo::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Centipede_GetInfo*pad=_GetPad<CBgp_Centipede_GetInfo>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
	{
		CLoCentipede *loCentipede=(CLoCentipede *)ctx->lo;

		float r=loCentipede->GetUnbrokenRate();
		if (pad->var!=StringID_Invalid)
		{
			_SetFloat(pad->var,r);
		}
		_OutputOk(outputs,1,"成功");
		return;
	}

	_OutputFail(outputs,2,"失败");
}

