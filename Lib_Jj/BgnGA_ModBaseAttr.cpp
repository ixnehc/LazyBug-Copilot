/********************************************************************
	created:	2023/2/1 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_ModBaseAttr.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ModBaseAttr
BIND_BGN_CLASS(CBgnGA_ModBaseAttr,CBgpGA_ModBaseAttr);


void CBgnGA_ModBaseAttr::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ModBaseAttr*pad=_GetPad<CBgpGA_ModBaseAttr>();
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
				if (pad->tp==0)
					decider->MakeMod_Str(lo,pad->nMod,LevelOpLink());
				if (pad->tp==1)
					decider->MakeMod_Magic(lo,pad->nMod,LevelOpLink());
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
