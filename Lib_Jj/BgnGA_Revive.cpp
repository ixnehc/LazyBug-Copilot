/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:复活自己
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "BgnGA_Revive.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"



////////////////////////////////////////////////////////////////////////
//CBgnGA_Revive
BIND_BGN_CLASS(CBgnGA_Revive,CBgpGA_Revive);


void CBgnGA_Revive::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_Revive*pad=_GetPad<CBgpGA_Revive>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			LevelOSB osb(_GetLo());
			decider->Revive(osb,_GetLo(),LevelOpLink());
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
