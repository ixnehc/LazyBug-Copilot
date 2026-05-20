/********************************************************************
	created:	2015/04/19 
	author:		cxi
	
	purpose:	 检测AIScenario
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "LoUnit.h"

#include "BgnCheckAIScenario.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckAIScenario
BIND_BGN_CLASS(CBgn_CheckAIScenario,CBgp_CheckAIScenario);
void CBgn_CheckAIScenario::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckAIScenario*pad=_GetPad<CBgp_CheckAIScenario>();

	CLevelObj *lo=_GetLo();

	if (lo->GetType()==LevelObjType_Unit)
	{
		if (((CLoUnit*)lo)->GetAIScenario()==pad->nmScenario)
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}

	_OutputFail(outputs,2,"否");
}
