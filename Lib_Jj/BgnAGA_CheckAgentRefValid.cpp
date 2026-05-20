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

#include "BgnAGA_CheckAgentRefValid.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "LoGeneralAgent.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnAGA_CallAgent
BIND_BGN_CLASS(CBgnAGA_CheckAgentRefValid,CBgpAGA_CheckAgentRefValid);


void CBgnAGA_CheckAgentRefValid::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpAGA_CheckAgentRefValid*pad=_GetPad<CBgpAGA_CheckAgentRefValid>();

	if (pad->refAgent.guid!=LevelGUID_Invalid)
	{
		CLevel *level=_GetLevel();
		CLoGeneralAgent *loGA=level->GetIDs()->LoFromGUID<CLoGeneralAgent>(pad->refAgent.guid);
		if (loGA)
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}
	_OutputFail(outputs,2,"否");
}



