/********************************************************************
	created:	2020/8/21 
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"


#include "Bgn_SnailP1_舌虫已返回.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoSnailP1.h"

#include "Random/Random.h"

#include "Log/LogDump.h"

#include "Buff_TongueFly.h"



////////////////////////////////////////////////////////////////////////
//CBgn_SnailP1_舌虫已返回
BIND_BGN_CLASS(CBgn_SnailP1_舌虫已返回,CBgp_SnailP1_舌虫已返回);


void CBgn_SnailP1_舌虫已返回::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SnailP1_舌虫已返回*pad=_GetPad<CBgp_SnailP1_舌虫已返回>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	extern Buff_TongueFly *FindTongueFlyBuff(CLevel *level);

	Buff_TongueFly *buff=FindTongueFlyBuff(level);
	if (buff)
	{
		if (buff->IsNearlyWithdrawn())
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}

	_OutputFail(outputs,2,"否");
	return;
}
