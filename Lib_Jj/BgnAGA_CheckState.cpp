/********************************************************************
	created:	2015/07/03 
	author:		cxi
	
	purpose:	 调用Agent的函数
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnAGA_CheckState.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "LevelSkillDriver.h"

#include "LoGeneralAgent.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnAGA_CallAgent
BIND_BGN_CLASS(CBgnAGA_CheckState,CBgpAGA_CheckState);


void CBgnAGA_CheckState::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpAGA_CheckState*pad=_GetPad<CBgpAGA_CheckState>();

	if (pad->refAgent.guid!=LevelGUID_Invalid)
	{
		CLevel *level=_GetLevel();
		CLoGeneralAgent *loGA=level->GetIDs()->LoFromGUID<CLoGeneralAgent>(pad->refAgent.guid);
		if (loGA)
		{
			CLevelBehavior *bhv=loGA->GetBehavior(LevelPlayerID_Invalid);
			if (bhv)
			{
				CBehaviorGraph *bg=bhv->GetBg();
				if (bg)
				{
					PadID idPad=bg->PadIDFromStateName(pad->nmState);
					if (idPad!=PadID_Null)
					{
						CBehaviorMem *mem=bhv->GetMem(0);
						if (mem)
						{
							if (mem->CheckState(idPad))
							{
								_OutputOk(outputs,1,"是");
								return;
							}
						}
					}
				}
			}
		}
	}
	_OutputFail(outputs,2,"否");
}



