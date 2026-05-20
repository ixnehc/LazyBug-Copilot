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


#include "Bgn_SnailP1_舌已打断.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoSnailP1.h"

#include "Random/Random.h"

#include "Log/LogDump.h"

#include "Buff_TongueFly.h"



////////////////////////////////////////////////////////////////////////
//CBgn_SnailP1_舌已打断
BIND_BGN_CLASS(CBgn_SnailP1_舌已打断,CBgp_SnailP1_舌已打断);
 

void CBgn_SnailP1_舌已打断::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SnailP1_舌已打断*pad=_GetPad<CBgp_SnailP1_舌已打断>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		if (level->GetUniqueObj(LevelUniqueObj_SnailP1))
		{
			CLoSnailP1 *loSnailP1=(CLoSnailP1 *)_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
			if (loSnailP1->IsTongueBrokenForAWhile(pad->_dur))
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}

		//用即时性更强的方式来检查
		if (pad->_dur<=0)
		{
			extern Buff_TongueFly *FindTongueFlyBuff(CLevel *level);
			Buff_TongueFly *buff=FindTongueFlyBuff(level);
			if (buff)
			{
				if (buff->CheckKnotKilled())
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}

	}


	_OutputFail(outputs,2,"否");
	return;
}
