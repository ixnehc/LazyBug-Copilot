/********************************************************************
	created:	2022/12/14 
	
	purpose:	GA功能:Enable/Disable Body
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelOSB.h"

#include "BgnGA_EnableBody.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_EnableBody
BIND_BGN_CLASS(CBgnGA_EnableBody,CBgpGA_EnableBody);


void CBgnGA_EnableBody::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_EnableBody*pad=_GetPad<CBgpGA_EnableBody>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			if (ctx->lo)
			{
				if (IsClass2(ctx->lo,CLoGeneralAgent))
				{
					decider->MakeBodyModify(LevelOSB(ctx->lo),ctx->lo,pad->op);
					((CLoGeneralAgent*)ctx->lo)->EnableBody(pad->op==CBgpGA_EnableBody::Enable);
				}
			}
		}
	}
	
	_OutputOk(outputs,1,"结束");
	return;
}
