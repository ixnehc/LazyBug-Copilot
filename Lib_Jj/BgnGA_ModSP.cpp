/********************************************************************
	created:	2017/2/15 
	author:		cxi
	
	purpose:	GA功能:修改SP
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_ModSP.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ModSP
BIND_BGN_CLASS(CBgnGA_ModSP,CBgpGA_ModSP);


void CBgnGA_ModSP::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ModSP*pad=_GetPad<CBgpGA_ModSP>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			CLevelObj *lo=_GetTalkLo();
			if(lo)
			{
				if (!pad->bMaxSP)
					decider->CommitSPMod((float)pad->nMod,LevelOSB(_GetLo()),lo,LevelOpLink(),TRUE);
				else
					decider->MakeCure_FullSP(LevelOSB(_GetLo()),lo,pad->nMod,LevelOpLink());
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
