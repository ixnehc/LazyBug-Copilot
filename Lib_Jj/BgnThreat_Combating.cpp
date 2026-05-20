/********************************************************************
	created:	2016/09/14 
	author:		cxi
	
	purpose:	 ¹¥»÷Threat
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_Combating.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelAIContext.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_Combating
BIND_BGN_CLASS(CBgnThreat_Combating,CBgpThreat_Combating);

void CBgnThreat_Combating::Destroy()
{
	if (_threat)
	{
		if (_threat->IsAlive())
		{
			LevelAIContext *ctx=_threat->GetAIContext();
			if (ctx)
				ctx->RemoveCombated(_GetLo()->GetID());
		}
	}
	SAFE_RELEASE(_threat);
}



void CBgnThreat_Combating::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *target=_GetThreat();
	if (target)
	{
		SAFE_REPLACE(_threat,target);
		LevelAIContext *ctx=_threat->ObtainAIContext();
		if (ctx)
			ctx->AddCombated(_GetLo()->GetID());
	}

}


void CBgnThreat_Combating::Break(BGNOutputs &outputs)
{
	Destroy();
}



