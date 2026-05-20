/********************************************************************
	created:	2020/07/10 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelUtil.h"

#include "Bgn_SlimeOp.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "Buff_Slime.h"


////////////////////////////////////////////////////////////////////////
//CBgn_SlimeOp
BIND_BGN_CLASS(CBgn_SlimeOp,CBgp_SlimeOp);

void CBgn_SlimeOp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SlimeOp*pad=_GetPad<CBgp_SlimeOp>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	Buff_Slime *buff=(Buff_Slime *)LevelUtil_FindBuff(lo,Class_Ptr2(Buff_Slime));

	if (buff)
	{
		if (buff->_state==Buff_Slime::State_Ready)
		{
			_OutputOk(outputs,1,"成功");
			return;
		}
	}

	_OutputFail(outputs,2,"失败");
}

