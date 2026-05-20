/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:修改资源
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_ModRes.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ModRes
BIND_BGN_CLASS(CBgnGA_ModRes,CBgpGA_ModRes);


void CBgnGA_ModRes::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ModRes*pad=_GetPad<CBgpGA_ModRes>();
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
				if (pad->op==CBgpGA_ModRes::Add)
					decider->MakeResModify(lo,pad->tp,pad->nRef);
				if (pad->op==CBgpGA_ModRes::Sub)
					decider->MakeResModify(lo,pad->tp,-pad->nRef);
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
