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


#include "Bgn_SnailP1_初始化.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoSnailP1.h"

#include "Random/Random.h"

#include "Log/LogDump.h"



////////////////////////////////////////////////////////////////////////
//CBgn_SnailP1_初始化
BIND_BGN_CLASS(CBgn_SnailP1_初始化,CBgp_SnailP1_初始化);


void CBgn_SnailP1_初始化::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SnailP1_初始化*pad=_GetPad<CBgp_SnailP1_初始化>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelObj *lo=level->GetUniqueObj(LevelUniqueObj_SnailP1);
		if (lo)
		{
			CLoSnailP1 *loSnailP1=(CLoSnailP1 *)lo;
			loSnailP1->SetSnailUnit(_GetLo());

			_OutputOk(outputs,1,"成功");
			return;
		}
	}

	_OutputFail(outputs,2,"失败");
	return;
}
