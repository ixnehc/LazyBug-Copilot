/********************************************************************
	created:	2021/01/25
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"


#include "Bgn_SnailP1舌虫_工作模式.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "Buff_TongueFly.h"


////////////////////////////////////////////////////////////////////////
//CBgn_SnailP1舌虫_工作模式
BIND_BGN_CLASS(CBgn_SnailP1舌虫_工作模式,CBgp_SnailP1舌虫_工作模式);


void CBgn_SnailP1舌虫_工作模式::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SnailP1舌虫_工作模式*pad=_GetPad<CBgp_SnailP1舌虫_工作模式>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx->lo)
	{
		extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
		Buff_TongueFly *buff=(Buff_TongueFly *)LevelUtil_FindBuff(ctx->lo,Class_Ptr2(Buff_TongueFly));
		if (buff)
		{
			if (pad->_mode==2)
				buff->SetWorkMode_Withdraw();
			if (pad->_mode==3)
				buff->SetWorkMode_FastWithdraw();
		}
	}


	_OutputOk(outputs,1,"结束");
	return;
}
