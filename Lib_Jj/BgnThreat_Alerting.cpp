/********************************************************************
	created:	2016/09/14 
	author:		cxi
	
	purpose:	 对Threat进行Alert过程
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_Alerting.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelAIContext.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_Alerting
BIND_BGN_CLASS(CBgnThreat_Alerting,CBgpThreat_Alerting);

void CBgnThreat_Alerting::Destroy()
{
	if (_threat)
	{
		if (_threat->IsAlive())
		{
			LevelAIContext *ctx=_threat->GetAIContext();
			if (ctx)
				ctx->RemoveAlerted(_GetLo()->GetID());
		}
	}
	SAFE_RELEASE(_threat);
}



void CBgnThreat_Alerting::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *target=_GetThreat();
	if (target)
	{
		SAFE_REPLACE(_threat,target);
		LevelAIContext *ctx=_threat->ObtainAIContext();
		if (ctx)
			ctx->AddAlerted(_GetLo()->GetID());
	}
}


void CBgnThreat_Alerting::Break(BGNOutputs &outputs)
{
	Destroy();
}



