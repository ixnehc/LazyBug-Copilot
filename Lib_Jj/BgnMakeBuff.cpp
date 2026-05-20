/********************************************************************
	created:	2016/05/15 
	author:		cxi
	
	purpose:	Make Buff
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "behaviorgraph/BehaviorGraphs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnMakeBuff.h"

#include "LevelObj.h"

#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgn_MakeBuff
BIND_BGN_CLASS(CBgn_MakeBuff,CBgp_MakeBuff);

void CBgn_MakeBuff::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_MakeBuff*pad=_GetPad<CBgp_MakeBuff>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	if (pad->_tpTarget==0)
	{
		if (pad->_dur==0)
			level->GetDecider()->MakeBuff(lo,pad->_idBuff,ANIMTICK_INFINITE,NULL,TRUE);
		else
			level->GetDecider()->MakeBuff(lo,pad->_idBuff,pad->_dur,NULL,TRUE);
	}
	if (pad->_tpTarget==1)
	{
		CLevelObj *loTalk=_GetTalkLo();
		if (loTalk)
		{
			if (pad->_dur==0)
				level->GetDecider()->MakeBuff(loTalk,pad->_idBuff,ANIMTICK_INFINITE,NULL,TRUE);
			else
				level->GetDecider()->MakeBuff(loTalk,pad->_idBuff,pad->_dur,NULL,TRUE);
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
