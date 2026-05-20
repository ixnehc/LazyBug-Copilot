/********************************************************************
	created:	2019/12/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_Centipede_Stretch.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoCentipede.h"



////////////////////////////////////////////////////////////////////////
//CBgn_Centipede_Stretch
BIND_BGN_CLASS(CBgn_Centipede_Stretch,CBgp_Centipede_Stretch);

void CBgn_Centipede_Stretch::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Centipede_Stretch*pad=_GetPad<CBgp_Centipede_Stretch>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx->lo->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
	{
		CLoCentipede *loCentipede=(CLoCentipede *)ctx->lo;
		if (pad->_bStretchOut)
			loCentipede->StretchOut(ANIMTICK_FROM_SECOND(5.0f));
		else
			loCentipede->StretchIn(ANIMTICK_FROM_SECOND(5.0f));
	}

	_OutputOk(outputs,1,"结束");
}

