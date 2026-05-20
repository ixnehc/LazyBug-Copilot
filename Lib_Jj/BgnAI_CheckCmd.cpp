/********************************************************************
	created:	2016/3/2 
	author:		cxi
	
	purpose:	检测Troop命令
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "LevelTroops.h"
#include "LevelAIContext.h"

#include "BgnAI_CheckCmd.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgnAI_CheckCmd
BIND_BGN_CLASS(CBgnAI_CheckCmd,CBgpAI_CheckCmd);

void CBgnAI_CheckCmd::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpAI_CheckCmd*pad=_GetPad<CBgpAI_CheckCmd>();
	CLevelObj *lo=_GetLo();

	if (lo)
	{
		StringID idCmd=lo->GetAICmd();
		if (idCmd!=StringID_Invalid)
		{
			if (idCmd==pad->_cmd)
			{
				_OutputOk(outputs,1,"检测到");
				return;
			}
		}
	}

	_OutputFail(outputs,2,"未检测到");
}

